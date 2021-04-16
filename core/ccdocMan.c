#include <ccdoc.h>

#define _UNDOCUMENTED "undocumented"

#define _RETURN "Return:"

#define _TYPE "Type:"

#define _CMDARG_RQ_Y "yes"
#define _CMDARG_RQ_N "no"

#define T_MACRO       "Macro"
#define T_MACRO_DESC  "List of macro"
#define T_ENUM        "Enum"
#define T_ENUM_DESC   "List of enum"
#define T_TYPE        "Type"
#define T_TYPE_DESC   "List of type"
#define T_STRUCT      "Struct"
#define T_STRUCT_DESC "List of struct"
#define T_FN          "Api"
#define T_FN_DESC     "List of functions"

#define T_CODE "code:"

#define _NL ".br\n"
#define _TH ".TH "
#define _RE ".RE\n"
#define _TP ".TP\n"

__private void man_cmd_st(char** out){
	size_t len = ds_len(*out);
	if( len < 1 ) return;
	if( (*out)[len-1] != '\n' ) ds_push(out, '\n');
}

__private void man_nl(char** out){
	man_cmd_st(out);
	ds_cat(out, _NL, str_len(_NL));
}

__private void man_th(char** out, ccdoc_s* ccdoc, const char* title, size_t len, int page){
	__mem_free char* v = NULL;
	int p = 0;
	ds_cat(out, _TH, str_len(_TH));
	ds_cat(out, title, len);
	if( !ccdoc_project_info(&v, &p, ccdoc) ){
		dbg_info("project info version:%s type:%d", v, p);
		if( page ) ds_sprintf(out, ds_len(*out), " %d\n", page);
		else ds_push(out, '\n');
		return;
	}
	page = p == 1 ? 1 : 3;
	ds_sprintf(out, ds_len(*out), " %d \"%s\"\n", page, v ? v : "");
}

__private void man_sh(char** out, const char* title, size_t len){
	man_cmd_st(out);
	ds_sprintf(out, ds_len(*out), ".SH %.*s\n", (int)len, title);
}

__private void man_title(char** out, int titleid, const char* title, size_t len){
	static int intitle = 0;
	if( intitle ){
		man_cmd_st(out);
		ds_cat(out, _RE, str_len(_RE));
	}
	else{
		intitle = 1;
	}
	if( titleid > 0 ) --titleid;
	if( titleid ){
		man_cmd_st(out);
		ds_sprintf(out, ds_len(*out), ".PP\n.RS %d\n.B %.*s", titleid, (int)len, title);
		man_nl(out);
	}
	else{
		man_sh(out, title, len);
	}
}

__private void man_attribute(char** out, int type, substr_s* txt){
	man_cmd_st(out);
	switch(type){
		case 0: ds_sprintf(out, ds_len(*out), ".B %.*s\n", substr_format(txt)); break;
		case 1: ds_sprintf(out, ds_len(*out), ".I %.*s\n", substr_format(txt)); break;
		case 2: ds_sprintf(out, ds_len(*out), "%.*s", substr_format(txt)); break;
		default: die("unknown format");
	}
}

__private ccfile_s* get_index(ccdoc_s* ccdoc){
	vector_foreach(ccdoc->vfiles, i){
		if( ccdoc->vfiles[i].visual == VISUAL_INDEX ) return &ccdoc->vfiles[i];
	}
	die("no index");
}

__private void man_link(char** page, ccdoc_s* ccdoc,  substr_s* name){
	ccfile_s* index = get_index(ccdoc);
	man_cmd_st(page);
	if( !strncmp(index->name.begin, name->begin, substr_len(name)) ){
		ds_sprintf(page, ds_len(*page), ".B %.*s\n", substr_format(name));
	}
	else{
		ds_sprintf(page, ds_len(*page), ".B %.*s_%.*s\n", substr_format(&index->name), substr_format(name));
	}
}

__private void man_push_ref(char** page, ref_s* ref, void* ctx){
	switch( ref->type ){
		default: case REF_REF: die("wrong ref"); break;
		case REF_FILE:{
			ccfile_s* f = ref->data;
			man_link(page, ctx, &f->name);
			break;
		}
		
		case REF_DEF:  break;
		
		case REF_STR:
			ds_cat(page, ref->value.begin, substr_len(&ref->value));
		break;
	}
}

__private void man_argument(char** page, ccdoc_s* ccdoc, substr_s* type, substr_s* name, substr_s* desc, substr_s* adesc){
	man_cmd_st(page);
	ds_cat(page, _TP, str_len(_TP));
	if( type && type->begin ){
		man_attribute(page, 0, type);
	}
	if( name && name->begin ){
		man_attribute(page, 1, name);
	}
	if( desc && desc->begin ) ccdoc_cat_ref_resolver(page, ccdoc, desc->begin, substr_len(desc), man_push_ref, ccdoc);
	if( adesc && adesc->begin ) ccdoc_cat_ref_resolver(page, ccdoc, adesc->begin, substr_len(adesc), man_push_ref, ccdoc);
	ds_push(page, '\n');
}

