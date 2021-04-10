#ifndef __EF_DSTR_H__
#define __EF_DSTR_H__

#include <ef/type.h>

char* ds_new(size_t size);
size_t ds_available(const char* dstr);
size_t ds_len(const char* dstr);
void ds_len_update(const char* dstr, size_t len);
void ds_nullc(char* dstr, size_t at);
void ds_clear(char* dstr);
char* ds_dup(const char* str, size_t len);
void ds_cpy(char** dst, const char* src, size_t len);
char* ds_vprintf(const char* format, va_list va1, va_list va2);
char* ds_printf(const char* format, ...);
void ds_svprintf(char** out, size_t at, const char* format, va_list va1, va_list va2);
void ds_sprintf(char** out, size_t at, const char* format, ...);
void ds_chomp(char* dstr);
void ds_insch(char** dst, size_t at, const char ch);
void ds_ins(char** dst, size_t at, const char* restrict src, size_t len);
void ds_push(char** dst, const char ch);
void ds_cat(char** dst, const char* str, size_t len);
void ds_del(char** dst, size_t at, size_t len);
void ds_replace(char** dst, const char* where, const char* replace, size_t lenR);
const char* ds_between(char** out, const char* parse);

#endif
