#include <ccdoc.h>

#define X_TEMPLATE_TABLE() \
	XENTRY(_NAVBAR_NODE,   "<!--navbar.node"),\
	XENTRY(_NAVBAR_CURR,   "<!--navbar.current"),\
	XENTRY(_SIDEBAR_NODE,  "<!--sidebar.node"),\
	XENTRY(_SECTION,       "<!--content.section"),\
	XENTRY(_SECTION_MAIN,  "<!--content.section.main"),\
	XENTRY(_SECTION_EXTRA, "<!--content.section.extra"),\
	XENTRY(_TITLE_BEGIN,   "<!--content.title.begin"),\
	XENTRY(_TITLE_END,     "<!--content.title.end"),\
	XENTRY(_TEXT_BEGIN,    "<!--content.text.begin"),\
	XENTRY(_TEXT_END,      "<!--content.text.end"),\
	XENTRY(_ARGS_BEGIN,    "<!--content.args.begin"),\
	XENTRY(_ARGS_END,      "<!--content.args.end"),\
	XENTRY(_ARG,           "<!--content.args.content"),\
	XENTRY(_CODE_BEGIN,    "<!--content.code.begin"),\
	XENTRY(_CODE_END,      "<!--content.code.end"),\
	XENTRY(_LINK,          "<!--content.link"),\
	XENTRY(_RETURN,        "<!--content.return"),\
	XENTRY(_MACRO_T,       "<!--section.macro.title"),\
	XENTRY(_MACRO_D,       "<!--section.macro.desc"),\
	XENTRY(_ENUM_T,        "<!--section.enum.title"),\
	XENTRY(_ENUM_D,        "<!--section.enum.desc"),\
	XENTRY(_TYPE_T,        "<!--section.type.title"),\
	XENTRY(_TYPE_D,        "<!--section.type.desc"),\
	XENTRY(_STRUCT_T,      "<!--section.struct.title"),\
	XENTRY(_STRUCT_D,      "<!--section.struct.desc"),\
	XENTRY(_FN_T,          "<!--section.fn.title"),\
	XENTRY(_FN_D,          "<!--section.fn.desc"),\
	XENTRY(_COUNT,         NULL)

#define _UNDUCUMENTED "unducumented"

#define S_LINK     "&&LINK&&"
#define S_NAME     "&&NAME&&"
#define S_TYPE     "&&TYPE&&"
#define S_DESC     "&&DESC&&"
#define S_CONTENT  "&&CONTENT&&"
#define S_TITLEID  "&&TITLEID&&"
#define S_NAVBAR   "<!-- visual top -->"
#define S_SIDEBAR  "<!-- visual side -->"
#define S_PCONTENT "<!-- content -->"
#define S_END     "-->"

#define XENTRY(NAME,STR) I ## NAME
typedef enum{
	X_TEMPLATE_TABLE()
}template_e;
#undef XENTRY

#define XENTRY(NAME,STR) [I ## NAME] = STR
__private const char* NAMETEMPLATESTR[] = {
	X_TEMPLATE_TABLE()
};
#undef XENTRY

typedef struct htmlEscape{
	const char* from;
	const char* to;
}htmlEscape_s;
__private htmlEscape_s HTMLESCAPE[] = {
	{.from = "&", .to = "&amp;"},
	{.from = "<", .to = "&lt;"},
	{.from = ">", .to = "&gt;"},
	{.from = "\n", .to = "</br>"},
	{.from = NULL, .to = NULL}
};

typedef struct ccdocHTML{
	char* template;
	substr_s tdef[I_COUNT];
}ccdocHTML_s;

__private void html_template_elelemt(substr_s* out, const char* template, const char* where, const char* ends){
	out->begin = str_find(template, where);
	if( *out->begin ){
		out->begin = str_skip_hn(out->begin + str_len(where));
		out->end   = str_find(out->begin, ends);
	}
	else{
		out->end = out->begin;
	}
}

