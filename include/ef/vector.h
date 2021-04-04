#ifndef __EF_VECTOR_H__
#define __EF_VECTOR_H__

#include <ef/type.h>
#include <ef/memory.h>

/** create new vector
 * @param sof sizeof element
 * @param size begine element size 
 * @return memory
 */
void* vector_new_raw(size_t sof, size_t size);

/** create new vector
 * @param T type vector
 * @param S begin element size 
 * @return memory
 */
#define vector_new(T, S) (T*)vector_new_raw(sizeof(T), S)
#define VNEW(T, S) vector_new(T, S)

/** clear vector, set count to 0
 * @param m mem of vector
 */
void vector_clear(void* m);

/** get max element count*/
size_t vector_max(void* m);

/** get vector count */
size_t vector_count(void* m);

/** change vector count */
void vector_count_update(void* m, size_t count);

/** return pointer to vector count*/
size_t* vector_count_ref(void* m);

/** get sizeof */
size_t vector_size_of(void* m);

/** resize of vector
 * @param mem vector
 * @param count elements 
 */
void vector_resize(void** mem, size_t count);

/** increase size of vector of count element if need
 * @param ptrmem pointer to vector mem
 * @param count elements to upsize
 */
void vector_upsize(void* ptrmem, size_t count);

/** decrease size of vector of count element if need
 * @param ptrmem pointer to vector mem
 */
void vector_downsize(void* ptrmem);

/** check id vector is empty
 * @param M mem of vector
 * 1 empty 0 no
 */ 
#define vector_isempty(M) (vector_count(M)==0)

/** remove element from index
 * @param ptrmem address of vector mem, can be change
 * @param index element to remove
 */
void vector_remove_raw(void** ptrmem, const size_t index);

/** remove element from index
 * @param M vector, address can change
 * @param I element to remove
 */
#define vector_remove(M, I) vector_remove_raw((void**)&(M), I);

/** add space in index position for setting new value
 * @param ptrmem pointer to mem of vector
 * @param index position where add new space
 */
void vector_add_raw(void* ptrmem, const size_t index);

/** add element in position I, warning address of M can change
 * @param M mem of vector
 * @param I index
 * @param E element
 */
#define vector_add(M, I, E) do{ vector_add_raw(&(M), I); (M)[I] = E; } while(0)

/** add element in position I, warning address of M can change
 * @param M mem of vector
 * @param I index
 * @return ref
 */
#define vector_add_ref(M, I) ({\
	vector_add_raw(&(M), I);\
	&(M)[I];\
})


/** get a ptr of new last element of vector
 * @param M mem of vector
 * @return prt of new element
 */
#define vector_push_ref(M) ({\
	void* _ret_ = NULL;\
	vector_upsize(&(M), 1);\
	_ret_ = &(M)[(*vector_count_ref(M))++];\
	_ret_;\
})

/** copy element to new last element in vector
 * @param M mem of vector
 * @param ELEMENT element to copy
 */
#define vector_push(M, ELEMENT) do{\
	vector_upsize(&M, 1);\
	(M)[(*vector_count_ref(M))++] = ELEMENT;\
}while(0)

/** extract last element of vector, warning not check if vector is empty
 * @param M mem of vector
 * @return ELEMENT element to copy
 */
#define vector_pull(M) (M)[--(*vector_count_ref(M))]

/** foreach element of vector
 * @param M mem of vector
 * @param I iterator name
 */
#define vector_foreach(M, I) for(size_t I = 0; I < vector_count(M); ++I )

/** qsort vector
 * @param M mem of vector
 * @param CMPFN compare function
 */
#define vector_qsort(M, CMPFN) qsort(M, vector_count(M), vector_sizeof(M), CMPFN)

/** resize vector to count object
 * @param ptrmem pointer to mem of vector
 * @return -1 error 0 successfull
 */
void vector_fitting(void* ptrmem);

/** shuffle a vector, call mth_random_begin() before use
 * @param mem vector
 * @param begin begin index
 * @param end end index
 */
void vector_shuffle(void* mem, size_t begin, size_t end);

#define vector_shuffle_all(V) vector_shuffle(V, 0, vector_count(V)-1)


#endif 
