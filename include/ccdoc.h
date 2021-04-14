#ifndef __CCDOC_H__
#define __CCDOC_H__

#include <ef/str.h>
#include <ef/dstr.h>
#include <ef/substr.h>
#include <ef/vector.h>
#include <ef/rbhash.h>
#include <ef/file.h>
#include <ef/fconfig.h>

/*-file 'file name' descript */
/*-title 'title' descript*/
/*-visual top/side/index*/
/*-alias 'name' 'alias'*/
/*-str 'name' 'value'*/
/*- doc
 * @| <shortch> 'long arg' <1/0 required arg> 'descript'
 * | ...
 * | ...
 * @< 'return'
 * @>1 'argument'
 * @*name replace with link to *name
 * @*'refname' same @*name 
 * @^2 'title' subtitle
 * @{ code start @} code end
 * @? 'name' 'link'
 * @b 'bold'
 * @i 'italic'
 * @s 'strike'
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
#define CCDOC_DC_CMDARG    '|'
#define CCDOC_DC_LINK      '?'
#define CCDOC_DC_BOLD      'b'
#define CCDOC_DC_ITALIC    'i'
#define CCDOC_DC_STRIKE    's'
#define CCDOC_DC_ESCAPE    '\\'

#define CCDOC_SECTION_HID    1
#define CCDOC_SUBSECTION_HID 3
#define CCDOC_FC_MAX_VAR_NAME 64

#define CCDOC_HTML_EXT ".html"

#define CCDOC_DEF_CONFIG "./cc.doc"
#define CCDOC_DEF_DESTDIR "./doc"
#define CCDOC_DEF_DESTDIR_HTML CCDOC_DEF_DESTDIR "/html"
#define CCDOC_DEF_DESTDIR_README CCDOC_DEF_DESTDIR "/md"
#define CCDOC_DEF_DESTDIR_WIKI CCDOC_DEF_DESTDIR "/wiki"
#define CCDOC_DEF_DESTDIR_MAN CCDOC_DEF_DESTDIR "/man"
#define CCDOC_DEF_TEMPLATE "./template"
#define CCDOC_DEF_TEMPLATE_HTML CCDOC_DEF_TEMPLATE "template.html"
#define CCODC_DEF_TEMPLATE_CSS  CCDOC_DEF_TEMPLATE
#define CCDOC_DEF_MESON "./meson.build"

#define CCDOC_CONF_DESTDIR_HTML   "destdir_html"
#define CCDOC_CONF_DESTDIR_README "destdir_readme"
#define CCDOC_CONF_DESTDIR_WIKI   "destdir_wiki"
#define CCDOC_CONF_DESTDIR_MAN    "destdir_man"
#define CCDOC_CONF_TEMPLATE_HTML  "template_html"
#define CCDOC_CONF_TEMPLATE_CSS   "template_css"
#define CCDOC_CONF_WIKI_SITE      "wiki_site"
#define CCDOC_CONF_MESON          "meson_path"
#define CCDOC_CONF_FILES          "src"
#define CCDOC_CONF_DUMP           "dump"
#define CCDOC_CONF_HTML           "html"
#define CCDOC_CONF_CSS            "css"
#define CCDOC_CONF_README         "readme"
#define CCDOC_CONF_WIKI           "wiki"
#define CCDOC_CONF_MAN            "man"

typedef enum{
	C_NULL,
	C_MACRO,
	C_TYPE,
	C_STRUCT,
	C_ENUM,
	C_FN,
	C_COUNT
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
	fconfig_t* fc;
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

ccdoc_s* ccdoc_new(const char* fpath);
void ccdoc_load(ccdoc_s* ccdoc, const char* file);
void ccdoc_copy_css(const char* destdir, const char* srcdir);
void ccdoc_parse_ret(const char** parse, substr_s* desc);
void ccdoc_parse_arg(const char** pparse, int* argid, substr_s* desc);
void ccdoc_parse_title(const char** pparse, int* id, substr_s* title);
int ccdoc_parse_cmdarg(const char** parse, char* argsh, substr_s* argln, int* argrq, substr_s* desc);
void ccdoc_parse_ref(ccdoc_s* ccdoc, const char** pparse, char** dest,  void(*catref)(char** dst, ref_s* ref, void* ctx), void* ctx);
void ccdoc_cat_ref_resolver(char** dest, ccdoc_s* ccdoc, const char* str, size_t len, void(*catref)(char** dst,ref_s* ref, void* ctx), void* ctx);
void ccdoc_parse_link(const char** parse, substr_s* name, substr_s* link);
int ccdoc_parse_attribute(const char** parse, substr_s* txt);
void ccdoc_parse_skip_arg(const char** parse);
substr_s* ccdoc_parse_args(substr_s* desc, size_t count);
void ccdoc_dump(ccdoc_s* ccdoc);
int ccdoc_project_info(char** version, int* typeEL, ccdoc_s* ccdoc);

void ccdoc_build_html(ccdoc_s* ccdoc, const char* htmlTemplate, const char* destdir);

char* ccdoc_md_build(ccdoc_s* ccdoc, ccfile_s* ccf);

void ccdoc_build_readme(ccdoc_s* ccdoc, const char* destdir);

void ccdoc_build_wiki(ccdoc_s* ccdoc, const char* destdir);

void ccdoc_build_man(ccdoc_s* ccdoc, const char* destdir);


#endif
