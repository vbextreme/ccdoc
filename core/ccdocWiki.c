#include <ccdoc.h>

#define _SIDEBAR "_Sidebar"

__private char* md_link_escape(const char* str, size_t len){
	char* ds = ds_dup(str, len);
	ds_replace(&ds, " ", "%20", 2);
	return ds;
}

__private void build_md_sidebar(ccdoc_s* ccdoc, const char* destdir){
	__mem_free char* side = ds_new(CCDOC_STRING_SIZE);
	fconfigVar_s* wikiSite = fconfig_get(ccdoc->fc, CCDOC_CONF_WIKI_SITE, str_len(CCDOC_CONF_WIKI_SITE));
	if( !wikiSite || wikiSite->type != FCVAR_STR ) die("wrong wiki config");
	vector_foreach(ccdoc->vfiles, i){
		__mem_free char* lk = md_link_escape(ccdoc->vfiles[i].name.begin, substr_len(&ccdoc->vfiles[i].name));
		ds_sprintf(&side, ds_len(side), "* [%.*s](%s/%s.md)\n", substr_format(&ccdoc->vfiles[i].name), wikiSite->fcstr, lk);
	}

	dbg_info("save");
	__mem_free char* fdest = ds_printf("%s/%s.md", destdir, _SIDEBAR);
	__fd_close int fd = fd_open(fdest, "w", CCDOC_FILE_PRIV);
	if( fd < 0 ) die("error on open file %s :: %s", fdest, str_errno());
	if( fd_write(fd, side, ds_len(side)) != (ssize_t)ds_len(side) ) die("wrong write sidebar on file %s :: %s", fdest, str_errno());
}

void ccdoc_build_wiki(ccdoc_s* ccdoc, const char* destdir){
	vector_foreach(ccdoc->vfiles, i){
		__mem_free char* page = ccdoc_md_build(ccdoc, &ccdoc->vfiles[i]);
		dbg_info("save");
		__mem_free char* fdest = ds_printf("%s/%.*s.md",destdir, substr_format(&ccdoc->vfiles[i].name));
		__fd_close int fd = fd_open(fdest, "w", CCDOC_FILE_PRIV);
		if( fd < 0 ) die("error on open file %s :: %s", fdest, str_errno());
		if( fd_write(fd, page, ds_len(page)) != (ssize_t)ds_len(page) ) die("wrong write page on file %s :: %s", fdest, str_errno());
	}
	build_md_sidebar(ccdoc, destdir);
}


