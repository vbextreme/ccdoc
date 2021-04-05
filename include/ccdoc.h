#ifndef __CCDOC_H__
#define __CCDOC_H__

#include <ef/str.h>
#include <ef/dstr.h>
#include <ef/substr.h>
#include <ef/vector.h>
#include <ef/rbhash.h>
#include <ef/file.h>

/*-title 'title' descript*/
/*-visual top/side/index*/
/*-alias 'name' 'alias'*/
/*-str 'name' 'value'*/
/*- doc
 * @< 'return'
 * @>1 'argument'
 * @*name replace with link to *name
 * @*'refname' same @*name 
 * @^2 'title' subtitle
 * @{ code start @} code end
 * */

#define CCDOC_VFILES 4
#define CCDOC_VCODE  4
#define CCDOC_VDEF   4
#define CCDOC_HASH_BEGIN 32
#define CCDOC_HASH_MIN   10
#define CCDOC_HASH_HASH  hash_fasthash
#define CCDOC_REF_KEYSIZE 256
#define CCDOC_CHUNK 4096
#define CCDOC_STRING_SIZE 32
#define CCDOC_FILE_PRIV 0666
#define CCDOC_HTML_EXT  ".html"
#define CCDOC_COMMAND  "-"

#define CCDOC_DESC_COMMAND '@'
#define CCDOC_DESC_OBJ     '<'
#define CCDOC_DC_RET       '<'
#define CCDOC_DC_ARG       '>'
#define CCDOC_DC_REF       '*'
#define CCDOC_DC_TITLE     '^'
#define CCDOC_DC_CODE_B    '{'
#define CCDOC_DC_CODE_E    '}'
#define CCDOC_DC_ESCAPE    '\\'

#define CCDOC_SECTION_HID    1
#define CCDOC_SUBSECTION_HID 3

#define CCDOC_HTML_EXT ".html"
#define CCDOC_DEF_DESTDIR "./doc"
#define CCDOC_DEF_TEMPLATE "./template/template.html"

typedef enum{
	C_NULL,
	C_MACRO,
	C_TYPE,
	C_STRUCT,
	C_ENUM,
	C_FN
}ctype_e;

typedef struct celement{
	substr_s type;
	substr_s name;
	substr_s desc;
}celement_s;

typedef struct cdef{
	ctype_e     type;     //type cdef
	substr_s typedec;   //type declared    
	substr_s code;     //code on file
	substr_s name;     //name of code
	celement_s  ret;      //if cdef return
	celement_s* velement; //if cdef have arguments
}cdef_s;

typedef struct ccfile ccfile_s;

typedef struct ccdef{
	ccfile_s* parent;
	substr_s comment;
	cdef_s def;
}ccdef_s;

typedef enum{VISUAL_NONE, VISUAL_TOP, VISUAL_SIDE, VISUAL_INDEX, VISUAL_COUNT}visual_e;

typedef struct ccfile{
	substr_s name;
	substr_s desc;
	visual_e    visual;
	ccdef_s*    vdefs;
}ccfile_s;

typedef enum{ REF_REF, REF_STR, REF_FILE, REF_DEF } reftype_e;

typedef struct ref{
	reftype_e type;
	substr_s name;
	substr_s value;
	void* data;
}ref_s;

typedef struct ccdoc{
	ccfile_s* vfiles;
	ccfile_s* fsel;
	const char** vcode;
	rbhash_t* refs;
}ccdoc_s;

typedef enum{
	CC_NULL,
	CC_REF,
	CC_STR,
	CC_FILE,
	CC_SEL,
	CC_VISUAL,
	CC_DEF,
	CC_COUNT
}cmdtype_e;

const char* cparse_comment_command(substr_s* out, const char* code);
const char* cparse_definition_get(cdef_s* def, const char* code);

const char* ccparse_skip_hn(const char* code);
const char* ccparse_string(substr_s* out, const char* comment);
char* ccparse_remove_comment_command(substr_s* scode);
const char* ccparse_type(cmdtype_e* out, const char* commentStart);
void ccparse_ref(ref_s* ref, substr_s* comment);
void ccparse_str(ref_s* ref, substr_s* comment);
void ccparse_file(ccfile_s* file, substr_s* comment);
void ccparse_sel(substr_s* out, substr_s* comment);
void ccparse_visual(visual_e* out, substr_s* comment);
const char* ccparse_def(ccdef_s* def, substr_s* comment);

ccdoc_s* ccdoc_new(void);
void ccdoc_load(ccdoc_s* ccdoc, const char* file);
void ccdoc_copy_css(const char* destdir, const char* srcdir);
void ccdoc_dump(ccdoc_s* ccdoc);

void ccdoc_build_html(ccdoc_s* ccdoc, const char* htmlTemplate, const char* destdir);

#endif
