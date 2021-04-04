#ifndef __EF_LIST_H__
#define __EF_LIST_H__

#include <ef/type.h>

/** callback list compare */
typedef int(*listCmp_f)(void* a, void* b);

/**************************/
/*** simple linked list ***/
/**************************/

/** set mem as new list
 * @param mem memory allocated with mem_many
 * @return pointer to data of new list or NULL for error
 */
void* list_simple_set(void* mem);

/** return next */
void* list_simple_next(void* lst);

/** add to head
 * @param head address of pointer to data head list
 * @param lst data list to add in head
 */
void list_simple_add_head(void* ptrhead, void* lst);

/** add to tail
 * @param head address of pointer to data head list
 * @param lst data list to add to tail
 */
void list_simple_add_tail(void* ptrhead, void* lst);

/** add before element
 * @param head address of pointer to head list
 * @param before add before this element
 * @param lst list to add
 */
void list_simple_add_before(void* ptrhead, void* before, void* lst);

/** add after element
 * @param head address of pointer to head list
 * @param after add after this element
 * @param lst list to add
 */
void list_simple_add_after(void* ptrhead, void* after, void* lst);

/** extract element from list
 * @param head address of pointer to head list
 * @param lst list to extract
 * @return an extracted list or null if not find
 */
void* list_simple_extract(void* ptrhead, void* lst);

/** extract element from list when listcmp return 0
 * @param head address of pointer to head list
 * @param userdata data to compare
 * @param fn callback function for compare data, fn(head, userdata)
 * @return an extracted list or null if not find
 */
void* list_simple_find_extract(void* ptrhead, void* userdata, listCmp_f fn);

/** foreach element in list
 * @param HEAD list
 * @param IT variable to set a data
 * @code
 * int* head = ....;
 * int* element;
 * listsimple_foreach(head, element){
 *	if( *element == ... ) ...;
 * }
 * @endcode
 */
#define list_simple_foreach(HEAD, IT) for(IT = HEAD; IT; IT = list_simple_next(IT))

/***************************************/
/*** double linked list concatenated ***/
/***************************************/

/** set mem as new list
 * @param mem memory allocated with mem_many
 * @return pointer to data of new list or NULL for error
 */
void* list_doubly_set(void* mem);

void* list_doubly_next(void* lst);

void* list_doubly_prev(void* lst);

/** add before element
 * @param before add before this element
 * @param lst list to add
 */
void list_doubly_add_before(void* before, void* lst);

/** add after element
 * @param after add after this element
 * @param lst list to add
 */
void list_doubly_add_after(void* after, void* lst);

/** merge list b after list a
 * @param a add list after this element
 * @param b list to merge
 */
void list_doubly_merge(void* a, void* b);

/** extract element from list
 * @param lst list to extract
 * @return same lst pass, but setted next and prev to lst
 */
void* list_doubly_extract(void* lst);

/** return 1 if have only root in list **/
int list_doubly_only_root(void* lst);

/** do while element in list, for traversing all list
 * @param HEAD head or element list
 * @param IT iterator name
 * @code
 * int* head = list_doubly_new(...);
 * ...
 * int* iterator;
 * list_doubly_do(head, iterator){
 *  if( *(iterator) == 1234 ) break;
 * }list_doubly_while(head, iterator);
 * @endcode
 */
#define list_doubly_do(HEAD, IT) do{ void* ___ ## IT ## ___ = HEAD; IT = HEAD; do
#define list_doubly_while(HEAD,IT) while( (IT=LIST_DOUBLY(IT)->next) != ___ ## IT ## ___ );}while(0)

#endif
