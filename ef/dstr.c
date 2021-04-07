#include <ef/str.h>
#include <ef/memory.h>
#include <ef/vector.h>
#include <stdarg.h>

typedef struct dstr{
	size_t len;
}dstr_s;

char* ds_new(size_t size){
	char* str = mem_alloc(size+1, MEM_ALIGN, sizeof(dstr_s), MEM_FLAGS, mem_mk_type(MEM_GROUP_STRING,MEM_TYPE_CHAR), MEM_NAME, MEM_MODE, MEM_PRIV);
	dstr_s* ds = mem_extend_info(str);
	ds->len = 0;
	*str = 0;
	return str;
}

size_t ds_available(const char* dstr){
	dstr_s* ds = mem_extend_info(dstr);
	return mem_size(dstr) - (ds->len + 1);
}

size_t ds_len(const char* dstr){
	return ((dstr_s*)mem_extend_info(dstr))->len;
}

void ds_len_update(const char* dstr, size_t len){
	((dstr_s*)mem_extend_info(dstr))->len = len;
}

void ds_nullc(char* dstr, size_t at){
	dstr[at] = 0;
	ds_len_update(dstr, at);
}

void ds_clear(char* dstr){
	ds_len_update(dstr, 0);
	*dstr = 0;
}

char* ds_dup(const char* str, size_t len){
	if( !len ) len = str_len(str);
	char* dret = ds_new(len+1);
	dstr_s* dr = mem_extend_info(dret);
	dr->len = len;
	memcpy(dret, str, len);
	dret[dr->len] = 0;
	return dret;
}

void ds_cpy(char** dst, const char* src, size_t len){
	if( len == 0 ) len = str_len(src);
	RESIZE(char, *dst, len);
	memcpy(*dst, src, len);
	(*dst)[len] = 0;
	ds_len_update(*dst, len);
}

char* ds_vprintf(const char* format, va_list va1, va_list va2){
	size_t len = vsnprintf(NULL, 0, format, va1);
	char* ret = ds_new(len+1);
	vsprintf(ret, format, va2);
	ds_len_update(ret, len);
	return ret;
}

char* ds_printf(const char* format, ...){
	va_list va1,va2;
	va_start(va1, format);
	va_start(va2, format);
	char* ret = ds_vprintf(format, va1, va2);
	va_end(va1);
	va_end(va2);
	return ret;
}

void ds_svprintf(char** out, size_t at, const char* format, va_list va1, va_list va2){
	const size_t dslen = ds_len(*out);
	if( at > dslen ) die("buffer overflow");
	size_t len = vsnprintf(NULL, 0, format, va1);
	if( ds_available(*out) < len + 1 ){
		RESIZE(char, *out, dslen+len+1);
	}

	if( at < dslen ){	
		memmove(*out+at+len, *out+at, (dslen+1)-at);
	}
	vsprintf(&(*out)[at], format, va2);
	ds_len_update(*out, dslen + len);
}

void ds_sprintf(char** out, size_t at, const char* format, ...){
	va_list va1,va2;
	va_start(va1, format);
	va_start(va2, format);
	ds_svprintf(out, at, format, va1, va2);
	va_end(va1);
	va_end(va2);
}

void ds_chomp(char* dstr){
	dstr_s* ds = mem_extend_info(dstr);
	if( ds->len > 0 && dstr[ds->len-1] == '\n' ){
		dstr[ds->len-1] = 0;
		--ds->len;
	}
}

void ds_insch(char** dst, size_t at, const char ch){
	const size_t dslen = ds_len(*dst);
	if( at > dslen ) die("buffer overflow");
	RESIZE(char, *dst, dslen+2);
	memmove(*dst+at+1, *dst+at, (dslen+1)-at);
	(*dst)[at] = ch;
	ds_len_update(*dst, dslen+1);
}

void ds_ins(char** dst, size_t at, const char* restrict src, size_t len){
	const size_t dslen = ds_len(*dst);
	if( len == 0 ) len = str_len(src);
	if( at > dslen ) die("buffer overflow");
	RESIZE(char, *dst, dslen+len+1);
	memmove(*dst+at+len, *dst+at, (dslen+1)-at);
	memcpy(*dst+at, src, len);
	ds_len_update(*dst, dslen + len);
}

void ds_push(char** dst, const char ch){
	const size_t dslen = ds_len(*dst);
	RESIZE(char, *dst, dslen+2);
	(*dst)[dslen] = ch;
	(*dst)[dslen+1] = 0;
	ds_len_update(*dst, dslen+1);
}

void ds_cat(char** dst, const char* str, size_t len){
	const size_t dslen = ds_len(*dst);
	if( !len ) len = str_len(str);
	//dbg_info("cat:(%s||%.*s)", *dst, (int)len, str);
	RESIZE(char, *dst, dslen+len+1);
	memcpy(*dst+dslen, str, len);
	(*dst)[dslen+len] = 0;
	ds_len_update(*dst, dslen + len);
	//dbg_info("out: %s", *dst);
	iassert((*dst)[ds_len(*dst)] == 0);
}

void ds_del(char** dst, size_t at, size_t len){
	const size_t dslen = ds_len(*dst);
	if( at > dslen ) die("buffer overflow, len:%lu at:%lu", len, at);
	iassert(len);
	if( at + len >= dslen ){
		(*dst)[at] = 0;
		ds_len_update(*dst, at);
	}
	else{
		memmove(*dst+at, *dst+at+len, (dslen+1)-(at+len));
		ds_len_update(*dst, dslen - len);
	}
	iassert((*dst)[ds_len(*dst)] == 0); 
	RESIZE(char, *dst, ds_len(*dst));
}

void ds_replace(char** dst, const char* where, const char* replace, size_t lenR){
	iassert(where);
	size_t lenW = str_len(where);
	//dbg_info("replace:: '%s' | s/%s/%.*s/g", *dst, where, (int)lenR, replace);
	if( !replace ) lenR = 0;
	else if( !lenR ) lenR = str_len(replace);
	const char* w = *dst;
   	while( *(w = str_find(w, where)) ){
		size_t at = w-*dst;
		ds_del(dst, at, lenW);
		if( lenR ){
			ds_ins(dst, at, replace, lenR);
			at += lenR;
		}
		w = &(*dst)[at];
	}
	//dbg_info("out '%s'", *dst);
}












