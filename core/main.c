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
 * can find format configuration on @*'configure format'\n
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
 * probably you want set destdir_readme to ./ and destsir_wiki to ./wiki.usernama for automatic overwrite README.md and Wiki pages.\n
 * man pages generate name in this mode:\n
 * for index page the name is same to filename, and other page are formatting to index_filename\n
 *
 * @^2 'C Comment'
 * ccdoc add some special command to write in C comment, in this mode you can control output, only comment start with @b'-' is readed\n
 * read @*'Comment Command' for more details
 *
 * @^2 'News'
 * @b'0.1'  ready, html,README,wiki,man\n
 * @b'0.0'  begin\n
 *
*/

/*-visual index*/

/*-file 'Comment Command'
 * ccdoc add some special command to write in C comment, in this mode you can control output.\n
 * only comment start with @C- is readed\n
 * 
 * @^2 'file'
 * when start write documentation you need to create a file, each file.c, file.h, file.* can contains more file command with different filename.\n
 * if you not write file command, other command are associated to last file selected.\n
 * command file need an argument, @i'file name', in quote, separated or no with space.\n
 * text after filename is a file description.\n
 * examples:
 * @{
 * @C-file 'Test' description
 * * descript ....
 * @c
 * @}
 * 
 * @^2 'sel'
 * if you want change current file associations you can use sel, for examples, you can create file on .h and other comment in other file.\n
 * examples:\n
 * @i'file.h'
 * @{
 * @C-file 'test' simple test @c
 * @}
 * @i'file.c'
 * @{ 
 * @C-sel 'test' @c 
 * @}
 * 
 * @^2 'visual'
 * each file need a visual, where display a page.\n
 * visual accept 3 type:\n
 * @b'index' the begin page, this file is used for create README and  used as prefix name of other man page\n
 * @b'side'  this page is display in sidebar on WIKI or HTML\n
 * @b'top'   this page is display in topbar on HTML and sidebar on WIKI\n
 * examples:\n
 * @{
 * @C-file 'test' simple test@c
 * @C-visual index@c
 * @}
 *
 * @^2 'alias'
 * create alias name for each reference, accept two arguments.\n
 * @i'name of reference', @i'name of alias'\n 
 * examples:\n
 * @{
 * @C-file 'test' simple test@c
 * @C-visual index@c
 * @C-alias 'test' 'simple test'@c
 * @}
 * now you can refer to test file with 'simple test'
 *
 * @^2 'str'
 * create a string, if you need to rewrite many time a string you can use this\n
 * examples:\n
 * @{
 * @C-file 'test' simple test@c
 * @C-visual index@c
 * @C-str 'hw' 'Hello, world!'@c
 * @}
 *
 * @^2 'Auto Doc'
 * the auto doc command is @C- without other command, in this mode ccdoc reading descript and parse C code for generate documentation.\n
 * this mode is usefull when write documentation for macro, structure, function, etc.\n
 * examples:\n
 * @{
 * @C- simple void function@c
 * void foo(void);
 * @}
 *
 * @^2 'Descript Command'
 * In addition on the basic command ccdoc export description command, description command is special command for writing best description.\n
 * all descript command start with \@, only new line you can write with \\n.
 *
 * @^3 'Command Arguments'
 * if you writing cli command probably you want generate usage documentation, is very simple with @i'command arguments'
 * the format begin with \@| and next usage can add with |\n
 * \@| @i'short' @i'long' @i'required' @i'descript'\n
 * | @i'short' @i'long' @i'required' @i'descript'\n
 * @b'short' is char for short format option\n
 * @b'long' is long format option\n
 * @b'required' 1,0 display yes/no on required arguments\n
 * @b'descript' a description of command.\n
 * examples:
 * @{
 * @C-file 'test'
 * * simple software
 * * @| h 'help' 0 'display this'
 * * | f 'file' 1 'select filename'
 * @c
 * @}
 *
 * @^3 'Return'
 * The return command, \@< , accept one quoted argument than descript return comment\n
 * examples:
 * @{
 * @C- examples
 * * @< '0 success, -1 on error'
 * @c
 * int test(void);
 * @}
 *
 * @^3 'Argument'
 * The argument command, \@> , descript function or macro argument.\n
 * Accept one number with no space refered to function argument and a description to argument quoted.\n
 * The first argument is 0, it's write undocumented if one argument is not descript\n
 * examples:
 * @{
 * @C- examples
 * * @>0 'first arg'
 * * @>1 'second arg'
 * @c
 * void foo(int bar, int cat);
 * @}
 *
 * @^3 'Reference'
 * Reference command, \@* , add link or expand string.\n
 * name of reference can be quoted, if have a space, or no.\n
 * examples:
 * @{
 * @C-file 'my files' reference examples@c
 * @C-visual index@c
 * @C-str 'hello' 'hello world'@c
 * @C- foo descript in @*'my files'
 * * @< @*hello
 * @c
 * char* foo(void);
 * @}
 *
 * @^3 'Title'
 * Command title, \@^ , add subtitle on current section.\n
 * accept numbers witout space indicating importance, 1 very important 1+N little important, the title name is quoted string.\n
 * examples:
 * @{
 * @C-file 'test'
 * descript a file
 * @^2 'subtitle'
 * this is subsection
 * @c
 * @}
 *
 * @^3 'Code'
 * Command code start with, \@{ , and end with, \@}, and mark comment as code.\n
 * in the code the comment is not touch and only escape available is Escape Comment descript in Escape section.\n
 * for now is not possible to write examples. code section cant escape command code, see main.c for code command examples
 *
 * @^3 'Link'
 * command link, \@? , creating manual link, accept quoted visible name and quoted link.\n
 * examples:
 * @{
 * @C-file 'test' create link @? 'localhost' 'http://127.0.0.1'@c
 * @}
 *
 * @^3 'Text Attribute'
 * in descript can add text attribute, text attribute is a command with quoted string separated or no with space.\n
 * bold \@b\n
 * italic \@i\n
 * strike \@s\n
 * examples:
 * @{
 * @C-file 'test' @b'bold' @i 'italic' @s'strike'@c
 * @}
 *
 * @^3 'Escape'
 * for writing some special characters need to escape this.\n
 * \\ is used to escape
 * \\n for writing new line
 * \\\@ for writing \@
 * \@C writing @C, only this and \@c works in code command, in this mode not get error on compiler.
 * \@c writing @c
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







