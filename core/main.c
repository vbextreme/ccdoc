#include <ccdoc.h>
#include <ef/optex.h>

/*-file 'ccdoc'
 * this project was born because doxygen has never impressed me, I have always found it difficult for create html doc, man, readme and wiki.\n
 * ccdoc only works with C, if you don't use C then you either change language or change software.\n
 * ccdoc can generate HTML, README.md, WIKI and man page from your C Comment
 * \n
 * Released under GPL v3\n
 * \n
 * @^2 'todo' write documentation for wrinting documentation
 * @^2 'How To'
 * @^3 'Build and Install'
 * @{
 * $ meson build
 * $ cd build
 * $ ninja
 * @}
 *
 * @^2 'Usage:'
 * ccdoc search a cc.doc file where reading project info\n
 * @| c 'config' 1 'change default config path, default is ./cc.doc'
 * | h 'help'     0 'show this'
 *
 * @^2 'configure'
 * you can change all layout of html page, *ccodcHTML for more info.\n
 * \n
 * can find format configuration on *'fconfigure\ format'\n
 * @{
 * destdir_html   = ./doc/html               // default value where stored .html
 * destdir_readme = ./doc/md                 // default value where stored README.md
 * destdir_wiki   = ./doc/wiki               // default value where stored wiki files
 * destdir_readme = ./doc/man                // default value where stored man files
 * meson_path     = ./meson.build            // if you use meson, ccdoc read this file for get version and type of software
 * template_html  = ./template/template.html // default value
 * template_css   = ./template               // default value where read all css files to copy in destdir_html
 * src            = []                       // src is not default defined, required a vector of path where reading files
 * dump                                      // if exists dump ccdoc
 * html                                      // if exists build html doc
 * css                                       // if exists copy css
 * readme                                    // if exists build readme
 * wiki                                      // if exists build wiki
 * man                                       // if exists build man
 * @}
 * // is a comment\n
 * src contains files where search C Comment, you can add in this mode:\n
 * @{
 * src = [ a.c, b.c]
 * @}
 * or
 * @{
 * src = []
 * src += [ a.c ]
 * src += [ b.c ]
 * @}
 * if string contains space you can quote or double quote.\n
 * probably you want set destdir_readme to ./ and destsir_wiki to ./wiki.<usernama> for automatic overwrite README.md and Wiki pages.\n
 * man pages generate name in this mode:\n
 * for index page the name is same to filename, and other page are formatting to <index>_<filename>\n
 *
 * @^2 'C Comment'
 * ccdoc add some special command to write in C comment, in this mode you can control output, only comment start with @b'-' is readed\n
 * read *'Comment Command' for more details
 *
 * @^2 'News'
 * @b'0.1'  ready, html,README,wiki,man\n
 * @b'0.0'  begin\n
 *
*/

/*-visual index*/

/*-file 'Comment Command'
 * ccdoc add some special command to write in C comment, in this mode you can control output.\n
 * only comment start with @b'-' is readed\n
 * @^2 'file'
 * when start write documentation you need to create a file, each file.c, file.h, file.* can contains more file command with different filename.\n
 * if you not write file command, other command are associated to last file selected.\n
 * command file need an argument, @i'file name', in quote, separated or no with space.\n
 * examples:
 * @{
 * /*-
*/

/*-visual side*/

typedef enum{
	A_CONFIG,
	A_HELP
}a_e;

__private argdef_s args[] = {
	{0, 'c', "config",  ARGDEF_STR  , NULL, "copy all .css from dir"},
	{0, 'h', "help",    ARGDEF_NOARG, NULL, "show this"},
	{0,  0 , NULL,      ARGDEF_NOARG, NULL, NULL},
};

void conf_set(fconfig_t* fc, const char* name, size_t len, const char* value, int path){
	dbg_info("%.*s ? %s :: %d", (int)len, name, value, path);
	fconfigVar_s* var = fconfig_get(fc, name, len );
	if( var ){
		if( var->type != FCVAR_STR ) die("wrong var %s type", name);
		if( path ){
			dbg_info("resolve(%p):%s", var->fcstr, var->fcstr);
			char* tmp = var->fcstr;
			var->fcstr = path_resolve(tmp);
			mem_link(var, var->fcstr);
			mem_free(tmp);
		}
	}
	else{
		var = NEW(fconfigVar_s);
		var->type = FCVAR_STR;
		var->fcstr = path ? path_resolve(value) : ds_dup(value, 0);
		mem_link(var, var->fcstr);
		fconfig_add(fc, name, len, var);
	}
}

