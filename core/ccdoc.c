#include <ccdoc.h>

__private substr_s SSNULL;

__private const char* ccdoc_code_load(ccdoc_s* ccdoc, const char* file){
	__fd_close int fd = fd_open(file, "r", 0);
	if( fd == -1 ) die("file '%s':%s", file, str_errno());
	char* code = fd_slurp(NULL, fd, CCDOC_CHUNK, 1);
	if( !code ) die("error on read file '%s'", file);
	vector_push(ccdoc->vcode, code);
	mem_link(ccdoc->vcode, code);
	return code;
}

ccdoc_s* ccdoc_new(void){
	ccdoc_s* ccdoc = NEW(ccdoc_s);
	mem_link(ccdoc, (ccdoc->vfiles = vector_new(ccfile_s, CCDOC_VFILES)));
	mem_link(ccdoc, (ccdoc->vcode  = vector_new(const char*, CCDOC_VCODE)));
	mem_link(ccdoc, (ccdoc->refs = rbhash_new(CCDOC_HASH_BEGIN, CCDOC_HASH_MIN, CCDOC_REF_KEYSIZE, CCDOC_HASH_HASH)));
	return ccdoc;
}

__private void ccdoc_ref_new(ccdoc_s* ccdoc, reftype_e type, substr_s* name, substr_s* val, void* data){
	ref_s* ref = NEW(ref_s);
	mem_link(ccdoc->refs, ref);
	ref->type = type;
	ref->name  = *name;
	ref->value = *val;
	ref->data  = data;
	if( rbhash_add_unique(ccdoc->refs, ref->name.begin, substr_len(&ref->name), ref) ){
		die("reference '%.*s', already exists", (int)substr_len(&ref->name), ref->name.begin);
	}
}

void ccdoc_load(ccdoc_s* ccdoc, const char* file){
	const char* code = ccdoc_code_load(ccdoc, file);
	dbg_info("loaded code:%s", code);
	substr_s command;
	code = cparse_comment_command(&command, code);
	dbg_info("comment command:%.*s", substr_format(&command));
	do{
		cmdtype_e cmdtype;
		command.begin = ccparse_type(&cmdtype, command.begin);
		switch( cmdtype ){
			case CC_NULL: 
				dbg_info("command type null");	
			break;
			
			case CC_REF:{
				dbg_info("command type ref");
				ref_s* ref = NEW(ref_s);
				mem_zero(ref);
				mem_link(ccdoc->refs, ref);
				ccparse_ref(ref, &command);
				if( rbhash_add_unique(ccdoc->refs, ref->name.begin, substr_len(&ref->name), ref) ){
					die("reference '%.*s', already exists", (int)substr_len(&ref->name), ref->name.begin);
				}
				break;
			}
			break;
			
			case CC_STR:{
				dbg_info("command type str");
				ref_s* ref = NEW(ref_s);
				mem_zero(ref);
				mem_link(ccdoc->refs, ref);
				ccparse_str(ref, &command);
				if( rbhash_add_unique(ccdoc->refs, ref->name.begin, substr_len(&ref->name), ref) ){
					die("reference '%.*s', already exists", (int)substr_len(&ref->name), ref->name.begin);
				}
				break;
			}
			
			case CC_FILE:{
				dbg_info("command type file");
				ccfile_s* cf = vector_push_ref(ccdoc->vfiles);
				cf->desc.begin = cf->desc.end = cf->name.begin = cf->name.end = NULL;
				cf->visual = VISUAL_SIDE;
				mem_link(ccdoc, (cf->vdefs = vector_new(ccdef_s, CCDOC_VDEF)));
				ccparse_file(cf, &command);
				ccdoc->fsel = cf;
				ccdoc_ref_new(ccdoc, REF_FILE, &cf->name, &SSNULL, cf);
				break;
			}

			case CC_SEL:{
				dbg_info("command type sel");
				substr_s fname;
				ccparse_sel(&fname, &command);
				ref_s* ref = rbhash_find(ccdoc->refs, fname.begin, substr_len(&fname));
				if( !ref ) die("file '%.*s' not exists",  (int)substr_len(&fname), fname.begin);
				if( ref->type != REF_FILE ) die("you can select only a file");
				ccdoc->fsel = ref->data;
				break;
			}
	
			case CC_VISUAL:{
				dbg_info("command type visual");
				if( !ccdoc->fsel ) die("no file select");
				ccparse_visual(&ccdoc->fsel->visual, &command);
				break;
			}

			case CC_DEF:{
				dbg_info("command type def");
				if( !ccdoc->fsel ) die("no file select");
				ccdef_s* ccd = vector_push_ref(ccdoc->fsel->vdefs);
				memset(ccd, 0, sizeof(ccdef_s));
				ccd->parent = ccdoc->fsel;
				code = ccparse_def(ccd, &command);
				if( ccd->def.velement ) mem_link(ccdoc, ccd->def.velement);
				ccdoc_ref_new(ccdoc, REF_DEF, &ccd->def.name, &SSNULL, ccd);
				break;
			}

			default: case CC_COUNT: die("wrong command comment"); break;
		}
		code = cparse_comment_command(&command, code);
		if( *code ) dbg_info("comment command:%.*s", substr_format(&command));
	}while( *code );
}

