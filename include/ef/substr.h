#ifndef __EF_SUBSTR_H__
#define __EF_SUBSTR_H__

#include <ef/type.h>

typedef struct substr{
	const char* begin;
	const char* end;
}substr_s;

#define substr_len(SUB) ((SUB)->end - (SUB)->begin)

#define substr_cmp(SUB, STR) strncmp((SUB)->begin, STR, substr_len(SUB))

#define substr_format(SUB) (int)substr_len(SUB), (SUB)->begin


#endif
