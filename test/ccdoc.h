#ifndef __CCDOC_H__
#define __CCDOC_H__

/*-str 'test' 'string for test'*/

/*-file 'ccdoc' comment C documentations 
 * test title @^4 'subtitle' ok @*test \n
 * test link @*extra
 * */

/*-visual index*/


/*- simple define*/
#define CCDOC_VFILES 4

/*- simple type*/
typedef int err_t;

/*- get a substring*/
typedef struct substring{
	const char* start; /*< where start substring*/
	const char* end;   /*< where end substring*/
}substring_s;

/*- substring lenght
 * @< 'lenght'
 * @>0 'addr of substring' 
 * @{ 
 * substring_s hello = { .start=str, .end = str+strlen(str) };
 * printf("len: %lu\n", substring_len(&hello));
 * @}
 * */
#define substring_len(SUB) ((SUB)->end - (SUB)->start)

/*- ctype*/
typedef enum{
	C_NULL,  /*< no type*/
	C_MACRO, /*< is macro*/
	C_TYPE,
	C_STRUCT,
	C_ENUM,  /* this is not descript of C_ENUM */
	C_FN     /*< is function*/
}ctype_e;

/*- find where start comment command
 * @< 'next code'
 * @>0 'out command'
 * @>1  'start code'*/ 
const char* cparse_comment_command(substring_s* out, const char* code);


#endif
