#ifndef __EF_VECTORIZATION_H__
#define __EF_VECTORIZATION_H__

#include <ef/type.h>

typedef struct valign{
	size_t pre;
	size_t aligned;
	size_t post;
	size_t start;
	size_t end;
	void* scalarpre;
	void* scalarpost;
	void* vector;
}valign_s;

#define __vector4 __attribute__((vector_size(4)))
#define __vector8 __attribute__((vector_size(8)))
#define __vector16 __attribute__((vector_size(16)))
#define __vector32 __attribute__((vector_size(32)))
#define __vector64 __attribute__((vector_size(64)))
#define __vector_aligned(N) __attribute__((aligned(__BIGGEST_ALIGNMENT__)))
#define __is_aligned(EXP,N) __builtin_assume_aligned(EXP,N)
#define __is_aligned_but(EXP,N,UNALIGNED) __builtin_assume_aligned(EXP,N,UNALIGNED)

typedef char char8_v __vector8;
typedef char char16_v __vector16;
typedef char char32_v __vector32;
typedef char char64_v __vector64;
typedef unsigned char uchar8_v __vector8;
typedef unsigned char uchar16_v __vector16;
typedef unsigned char uchar32_v __vector32;
typedef unsigned char uchar64_v __vector64;

typedef short int short8_v __vector16;
typedef short int short16_v __vector32;
typedef short int short32_v __vector64;
typedef unsigned short int ushort8_v __vector16;
typedef unsigned short int ushort16_v __vector32;
typedef unsigned short int ushort32_v __vector64;

typedef int int4_v __vector16;
typedef int int8_v __vector32;
typedef int int16_v __vector64;
typedef unsigned int uint4_v __vector16;
typedef unsigned int uint8_v __vector32;
typedef unsigned int uint16_v __vector64;

typedef long long2_v __vector16;
typedef long long4_v __vector32;
typedef long long8_v __vector64;
typedef unsigned long ulong2_v __vector16;
typedef unsigned long ulong4_v __vector32;
typedef unsigned long ulong8_v __vector64;

typedef float float4_v  __vector16;
typedef float float8_v  __vector32;
typedef float float16_v __vector64;

typedef double double2_v __vector16;
typedef double double4_v __vector32;
typedef double double8_v __vector64;

/**vectorization loop*/
#define vectorize_loop(VECTYPE, TYPE, PTR, START, END, SCALARBODY, VECTORBODY) do{\
	valign_s __va__ = {\
		.start = START,\
		.end = END,\
		.scalarpre = PTR,\
		.scalarpost = NULL,\
		.vector = NULL\
	};\
	__vectorize_begin(&__va__, sizeof(VECTYPE), sizeof(TYPE));\
	TYPE* scalar = __va__.scalarpre;\
	for(unsigned __iterator__ = 0; __iterator__ < __va__.pre; ++__iterator__) SCALARBODY\
	VECTYPE* vector = __is_aligned(__va__.vector,sizeof(VECTYPE));\
	for(unsigned __iterator__ = 0; __iterator__ < __va__.aligned; ++__iterator__) VECTORBODY\
	scalar = __va__.scalarpost;\
	for(unsigned __iterator__ = 0; __iterator__ < __va__.post; ++__iterator__) SCALARBODY\
}while(0)

#define vectorize_pair_loop(VECTYPE, TYPE, PTRA, STARTA, ENDA, PTRB, STARTB, ENDB, SCALARBODY, VECTORBODY) do{\
	valign_s __va__ = {\
		.start = STARTA,\
		.end = ENDA,\
		.scalarpre = PTRA,\
		.scalarpost = NULL,\
		.vector = NULL\
	};\
	valign_s __vb__ = {\
		.start = STARTB,\
		.end = ENDB,\
		.scalarpre = PTRB,\
		.scalarpost = NULL,\
		.vector = NULL\
	};\
	__vectorize_pair_begin(&__va__, &__vb__, sizeof(VECTYPE), sizeof(TYPE));\
	TYPE* Ascalar = __va__.scalarpre;\
	TYPE* Bscalar = __vb__.scalarpre;\
	for(unsigned __iterator__ = 0; __iterator__ < __va__.pre; ++__iterator__) SCALARBODY\
	VECTYPE* Avector = __is_aligned(__va__.vector,sizeof(VECTYPE));\
	VECTYPE* Bvector = __is_aligned(__vb__.vector,sizeof(VECTYPE));\
	for(unsigned __iterator__ = 0; __iterator__ < __va__.aligned; ++__iterator__) VECTORBODY\
	Ascalar = __va__.scalarpost;\
	Bscalar = __vb__.scalarpost;\
	for(unsigned __iterator__ = 0; __iterator__ < __va__.post; ++__iterator__) SCALARBODY\
}while(0)

#define vector2_set_all(VAL) {VAL,VAL}
#define vector4_set_all(VAL) {VAL,VAL,VAL,VAL}
#define vector8_set_all(VAL) {VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL}
#define vector16_set_all(VAL) {VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL}
#define vector32_set_all(VAL) {VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL}
#define vector64_set_all(VAL) {VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL}

/**get aligned address*/
void __vectorize_begin(valign_s* va, size_t const vsize, size_t const ssize);
/**get aligned address*/
void __vectorize_pair_begin(valign_s* va, valign_s* vb, size_t const vsize, size_t const ssize);

#endif
