#include <ef/memory.h>
#include <ef/mth.h>

typedef struct vector{
	size_t count;
	size_t sof;
}vector_s;

void* vector_new_raw(size_t sof, size_t size){
	void* vmem = mem_alloc(sof * size, MEM_ALIGN, sizeof(vector_s), MEM_FLAGS, mem_mk_type(MEM_GROUP_VECTOR, MEM_TYPE_GENERIC), MEM_NAME, MEM_MODE, MEM_PRIV );
	vector_s* v = mem_extend_info(vmem);
	v->count = 0;
	v->sof = sof;
	return vmem;
}

void vector_clear(void* m){
	if( !m ) return;
	mem_group_check(m, MEM_GROUP_VECTOR);
	vector_s* v = mem_extend_info(m);
	v->count = 0;
}

size_t vector_max(void* m){
	mem_group_check(m, MEM_GROUP_VECTOR);
	vector_s* v = mem_extend_info(m);
	return mem_size(m) / v->sof;
}

size_t vector_count(void* m){
	mem_group_check(m, MEM_GROUP_VECTOR);
	vector_s* v = mem_extend_info(m);
	return v->count;
}

void vector_count_update(void* m, size_t count){
	mem_group_check(m, MEM_GROUP_VECTOR);
	iassert( count < vector_max(m) );
	vector_s* v = mem_extend_info(m);
	v->count = count;
}

size_t* vector_count_ref(void* m){
	mem_group_check(m, MEM_GROUP_VECTOR);
	vector_s* v = mem_extend_info(m);
	return &v->count;
}

size_t vector_size_of(void* m){
	mem_group_check(m, MEM_GROUP_VECTOR);
	vector_s* v = mem_extend_info(m);
	return v->sof;
}

void vector_resize(void** ptrmem, size_t count){
	void* mem = *(void**)ptrmem;
	mem_group_check(mem, MEM_GROUP_VECTOR);
	vector_s* v = mem_extend_info(mem);
	count *= v->sof;
	size_t vsize = mem_size(mem);
	if( vsize != count ){
		*ptrmem = mem_alloc_resize(mem, count, -1);
	}
}

void vector_upsize(void* ptrmem, size_t count){
	void** mem = ptrmem;
	mem_group_check(*mem, MEM_GROUP_VECTOR);
	vector_s* v = mem_extend_info(*mem);
	*mem = mem_alloc_resize(*mem, (count + v->count) * v->sof, -1);
}

void vector_downsize(void* ptrmem){
	void** mem = ptrmem;
	mem_group_check(*mem, MEM_GROUP_VECTOR);
	vector_s* v = mem_extend_info(*mem);
	*mem = mem_alloc_resize(*mem, v->count * v->sof, -1);
}

void vector_remove_raw(void** ptrmem, const size_t index){
	void* mem = *(void**)ptrmem;
	mem_group_check(mem, MEM_GROUP_VECTOR);

	vector_s* v = mem_extend_info(mem);

	if( v->count == 0 || index >= v->count){
		dbg_warning("index out of bound");	
		return;
	}

	if( index < v->count - 1 ){
		void* dst = (void*)((uintptr_t)mem + index * v->sof);
		const void* src = (void*)((uintptr_t)mem + ((index+1) * v->sof));
		const size_t msz = (v->count - index) * v->sof;
		memmove(dst, src, msz);
	}
	--v->count;

	vector_downsize(ptrmem);
}

void vector_add_raw(void* ptrmem, const size_t index){
	void* mem = *(void**)ptrmem;
	mem_group_check(mem, MEM_GROUP_VECTOR);

	vector_s* v = mem_extend_info(mem);

	if( v->count == 0 || index >= v->count){
		die("index out of bound");	
	}
	
	vector_upsize(ptrmem, 1);
	mem = *(void**)ptrmem;

	void* dst = (void*)((uintptr_t)mem + ((index + 1) * v->sof));
	const void* src = (void*)((uintptr_t)mem + (index * v->sof));
	const size_t msz = (v->count - index) * v->sof;
	memmove(dst, src, msz);
	++v->count;
}

void vector_fitting(void* ptrmem){
	mem_group_check(*(void**)ptrmem, MEM_GROUP_VECTOR);

	vector_s* v = mem_extend_info(*(void**)ptrmem);
	vector_resize(ptrmem, v->count);
}

void vector_shuffle(void* m, size_t begin, size_t end){
	mem_group_check(m, MEM_GROUP_VECTOR);

	vector_s* v = mem_extend_info(m);

	const size_t count = (end - begin) + 1;
	for( size_t i = begin; i <= end; ++i ){
		size_t j = begin + mth_random(count);
		if( j != i ){
			mem_swap(ADDR(m) + (i * v->sof) , v->sof, ADDR(m) + (j * v->sof), v->sof);
		}
	}
}