__private void html_load_template(ccdoc_s* ccdoc, ccdocHTML_s* html, const char* htmlTemplate){
	__fd_close int fd = fd_open(htmlTemplate, "r", 0);
	if( fd == -1 ) die("file '%s':%s", htmlTemplate, str_errno());
	html->template = fd_slurp(NULL, fd, CCDOC_CHUNK, 1);
	if( !html->template ) die("error on read file '%s'", htmlTemplate);
	mem_link(ccdoc, html->template);

	for( size_t i = 0; NAMETEMPLATESTR[i]; ++i ){
		html_template_elelemt(&html->tdef[i], html->template, NAMETEMPLATESTR[i], S_END);
	}
}

__private char* bar_element(substr_s* template, substr_s* name, substr_s* link){
	char* ele = ds_dup(template->begin, substr_len(template));
	__mem_free char* lk = ds_dup(link->begin, substr_len(link));
	ds_cat(&lk, CCDOC_HTML_EXT, str_len(CCDOC_HTML_EXT));
	ds_replace(&ele, S_LINK, lk, ds_len(lk));
	ds_replace(&ele, S_NAME, name->begin, substr_len(name));
	return ele;
}

__private char* html_navbar_elements(ccdoc_s* ccdoc, ccdocHTML_s* html, ccfile_s* current){
	char* nav = ds_new(CCDOC_STRING_SIZE);

	vector_foreach(ccdoc->vfiles, i){
		if( ccdoc->vfiles[i].visual == VISUAL_INDEX ){
			substr_s* node = &ccdoc->vfiles[i] == current ? &html->tdef[I_NAVBAR_CURR] : &html->tdef[I_NAVBAR_NODE];
			__mem_free char* element = bar_element(node, &ccdoc->vfiles[i].name, &ccdoc->vfiles[i].name);
			ds_cat(&nav, element, ds_len(element));
			break;
		}
	}

	vector_foreach(ccdoc->vfiles, i){
		if( ccdoc->vfiles[i].visual != VISUAL_TOP ) continue;
		substr_s* node = &ccdoc->vfiles[i] == current ? &html->tdef[I_NAVBAR_CURR] : &html->tdef[I_NAVBAR_NODE];
		__mem_free char* element = bar_element(node, &ccdoc->vfiles[i].name, &ccdoc->vfiles[i].name);
		ds_cat(&nav, element, ds_len(element));
		break;
	}

	return nav;
}

__private char* html_sidebar_elements(ccdoc_s* ccdoc, ccdocHTML_s* html){
	char* side = ds_new(CCDOC_STRING_SIZE);

	vector_foreach(ccdoc->vfiles, i){
		if( ccdoc->vfiles[i].visual != VISUAL_SIDE ) continue;
		__mem_free char* element = bar_element(&html->tdef[I_SIDEBAR_NODE], &ccdoc->vfiles[i].name, &ccdoc->vfiles[i].name);
		ds_cat(&side, element, ds_len(element));
		break;
	}

	return side;
}

__private char* title_element(ccdocHTML_s* html, int titleid, substr_s* name){
	char* title = ds_dup(html->tdef[I_TITLE_BEGIN].begin, substr_len(&html->tdef[I_TITLE_BEGIN]));
	char id[32];
	size_t lid = sprintf(id, "%d", titleid);
	ds_cat(&title, name->begin, substr_len(name));
	ds_cat(&title, html->tdef[I_TITLE_END].begin, substr_len(&html->tdef[I_TITLE_END]));
	ds_replace(&title, S_TITLEID, id, lid);
	return title;
}

__private char* link_element(ccdocHTML_s* html, substr_s* href, substr_s* name){
	char* link = ds_dup(html->tdef[I_LINK].begin, substr_len(&html->tdef[I_LINK]));
	__mem_free char* lk = ds_dup(href->begin, substr_len(href));
	ds_cat(&lk, CCDOC_HTML_EXT, str_len(CCDOC_HTML_EXT));
	ds_replace(&link, S_LINK, lk, ds_len(lk));
	ds_replace(&link, S_NAME, name->begin, substr_len(name));
	return link;
}

