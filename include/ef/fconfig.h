#ifndef __EF_FCONFIG_H__
#define __EF_FCONFIG_H__

#include <ef/type.h>

typedef struct fconfig fconfig_t;

typedef enum{
	FCVAR_NULL,
	FCVAR_LONG,
	FCVAR_DOUBLE,
	FCVAR_STR,
	FCVAR_VECTOR
}fconfigVar_e;

typedef struct fconfigVar{
	fconfigVar_e type;
	union{
		void*  fcnull;
		long   fclong;
		double fcdouble;
		char*  fcstr;
		struct fconfigVar** fcvector;
	};
}fconfigVar_s;

fconfig_t* fconfig_load(const char* file, size_t maxVarLen);
const char* fconfig_error(fconfig_t* fc);
fconfigVar_s* fconfig_get(fconfig_t*  fc, const char* name, size_t len);
int fconfig_add(fconfig_t* fc, const char* name, size_t len, fconfigVar_s* value);

#endif
