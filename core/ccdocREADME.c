#include <ccdoc.h>

#define README_MD "README.md"

#define _UNDOCUMENTED "undocumented"

#define _RETURN "##Return:\n"

#define _CMDARG "Short|Long|Required|Descript\n"\
				"-----|----|--------|--------\n"

#define _CMDARG_RQ_Y "yes"
#define _CMDARG_RQ_N "no"

__private char* EXTPREVIEW[]={
	".jpg", ".jpeg",
	".gif", 
	".png",
	".bmp",
	"youtube",
	NULL
};

__private void md_nl(char** out){
	ds_cat(out, "<br />", strlen("<br />"));
}

__private void md_title(char** out, int tid, const char* title, size_t len){
	while( tid-->0 ){
		ds_push(out, '#');
	}
	ds_push(out, ' ');
	ds_cat(out, title, len);
	ds_push(out, '\n');
}

__private void md_code(char** out, const char* syntax){
	ds_cat(out, "```", strlen("```"));
	if( syntax ){
		ds_cat(out, syntax, str_len(syntax));
	}
	ds_push(out, '\n');
}

__private int link_preview(const char* link, size_t len){
	for( size_t i = 0; EXTPREVIEW[i]; ++i ){
		if( str_nfind(link, EXTPREVIEW[i], len) < link+len ) return 1;
	}
	return 0;
}

__private void md_link(char** out, const char* text, size_t lenText, const char* link, size_t lenLink){
	if( link_preview(link, lenLink) ){
		ds_push(out, '!');
	}
	ds_sprintf(out, ds_len(*out), "[%.*s](%.*s)", (int)lenText, text, (int)lenLink, link);
}

__private void md_attribute(char** out, int type, substr_s* txt){
	switch(type){
		case 0: ds_sprintf(out, ds_len(*out), "**%.*s**", substr_format(txt)); break;
		case 1: ds_sprintf(out, ds_len(*out), "_%.*s_", substr_format(txt)); break;
		case 2: ds_sprintf(out, ds_len(*out), "~~%.*s~~", substr_format(txt)); break;
		default: die("unknown format");
	}
}

__private void md_push_ref(char** page, ref_s* ref, __unused void* ctx){
	switch( ref->type ){
		default: case REF_REF: die("wrong ref"); break;
		case REF_FILE: break;
		case REF_DEF:  break;
		case REF_STR:
			ds_cat(page, ref->value.begin, substr_len(&ref->value));
		break;
	}
}

__private const char* parse_cmdarg(ccdoc_s* ccdoc, char** page, const char* parse){
	char argsh;
	substr_s argln;
	int argrq;
	substr_s argdesc;
	
	ds_cat(page, _CMDARG, str_len(_CMDARG));
	
	while( ccdoc_parse_cmdarg(&parse, &argsh, &argln, &argrq, &argdesc) ){
		ds_sprintf(page, ds_len(*page), "-%c|--%.*s|%s|",
			argsh,
			substr_format(&argln),
			argrq ? _CMDARG_RQ_Y : _CMDARG_RQ_N
		);
		ccdoc_cat_ref_resolver(page, ccdoc, argdesc.begin, substr_len(&argdesc), md_push_ref, NULL);
		ds_push(page, '\n');
	}

	ds_push(page, '\n');
	return parse;
}

