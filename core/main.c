#include <ccdoc.h>
#include <ef/optex.h>

/*-str 'CCDOC_DEF_DESTDIR'  './doc'*/
/*-str 'CCDOC_DEF_TEMPLATE' './template/template.html'*/

/*-file 'ccdoc'
 * this project was born because doxygen has never impressed me, I have always found it difficult for create html doc, man, readme and wiki.\n
 * ccdoc only works with C, if you don't use C then you either change language or change software.\n
 * \n
 * ccdoc is wip, really not use in production\n
 * \n
 * Released under GPL v3\n
 * \n
 * @^2 'TODO' write documentation for wrinting documentation
 * @^2 'How To'
 * @^3 'Build and Install'
 * @{
 * $ meson build
 * $ cd build
 * $ ninja
 * @}
 * @^3 'Test'
 * @{
 * $ mkdir ../doc
 * $ ./ccdoc -d -H -r -c ../template -t ../template/template.html -D ../doc ../test/ccdoc.h ../test/extra.h
 * @}
 * @^3 'Template' for change html output edit template/template.html file
 *
 * @^2 'Usage:'
 * @| d 'dump'    0 'dump doc'
 * | H 'html'     0 'build html doc'
 * | r 'readme'   0 'build README doc'
 * | D 'destdir'  1 'destdir, default "@*CCDOC_DEF_DESTDIR"'
 * | c 'css'      0 'copy all .css from dir'
 * | t 'template' 1 'template filename, default "@*CCDOC_DEF_TEMPLATE"'
 * | h 'help'     0 'show this'
 *
 * @^2 'News'
 * **0.0**  begin\n

*/

/*-visual index*/


typedef enum{
	A_DUMP,
	A_HTML,
	A_README,
	A_DESTDIR,
	A_CSS,
	A_TEMPLATE,
	A_HELP
}a_e;

__private argdef_s args[] = {
	{0, 'd', "dump",    ARGDEF_NOARG, NULL, "dump doc"},
	{0, 'H', "html",    ARGDEF_NOARG, NULL, "build html doc"},
	{0, 'r', "readme",  ARGDEF_NOARG, NULL, "build README doc"},
	{0, 'D', "destdir", ARGDEF_STR  , NULL, "destdir, default " CCDOC_DEF_DESTDIR},
	{0, 'c', "css",     ARGDEF_STR  , NULL, "copy all .css from dir"},
	{0, 't', "template",ARGDEF_STR  , NULL, "template filename, default " CCDOC_DEF_TEMPLATE},
	{0, 'h', "help",    ARGDEF_NOARG, NULL, "show this"},
	{0,  0 , NULL,      ARGDEF_NOARG, NULL, NULL},
};

int main(int argc, char** argv){
	char* ftemplate = CCDOC_DEF_TEMPLATE;
	char* destdir   = CCDOC_DEF_DESTDIR;
	__mem_free ccdoc_s* ccdoc = ccdoc_new();

	int last = opt_parse(args, argv, argc);
	if( last < 0 ){
		opt_error(argc, argv);
		return 1;
	}

	if( opt_enabled(args, A_HELP) ){
		opt_usage(args, argv[0]);
		return 0;
	}

	if( opt_enabled(args, A_DESTDIR) ){
		destdir = (char*)opt_arg_str(args, A_DESTDIR);
	}
	if( opt_enabled(args, A_TEMPLATE) ){
		ftemplate = (char*)opt_arg_str(args, A_TEMPLATE);
	}

	ftemplate = path_resolve(ftemplate);
	mem_link(ccdoc, ftemplate);
	destdir   = path_resolve(destdir);
	mem_link(ccdoc, destdir);

	for( int i = last; i < argc; ++i ){
		__mem_free char* path = path_resolve(argv[i]);
		if( !file_exists(path) ){
			die("file '%s' not exists", path);
		}
		ccdoc_load(ccdoc, path);
	}

	if( opt_enabled(args, A_DUMP)    ) ccdoc_dump(ccdoc);
	if( opt_enabled(args, A_HTML)    ) ccdoc_build_html(ccdoc, ftemplate, destdir);
	if( opt_enabled(args, A_CSS )    ) ccdoc_copy_css(destdir, opt_arg_str(args, A_CSS)); 
	if( opt_enabled(args, A_README ) ) ccdoc_build_readme(ccdoc, destdir); 

	return 0;
}