void ccdoc_copy_css(const char* destdir, const char* srcdir){
	__dir_close dir_s* src = dir_open(srcdir);
	if( !src ) die("wrong dir '%s':%s", srcdir, str_errno());
	dir_foreach(src, file){
		if( dirent_currentback(file) ) continue;
		const char* ext = str_find(dirent_name(file), ".css");
		if( !*ext ) continue;
		if( ext[str_len(".css")] ) continue; 
		__mem_free char* destfile = ds_printf("%s/%s", destdir, dirent_name(file));
		__mem_free char* srcfile = ds_printf("%s/%s", srcdir, dirent_name(file));
		__fd_close int fds = fd_open(srcfile, "r", 0);
		__fd_close int fdd = fd_open(destfile, "w", CCDOC_FILE_PRIV);
		if( fds < 0 || fdd < 0 ) continue;
		fd_copy(fdd, fds);
	}
}

void ccdoc_dump(ccdoc_s* ccdoc){
	vector_foreach(ccdoc->vfiles, i){
		printf("<%.*s>%.*s\n", 
			substr_format(&ccdoc->vfiles[i].name),
			substr_format(&ccdoc->vfiles[i].desc)
		);

		vector_foreach(ccdoc->vfiles[i].vdefs, j){
			printf("\t<%.*s type:%d>%.*s", 
				substr_format(&ccdoc->vfiles[i].vdefs[j].def.name),
				ccdoc->vfiles[i].vdefs[j].def.type,
				substr_format(&ccdoc->vfiles[i].vdefs[j].comment)
			);
			if( (ccdoc->vfiles[i].vdefs[j].def.type == C_FN || ccdoc->vfiles[i].vdefs[j].def.type == C_MACRO) && ccdoc->vfiles[i].vdefs[j].def.velement ){
				vector_foreach(ccdoc->vfiles[i].vdefs[j].def.velement, k){
					printf("<arg type=\"%.*s\" name:\"%.*s\">%.*s</arg>",
						substr_format(&ccdoc->vfiles[i].vdefs[j].def.velement[k].type),
						substr_format(&ccdoc->vfiles[i].vdefs[j].def.velement[k].name),
						substr_format(&ccdoc->vfiles[i].vdefs[j].def.velement[k].desc)
					);
				}
			}
			printf("</%.*s>\n", substr_format(&ccdoc->vfiles[i].vdefs[j].def.name));
		}

		printf("</%.*s>\n", substr_format(&ccdoc->vfiles[i].name));
	}
}