__private const char* parse_cmdarg(ccdoc_s* ccdoc, char** page, const char* parse){
	char argsh;
	substr_s argln;
	int argrq;
	substr_s argdesc;
	
	man_cmd_st(page);
	while( ccdoc_parse_cmdarg(&parse, &argsh, &argln, &argrq, &argdesc) ){
		ds_sprintf(page, ds_len(*page), ".TP\n.B \\-%c \\-\\-%.*s\n.I %s\n",
			argsh,
			substr_format(&argln),
			argrq ? _CMDARG_RQ_Y : _CMDARG_RQ_N
		);
		ccdoc_cat_ref_resolver(page, ccdoc, argdesc.begin, substr_len(&argdesc), man_push_ref, ccdoc);
		ds_push(page, '\n');
	}

	ds_push(page, '\n');
	return parse;
}

__private void desc_parse(char** page, ccdoc_s* ccdoc, substr_s* desc, celement_s* ret, celement_s* vargs){
	int incode = 0;
	__mem_free substr_s* vsubargs = NULL;
	if( vargs && vector_count(vargs) > 0){
		vsubargs = ccdoc_parse_args(desc, vector_count(vargs));
	}

	const char* parse = desc->begin;
	while( parse < desc->end ){
		if( *parse == '\n' ){
			parse = ccparse_skip_hn(parse);
			if( incode ) man_nl(page);
			continue;
		}
		else if( !incode && parse[0] == '\\' && parse[1] == 'n' ){
			parse += 2;
			man_nl(page);
			continue;	
		}

		if( !incode && *parse == CCDOC_DESC_COMMAND ){
			++parse;
			switch( *parse ){
				case CCDOC_DC_RET:{
					dbg_error("DC_RET");
					++parse;
					substr_s retdesc;
					ccdoc_parse_ret(&parse, &retdesc);
					man_title(page, CCDOC_SUBSECTION_HID + 1, _RETURN, str_len(_RETURN));
					if( ret && ret->type.begin ){
						dbg_info("write type");
						//substr_s tmp = {.begin = _TYPE };
						//tmp.end = tmp.begin + str_len(_TYPE);
						//man_attribute(page, 1, &tmp);
						man_attribute(page, 1, &ret->type);
					}
					ds_cat(page, retdesc.begin, substr_len(&retdesc));
					ds_push(page, '\n');
					
					break;
				}

				case CCDOC_DC_ARG:	ccdoc_parse_skip_arg(&parse); break;

				case CCDOC_DC_TITLE:{
					dbg_info("DC_TITLE");
					++parse;
					int titleid;
					substr_s t;
					ccdoc_parse_title(&parse, &titleid, &t);
					ds_push(page, '\n');
					man_title(page, titleid, t.begin, substr_len(&t));
					break;
				}
			
				case CCDOC_DC_CODE_B:{
					dbg_info("DC_CODE_B");
					++parse;
					incode = 1;
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
					man_link(page, ccdoc, &link);
					break;
				}
				
				case CCDOC_DC_BOLD: case CCDOC_DC_ITALIC: case CCDOC_DC_STRIKE:{
					dbg_info("DC_ATTRIBUTE");
					substr_s txt;
					int att = ccdoc_parse_attribute(&parse, &txt);
					man_attribute(page, att, &txt);
					break;
				}

				case CCDOC_DC_REF:{
					dbg_info("DC_REF");
					++parse;
					ccdoc_parse_ref(ccdoc, &parse, page, man_push_ref, ccdoc);
					break;
				}

				case CCDOC_DC_OPENC:
					++parse;
					ds_cat(page, "/*", 0);
				break;

				case CCDOC_DC_CLOSEC:
					++parse;
					ds_cat(page, "*/", 0);
				break;

				default: die("unknown command desc @%c", *parse); break;
			}
		}
		else if( incode && *parse == CCDOC_DESC_COMMAND && *(parse+1) == CCDOC_DC_CODE_E ){
			dbg_info("DC_CODE_E");
			incode = 0;
			parse += 2;
			//if( (*page)[ds_len(*page)-1] != '\n' ) ds_push(page, '\n');
		}
		else if( !incode && *parse == CCDOC_DC_ESCAPE ){
			dbg_info("DC_ESCAPE");
			++parse;
			if( *parse == 'n' ){
				++parse;
				ds_push(page, '\n');
			}
			else{
				ds_push(page, *parse++);
			}
		}
		else if( incode ){
			if( parse[0] == CCDOC_DESC_COMMAND && parse[1] == CCDOC_DC_OPENC ){
				ds_push(page, '/');
				ds_push(page, '*');
				parse += 2;
			}
			if( parse[0] == CCDOC_DESC_COMMAND && parse[1] == CCDOC_DC_CLOSEC ){
				ds_push(page, '*');
				ds_push(page, '/');
				parse += 2;
			}
			else{
				ds_push(page, *parse++);
			}
		}
		else{
			//dbg_info("CHAR:%c", *parse);
			ds_push(page, *parse++);
		}
	}

	if( vsubargs ){
		substr_s undoc = {.begin = _UNDOCUMENTED};
		undoc.end = undoc.begin + str_len(_UNDOCUMENTED);
		vector_foreach(vsubargs, i){
			man_argument(page, ccdoc, &vargs[i].type, &vargs[i].name, vsubargs[i].begin || vargs[i].desc.begin ? &vsubargs[i] : &undoc, &vargs[i].desc );
		}
	}
}

