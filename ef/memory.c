/* TODO big memory overhead, is not realy important because probalby use this type of memory on big data where data can change many time on runtime, but can reduce memory overhead?*/
/* TODO how to use parent with shared if parent is vector on heap?*/

/*
   1(347)      2(567)

 3(1) 4(1)  5(2) 6(2)   7(12)
*/


#include <ef/mth.h>
#include <ef/str.h>

#include <stdatomic.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <linux/futex.h>

#define MEM_AREA_MALLOC      0x00
#define MEM_AREA_MMAP        0x01
#define MEM_AREA_SHARED      0x02
#define MEM_AREA_SHARED_NAME 0x03
#define MEM_FLAG_MANY        0x10
#define MEM_FLAG_SYNC        0x20
#define MEM_FLAG_SHARED_KEEP_ALIVE 0x80

#define GLB_STR_SIZE 32

uint16_t MEM_FLAGS = MEM_AREA_MALLOC | MEM_FLAG_MANY;
uint16_t MEM_ALIGN = 0;
char MEM_NAME[PATH_MAX];
char MEM_MODE[GLB_STR_SIZE];
char MEM_PRIV[GLB_STR_SIZE];

typedef void(*mcleanup_f)(void* mem);

typedef struct hlink{
	struct hlink* next;
	void* hm;
}hlink_s;

typedef struct hmem{
	uint16_t   magic;
	uint8_t    gtype;
	uint8_t    flags;
	uint16_t   crc;
	uint16_t   aligned;
	uint16_t   extend;
	uint16_t   link;
	uint32_t   lock;
	uint32_t   offsetAddr;
	uint32_t   realsize;
	size_t     size;
	hlink_s*   parents;
	hlink_s*   childs;
	mcleanup_f clean;
}hmem_s;

#define MAGIC (0xF1CAUL)
#define MEM_AREA(F) ((F) & 0x7)

#define PAGE_SIZE() sysconf(_SC_PAGESIZE)

#define hmem_get(ADDR) ((hmem_s*)((uintptr_t)(ADDR) - sizeof(hmem_s)))
#define hmem_addr(HM,MEM) (void*)((uintptr_t)MEM-(HM)->offsetAddr)
#define hmem_data(HM) (void*)((uintptr_t)HM+sizeof(hmem_s))

__private int futex(uint32_t *uaddr, int futex_op, int val, const struct timespec *timeout, int *uaddr2, int val3){
	return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr2, val3);
}

/* https://gist.github.com/smokku/653c469d695d60be4fe8170630ba8205 */

#define cpu_relax() __builtin_ia32_pause()
#define LOCK_OPEN    1
#define LOCK_WLOCKED 0

__private void _unlock(hmem_s* hm){
	if( !(hm->flags & MEM_FLAG_SYNC) ) return;
	unsigned current, wanted;
	do {
		current = hm->lock;
        if( current == LOCK_OPEN ) return;
		wanted = current == LOCK_WLOCKED ? LOCK_OPEN : current - 1;
	}while( __sync_val_compare_and_swap(&hm->lock, current, wanted) != current );
	futex(&hm->lock, FUTEX_WAKE, 1, NULL, NULL, 0);
}

__private void _lock_read(hmem_s* hm){
	if( !(hm->flags & MEM_FLAG_SYNC) ) return;
    unsigned current;
	while( (current = hm->lock) == LOCK_WLOCKED || __sync_val_compare_and_swap(&hm->lock, current, current + 1) != current ){
		while( futex(&hm->lock, FUTEX_WAIT, current, NULL, NULL, 0) != 0 ){
            cpu_relax();
            if (hm->lock >= LOCK_OPEN) break;
		}
	}
}

__private void _lock_write(hmem_s* hm){
	if( !(hm->flags & MEM_FLAG_SYNC) ) return;
	unsigned current;
	while( (current = __sync_val_compare_and_swap(&hm->lock, LOCK_OPEN, LOCK_WLOCKED)) != LOCK_OPEN ){
		while( futex(&hm->lock, FUTEX_WAIT, current, NULL, NULL, 0) != 0 ){
			cpu_relax();
			if( hm->lock == LOCK_OPEN ) break;
		}
		if( hm->lock != LOCK_OPEN ){
			futex(&hm->lock, FUTEX_WAKE, 1, NULL, NULL, 0);
		}
	}
}

/******************************************************************/