int main(int argc, char** argv){
	
	if( opt_parse(args, argv, argc) < 0 ){
		opt_error(argc, argv);
		return 1;
	}

	if( opt_enabled(args, A_HELP) ){
		opt_usage(args, argv[0]);
		return 0;
	}

	__mem_free char* conf = opt_enabled(args, A_CONFIG) ?
		path_resolve(opt_arg_str(args, A_CONFIG)) :
		path_resolve(CCDOC_DEF_CONFIG)
	;

	__mem_free ccdoc_s* ccdoc = ccdoc_new(conf);
	
	conf_set(ccdoc->fc, CCDOC_CONF_DESTDIR_HTML,   str_len(CCDOC_CONF_DESTDIR_HTML),   CCDOC_DEF_DESTDIR_HTML, 1);
	conf_set(ccdoc->fc, CCDOC_CONF_DESTDIR_README, str_len(CCDOC_CONF_DESTDIR_README), CCDOC_DEF_DESTDIR_README, 1);
	conf_set(ccdoc->fc, CCDOC_CONF_DESTDIR_WIKI,   str_len(CCDOC_CONF_DESTDIR_WIKI),   CCDOC_DEF_DESTDIR_WIKI, 1);
	conf_set(ccdoc->fc, CCDOC_CONF_DESTDIR_MAN,    str_len(CCDOC_CONF_DESTDIR_MAN),    CCDOC_DEF_DESTDIR_MAN, 1);
	conf_set(ccdoc->fc, CCDOC_CONF_TEMPLATE_HTML,  str_len(CCDOC_CONF_TEMPLATE_HTML),  CCDOC_DEF_TEMPLATE_HTML, 1);
	conf_set(ccdoc->fc, CCDOC_CONF_TEMPLATE_CSS,   str_len(CCDOC_CONF_TEMPLATE_CSS),   CCODC_DEF_TEMPLATE_CSS, 1);
	conf_set(ccdoc->fc, CCDOC_CONF_MESON,          str_len(CCDOC_CONF_MESON),          CCDOC_DEF_MESON, 1);

	fconfigVar_s* varFiles = fconfig_get(ccdoc->fc, CCDOC_CONF_FILES, str_len(CCDOC_CONF_FILES));
	if( !varFiles || varFiles->type != FCVAR_VECTOR ) die("no vector src set in %s", conf);
	
	vector_foreach(varFiles->fcvector, i){
		if( varFiles->fcvector[i]->type != FCVAR_STR ) die("src[%lu] is not string type", i);
		__mem_free char* path = path_resolve(varFiles->fcvector[i]->fcstr);
		if( !file_exists(path) ) die("file '%s' not exists", path);
		ccdoc_load(ccdoc, path);
	}
	
	if( fconfig_get(ccdoc->fc, CCDOC_CONF_DUMP, str_len(CCDOC_CONF_DUMP)) ){
		ccdoc_dump(ccdoc);
	}

	if( fconfig_get(ccdoc->fc, CCDOC_CONF_HTML, str_len(CCDOC_CONF_HTML)) ){
		ccdoc_build_html(ccdoc,
			fconfig_get(ccdoc->fc, CCDOC_CONF_TEMPLATE_HTML, str_len(CCDOC_CONF_TEMPLATE_HTML))->fcstr,
			fconfig_get(ccdoc->fc, CCDOC_CONF_DESTDIR_HTML,  str_len(CCDOC_CONF_DESTDIR_HTML))->fcstr
		);
	}

	if( fconfig_get(ccdoc->fc, CCDOC_CONF_CSS, str_len(CCDOC_CONF_CSS)) ){
		ccdoc_copy_css(
			fconfig_get(ccdoc->fc, CCDOC_CONF_DESTDIR_HTML, str_len(CCDOC_CONF_DESTDIR_HTML))->fcstr,
			fconfig_get(ccdoc->fc, CCDOC_CONF_TEMPLATE_CSS, str_len(CCDOC_CONF_TEMPLATE_CSS))->fcstr
		);
	}

	if( fconfig_get(ccdoc->fc, CCDOC_CONF_README, str_len(CCDOC_CONF_README)) ){
		ccdoc_build_readme(ccdoc,
			fconfig_get(ccdoc->fc, CCDOC_CONF_DESTDIR_README, str_len(CCDOC_CONF_DESTDIR_README))->fcstr
		);
	}

	if( fconfig_get(ccdoc->fc, CCDOC_CONF_WIKI, str_len(CCDOC_CONF_WIKI)) ){
		ccdoc_build_wiki(ccdoc,
			fconfig_get(ccdoc->fc, CCDOC_CONF_DESTDIR_WIKI, str_len(CCDOC_CONF_DESTDIR_WIKI))->fcstr
		);
	}

	if( fconfig_get(ccdoc->fc, CCDOC_CONF_MAN, str_len(CCDOC_CONF_MAN)) ){
		ccdoc_build_man(ccdoc,
			fconfig_get(ccdoc->fc, CCDOC_CONF_DESTDIR_MAN, str_len(CCDOC_CONF_DESTDIR_MAN))->fcstr
		);
	}

	return 0;
}







