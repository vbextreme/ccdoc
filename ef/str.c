#include <ef/str.h>
#include <ef/memory.h>
#include <ef/vector.h>
#include <stdarg.h>

size_t str_available(const char* str, size_t len){
	if( len == 0 ) len = str_len(str);
	return mem_size(str) - (len + 1);
}

void str_upsize(char** str, size_t count, size_t len){
	size_t av = str_available(*str, len);
	if( av < count ){
		size_t ms = (mem_size(*str) + count) * 2;
		RESIZE(char, *str, ms);
	}
}

void str_downsize(char** str){
	size_t len = strlen(*str);
	size_t ms = mem_size(*str);
	if( ms / 4 > len  ){
		ms /= 2;
		RESIZE(char, *str, ms);
	}
}

char* str_dup(const char* src, size_t len){
	char* str = MANY(char, len + 1);
	mem_type_set(str, MEM_TYPE_CHAR);	

	memcpy(str, src, len);
	str[len] = 0;
	return str;
}

char* str_cpy(char* dst, const char* src){
	size_t len = strlen(src);
	memcpy(dst, src, len+1);
	return &dst[len];
}

char* str_vprintf(const char* format, va_list va1, va_list va2){
	size_t len = vsnprintf(NULL, 0, format, va1);
	char* ret = MANY(char, len+1);
	mem_type_set(ret, MEM_TYPE_CHAR);
	vsprintf(ret, format, va2);
	return ret;
}

char* str_printf(const char* format, ...){
	va_list va1,va2;
	va_start(va1, format);
	va_start(va2, format);
	char* ret = str_vprintf(format, va1, va2);
	va_end(va1);
	va_end(va2);
	return ret;
}

const char* str_skip_h(const char* str){
	while( *str && (*str == ' ' || *str == '\t') ) ++str;
	return str;
}

const char* str_skip_hn(const char* str){
	while( *str && (*str == ' ' || *str == '\t' || *str == '\n') ) ++str;
	return str;
}

const char* str_next_line(const char* str){
	while( *str && *str != '\n' ) ++str;
	if( *str ) ++str;
	return str;
}

void str_swaplt(char* restrict a, char* restrict b){
	for(; *a && *b; ++a, ++b ){
		SWAP(*a, *b);
	}
	if( *a == *b ) return;
	if( *a == 0 ){
		strcpy(a,b);
		*b = 0;
	}
	else{
		strcpy(b,a);
		*a = 0;
	}
}

void str_swap(char* restrict a, char* restrict b){
	for(; *a && *b; ++a, ++b ){
		SWAP(*a, *b);
	}
	if( *a == *b ) return;
	if( *a == 0 ){
		strcpy(a,b);
		*b = 0;
	}
	else{
		strcpy(b,a);
		*a = 0;
	}
}

void str_chomp(char* str){
	const ssize_t len = str_len(str);
	if( len > 0 && str[len-1] == '\n' ){
		str[len-1] = 0;
	}
}

void str_toupper(char* dst, const char* src){
	while( (*dst++=toupper(*src++)) );
}

void str_tolower(char* dst, const char* src){
	while( (*dst++=toupper(*src++)) );
}

void str_tr(char* str, const char* find, const char replace){
	while( (str=strpbrk(str,find)) ) *str++ = replace;
}

const char* str_chr(const char* str, const char ch){
	const char* ret = strchr(str, ch);
	return ret ? ret : &str[strlen(str)];
}

const char* str_find(const char* str, const char* need){
	const char* ret = strstr(str, need);
	return ret ? ret : &str[strlen(str)];
}

const char* str_nfind(const char* str, const char* need, size_t max){
	const char* f = str;
	size_t len = str_len(need);
	size_t lr = max;
	while( (f=memchr(f, *need, lr)) ){
		if( memcmp(f, need, len) ) return f;
		++f;
		lr = max - (f-str);
	}
	return str+max;	
}

const char* str_anyof(const char* str, const char* any){
	const char* ret = strpbrk(str, any);
	return ret ? ret : &str[strlen(str)];
}

char** str_split(char* str, const char* delimit){
	size_t lendel = strlen(delimit);
	char** vd = vector_new(char*, 4);
	char* token = str;
	while( *(str=(char*)str_find(str, delimit)) ){
		*str = 0;
		vector_push(vd, token);
		str += lendel;
		token = str;
	}
	if( *token ){
		vector_push(vd, token);
	}
	return vd;
}

void str_insch(char* dst, const char ch){
	size_t ld = strlen(dst);
	memmove(dst+1, dst, ld);
	*dst = ch;
}

void str_ins(char* dst, const char* restrict src, size_t len){
	size_t ld = strlen(dst);
	memmove(dst+len, dst, ld+1);
	memcpy(dst, src, len);
}

void str_del(char* dst, size_t len){
	size_t ld = strlen(dst);
	memmove(dst, &dst[len], (ld - len)+1);
}

char* quote_printable_decode(size_t *len, const char* str){
	size_t strsize = strlen(str);
	char* ret = MANY(char, strsize + 1);
	mem_type_set(ret, MEM_TYPE_CHAR);
	if( !ret ){
		//err_pushno("eom");
		return NULL;
	}

	char* next = ret;
	while( *str ){
		if( *str != '=' ){
			*next++ = *str++;
		}
		else{
			++str;
			if( *str == '\r' ) ++str;
			if( *str == '\n' ){
				++str;
				continue;
			}	
			char val[3];
			val[0] = *str++;
			val[1] = *str++;
			val[2] = 0;
			*next++ = strtoul(val, NULL, 16);
		}
	}
	*next = 0;
	if( len ) *len = next - ret;
	return ret;
}

__private unsigned prv_part(const char** prv){
	unsigned n = 3;
	unsigned ret = 0;
	while( **prv && n --> 0 ){
		switch( **prv ){
			case 'r': ret |= 4; break;
			case 'w': ret |= 2; break;
			case 'x': ret |= 1; break;
		}
		++(*prv);
	}
	return ret;
}

unsigned str_to_prv(const char* prv){
	if( !prv ) return 0;
	int who = 6;
	unsigned ret = 0;
	while( *prv && who >=0 ){
		ret |= prv_part(&prv) << who;
		who -= 3;
	}
	return ret;
}

const char* str_errno(void){
	return strerror(errno);
}