__private char* desc_parse(ccdoc_s* ccdoc, ccdocHTML_s* html, int tid, const char* title, size_t lenT, substr_s* rawdesc, celement_s* ret, celement_s* vargs){
	char* desc = ds_new(CCDOC_STRING_SIZE);
	if( title ){
		substr_s sh1 = { .begin = title, .end = title+lenT };
		__mem_free char* h1 = title_element(html, tid, &sh1);
		ds_cat(&desc, h1, ds_len(h1));
	}
	ds_cat(&desc, html->tdef[I_TEXT_BEGIN].begin, substr_len(&html->tdef[I_TEXT_BEGIN]));


	__mem_free char** vargd = vector_new(char*, vargs && vector_count(vargs)?vector_count(vargs):1);

	if( vargs ){
		vector_foreach(vargs, i){
			char* vd = vargs[i].desc.begin ? ds_dup(vargs[i].desc.begin, substr_len(&vargs[i].desc)) : ds_new(CCDOC_STRING_SIZE);
			dbg_error("INITARGS[%lu]:%s", i, vd);
			mem_link(vargd, vd);
			vector_push(vargd, vd);
		}
	}
	__mem_free char* retd = ret && ret->desc.begin ? ds_dup(ret->desc.begin, substr_len(&ret->desc)) : ds_new(CCDOC_STRING_SIZE);
	
	int incode = 0;
	const char* parse = rawdesc->begin;
	while( parse < rawdesc->end ){
		if( *parse == '\n' ){
			parse = ccparse_skip_hn(parse);
			if( incode ){
				ds_cat(&desc, HTMLESCAPE[3].to, strlen(HTMLESCAPE[3].to));
			}
			else{
				ds_push(&desc, '\n');
			}
			continue;
		}
		else if( !incode && parse[0] == '\\' && parse[1] == 'n' ){
			ds_cat(&desc, HTMLESCAPE[3].to, strlen(HTMLESCAPE[3].to));
			parse += 2;
			continue;	
		}

		if( !incode && *parse == CCDOC_DESC_COMMAND ){
			++parse;
			switch( *parse ){
				case CCDOC_DC_RET:{
					dbg_info("DC_RET");
					substr_s retdesc;
					parse = str_skip_hn(parse+1);
					parse = ccparse_string(&retdesc, parse);
					if( !*parse || parse > rawdesc->end ) die("wrong return command desc"); 
					ds_cat(&retd, retdesc.begin, substr_len(&retdesc));
					parse = ccparse_skip_hn(parse);
					break;
				}

				case CCDOC_DC_ARG:{
					dbg_info("DC_ARG");
					substr_s argdesc;
					long argid = strtol(parse+1, (char**)&parse, 10);
					if( !parse || parse > rawdesc->end ) die("wrong arg command desc");
					parse = ccparse_string(&argdesc, str_skip_hn(parse));
					if( !vargs ) die("cparse fail reading args");
					if( argid >= (ssize_t)vector_count(vargd) ) die("argument not exists");
					ds_cat(&vargd[argid], argdesc.begin, substr_len(&argdesc));
					parse = ccparse_skip_hn(parse);
					//dbg_error("DC_ARG %ld \"%.*s\" <%s>", argid, substr_format(&argdesc), vargd[argid]);
					break;
				}

				case CCDOC_DC_TITLE:{
					dbg_info("DC_TITLE");
					long titleid = strtol(parse+1, (char**)&parse, 10);
					substr_s t;
					if( !parse || parse > rawdesc->end ) die("wrong arg command desc");
					parse = ccparse_string(&t, str_skip_hn(parse));
					__mem_free char* titlee = title_element(html, titleid, &t);
					ds_cat(&desc, html->tdef[I_TEXT_END].begin, substr_len(&html->tdef[I_TEXT_END]));
					ds_cat(&desc, titlee, ds_len(titlee));
					ds_cat(&desc, html->tdef[I_TEXT_BEGIN].begin, substr_len(&html->tdef[I_TEXT_BEGIN]));
					parse = ccparse_skip_hn(parse);
					break;
				}
			
				case CCDOC_DC_CODE_B:{
					dbg_info("DC_CODE_B");
					++parse;
					incode = 1;
					ds_cat(&desc, html->tdef[I_CODE_BEGIN].begin, substr_len(&html->tdef[I_CODE_BEGIN]));
					break;
				}
			
				case CCDOC_DC_REF:{
					dbg_info("DC_REF");
					++parse;
					substr_s refname;
					if( *parse == '\'' ){
						parse = ccparse_string(&refname, parse);
					}
					else{
						refname.begin = parse;
						parse = str_anyof(parse, " \t\n");
						if( !*parse || parse > rawdesc->end ) die("wrong ref command");
						refname.end = parse;
					}
					parse = ccparse_skip_hn(parse);
					ref_s* ref = rbhash_find(ccdoc->refs, refname.begin, substr_len(&refname));
					if( !ref ) die("ref '%.*s' not exists", substr_format(&refname));
					while( ref->type == REF_REF ){
						ref = rbhash_find(ccdoc->refs, ref->value.begin, substr_len(&ref->value));
						if( !ref ) die("ref %.*s not exists", substr_format(&refname));
					}
					switch( ref->type ){
						case REF_FILE:{
							ccfile_s* f = ref->data;
							__mem_free char* lk = link_element(html, &f->name, &f->name);
							ds_cat(&desc, lk, ds_len(lk));
							break;
						}
						case REF_DEF:{
							ccfile_s* f = ((ccdef_s*)ref->data)->parent;
							__mem_free char* lk = link_element(html, &f->name, &f->name);
							ds_cat(&desc, lk, ds_len(lk));
							break;
						}

						case REF_STR:{	
							ds_cat(&desc, ref->value.begin, substr_len(&ref->value));
							break;
						}

						default: case REF_REF: die("wrong ref"); break;
					}
					break;
				}

				default: die("unknown command desc @%c", *parse); break;
			}
		}
		else if( incode && *parse == CCDOC_DESC_COMMAND && *(parse+1) == CCDOC_DC_CODE_E ){
			dbg_info("DC_CODE_E");
			incode = 0;
			parse += 2;
			ds_cat(&desc, html->tdef[I_CODE_END].begin, substr_len(&html->tdef[I_CODE_END]));
		}
		else if( !incode && *parse == CCDOC_DC_ESCAPE ){
			dbg_info("DC_ESCAPE");
			ds_push(&desc, *parse++);
			ds_push(&desc, *parse++);
		}
		else if ( !incode ){
			dbg_info("CHAR:%c", *parse);
			ds_push(&desc, *parse++);
		}
		else{
			size_t i = 0;
			for(; HTMLESCAPE[i].from; ++i ){
				if( !strncmp(parse, HTMLESCAPE[i].from, str_len(HTMLESCAPE[i].from)) ){
					ds_cat(&desc, HTMLESCAPE[i].to, str_len(HTMLESCAPE[i].to));
					parse += str_len(HTMLESCAPE[i].from);
					break;
				}
			}
			if( !HTMLESCAPE[i].from ) ds_push(&desc, *parse++);
		}
	}

	ds_cat(&desc, html->tdef[I_TEXT_END].begin, substr_len(&html->tdef[I_TEXT_END]));

	if( vargs && vector_count(vargs) ){
		ds_cat(&desc, html->tdef[I_ARGS_BEGIN].begin, substr_len(&html->tdef[I_ARGS_BEGIN]));
		vector_foreach(vargs, i){
			__mem_free char* a = ds_dup(html->tdef[I_ARG].begin, substr_len(&html->tdef[I_ARG]));
			ds_replace(&a, S_TYPE, vargs[i].type.begin, substr_len(&vargs[i].type));
			ds_replace(&a, S_NAME, vargs[i].name.begin, substr_len(&vargs[i].name));
			if( *vargd[i] ){
				dbg_info("TYPE::%.*s NAME::%.*s DESC::%s", substr_format(&vargs[i].type), substr_format(&vargs[i].name), vargd[i]);
				ds_replace(&a, S_DESC, vargd[i], ds_len(vargd[i]));
			}
			else{
				ds_replace(&a, S_DESC, _UNDUCUMENTED, str_len(_UNDUCUMENTED));
			}
			dbg_info("ARG::%s", a);
			ds_cat(&desc, a, ds_len(a));
		}
		ds_cat(&desc, html->tdef[I_ARGS_END].begin, substr_len(&html->tdef[I_ARGS_END]));
	}

	if( *retd ){
		__mem_free char* r = ds_dup(html->tdef[I_RETURN].begin, substr_len(&html->tdef[I_RETURN]));
		if( ret && ret->type.begin ){
			ds_replace(&r, S_TYPE, ret->type.begin, substr_len(&ret->type));
		}
		else{
			ds_replace(&r, S_TYPE, NULL, 0);
		}	
		ds_replace(&r, S_DESC, retd, ds_len(retd));
		ds_cat(&desc, html->tdef[I_ARGS_BEGIN].begin, substr_len(&html->tdef[I_ARGS_BEGIN]));
		ds_cat(&desc, r, ds_len(r));
		ds_cat(&desc, html->tdef[I_ARGS_END].begin, substr_len(&html->tdef[I_ARGS_END]));
	}

	dbg_error("CONTENT::%s", desc);


	return desc;
}

