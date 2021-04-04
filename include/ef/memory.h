#ifndef __EF_MEMORY_H__
#define __EF_MEMORY_H__

#include <ef/type.h>
#include <ef/mth.h>

typedef void(*mcleanup_f)(void* mem);

typedef int pkey_t;

#define MEM_PROTECT_DISABLED 0
#ifdef MEM_UNDECLARE_PKEY
	#define MEM_PROTECT_WRITE 1
	#define MEM_PROTECT_RW 2
#else
	#define MEM_PROTECT_WRITE PKEY_DISABLE_WRITE
	#define MEM_PROTECT_RW PKEY_DISABLE_ACCESS
#endif

/****************/
/*** memory.c ***/
/****************/

#define GLB_STR_SIZE 32
extern uint16_t MEM_FLAGS;
extern uint16_t MEM_ALIGN;
extern char MEM_NAME[PATH_MAX];
extern char MEM_MODE[GLB_STR_SIZE];
extern char MEM_PRIV[GLB_STR_SIZE];

#define MEM_TYPE_GENERIC 0x1
#define MEM_TYPE_UCHAR   0x2
#define MEM_TYPE_CHAR    0x3
#define MEM_TYPE_SINT    0x4
#define MEM_TYPE_INT     0x5
#define MEM_TYPE_LINT    0x6
#define MEM_TYPE_USINT   0x7
#define MEM_TYPE_UINT    0x8
#define MEM_TYPE_ULINT   0x9
#define MEM_TYPE_FLOAT   0xA
#define MEM_TYPE_DOUBLE  0xB

#define MEM_GROUP_SYSTEM  0x0
#define MEM_GROUP_LISTS   0x1
#define MEM_GROUP_LISTD   0x2
#define MEM_GROUP_VECTOR  0x3
#define MEM_GROUP_RBHASH  0x4
#define MEM_GROUP_STRING  0x5
#define MEM_GROUP_USER    0xF

#define MEM_FLAG_MALLOC      0x00
#define MEM_FLAG_MMAP        0x01
#define MEM_FLAG_SHARED      0x02
#define MEM_FLAG_SHARED_NAME 0x03
#define MEM_FLAG_MANY        0x10
#define MEM_FLAG_SYNC        0x20
#define MEM_FLAG_SHARED_KEEP_ALIVE 0x80

#define mem_mk_type(GROUP, TYPE) (((GROUP)<<4)| (TYPE))

#define mem_type_to_id(T) _Generic((T),\
	char           : MEM_TYPE_CHAR,\
	unsigned char  : MEM_TYPE_UCHAR,\
	short int      : MEM_TYPE_SINT,\
	short unsigned : MEM_TYPE_USINT,\
	int            : MEM_TYPE_INT,\
	unsigned int   : MEM_TYPE_UINT,\
	long           : MEM_TYPE_LONG,\
	unsigned long  : MEM_TYPE_ULINT,\
	float          : MEM_TYPE_FLOAT,\
	double         : MEM_TYPE_DOUBLE,\
	char*          : MEM_TYPE_CHAR,\
	unsigned char* : MEM_TYPE_UCHAR,\
	short int*     : MEM_TYPE_SINT,\
	short unsigned*: MEM_TYPE_USINT,\
	int*           : MEM_TYPE_INT,\
	unsigned int*  : MEM_TYPE_UINT,\
	long*          : MEM_TYPE_LONG,\
	unsigned long* : MEM_TYPE_ULINT,\
	float*         : MEM_TYPE_FLOAT,\
	double*        : MEM_TYPE_DOUBLE,\
	default: MEM_TYPE_GENERIC\
)

#define MANY(TYPE, COUNT) (TYPE*)mem_alloc(sizeof(TYPE)*(COUNT), MEM_ALIGN, 0, MEM_FLAGS, mem_mk_type(MEM_GROUP_SYSTEM, MEM_TYPE_GENERIC), MEM_NAME, MEM_MODE, MEM_PRIV) 

#define NEW(TYPE) MANY(TYPE,1)

#define RESIZE(TYPE, MEM, COUNT) do{(MEM)=(TYPE*)mem_alloc_resize(MEM, sizeof(TYPE)*(COUNT), -1);}while(0)

#define SHARED(TYPE, COUNT, NAME, MODE, PRIV) (TYPE*)mem_alloc(sizeof(TYPE)*(COUNT), MEM_ALIGN, 0, MEM_FLAGS, mem_mk_type(MEM_GROUP_SYSTEM, MEM_TYPE_GENERIC), NAME, MODE, PRIV)

#define FLEXIBLE(TYPESTRUCT, SIZEOF_TYPEFLEX, COUNT) MANY(TYPESTRUCT, ROUND_UP((SIZEOF_TYPEFLEX) * (COUNT), sizeof(uintptr_t)))
#define FLEXIBLE_RESIZE(TYPESTRUCT, SIZEOF_TYPEFLEX, MEM, COUNT) RESIZE(TYPESTRUCT, MEM, ROUND_UP((SIZEOF_TYPEFLEX) * (COUNT), sizeof(uintptr_t)))

#ifdef EF_NOTBAD
	#define MBAD(MEM, SIZE)
#else
	#define MBAD(MEM, SIZE) mem_bad(MEM, SIZE)
#endif

#define mem_count(M) (mem_size(M)/sizeof((M)[0]))

int mem_bad(void* addr, size_t size);

int mem_check(const void* mem);

/* mem function not touch lock, lock is manual and thinked for multiple reader one writer */

int mem_lock_read(void* mem);

int mem_lock_write(void* mem);

int mem_unlock(void* mem);

#define mem_acquire_read(MEM)  for( int _acquire_ = mem_lock_read(MEM); _acquire_; --_acquire_, mem_unlock(MEM) )
#define mem_acquire_write(MEM) for( int _acquire_ = mem_lock_write(MEM); _acquire_; --_acquire_, mem_unlock(MEM) )

/******/

__malloc 
void* mem_alloc(size_t size, uint16_t aligned, uint16_t extend, uint8_t flags, uint16_t type, const char* name, const char* mode, const char* privilege);

void* mem_alloc_resize(void* mem, size_t size, ssize_t extend);
	
void* mem_extend_info_create(void* mem, size_t extend);

void mem_cleanup_fn(void* mem, mcleanup_f fn);

size_t mem_size(const void* mem);

size_t mem_real_size(const void* mem);

void mem_link(void* parent, void* mem);

void mem_reparent(void* parent, void* child);

void mem_child_displace(void* from, void* to);

void* mem_extend_info(const void* mem);

//shared_* unmap address if link, shared name unlink if !link and noot keep alive
void mem_free(void* mem);

void mem_free_auto(void* mem);

#define __mem_free __cleanup(mem_free_auto)

void mem_zero(void* mem);

void mem_group_set(void* mem, unsigned group);

void mem_type_set(void* mem, unsigned type);

void mem_group_check(void* mem, unsigned group);

void mem_type_check(void* mem, unsigned type);

unsigned mem_type(void* mem);


__target_vectorization
void mem_swap(void* restrict a, size_t sizeA, void* restrict b, size_t sizeB);

/*** protect.c ***/
/** get new key*/
pkey_t mem_pkey_new(unsigned int mode);
/** protect memory*/
err_t mem_protect(pkey_t* key, void* addr, size_t size, unsigned int mode);
/** changhe memory protection*/
err_t mem_protect_change(pkey_t key, unsigned int mode, void* addr);


#endif