__private inline uint16_t mem_mk_crc16(const hmem_s* hm){
	uint16_t ret  = crc16(hm->magic >> 8, CRC16_INIT_VALUE);
	ret = crc16(hm->magic & 0xFF, ret);
	ret = crc16(hm->gtype, ret);
	ret = crc16(hm->flags, ret);
	return ret;
}

__private unsigned mmap_mode(const char* mode){
	unsigned m = 0;
	if( !mode ) return PROT_NONE;
	for( ; *mode; ++mode ){
		switch( *mode ){
			case 'r': m |= PROT_READ; break;
			case 'w': m |= PROT_WRITE; break;
			case 'x': m |= PROT_EXEC; break;
		}
	}
	return m;
}

__private unsigned open_mode(const char* mode){
	unsigned m = 0;
	if( !mode ) return 0;
	for( ; *mode; ++mode ){
		switch( *mode ){
			case 'r': m |= 1; break;
			case 'w': m |= 2; break;
		}
	}

	switch( m ){
		case 1: return O_RDONLY;
		case 2: return O_WRONLY;
		case 3: return O_RDWR;
	}

	return 0;
}

int mem_check(const void* mem){
	if( !mem ) return 0;
	hmem_s* hm = hmem_get(mem);
	return hm->crc == mem_mk_crc16(hm) ? 1 : 0;
}

void mem_link(void* parent, void* mem){
	iassert(mem_check(mem));
	if(!parent) return;
	iassert(mem_check(parent));
	hmem_s* hm = hmem_get(mem);
	hmem_s* hp = hmem_get(parent);
	dbg_info("link: %u -> %u, %p -> %p", hm->link, hm->link+1, hm, hp);
	
	//add mem to parent
	hlink_s* l = malloc(sizeof(hlink_s));
	if( !l ) die("malloc: %s", str_errno());
	l->hm = hm;
	l->next = hp->childs;
	hp->childs = l;

	//add parent to mem
	l = malloc(sizeof(hlink_s));
	if( !l ) die("malloc: %s", str_errno());
	l->hm = hp;
	l->next = hm->parents;
	hm->parents = l;
	++hm->link;
}

__private void hlink_remove(hlink_s** l, void* addr){
	for(; *l; l = &((*l)->next) ){
		dbg_info("cmp %p == %p", (*l)->hm, addr);
		if( (*l)->hm == addr ){
			hlink_s* rm = *l;
			(*l) = rm->next;
			free(rm);
			return;
		}
	}
	die("hlink %p not exists", addr);
}

void mem_unlink(void* parent, void* mem){
	if( !parent ) return;
	iassert(mem_check(parent));
	iassert(mem_check(mem));

	hmem_s* hm = hmem_get(mem);
	hmem_s* hp = hmem_get(parent);	
	//remove child from parent
	dbg_info("remove child %p from parent %p", hm, hp);
	hlink_remove(&hp->childs, hm);
	//remove parent from child
	dbg_info("remove parent %p from child %p", hp, hm);
	hlink_remove(&hm->parents, hp);
	--hm->link;
}

void mem_reparent(void* parent, void* child){
	//dbg_info("reparent %p to %p", child, parent);
	iassert(mem_check(child));
	hmem_s* hm = hmem_get(child);
	hlink_s* next;
	while( hm->parents ){
		next = hm->parents->next;
		iassert(mem_check(hm->parents->hm));
		hmem_s* hp = hm->parents->hm;
		hlink_remove(&hp->childs, hm);
		free(hm->parents);
		hm->parents = next;
	}
	hm->link = 0;
	mem_link(parent, child);
}

void mem_child_displace(void* from, void* to){
	iassert(mem_check(to));
	if( !from ) return;
	iassert(mem_check(from));
	
	hmem_s* hm = hmem_get(to);
	hmem_s* hf = hmem_get(from);

	hlink_s* next;
	while( hf->childs ){
		next = hf->childs->next;
		hf->childs->next = hm->childs;
		hm->childs = hf->childs;
		hf->childs = next;
	}
}

__private void mem_update_address(hmem_s* old, hmem_s* hm){
	for( hlink_s* lp = hm->parents; lp; lp = lp->next){
		hmem_s* hp = lp->hm;
		for( hlink_s* lc = hp->childs; lc; lc = lc->next){
			if( lc->hm == old ){
				lc->hm = hm;
				break;
			}
		}
	}
	for( hlink_s* lc = hm->childs; lc; lc = lc->next){
		hmem_s* hc = lc->hm;
		for( hlink_s* lp = hc->parents; lp; lp = lp->next){
			if( lp->hm == old ){
				lp->hm = hm;
				break;
			}
		}
	}
}