__private char* html_code(ccdocHTML_s* html, substr_s* code){
	char* c = ds_dup(html->tdef[I_CODE_BEGIN].begin, substr_len(&html->tdef[I_CODE_BEGIN]));
	__mem_free char* ce = ccparse_remove_comment_command(code);
	dbg_info("CE:%s", ce);
	for( size_t i = 0; HTMLESCAPE[i].from; ++i ){
		ds_replace(&ce, HTMLESCAPE[i].from, HTMLESCAPE[i].to, str_len(HTMLESCAPE[i].to));
	}
	ds_cat(&c, ce, ds_len(ce));
	ds_cat(&c, html->tdef[I_CODE_END].begin, substr_len(&html->tdef[I_CODE_END]));
	return c;
}

__private char* html_section_h(ccdoc_s* ccdoc, ccdocHTML_s* html, ccfile_s* file){
	char* sec = ds_dup(html->tdef[I_SECTION].begin, substr_len(&html->tdef[I_SECTION]));
	__mem_free char* sm = ds_dup(html->tdef[I_SECTION_MAIN].begin, substr_len(&html->tdef[I_SECTION_MAIN]));
	__mem_free char* desc  = desc_parse(ccdoc, html, CCDOC_SECTION_HID, file->name.begin, substr_len(&file->name), &file->desc, NULL, NULL);
	ds_replace(&sm, S_CONTENT, desc, ds_len(desc));
	ds_replace(&sec, S_CONTENT, sm, ds_len(sm));
	return sec;
}