__private void desc_parse(char** page, ccdoc_s* ccdoc, substr_s* desc){
	int incode = 0;
	const char* parse = desc->begin;
	while( parse < desc->end ){
		if( *parse == '\n' ){
			parse = ccparse_skip_hn(parse);
			ds_push(page, '\n');
			continue;
		}
		else if( !incode && parse[0] == '\\' && parse[1] == 'n' ){
			parse += 2;
			md_nl(page);
			continue;	
		}

		if( !incode && *parse == CCDOC_DESC_COMMAND ){
			++parse;
			switch( *parse ){
				case CCDOC_DC_RET:{
					dbg_info("DC_RET");
					++parse;
					substr_s retdesc;
					ccdoc_parse_ret(&parse, &retdesc);
					ds_cat(page, _RETURN, str_len(_RETURN));
					ds_cat(page, retdesc.begin, substr_len(&retdesc));
					ds_push(page, '\n');
					break;
				}

				case CCDOC_DC_ARG:	break;

				case CCDOC_DC_TITLE:{
					dbg_info("DC_TITLE");
					++parse;
					int titleid;
					substr_s t;
					ccdoc_parse_title(&parse, &titleid, &t);
					md_title(page, titleid, t.begin, substr_len(&t));
					break;
				}
			
				case CCDOC_DC_CODE_B:{
					dbg_info("DC_CODE_B");
					++parse;
					incode = 1;
					md_code(page, "C");
					break;
				}
				
				case CCDOC_DC_CMDARG:
					dbg_info("DC_CODE_COMMAND_ARG");
					parse = parse_cmdarg(ccdoc, page, parse);
				break;
				
				case CCDOC_DC_LINK:{
					dbg_info("DC_LINK");
					++parse;
					substr_s name, link;
					ccdoc_parse_link(&parse, &name, &link);
					md_link(page, name.begin, substr_len(&name), link.begin, substr_len(&link));
					break;
				}
				
				case CCDOC_DC_BOLD: case CCDOC_DC_ITALIC: case CCDOC_DC_STRIKE:{
					dbg_info("DC_ATTRIBUTE");
					substr_s txt;
					int att = ccdoc_parse_attribute(&parse, &txt);
					md_attribute(page, att, &txt);
					break;
				}

				case CCDOC_DC_REF:{
					dbg_info("DC_REF");
					++parse;
					ccdoc_parse_ref(ccdoc, &parse, page, md_push_ref, NULL);
					break;
				}

				default: die("unknown command desc @%c", *parse); break;
			}
		}
		else if( incode && *parse == CCDOC_DESC_COMMAND && *(parse+1) == CCDOC_DC_CODE_E ){
			dbg_info("DC_CODE_E");
			incode = 0;
			parse += 2;
			if( (*page)[ds_len(*page)-1] != '\n' ) ds_push(page, '\n');
			md_code(page, NULL);
		}
		else if( !incode && *parse == CCDOC_DC_ESCAPE ){
			dbg_info("DC_ESCAPE");
			++parse;
			if( *parse == 'n' ){
				md_nl(page);
			}
			else if( *parse == '\\' ) {
				ds_push(page, *parse++);
				ds_push(page, *parse++);
			}
			else{
				ds_push(page, *parse++);
			}
		}
		else{
			dbg_info("CHAR:%c", *parse);
			ds_push(page, *parse++);
		}
	}
}

char* ccdoc_md_build(ccdoc_s* ccdoc, ccfile_s* ccf){
	char* page = ds_new(CCDOC_STRING_SIZE);
	md_title(&page, 1, ccf->name.begin, substr_len(&ccf->name));
	desc_parse(&page, ccdoc, &ccf->desc);
	return page;
}

void ccdoc_build_readme(ccdoc_s* ccdoc, const char* destdir){
	vector_foreach(ccdoc->vfiles, i){
		if( ccdoc->vfiles[i].visual != VISUAL_INDEX ) continue;
		__mem_free char* page = ccdoc_md_build(ccdoc, &ccdoc->vfiles[i]);
		dbg_info("save");
		__mem_free char* fdest = ds_printf("%s/%s",destdir, README_MD);
		__fd_close int fd = fd_open(fdest, "w", CCDOC_FILE_PRIV);
		if( fd < 0 ) die("error on open file %s :: %s", fdest, str_errno());
		if( fd_write(fd, page, ds_len(page)) != (ssize_t)ds_len(page) ) die("wrong write page on file %s :: %s", fdest, str_errno());
		break;
	}
}