int mem_bad(void* addr, size_t size){
	extern char _etext;
	if( !addr ) return 3;
	if( !((char*)addr > &_etext) ) return 3;
	int fd[2];
	int ret = 0;
	if( pipe(fd) ) die("pipe");
	if( write(fd[1], addr, size) < 0 && errno == EFAULT ) ret |= 2;
	if( read(fd[0], addr, size) < 0  && errno == EFAULT )  ret |= 1;
	close(fd[0]);
	close(fd[1]);
	return ret;
}

int mem_lock_read(void* mem){
	iassert(mem_check(mem));
	hmem_s* hm = hmem_get(mem);
	_lock_read(hm);
	return 1;
}

int mem_lock_write(void* mem){
	iassert(mem_check(mem));
	hmem_s* hm = hmem_get(mem);
	_lock_write(hm);
	return 1;
}

int mem_unlock(void* mem){
	iassert(mem_check(mem));
	hmem_s* hm = hmem_get(mem);
	_unlock(hm);
	return 1;
}

__malloc 
void* mem_alloc(size_t size, uint16_t aligned, uint16_t extend, uint8_t flags, uint16_t type, const char* name, const char* mode, const char* privilege){
	iassert( sizeof(hmem_s) % sizeof(uintptr_t) == 0);
	iassert( IS_POW_TWO(aligned) );

	uint16_t hmalg = aligned;
	if( aligned == 0 ) aligned = sizeof(uintptr_t);
	if( extend ) extend = ROUND_UP(extend, sizeof(uintptr_t));
	uint16_t algsz = aligned > sizeof(uintptr_t) ? aligned - 1 : 0;
	size_t preSize  = 0;
	size_t realsize = 0;
	void* mem = NULL;
	int creat = 1;

	switch( MEM_AREA(flags) ){
		case MEM_AREA_MALLOC:{
			preSize  = sizeof(hmem_s) + extend + algsz;
			realsize = size + preSize;
			if( flags & MEM_FLAG_MANY ) realsize += size;
			mem = malloc(realsize);
			if( !mem ) die("malloc(%ld) error:%s", realsize, str_errno());
			break;
		}

		case MEM_AREA_MMAP:{
			preSize  = sizeof(hmem_s) + extend + algsz;
			realsize = preSize + size;
			if( flags & MEM_FLAG_MANY ) realsize += size;
			realsize = ROUND_UP(realsize, PAGE_SIZE());
			unsigned mmode = mmap_mode(mode);
			mem = mmap(NULL, realsize, mmode, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			if( mem == MAP_FAILED ) die("mmap error:%s", str_errno());
			break;
		}

		case MEM_AREA_SHARED_NAME:
		case MEM_AREA_SHARED:{
			preSize = sizeof(hmem_s) + extend + algsz;
			realsize = size + preSize;
			unsigned openmode = open_mode(mode);
			unsigned mapmode  = mmap_mode(mode);
			unsigned prv = str_to_prv(privilege);
			int fm = -1;
			if( name ){
				flags |= MEM_AREA_SHARED_NAME;
				size_t len = strlen(name);
				preSize  += len;
				realsize += len;
				fm = shm_open( name, O_CREAT | O_EXCL | openmode, prv);
				if( fm == -1 ){
					fm = shm_open( name, openmode, 0);
					if( fm == -1 ) die("unable to create/attach shared mem '%s':%s", name, str_errno());
					struct stat ss;
					if( fstat(fm, &ss) == -1 ){
						close(fm);
						die("unable to attach shared mem '%s':%s", name, str_errno());
					}
					realsize = ss.st_size;
					creat = 0;
				}
				else{
					if( ftruncate(fm, realsize) == -1 ){
						close(fm);
						shm_unlink(name);
						die("unable to create shared mem '%s':%s", name, str_errno());
					}
				}
			}
			if( flags & MEM_FLAG_MANY ) realsize += size;
			realsize = ROUND_UP(realsize, PAGE_SIZE());
			mem = mmap(NULL, realsize, mapmode, MAP_SHARED, fm, 0);
			if( fm != -1 ) close(fm);
			if( mem == MAP_FAILED ){
				if( creat ) shm_unlink(name);
				die("mmap error:%s", str_errno());
			}
			break;
		}

		default: die("unknown memarea"); break;
	}
	
	iassert(mem);
	iassert(preSize);
	iassert(realsize);

	uintptr_t offset = (((uintptr_t)mem + preSize) & (~(aligned-1))) - (uintptr_t)mem;
	iassert( offset < UINT16_MAX );

	void* addr = (void*)((uintptr_t)mem+offset);

	/*dbg_info("mem:%lu presize:%lu offset:%lu aligned:%lu addr:%lu modaddr:%lu", 
			(uintptr_t)mem, 
			preSize, 
			offset, 
			aligned ? aligned : sizeof(void*), 
			(uintptr_t)addr, 
			(uintptr_t)addr % aligned
		);*/
	iassert( (uintptr_t)addr % aligned  == 0);

	hmem_s* hm = hmem_get(addr);
	iassert( hm && (void*)hm >= mem );
	iassert( (uintptr_t)hm % sizeof(uintptr_t) == 0 );
	iassert( (realsize - size <= UINT32_MAX) );

	if( creat ){
		hm->magic      = MAGIC;
		hm->gtype      = type;
		hm->flags      = flags;
		hm->crc        = mem_mk_crc16(hm);
		hm->aligned    = hmalg;
		hm->extend     = extend;
		hm->link       = 0;
		hm->offsetAddr = offset;
		hm->size       = size;
		hm->realsize   = realsize - size;
		hm->parents    = NULL;
		hm->childs     = NULL;
		hm->clean      = NULL;
	}
	else{
		if( hm->crc != mem_mk_crc16(hm) ) die("invalid memory: crc failed");
		++hm->link;
	}
	
	dbg_info("mem:%p presize:%lu offset:%lu aligned:%u addr:%p getaddr:%p hm:%p size:%lu realsize:%u", 
			mem, 
			preSize, 
			offset, 
			aligned,
			addr,
			hmem_addr(hm,addr),
			hm,
			hm->size,
			hm->realsize
	);

	iassert( (uintptr_t)addr + hm->size <= (uintptr_t)mem + hm->size + hm->realsize);

	return addr;
}

__private size_t size_resize(uint8_t flags, size_t size, size_t hmsize, size_t hmrealsize, size_t preSize){
	if( flags & MEM_FLAG_MANY ){
		if( size < hmsize && hmrealsize - preSize / 4 < size ){
			dbg_info("realsize, size(%lu) < hm->size(%lu) && (hm->size) / 4(%lu) < size(%lu)", 
				size, 
				hmsize, 
				(hmrealsize-preSize)/4,
				size
			);
			return (hmrealsize - preSize) / 4 + preSize;
		}
		return size+preSize > hmrealsize ? (size*2)+preSize : hmrealsize;
	}
	return size+preSize;
}

void* mem_alloc_resize(void* mem, size_t size, ssize_t extend){
	iassert(mem_check(mem));
	hmem_s* hm = hmem_get(mem);
	hmem_s* orghm = hm;
	hlink_s* parents = hm->parents;

	uint16_t algsz = hm->aligned ? hm->aligned - 1 : 0;
	uint16_t aligned = hm->aligned ? hm->aligned: sizeof(uintptr_t);
	size_t preSize = 0;
	size_t realsize = 0;
	size_t hmrealsize = hm->realsize + hm->size;
	size_t orgoffset = hm->offsetAddr;
	uint16_t orgextend = hm->extend;
	void* addr = NULL;
	void* orga = hmem_addr(hm, mem);
	extend = extend < 0 ?  hm->extend : ROUND_UP(extend, sizeof(uintptr_t));

	switch( MEM_AREA(hm->flags) ){
		case MEM_AREA_MALLOC:{
			//dbg_info("realloc");
			preSize  = sizeof(hmem_s) + extend + algsz;
			realsize = size_resize(hm->flags, size, hm->size, hmrealsize, preSize);
			//dbg_info("resizing(%lu) size(%lu) hm->size(%lu) hm->realsize(%lu) preSize(%lu) oldpreSize(%lu) total(%lu)", 
			//		realsize, size, hm->size, hmrealsize, preSize, sizeof(hmem_s)+orgextend+algsz, size+preSize
			//);
			if( realsize != hmrealsize ){
				//dbg_info("realloc orga:%p size:%lu realsize:%lu", orga, size, realsize);
				addr = realloc(orga, realsize);
				dbg_info("realloc mempry addr:%p old:%p", addr, orga);
				if( !addr )	die("realloc error:%s", str_errno());
			}
			else{
				dbg_info("realloc reuse memory address size:%lu realsize:%lu (total:%lu)", size, realsize, size+preSize);
				addr = orga;
			}
			break;
		}

		case MEM_AREA_SHARED:
		case MEM_AREA_MMAP:{
			dbg_info("mremap");
			preSize  = sizeof(hmem_s) + extend + algsz;	
			realsize = size_resize(hm->flags, size, hm->size, hmrealsize, preSize);
			realsize = ROUND_UP(realsize, PAGE_SIZE());
			if( realsize != hmrealsize ){
				dbg_info("mremap orga:%p size:%lu realsize:%lu", orga, size, realsize);
				addr = mremap(orga, hmrealsize, realsize, MREMAP_MAYMOVE);
				if( addr == MAP_FAILED ) die("mmap error:%s", str_errno());
			}
			else{
				dbg_info("reuse memory address");		
				addr = orga;
			}
			break;
		}

		case MEM_AREA_SHARED_NAME:{
			dbg_info("shm_realloc");
			int fm = -1;
			const char* name = hmem_addr(hm, mem);
			size_t len = strlen(name);
			unsigned mmapmode = 0;
			/* not a good thing */
			unsigned bad = ~mem_bad(orga,1);
			if( bad == 0 ){
				mmapmode = PROT_NONE;
			}
			else{
				if( bad & 1 ) mmapmode |= PROT_READ;
				if( bad & 2 ) mmapmode |= PROT_WRITE;
			}
			/*******************/
			preSize  = sizeof(hmem_s) + extend + algsz + len;
			realsize = size_resize(hm->flags, size, hm->size, hmrealsize, preSize);
			realsize = ROUND_UP(realsize, PAGE_SIZE());
			if( realsize != hmrealsize ){
				fm = shm_open( name, O_RDWR, 0);
				if( fm == -1 ) die("unable to resize shared mem '%s':%s", name, str_errno());
				if( ftruncate(fm, realsize) == -1 ){
					close(fm);
					die("unable to resize shared mem '%s':%s", name, str_errno());
				}
				dbg_info("unmap/mmap orga:%p size:%lu realsize:%lu", orga, size, realsize);
				munmap(orga, hmrealsize);
				addr = mmap(NULL, realsize, mmapmode, MAP_SHARED, fm, 0);
				close(fm);
				if( addr == MAP_FAILED ) die("mmap error:%s", str_errno());
			}
			else{
				dbg_info("reuse memory address");
				addr = orga;
			}
			break;
		}

		default: die("unknown memarea"); break;
	}
	iassert(addr);

	if( orgextend != extend ){
		//dbg_info("extend changed");
		uintptr_t offset = (((uintptr_t)addr + preSize) & (~(aligned-1))) - (uintptr_t)addr;
		iassert( offset < UINT16_MAX );
		mem = (void*)((uintptr_t)addr+offset);
		iassert( (uintptr_t)mem % aligned == 0);
		iassert( (uintptr_t)mem + size <= (uintptr_t)addr + realsize);
		iassert( (uintptr_t)addr + orgoffset + size <= (uintptr_t)addr + realsize);
		iassert( realsize - size <= UINT32_MAX );
		memmove((void*)(((uintptr_t)mem-sizeof(hmem_s))), (void*)((uintptr_t)addr+(orgoffset - sizeof(hmem_s))), size+sizeof(hmem_s));

		iassert(mem_check(mem));
		hm = hmem_get(mem);
		hm->size = size;
		hm->realsize = realsize - size;
		hm->offsetAddr = offset;
		hm->extend = extend;
		if( parents && orga != addr) mem_update_address(orghm, hm);
		iassert( (uintptr_t)mem + hm->size <= (uintptr_t)addr + hm->realsize);
		return mem;
	}

	if( orga == addr ){
		//dbg_info("address not change");
		iassert(mem_check(mem));
		hm->size = size;
		hm->realsize = realsize - size;
		iassert( (uintptr_t)mem + hm->size <= (uintptr_t)addr + hm->realsize + hm->size);
		return mem;
	}
	
	unsigned unaligned = ((uintptr_t)addr + orgoffset) % aligned;

	if( !unaligned ){
		//dbg_info("reallocated mem is aligned");
		mem = (void*)((uintptr_t)addr + orgoffset);
		iassert(mem_check(mem));
		hm = hmem_get(mem);
		hm->size = size;
		hm->realsize = realsize - size;
		if( parents && orga != addr) mem_update_address(orghm, hm);
		iassert( (uintptr_t)mem + hm->size <= (uintptr_t)addr + realsize);
		return mem;
	}

	dbg_info("reallocated mem is not aligned");

	uintptr_t offset = orgoffset + aligned - unaligned;
	iassert( offset < UINT16_MAX );

	mem = (void*)((uintptr_t)addr+offset);
	iassert( (uintptr_t)mem % aligned  == 0);
	iassert( (uintptr_t)mem + size <= (uintptr_t)addr + realsize);
	iassert( (uintptr_t)mem  + size <= (uintptr_t)addr + hmrealsize);
	iassert( (uintptr_t)addr + orgoffset + size <= (uintptr_t)addr + hmrealsize);
	memmove((void*)(((uintptr_t)mem-preSize)), (void*)((uintptr_t)addr+(orgoffset-preSize)), size+preSize);

	iassert(mem_check(mem));
	hm = hmem_get(mem);
	hm->size = size;
	hm->realsize = realsize - size;
	hm->offsetAddr = offset;
	if( parents && orga != addr) mem_update_address(orghm, hm);
	iassert( (uintptr_t)mem + hm->size <= (uintptr_t)addr + hmrealsize);
	return mem;
}

void* mem_extend_info_create(void* mem, size_t extend){
	iassert(mem_check(mem));
	hmem_s* hm = hmem_get(mem);
	if( hm->extend ) return NULL;
	return mem_alloc_resize(mem, hm->size, extend);
}

void mem_cleanup_fn(void* mem, mcleanup_f fn){
	iassert(mem_check(mem));
	hmem_s* hm = hmem_get(mem);
	hm->clean = fn;
	dbg_info("set cleanup %p to hmem %p", fn, hm);
}

size_t mem_size(const void* mem){
	iassert(mem_check(mem));
	hmem_s* hm = hmem_get(mem);
	return hm->size;
}

size_t mem_real_size(const void* mem){
	iassert(mem_check(mem));
	hmem_s* hm = hmem_get(mem);
	return hm->realsize;
}

void* mem_extend_info(const void* mem){
	iassert(mem_check(mem));
	hmem_s* hm = hmem_get(mem);
	void* ext = (void*)((uintptr_t)mem - (sizeof(hmem_s)+hm->extend));
	iassert((uintptr_t)ext % sizeof(uintptr_t) == 0);
	return ext;
}

__private void mem_delete_rec(hmem_s* parent, hmem_s* hm){
	iassert(hm);
	dbg_info("delete:(%p)%p", parent, hm);
	
	while( hm->childs ){
		mem_delete_rec(hm, hm->childs->hm);
	}

	void* mem  = hmem_data(hm);
	void* addr = hmem_addr(hm, mem);
//	dbg_info("mem:%p addr:%p", mem, addr);
	if( hm->clean ) hm->clean(mem);
	mem_unlink(parent ? hmem_data(parent) : NULL, mem);
	
	if( !hm->link ){
		switch( MEM_AREA(hm->flags) ){
			case MEM_AREA_MALLOC:
				dbg_info("free %p", addr);
				free(addr);
			break;

			case MEM_AREA_SHARED:
			case MEM_AREA_MMAP:
				dbg_info("unmap %p", addr);
				munmap(addr, hm->realsize);
			break;

			case MEM_AREA_SHARED_NAME:{
				char name[PATH_MAX];
				size_t len = strlen(addr);
				if( len+1 > PATH_MAX ) die("name too long");
				strcpy(name, addr);
				munmap(addr, hm->realsize);
				if( !(hm->flags & MEM_FLAG_SHARED_KEEP_ALIVE) ) shm_unlink(name);
			}break;

			default:
				die("unknown mem area");
			break;
		}
	}
	else{
		dbg_info("only unlinked: %p", addr);
		switch( MEM_AREA(hm->flags) ){
			case MEM_AREA_SHARED:
			case MEM_AREA_SHARED_NAME:
				munmap(addr, hm->realsize);
			break;
		}
	}
}

void mem_free(void* mem){
	if( mem == NULL ) return;
	iassert(mem_check(mem));
	hmem_s* hm = hmem_get(mem);
	dbg_info("free mem %lu bytes at address %p aligned on %p and header at %p linked on:%u", hm->size, hmem_addr(hm,mem), mem, hm, hm->link);
	mem_reparent(NULL, mem);
	mem_delete_rec(NULL, hm);
}

void mem_free_auto(void* mem){
	void** mug = (void**)mem;
	mem_free(*mug);
}

void mem_zero(void* mem){
	iassert(mem_check(mem));
	hmem_s* hm = hmem_get(mem);
	memset(mem, 0, hm->size);
}

void mem_group_set(void* mem, unsigned group){
	iassert(mem_check(mem));
	hmem_s* hm = hmem_get(mem);
	hm->gtype = (group<<4) | (hm->gtype & 0xF);
	hm->crc = mem_mk_crc16(hm); 
}

void mem_type_set(void* mem, unsigned type){
	iassert(mem_check(mem));
	hmem_s* hm = hmem_get(mem);
	hm->gtype = (hm->gtype & 0xF0) | (type & 0xF);
	hm->crc = mem_mk_crc16(hm);
}

void mem_group_check(void* mem, unsigned group){
	iassert(mem_check(mem));
	hmem_s* hm = hmem_get(mem);
	if( hm->gtype >> 4 != group ) die("invalid type, %p is %u but aspected %u", mem, hm->gtype, group);
}

void mem_type_check(void* mem, unsigned type){
	iassert(mem_check(mem));
	hmem_s* hm = hmem_get(mem);
	if( (hm->gtype & 0xF) != (type & 0xF) ) die("invalid type, %p is %u but aspected %u", mem, hm->gtype, type);
}

unsigned mem_type(void* mem){
	iassert(mem_check(mem));
	hmem_s* hm = hmem_get(mem);
	return hm->gtype;
}

__target_vectorization
void mem_swap(void* restrict a, size_t sizeA, void* restrict b, size_t sizeB){
	iassert( a );
	iassert( b );

	if( sizeA >= sizeof(size_t) && sizeB >= sizeof(size_t) ){
		const size_t la = sizeA - (sizeA % sizeof(size_t));
		const size_t lb = sizeB - (sizeB % sizeof(size_t));
		const size_t bcount = MTH_MIN(la, lb);
		const size_t count = bcount / sizeof(size_t);
		size_t* memA = a;
		size_t* memB = b;

		for( size_t i = 0; i < count; ++i ){
			SWAP(memA[i], memB[i]);
		}

		sizeA -= bcount;
		sizeB -= bcount;
		a = &memA[count];
		b = &memB[count];
	}
	
	if( sizeA >= sizeof(unsigned) && sizeB >= sizeof(unsigned) ){
		const size_t la = sizeA - (sizeA % sizeof(unsigned));
		const size_t lb = sizeB - (sizeB % sizeof(unsigned));
		const size_t bcount = MTH_MIN(la, lb);
		const size_t count = bcount / sizeof(unsigned);
		unsigned* memA = a;
		unsigned* memB = b;

		for( size_t i = 0; i < count; ++i ){
			SWAP(memA[i], memB[i]);
		}

		sizeA -= bcount;
		sizeB -= bcount;
		a = &memA[count];
		b = &memB[count];
	}

	if( sizeA >= sizeof(unsigned short) && sizeB >= sizeof(unsigned short) ){
		const size_t la = sizeA - (sizeA % sizeof(unsigned short));
		const size_t lb = sizeB - (sizeB % sizeof(unsigned short));
		const size_t bcount = MTH_MIN(la, lb);
		const size_t count = bcount / sizeof(unsigned short);
		unsigned* memA = a;
		unsigned* memB = b;

		for( size_t i = 0; i < count; ++i ){
			SWAP(memA[i], memB[i]);
		}

		sizeA -= bcount;
		sizeB -= bcount;
		a = &memA[count];
		b = &memB[count];
	}

	{
		const size_t count = MTH_MIN(sizeA, sizeB);
		char* memA = a;
		char* memB = b;
	
		for(size_t i = 0; i < count; ++i){
			SWAP(memA[i], memB[i]);
		}
		a = &memA[count];
		b = &memB[count];
		sizeA -= count;
		sizeB -= count;
	}

	iassert( sizeA == 0 || sizeB == 0);

	if( sizeA == 0 && sizeB == 0 ) return;

	if( sizeA == 0 ){
		memcpy(a, b, sizeB);	
	}
	else if( sizeB == 0 ){
		memcpy(b, a, sizeA);
	}
}
