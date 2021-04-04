#include <ef/list.h>
#include <ef/memory.h>

typedef struct listSimple{
	void* next;
}listSimple_s;

typedef struct listDoubly{
	void* next;
	void* prev;
}listDoubly_s;

/**********************/
/* simple linked list */
/**********************/

void* list_simple_set(void* mem){
	void* newmem = mem_extend_info_create(mem, sizeof(listSimple_s));
	if( !newmem ) return NULL;
	listSimple_s* lst = mem_extend_info(newmem);
	lst->next = NULL;
	mem_group_set(newmem,MEM_GROUP_LISTS);
	return newmem;
}

void* list_simple_next(void* lst){
	mem_group_check(lst, MEM_GROUP_LISTS);
	return ((listSimple_s*)mem_extend_info(lst))->next;
}

void list_simple_add_head(void* ptrhead, void* lst){
	void** r = (void**)ptrhead;
	mem_group_check(lst, MEM_GROUP_LISTS);
	listSimple_s* l = mem_extend_info(lst);
	iassert(l);
	l->next = *r;
	*r = lst;
}

void list_simple_add_tail(void* ptrhead, void* lst){
	void** tail;
	for(tail = (void**)ptrhead; *tail; tail = &(((listSimple_s*)mem_extend_info(*tail))->next));
	*tail = lst;
}

void list_simple_add_before(void* ptrhead, void* before, void* lst){
	void** be;
	for(be = (void**)ptrhead; *be && *be != before; be = &(((listSimple_s*)mem_extend_info(*be))->next));
	iassert( be );
	((listSimple_s*)mem_extend_info(lst))->next = *be;
	*be = lst;
}

void list_simple_add_after(void* ptrhead, void* after, void* lst){
	void** h = (void**)ptrhead;
	if( *h == NULL ){
		list_simple_add_head(ptrhead, lst);
		return;
	}
	void* af;
	listSimple_s* al = NULL;
	for(af = *h; (al=mem_extend_info(af))->next && af != after; af = al->next);
	iassert(al);
	iassert(af);
	((listSimple_s*)mem_extend_info(lst))->next = al->next;
	al->next = lst;
}

void* list_simple_extract(void* head, void* lst){
	void** be;
	for(be = (void**)head; *be && *be != lst; be = &(((listSimple_s*)mem_extend_info(*be))->next));
	iassert(be);
	listSimple_s* rmlst = mem_extend_info(lst);
	*be = rmlst->next;
	rmlst->next = NULL;
	return lst;
}

void* list_simple_find_extract(void* head, void* userdata, listCmp_f fn){
	void** be;
	for(be = (void**)head; *be && fn(*be, userdata); be =&(((listSimple_s*)mem_extend_info(*be))->next) );
	if( !*be ) return NULL;
	void* ret = *be;
	listSimple_s* rmlst = mem_extend_info(ret);
	*be = rmlst->next;
	rmlst->next = NULL;
	return ret;
}


/**********************/
/* double linked list */
/**********************/

#define list_doubly(MEM) ((listDoubly_s*)mem_extend_info(MEM)) 

void* list_doubly_set(void* mem){
	void* newmem = mem_extend_info_create(mem, sizeof(listDoubly_s));
	if( !newmem ) return NULL;
	listDoubly_s* lst = mem_extend_info(newmem);
	lst->next = mem;
	lst->prev = mem;
	mem_group_set(newmem, MEM_GROUP_LISTD);
	return newmem;
}

void* list_doubly_next(void* lst){
	mem_group_check(lst, MEM_GROUP_LISTD);
	return list_doubly(lst)->next;
}

void* list_doubly_prev(void* lst){
	mem_group_check(lst, MEM_GROUP_LISTD);
	return list_doubly(lst)->prev;
}

void list_doubly_add_before(void* before, void* lst){
	mem_group_check(before, MEM_GROUP_LISTD);
	mem_group_check(lst, MEM_GROUP_LISTD);
	listDoubly_s* b = list_doubly(before);
	listDoubly_s* l = list_doubly(lst);
	l->next = before;
	l->prev = b->prev;
	iassert( l->prev );
	list_doubly(l->prev)->next = lst;
	b->prev = lst;
}

void list_doubly_add_after(void* after, void* lst){
	mem_group_check(after, MEM_GROUP_LISTD);
	mem_group_check(lst, MEM_GROUP_LISTD);
	listDoubly_s* a = list_doubly(after);
	listDoubly_s* l = list_doubly(lst);
	l->prev = after;
	l->next = a->next;
	a->next = lst;
	iassert(l->next);
	list_doubly(l->next)->prev = lst;
}

void list_doubly_merge(void* a, void* b){
	mem_group_check(a, MEM_GROUP_LISTD);
	mem_group_check(b, MEM_GROUP_LISTD);
	listDoubly_s* la = list_doubly(a);
	listDoubly_s* lb = list_doubly(b);
	iassert( lb->prev );
	iassert( la->next );
	list_doubly(lb->prev)->next = la->next;
	list_doubly(la->next)->prev = lb->prev;
	la->next = b;
	lb->prev = a;
}

void* list_doubly_extract(void* lst){
	mem_group_check(lst, MEM_GROUP_LISTD);
	listDoubly_s* l = list_doubly(lst);
	iassert( l->prev );
	iassert( l->next );
	list_doubly(l->prev)->next = l->next;
	list_doubly(l->next)->prev = l->prev;
	l->next = lst;
	l->prev = lst;
	return lst;
}

int list_doubly_only_root(void* lst){
	mem_group_check(lst, MEM_GROUP_LISTD);
	return list_doubly(lst)->next == lst ? 1 : 0;
}