__private void man_section_c(char** page, ccdoc_s* ccdoc, ccfile_s* file, const char* secName, const char* secDesc, ctype_e type ){
	int count = 0;
	vector_foreach(file->vdefs, i){
		if( file->vdefs[i].def.type != type ) continue;
		++count;
		break;
	}
	if( !count ) return;

	man_sh(page, secName, str_len(secName));
	substr_s tmp = {.begin = secDesc, .end = secDesc + str_len(secDesc)};
	desc_parse(page, ccdoc, &tmp, NULL, NULL);
	//ds_push(page, '\n');

	vector_foreach(file->vdefs, i){
		if( file->vdefs[i].def.type != type ) continue;
		dbg_info("(%d):%.*s", type, substr_format(&file->vdefs[i].def.name));
		man_title(page, CCDOC_SUBSECTION_HID, file->vdefs[i].def.name.begin, substr_len(&file->vdefs[i].def.name));
		desc_parse(page, ccdoc, &file->vdefs[i].comment, &file->vdefs[i].def.ret, file->vdefs[i].def.velement );
		man_nl(page);
		if( file->vdefs[i].def.code.begin ){
			__mem_free char* code = ccparse_remove_comment_command(&file->vdefs[i].def.code);
			ds_replace(&code, "\n", "\n"_NL, str_len("\n"_NL));
			man_title(page, CCDOC_SUBSECTION_HID+1, T_CODE, str_len(T_CODE));
			ds_cat(page, code, ds_len(code));
			man_nl(page);
		}
	}
	ds_push(page, '\n');
}

__private char* ccdoc_man_build_file(ccdoc_s* ccdoc, ccfile_s* ccf, char* manfile){
	char* page = ds_new(CCDOC_STRING_SIZE);
	man_th(&page, ccdoc, manfile, ds_len(manfile), 1);
	man_sh(&page, ccf->name.begin, substr_len(&ccf->name));
	desc_parse(&page, ccdoc, &ccf->desc, NULL, NULL);

	man_section_c(&page, ccdoc, ccf, T_MACRO, T_MACRO_DESC, C_MACRO);
	man_section_c(&page, ccdoc, ccf, T_ENUM, T_ENUM_DESC, C_ENUM);
	man_section_c(&page, ccdoc, ccf, T_TYPE, T_TYPE_DESC, C_TYPE);
	man_section_c(&page, ccdoc, ccf, T_STRUCT, T_STRUCT_DESC, C_STRUCT);
	man_section_c(&page, ccdoc, ccf, T_FN, T_FN_DESC, C_FN);
	return page;
}

void ccdoc_build_man(ccdoc_s* ccdoc, const char* destdir){
	int pn;
	if( ccdoc_project_info(NULL, &pn, ccdoc) ) pn = 1;
	else if( pn == 0 ) pn = 1;
	else if( pn == 2 ) pn = 3;
	
	vector_foreach(ccdoc->vfiles, i){
		__mem_free char* manfile = ccdoc->vfiles[i].visual == VISUAL_INDEX ?
			ds_dup(ccdoc->vfiles[i].name.begin, substr_len(&ccdoc->vfiles[i].name)) :
			ds_printf("%.*s_%.*s", substr_format(&get_index(ccdoc)->name), substr_format(&ccdoc->vfiles[i].name))
		;
		dbg_info("man fname:%s", manfile);

		dbg_info("man page");
		__mem_free char* page = ccdoc_man_build_file(ccdoc, &ccdoc->vfiles[i], manfile);
		dbg_info("save");
		__mem_free char* fdest = ds_printf("%s/%s.%d", destdir, manfile, pn);
		__fd_close int fd = fd_open(fdest, "w", CCDOC_FILE_PRIV);
		if( fd < 0 ) die("error on open file %s :: %s", fdest, str_errno());
		if( fd_write(fd, page, ds_len(page)) != (ssize_t)ds_len(page) ) die("wrong write page on file %s :: %s", fdest, str_errno());
	}
}


