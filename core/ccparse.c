#include <ccdoc.h>

#define _NULL   ""
#define _REF    "-ref "
#define _STR    "-str "
#define _FILE   "-file "
#define _SEL    "-sel "
#define _VISUAL "-visual "
#define _DEF    "- "

#define _QUOTE  '\''
#define _ESCAPE '\\'
#define _NXCOM  '*'
#define _NL     '\n'

#define _VISUAL_NONE  "none"
#define _VISUAL_TOP   "top"
#define _VISUAL_SIDE  "side"
#define _VISUAL_INDEX "index"

#define _COMMENT_CLOSE "*/"

__private const char* TYPENAME[] = {
	[CC_NULL]   = _NULL,
	[CC_REF ]   = _REF,
	[CC_STR ]   = _STR,
	[CC_FILE]   = _FILE,
	[CC_SEL ]   = _SEL,
	[CC_VISUAL] = _VISUAL,
	[CC_DEF ]   = _DEF,
	[CC_COUNT]  = NULL
};

__private const char* VISUALNAME[] = {
	[VISUAL_NONE ] = _VISUAL_NONE,
	[VISUAL_TOP  ] = _VISUAL_TOP,
	[VISUAL_SIDE ] = _VISUAL_SIDE,
	[VISUAL_INDEX] = _VISUAL_INDEX,
	[VISUAL_COUNT] = NULL
};

const char* ccparse_skip_hn(const char* code){
	code = str_skip_h(code);
	if( *code == _NL ){
		const char* bg = code+1;
		code = str_skip_h(bg);
		if( *code == _NXCOM ){
			++code;
			if (*code == ' ' || *code == '\t') ++code;
		}
		else{
			code = bg;
		}
	}
	return code;
}

const char* ccparse_string(ccfile_s* cf, substr_s* out, const char* comment){
	if( *comment != _QUOTE ) ccdoc_die(cf, comment, "aspected string quoted by %c", _QUOTE);
	out->begin = ++comment;
	while( *comment && *comment != _QUOTE && *comment != '\n' ){
		if( comment[0] == _ESCAPE && comment[1] == _QUOTE ) ++comment;
		++comment;
	}
	if( *comment != _QUOTE ) ccdoc_die(cf, out->begin, "aspected terminated string quoted by %c", _QUOTE);
	out->end = comment;
	return comment + 1;
}

__private const char* ccparse_token(substr_s* out, const char* comment){
	comment = str_skip_hn(comment);
	out->begin = comment;
	while( (*comment >= 'a' && *comment <= 'z') || (*comment >= 'A' && *comment <= 'Z') || (*comment >= '0' && *comment <= '9')  || (*comment == '_') ){
		++comment;
	}
	out->end = comment;
	dbg_info("token(%lu):%.*s", substr_len(out), (int)substr_len(out), out->begin);
	return comment;
}

char* ccparse_remove_comment_command(substr_s* scode){
	const char* code = scode->begin;
	char* ret = ds_new(substr_len(scode));
	char* ins = ret;
	while( code < scode->end ){	
		if( code[0] == 	'/' && code[1] == '*' && (code[2] == CCDOC_DESC_COMMAND || code[2] == CCDOC_DESC_OBJ )){
			code = str_find(code, _COMMENT_CLOSE);
			if( !*code ) die("comment not closed"); /*this will never happen because _COMMENT_CLOSE is already checked on cparse_comment_command*/
			code += str_len(_COMMENT_CLOSE);
			continue;
		}
		*ins++ = *code++;
	}
	ds_nullc(ret, ins-ret);
	return ret;
}

const char* ccparse_type(cmdtype_e* out, const char* commentStart){
	for( size_t i = 1; TYPENAME[i]; ++i ){
		if( !strncmp(commentStart, TYPENAME[i], str_len(TYPENAME[i])) ){
			*out = i;
			return commentStart + str_len(TYPENAME[i]);
		}
	}
	*out = CC_NULL;
	return commentStart;
}

/*-alias 'name' 'alias' */
void ccparse_ref(ccfile_s* cf, ref_s* ref, substr_s* comment){
	ref->type = REF_REF;
	const char* cc = ccparse_skip_hn(ccparse_string(cf, &ref->name, comment->begin));
	ccparse_string(cf, &ref->value, cc);
}

/*-str 'name' 'value' */
void ccparse_str(ccfile_s* cf, ref_s* ref, substr_s* comment){
	ref->type = REF_STR;
	const char* cc = ccparse_skip_hn(ccparse_string(cf, &ref->name, comment->begin));
	ccparse_string(cf, &ref->value, cc);
}

/*-file 'name' descript
 * descript */
void ccparse_file(ccfile_s* file, substr_s* comment){
	const char* cc = ccparse_skip_hn(ccparse_string(file, &file->name, comment->begin));
	file->desc.begin = cc;
	file->desc.end   = comment->end;
}

/*-visual index/top/side/none*/
void ccparse_visual(ccfile_s* cf, visual_e* out, substr_s* comment){
	comment->begin = str_skip_hn(comment->begin);
	substr_s tk={0};
	const char* next = ccparse_token(&tk, comment->begin);
	if( next > comment->end ) ccdoc_die(cf, tk.begin, "wrong visual at command");
	for( size_t i = 0; VISUALNAME[i]; ++i ){
		if( !strncmp(tk.begin, VISUALNAME[i], substr_len(&tk)) ){
			*out = i;
			return;
		}
	}
	ccdoc_die(cf, tk.begin, "unknown visual(%.*s)", substr_format(&tk));
}

/*-sel 'filename'*/
void ccparse_sel(ccfile_s* cf, substr_s* out, substr_s* comment){
	ccparse_string(cf, out, str_skip_h(comment->begin));
}

/*- decript
 * descript
 */
const char* ccparse_def(ccfile_s* cf, ccdef_s* def, substr_s* comment){
	def->comment = *comment;
	return cparse_definition_get(cf, &def->def, comment->end + str_len(_COMMENT_CLOSE));
}

















