#include <ef/type.h>
#include <ef/memory.h>
#include <ef/rbhash.h>
#include <ef/mth.h>

typedef struct rbhashElement{
	void* data;        /**< user data*/
	uint32_t hash;     /**< hash */
	uint32_t len;      /**< len of key*/
	uint32_t distance; /**< distance from hash*/
	char key[];        /**< flexible key*/
}rbhashElement_s;

typedef struct rbhash{
	rbhashElement_s* table; /**< hash table*/
	size_t size;            /**< total bucket of table*/
	size_t elementSize;     /**< sizeof rbhashElement*/
	size_t count;           /**< bucket used*/
	size_t min;             /**< percentage of free bucket*/
	size_t maxdistance;     /**< max distance from hash*/
	size_t keySize;         /**< key size*/
	rbhash_f hashing;       /**< function calcolate hash*/
}rbhash_t;

#define rbhash_slot(HASH,SIZE) FAST_MOD_POW_TWO(HASH, SIZE)
#define rbhash_slot_next(SLOT, SIZE) (rbhash_slot((SLOT)+1,SIZE))
#define rbhash_element_slot(TABLE, ESZ, SLOT) ((rbhashElement_s*)(ADDR(TABLE) + ((ESZ) * (SLOT))))
#define rbhash_element_next(TABLE, ESZ) ((rbhashElement_s*)(ADDR(TABLE) + (ESZ)))

rbhash_t* rbhash_new(size_t size, size_t min, size_t keysize, rbhash_f hashing){
	rbhash_t* rbh = NEW(rbhash_t);
	rbh->size = ROUND_UP_POW_TWO32(size);
	rbh->min = min;
	rbh->hashing = hashing;
	rbh->count = 0;
	rbh->maxdistance = 0;
	rbh->keySize = keysize;
	rbh->elementSize = ROUND_UP(sizeof(rbhashElement_s), sizeof(void*));
	rbh->elementSize = ROUND_UP(rbh->elementSize + keysize + 1, sizeof(void*));
	iassert((rbh->elementSize % sizeof(void*)) == 0);
	rbh->table = mem_alloc(rbh->elementSize * rbh->size, 0, 0, MEM_FLAGS, mem_mk_type(MEM_GROUP_RBHASH, MEM_TYPE_GENERIC), NULL, 0, 0 );
	mem_reparent(rbh, rbh->table);
	rbhashElement_s* table = rbh->table;
	for( size_t i = 0; i < rbh->size; ++i, table = rbhash_element_next(table,rbh->elementSize)){
		table->len = 0;
	}
	return rbh;
}

__private void rbhash_swapdown(rbhashElement_s* table, const size_t size, const size_t esize, size_t* maxdistance, rbhashElement_s* nw){
	uint32_t bucket = rbhash_slot(nw->hash, size);

	rbhashElement_s* tbl = rbhash_element_slot(table, esize, bucket);
	size_t i = 0;
	while( i < size && tbl->len != 0 ){
		if(  nw->distance > tbl->distance ){
			SWAP(tbl->data, nw->data);
			SWAP(tbl->distance, nw->distance);
			SWAP(tbl->hash, nw->hash);
			SWAP(tbl->len, nw->len);
			mem_swap(tbl->key, tbl->len, nw->key, nw->len);
		}
		++nw->distance;
		bucket = rbhash_slot_next(bucket, size);
		tbl = rbhash_element_slot(table, esize, bucket);
		++i;
		if( nw->distance > *maxdistance ) *maxdistance = nw->distance;
	}
	if( tbl->len != 0 ) die("hash lose element, the hash table is to small");
	memcpy(tbl,nw,esize);
}

__private void rbhash_upsize(rbhash_t* rbh){
	const size_t p = ((rbh->size * rbh->min) / 100) + 1;
	const size_t pm = rbh->count + p;
	dbg_info("upsize: required available %lu, used %lu, size %lu", p, rbh->count, rbh->size);
	if( rbh->size > pm ) return;
	
	const size_t newsize = ROUND_UP_POW_TWO32(rbh->size + p);
	dbg_info("resize to %lu", newsize);

	rbhashElement_s* newtable = mem_alloc(rbh->elementSize * newsize, 0, 0, MEM_FLAGS, mem_mk_type(MEM_GROUP_RBHASH, MEM_TYPE_GENERIC), NULL, 0, 0 );
	rbhashElement_s* table = newtable;
	for( size_t i = 0; i < newsize; ++i, table = rbhash_element_next(table,rbh->elementSize)){
		table->len = 0;
	}

	rbh->maxdistance = 0;
	table = rbh->table;
	for(size_t i = 0; i < rbh->size; ++i, table = rbhash_element_next(table,rbh->elementSize)){
		if( table->len == 0 ) continue;
		table->distance = 0;
		rbhash_swapdown(newtable, newsize, rbh->elementSize, &rbh->maxdistance, table);
	}
	mem_child_displace(rbh->table, newtable);
	mem_free(rbh->table);
	rbh->table = newtable;
	rbh->size = newsize;
}

