#ifndef __EF_RBHASH_H__
#define __EF_RBHASH_H__

#include <ef/type.h>

typedef uint32_t(*rbhash_f)(const void* name, size_t len);

typedef struct rbhash rbhash_t;

/*************/
/* hashalg.c */
/*************/

uint32_t hash_one_at_a_time(const void *key, size_t len);
uint32_t hash_fasthash(const void* key, size_t len);
uint32_t hash_kr(const void* key, size_t len);
uint32_t hash_sedgewicks(const void* key, size_t len);
uint32_t hash_sobel(const void* key, size_t len);
uint32_t hash_weinberger(const void* key, size_t len);
uint32_t hash_elf(const void* key, size_t len);
uint32_t hash_sdbm(const void* key, size_t len);
uint32_t hash_bernstein(const void* key, size_t len);
uint32_t hash_knuth(const void* key, size_t len);
uint32_t hash_partow(const void* key, size_t len);

/************/
/* rbhash.c */
/************/

/** create rbhash
 * @param size number of starting element of table
 * @param min percentage min need to be available, if 0 the hash table is not automatic reallocated
 * @param keysize the max size of key
 * @param hashing hash function
 * @param del the function to delete each element
 * @return 0 successfull -1 error, err is pushed and errno is setted
 */
rbhash_t* rbhash_new(size_t size, size_t min, size_t keysize, rbhash_f hashing);

/** add new element to hash table
 * @param rbh hashtable
 * @param hash the hash value of key
 * @param key the key
 * @param len len of key, 0 auto call strlen(key)
 * @param data userdata associated to key
 * @return 0 successfull -1 error, fail if no space left on hash table, EFBIG if key > keysize, err is pushed and errno is setted
 */
err_t rbhash_add_hash(rbhash_t* rbh, uint32_t hash, const void* key, size_t len, void* data);

/** add new element to hash table, call rbhash_add_hash calcolated hash with rbhash->hashing
 * @param rbh hashtable
 * @param key the key
 * @param len len of key, 0 auto call strlen(key)
 * @param data userdata associated to key
 * @return 0 successfull -1 error, fail if no space left on hash table, EFBIG if key > keysize, err is pushed and errno is setted
 */
err_t rbhash_add(rbhash_t* rbh, const void* key, size_t len, void* data);

/** add new element to hash table only if key not exists, call rbhash_find and use rbhash->hashing
 * @param rbh hashtable
 * @param key the key
 * @param len len of key, 0 auto call strlen(key)
 * @param data userdata associated to key
 * @return 0 successfull -1 error, fail if no space left on hash table, EFBIG if key > keysize, err is pushed and errno is setted
 */
err_t rbhash_add_unique(rbhash_t* rbh, const void* key, size_t len, void* data);

/** find rbhashElement
 * @param rbh hashtable
 * @param hash hash of key
 * @param key key to find
 * @param len len of key, 0 auto call strlen(key)
 * @return user data associated to key or NULL for error
 */
void* rbhash_find_hash(rbhash_t* rbh, uint32_t hash, const void* key, size_t len);

/** find rbhashElement, use rbhash_find_hash called with rbhash->hashing
 * @param rbh hashtable
 * @param key key to find
 * @param len len of key, 0 auto call strlen(key)
 * @return user data associated to key or NULL for error
 */
void* rbhash_find(rbhash_t* rbh, const void* key, size_t len);

/** remove element from hash table, automatic call delete function to user data
 * @param rbh hashtable
 * @param hash hash of key
 * @param key key to find
 * @param len len of key, 0 auto call strlen(key)
 * @return 0 successfull -1 error
 */
err_t rbhash_remove_hash(rbhash_t* rbh, uint32_t hash, const void* key, size_t len);

/** remove element from hash table, automatic call delete function to user data, call rbhash_remove_hash with rbhash->hashing
 * @param ht hashtable
 * @param key key to find
 * @param len len of key, 0 auto call strlen(key)
 * @return 0 successfull -1 error
 */
err_t rbhash_remove(rbhash_t* ht, const void* key, size_t len);

/** total memory usage
 * @param rbh
 * @return memory usage
 */
size_t rbhash_mem_total(rbhash_t* rbh);

/** count bucket
 * @param rbh
 * @return bucket count
 */
size_t rbhash_bucket_used(rbhash_t* rbh);

/** count number of collision, all hash with not have .distance == 0
 * @param rbh
 * @return total collision
 */
size_t rbhash_collision(rbhash_t* rbh);

/** max distance of slot
 * @param rbh
 * @return distance
 */
size_t rbhash_distance_max(rbhash_t* rbh);

#if DEBUG_ENABLE > 0
void rbhash_dbg_print(rbhash_t* rbh, size_t begin, size_t end);
#endif


#endif
