#include <ccdoc.h>
#include <ef/optex.h>

typedef enum{
	A_DUMP,
	A_HTML,
	A_DESTDIR,
	A_CSS,
	A_TEMPLATE,
	A_HELP
}a_e;

__private argdef_s args[] = {
	{0, 'd', "dump",    ARGDEF_NOARG, NULL, "dump doc"},
	{0, 'H', "html",    ARGDEF_NOARG, NULL, "build html doc"},
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

	if( opt_enabled(args, A_DUMP) ) ccdoc_dump(ccdoc);
	if( opt_enabled(args, A_HTML) ) ccdoc_build_html(ccdoc, ftemplate, destdir);
	if( opt_enabled(args, A_CSS ) ) ccdoc_copy_css(destdir, opt_arg_str(args, A_CSS)); 
	return 0;
}







