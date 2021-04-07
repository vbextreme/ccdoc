#ifndef __EF_STR_H__
#define __EF_STR_H__

#include <ef/type.h>

#define str_clear(S) do{*(S)=0; }while(0);
#define str_len(S) strlen(S)
#define str_cmp(A,B) strcmp(A,B)

size_t str_available(const char* str, size_t len);
void str_upsize(char** str, size_t count, size_t len);
void str_downsize(char** str);
char* str_dup(const char* src, size_t len);
char* str_cpy(char* dst, const char* src);
char* str_vprintf(const char* format, va_list va1, va_list va2);
char* str_printf(const char* format, ...);
const char* str_skip_h(const char* str);
const char* str_skip_hn(const char* str);
const char* str_next_line(const char* str);
void str_swaplt(char* restrict a, char* restrict b);
void str_swap(char* restrict a, char* restrict b);
void str_chomp(char* str);
void str_toupper(char* dst, const char* str);
void str_tolower(char* dst, const char* str);
void str_tr(char* str, const char* find, const char replace);
const char* str_chr(const char* str, const char ch);
const char* str_find(const char* str, const char* need);
const char* str_nfind(const char* str, const char* need, size_t max);
const char* str_anyof(const char* str, const char* any);
//delimit str with \0 and return vector of pointers to each token
char** str_split(char* str, const char* delimit);
void str_insch(char* dst, const char ch);
void str_ins(char* dst, const char* restrict src, size_t len);
void str_del(char* dst, size_t len);
char* quote_printable_decode(size_t *len, const char* str);
unsigned str_to_prv(const char* prv);
const char* str_errno(void);






#endif