err_t rbhash_add_hash(rbhash_t* rbh, uint32_t hash, const void* key, size_t len, void* data){
	if( len > rbh->keySize ){
		errno = EFBIG;
		return -1;
	}
	__mem_free rbhashElement_s* el = mem_alloc(rbh->elementSize, 0, 0, MEM_FLAG_MALLOC, 0, NULL, 0, 0 );
	el->data = data;
	el->distance = 0;
	el->hash = hash;
	el->len = len;
	memcpy(el->key, key, len);
	rbhash_swapdown(rbh->table, rbh->size, rbh->elementSize, &rbh->maxdistance, el);
	++rbh->count;
	//scan_build_unknown_cleanup(el);
	if( rbh->min ) rbhash_upsize(rbh);
	return 0;
}

err_t rbhash_add(rbhash_t* rbh, const void* key, size_t len, void* data){
	return rbhash_add_hash(rbh, rbh->hashing(key, len), key, len, data);
}

err_t rbhash_add_unique(rbhash_t* rbh, const void* key, size_t len, void* data){
	if( rbhash_find(rbh, key, len) ) return -1;
	return rbhash_add(rbh, key, len, data);
}

__private long rbhash_find_bucket(rbhash_t* rbh, uint32_t hash, const void* key, size_t len){
	uint32_t slot = rbhash_slot(hash, rbh->size);
	rbhashElement_s* table = rbhash_element_slot(rbh->table, rbh->elementSize, slot);

	size_t maxscan = rbh->maxdistance + 1;
	while( maxscan-->0 ){
		if( table->len == len && table->hash == hash && !memcmp(key, table->key, len) ){
			return slot;
		}
		slot = rbhash_slot_next(slot, rbh->size);
		table = rbhash_element_slot(rbh->table, rbh->elementSize, slot);
	}
	dbg_warning("not find key %.*s",(int)len, (char*)key);
	errno = ESRCH;
	return -1;
}

__private rbhashElement_s* rbhash_find_hash_raw(rbhash_t* rbh, uint32_t hash, const void* key, size_t len){
	long bucket;
	if( (bucket = rbhash_find_bucket(rbh, hash, key, len)) == -1 ) return NULL;
	return rbhash_element_slot(rbh->table, rbh->elementSize, bucket);
}

void* rbhash_find_hash(rbhash_t* rbh, uint32_t hash, const void* key, size_t len){
	rbhashElement_s* el = rbhash_find_hash_raw(rbh, hash, key, len);
	return el ? el->data : NULL;
}

void* rbhash_find(rbhash_t* rbh, const void* key, size_t len){
	uint32_t hash = rbh->hashing(key, len);
	return rbhash_find_hash(rbh, hash, key, len);
}

__private void rbhash_swapup(rbhashElement_s* table, size_t size, size_t esize, size_t bucket){
	size_t bucketfit = bucket;
	bucket = rbhash_slot_next(bucket, size);
	rbhashElement_s* tbl = rbhash_element_slot(table, esize, bucket);

	while( tbl->len != 0 && tbl->distance ){
		rbhashElement_s* tblfit = rbhash_element_slot(table, esize, bucketfit);
		memcpy(tblfit, tbl, esize);
		tbl->len = 0;
		tbl->data = NULL;
		bucketfit = bucket;
		bucket = rbhash_slot_next(bucket, size);
		table = rbhash_element_slot(table, esize, bucket);
	}
}

err_t rbhash_remove_hash(rbhash_t* rbh, uint32_t hash, const void* key, size_t len){
	long bucket = rbhash_find_bucket(rbh, hash, key, len);
	if( bucket == -1 ) return -1;
	rbhashElement_s* el = rbhash_element_slot(rbh->table, rbh->elementSize, bucket);
	el->len = 0;
	el->data = 0;
	rbhash_swapup(rbh->table, rbh->size, rbh->elementSize, bucket);
	--rbh->count;
	return 0;	
}

err_t rbhash_remove(rbhash_t* ht, const void* key, size_t len){
	return rbhash_remove_hash(ht, ht->hashing(key, len), key, len);
}

size_t rbhash_mem_total(rbhash_t* rbh){
	size_t ram = sizeof(rbhash_t);
	ram += rbh->elementSize * rbh->size;
	return ram;
}

size_t rbhash_bucket_used(rbhash_t* rbh){
	return rbh->count;
}

size_t rbhash_collision(rbhash_t* rbh){
	rbhashElement_s* tbl = rbh->table;
	size_t collision = 0;
	for(size_t i =0; i < rbh->size; ++i, tbl = rbhash_element_next(tbl,rbh->elementSize)){
		if( tbl->len != 0 && tbl->distance != 0 ) ++collision;
	}
	return collision;
}

size_t rbhash_distance_max(rbhash_t* rbh){
	return rbh->maxdistance;
}

#if DEBUG_ENABLE > 0
void rbhash_dbg_print(rbhash_t* rbh, size_t begin, size_t end){
	dbg_info("max.d:%lu",rbh->maxdistance);
	dbg_info("count:%lu",rbh->count);
	dbg_info("size :%lu",rbh->size);
	dbg_info("esize:%lu",rbh->elementSize);
	dbg_info("ksize:%lu",rbh->keySize);
	dbg_info("slot ] distance|              string             |     hash    | hash %% size |");

	rbhashElement_s* table;
	for( size_t i = begin; i < end; ++i ){
		table = rbhash_element_slot(rbh->table,rbh->elementSize,i);
		dbg_info("%7lu] %8u| %32.*s| %12u| %12lu|", i, table->distance, table->len, table->key, table->hash, FAST_MOD_POW_TWO(table->hash, rbh->size));
	}	

}
#endif
