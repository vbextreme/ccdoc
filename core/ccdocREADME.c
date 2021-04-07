#include <ccdoc.h>

#define README_MD "README.md"

#define _UNDUCUMENTED "unducumented"

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

__private char* EXTVIDEO[]={
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

__private const char* parse_cmdarg(char** page, const char* parse){
	char argsh;
	substr_s argln;
	int argrq;
	substr_s argdesc;
	
	ds_cat(page, _CMDARG, str_len(_CMDARG));
	int cont;
	do{
		cont = ccdoc_parse_cmdarg(&parse, &argsh, &argln, &argrq, &argdesc);
		ds_sprintf(page, ds_len(*page), "-%c|--%.*s|%s|%.*s\n",
			argsh,
			substr_format(&argln),
			argrq ? _CMDARG_RQ_Y : _CMDARG_RQ_N,
			substr_format(&argdesc)
		);
	}while( cont );

	ds_push(page, '\n');
	return parse;
}

__private void desc_parse(char** page, ccdoc_s* ccdoc, substr_s* desc){
	int incode = 0;
	const char* parse = desc->begin;
	while( parse < desc->end ){
		if( *parse == '\n' ){
			++parse;
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
					parse = parse_cmdarg(page, parse);
				break;
				
				case CCDOC_DC_REF:{
					dbg_info("DC_REF");
					++parse;
					ref_s* ref = ccdoc_parse_ref(ccdoc, &parse);
					switch( ref->type ){
						case REF_FILE:{
							//ccfile_s* f = ref->data;
							break;
						}
						case REF_DEF:{
							//ccfile_s* f = ((ccdef_s*)ref->data)->parent;
							//__mem_free char* lk = link_element(html, &f->name, &f->name);
							//ds_cat(&desc, lk, ds_len(lk));
							break;
						}

						case REF_STR:{	
							ds_cat(page, ref->value.begin, substr_len(&ref->value));
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
			md_code(page, NULL);
		}
		else if( !incode && *parse == CCDOC_DC_ESCAPE ){
			dbg_info("DC_ESCAPE");
			++parse;
			if( *parse == 'n' ){
				md_nl(page);
			}
			else{
				ds_push(page, *parse++);
			}
		}
		else if ( !incode ){
			dbg_info("CHAR:%c", *parse);
			ds_push(page, *parse++);
		}
	}
}

void ccdoc_build_readme(ccdoc_s* ccdoc, const char* destdir){
	vector_foreach(ccdoc->vfiles, i){
		if( ccdoc->vfiles[i].visual != VISUAL_INDEX ) continue;
		__mem_free char* page = ds_new(CCDOC_STRING_SIZE);
		ccfile_s* index = &ccdoc->vfiles[i];
		md_title(&page, 1, index->name.begin, substr_len(&index->name));
		desc_parse(&page, ccdoc, &index->desc);
		dbg_info("save");
		__mem_free char* fdest = ds_printf("%s/%s",destdir, README_MD);
		__fd_close int fd = fd_open(fdest, "w", CCDOC_FILE_PRIV);
		if( fd < 0 ) die("error on open file %s :: %s", fdest, str_errno());
		if( fd_write(fd, page, ds_len(page)) != (ssize_t)ds_len(page) ) die("wrong write page on file %s :: %s", fdest, str_errno());
		break;
	}
}