__private char* html_section_c(ccdoc_s* ccdoc, ccdocHTML_s* html, ccfile_s* file, template_e stitle, template_e sdesc, ctype_e type ){
	char* sec = ds_dup(html->tdef[I_SECTION].begin, substr_len(&html->tdef[I_SECTION]));
	__mem_free char* sm = ds_dup(html->tdef[I_SECTION_MAIN].begin, substr_len(&html->tdef[I_SECTION_MAIN]));
	__mem_free char* desc  = desc_parse(ccdoc, html, CCDOC_SECTION_HID, html->tdef[stitle].begin, substr_len(&html->tdef[stitle]), &html->tdef[sdesc], NULL, NULL);
	int count = 0;

	vector_foreach(file->vdefs, i){
		if( file->vdefs[i].def.type != type ) continue;
		++count;
		__mem_free char* d  = desc_parse(ccdoc, html, 
			CCDOC_SUBSECTION_HID, file->vdefs[i].def.name.begin, substr_len(&file->vdefs[i].def.name), 
			&file->vdefs[i].comment, 
			&file->vdefs[i].def.ret, file->vdefs[i].def.velement
		);
		ds_cat(&desc, d, ds_len(d));
		__mem_free char* code = html_code(html, &file->vdefs[i].def.code);
		ds_cat(&desc, code, ds_len(code));
	}
	if( !count ){
		ds_clear(sec);
	}
	else{
		ds_replace(&sm, S_CONTENT, desc, ds_len(desc));
		ds_replace(&sec, S_CONTENT, sm, ds_len(sm));
	}
	return sec;
}

void ccdoc_build_html(ccdoc_s* ccdoc, const char* htmlTemplate, const char* destdir){
	ccdocHTML_s html;
	memset(&html, 0, sizeof(ccdocHTML_s));
	dbg_info("load template");
	html_load_template(ccdoc, &html, htmlTemplate);
	
	dbg_info("load sidebar");
	__mem_free char* side = html_sidebar_elements(ccdoc, &html);

	vector_foreach(ccdoc->vfiles, i){
		__mem_free char* page = ds_dup(html.template, 0);
		__mem_free char* content = ds_new(CCDOC_STRING_SIZE);

		{
			dbg_info("load navnbar");
			__mem_free char* elm = html_navbar_elements(ccdoc, &html, &ccdoc->vfiles[i]);
			ds_replace(&page, S_NAVBAR, elm, ds_len(elm));
		}

		dbg_info("set sedebar");
		ds_replace(&page, S_SIDEBAR, side, ds_len(side));
		
		{
			dbg_info("create section h");
			__mem_free char* elm = html_section_h(ccdoc, &html, &ccdoc->vfiles[i]);
			ds_cpy(&content, elm, ds_len(elm));
		}

		{
			dbg_info("create section macro");
			__mem_free char* elm = html_section_c(ccdoc, &html, &ccdoc->vfiles[i], I_MACRO_T, I_MACRO_D, C_MACRO);
			ds_cat(&content, elm, ds_len(elm));
		}

		{
			dbg_info("create section enum");
			__mem_free char* elm = html_section_c(ccdoc, &html, &ccdoc->vfiles[i], I_ENUM_T, I_ENUM_D, C_ENUM);
			ds_cat(&content, elm, ds_len(elm));
		}

		{
			dbg_info("create section type");
			__mem_free char* elm = html_section_c(ccdoc, &html, &ccdoc->vfiles[i], I_TYPE_T, I_TYPE_D, C_TYPE);
			ds_cat(&content, elm, ds_len(elm));
		}

		{
			dbg_info("create section struct");
			__mem_free char* elm = html_section_c(ccdoc, &html, &ccdoc->vfiles[i], I_STRUCT_T, I_STRUCT_D, C_STRUCT);
			ds_cat(&content, elm, ds_len(elm));
		}

		{
			dbg_info("create section fn");
			__mem_free char* elm = html_section_c(ccdoc, &html, &ccdoc->vfiles[i], I_FN_T, I_FN_D, C_FN);
			ds_cat(&content, elm, ds_len(elm));
		}

		ds_replace(&page, S_PCONTENT, content, ds_len(content));

		dbg_info("save");
		__mem_free char* fdest = ds_printf("%s/%.*s%s",destdir, substr_format(&ccdoc->vfiles[i].name), CCDOC_HTML_EXT);
		__fd_close int fd = fd_open(fdest, "w", CCDOC_FILE_PRIV);
		if( fd < 0 ) die("error on open file %s :: %s", fdest, str_errno());
		if( fd_write(fd, page, ds_len(page)) != (ssize_t)ds_len(page) ) die("wrong write page on file %s :: %s", fdest, str_errno());
	}
}

















