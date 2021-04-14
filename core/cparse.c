#include <ccdoc.h>

#define _PRE           '#'
#define _DEFINE        "define"
#define _COMMENT       '/'
#define _COMMENT_OPEN  "/*"
#define _COMMENT_CLOSE "*/"
#define _ARGS_OPEN     '('
#define _ARGS_CLOSE    ')'
#define _ARGS_NEXT     ','
#define _TYPEDEF       "typedef"
#define _STRUCT        "struct"
#define _ENUM          "enum"
#define _ENDL          ';'
#define _SCOPE_OPEN    '{'
#define _SCOPE_CLOSE   '}'

__private const char* cparse_token_next(const char* code){
	while(1){
		code = str_skip_hn(code);
		if( code[0] == '/' ){
			if( code[1] == '/' ){
				code += 2;
				while( *code && *code != '\n' ) ++code;
				continue;
			}
			else if( code[1] == '*' ){
				code += 2;
				while( code[0] && code[1] && !(code[0] == '*' && code[1] == '/') ) ++code;
				if( *code ) code += 2;
				continue;
			}
		}
		break;
	}
	return code;
}

const char* cparse_comment_command(substr_s* out, const char* code){
	code = str_find(code, _COMMENT_OPEN CCDOC_COMMAND);
	if( !*code ) return code;
	code += str_len(_COMMENT_OPEN);
	out->begin = code;
	code = str_find(code, _COMMENT_CLOSE);
	out->end = code;
	return *code ? code + str_len(_COMMENT_CLOSE) : code;
}

__private const char* cparse_comment_obj(substr_s* out, const char* code){
	code = str_skip_hn(code);
	if( code[0] == _COMMENT ){
		if( code[1] == _COMMENT && code[2] == CCDOC_DESC_OBJ ){
			code += 3;
			out->begin = code;
			while( *code && *code != '\n' ) ++code;
			out->end = code;
			return *code ? code+1 : code;
		}
		else if( code[1] == '*' && code[2] == CCDOC_DESC_OBJ ){
			code += 3;
			out->begin = code;
			while( code[0] && code[1] && !(code[0] == '*' && code[1] == _COMMENT) ) ++code;
			out->end = code;
			return *code ? code+2 : code;
		}
	}
	return code;
}

__private const char* cparse_token(substr_s* out, const char* code){
	out->begin = code;
	while( (*code >= 'a' && *code <= 'z') || (*code >= 'A' && *code <= 'Z') || (*code >= '0' && *code <= '9') || (*code == '_') ) ++code;
	out->end = code;
	const char* tmp = cparse_token_next(code);
	while( *tmp && *tmp == '*' ){
		out->end = code = tmp+1;
		tmp = cparse_token_next(code);		
	}
	return code;
}

__private const char* cparse_declar(substr_s* type, substr_s* name, const char* code){
	code = cparse_token_next(cparse_token(type, code));
	code = cparse_token_next(cparse_token(name, code));
	while( *code && *code != _SCOPE_CLOSE && *code != _SCOPE_OPEN && *code != _ENDL && *code != _ARGS_NEXT && *code != _ARGS_CLOSE && *code != _ARGS_OPEN){
		type->end = name->end;
		code = cparse_token_next(cparse_token(name, code));
	}
	return *code ? ++code : code;
}

__private const char* cparse_declar_enum(substr_s* name, const char* code){
	code = cparse_token(name, code);
	while( *code && *code != _SCOPE_CLOSE && *code != _ENDL && *code != _ARGS_NEXT && *code != _COMMENT) ++code;
	return *code && *code != _COMMENT ? ++code : code;
}

__private const char* cparse_macro(cdef_s* def, const char* code){
	def->type = C_MACRO;
	code = cparse_token(&def->name,code);
	def->code.begin = def->name.begin;
	if( *code == _ARGS_OPEN ){
		def->velement = vector_new(celement_s, 4);
		code = str_skip_hn(code+1);
		while( *code && *code != _ARGS_CLOSE ){
			celement_s* ce = vector_push_ref(def->velement);
			memset(ce, 0, sizeof(celement_s));
			code = cparse_token_next(cparse_token(&ce->name, code));
			dbg_info("macro.arg:%.*s", substr_format(&ce->name));
			if(*code == _ARGS_NEXT) code = cparse_token_next(code+1);
		}
		if( *code ) ++code;
	}
	def->code.end = code;
	dbg_info("macro:%.*s", substr_format(&def->name));
	dbg_info("code:%.*s",  substr_format(&def->code));
	return code;
}

__private const char* cparse_type(cdef_s* def, const char* code){
	def->code.begin = code;
	int td = 0;
	if( !strncmp(code, _TYPEDEF, str_len(_TYPEDEF)) ){
		td = 1;
		code = cparse_token_next(code+str_len(_TYPEDEF));
	}
	
	code = cparse_declar(&def->typedec, &def->name, code);
	dbg_info("type declare:%.*s", substr_format(&def->typedec));
	dbg_info("name declare:%.*s", substr_format(&def->name));

	if( !*code || code[-1] == _ENDL){
		def->type = C_TYPE;
		def->code.end = code;
		dbg_info("type:%.*s", substr_format(&def->name));
		dbg_info("code:%.*s", substr_format(&def->code));
		return code;
	}

	def->typedec.end = def->name.end;

	if( code[-1] != _SCOPE_OPEN ) die("aspect open scope");

	if( !strncmp(def->typedec.begin, _STRUCT, str_len(_STRUCT)) ) def->type = C_STRUCT;
	else if( !strncmp(def->typedec.begin, _ENUM, str_len(_ENUM)) ) def->type = C_ENUM;
	else die("unknown type");
	
	def->velement = vector_new(celement_s, 4);	
	code = cparse_token_next(code);

	if( def->type == C_STRUCT ){
		while( *code && *code != _SCOPE_CLOSE ){
			celement_s* ce = vector_push_ref(def->velement);
			memset(ce, 0, sizeof(celement_s));
			code = cparse_declar(&ce->type, &ce->name, code);
			dbg_info("obj.argument:%.*s::%.*s", substr_format(&ce->type), substr_format(&ce->name));
			code = cparse_token_next(cparse_comment_obj(&ce->desc, code));
			dbg_info("obj.arg.desc:%.*s", substr_format(&ce->desc));
		}
		++code;
		
		if( !td ){
			def->name.begin = def->typedec.begin;
			def->name.end   = def->typedec.end;
			cparse_token_next(code);
		}
		else{
			code = cparse_token_next(cparse_token(&def->name, code));
		}
		if( *code != _ENDL ) die("aspect end line after struct or enum");
		def->code.end = ++code;
	}
	else if( def->type == C_ENUM ){
		while( *code && *code != _SCOPE_CLOSE ){
			celement_s* ce = vector_push_ref(def->velement);
			memset(ce, 0, sizeof(celement_s));
			code = cparse_declar_enum(&ce->name, code);
			dbg_info("enum.argument:%.*s", substr_format(&ce->name));
			code = cparse_token_next(cparse_comment_obj(&ce->desc, code));
			dbg_info("enum.arg.desc:%.*s", substr_format(&ce->desc));
		}
		++code;
		
		if( !td ){
			def->name.begin = def->typedec.begin;
			def->name.end   = def->typedec.end;
			cparse_token_next(code);
		}
		else{
			code = cparse_token_next(cparse_token(&def->name, code));
		}
		if( *code != _ENDL ) die("aspect end line after struct or enum");
		def->code.end = ++code;
	}

	dbg_info("new type:%.*s", substr_format(&def->name));
	dbg_info("typeof:%.*s", substr_format(&def->typedec));
	dbg_info("code:%.*s", substr_format(&def->code));

	return code;
}

__private const char* cparse_fn(cdef_s* def, const char* code){
	def->code.begin = code;
	code = cparse_declar(&def->typedec, &def->name, code);
	if( !*code || code[-1] != _ARGS_OPEN){
		def->type = C_NULL;
		def->code.end = code;
		dbg_info("err function");
		return code;
	}
	def->ret.type = def->typedec;

	def->type = C_FN;
	def->velement = vector_new(celement_s, 4);	
	code = cparse_token_next(code);
	while( *code && code[-1] != _ARGS_CLOSE ){
		celement_s* ce = vector_push_ref(def->velement);
		memset(ce, 0, sizeof(celement_s));
		code = cparse_declar(&ce->type, &ce->name, code);
		code = cparse_comment_obj(&ce->desc, code);
		dbg_info("fn.arg:(%.*s)%.*s::%.*s", substr_format(&ce->type), substr_format(&ce->name), substr_format(&ce->desc));
		code = cparse_token_next(code);
	}
	if( *code ) ++code;
	def->code.end = code;
	dbg_info("fn:%.*s", substr_format(&def->name));
	dbg_info("code:%.*s", substr_format(&def->code));
	return code;
}

const char* cparse_definition_get(cdef_s* def, const char* code){
	substr_s token;
	code = cparse_token_next(code);
	if( !*code ) return code;
	
	if( *code == _PRE ){
		dbg_info("preprocessor");
		code = cparse_token_next(code+1);
		code = cparse_token_next(cparse_token(&token, code));
		dbg_info("token:%.*s", substr_format(&token));

		if( substr_cmp(&token, _DEFINE) ){
			dbg_info("is not define");
			def->type = C_NULL;
			return code;
		}
		return cparse_macro(def, cparse_token_next(code));
	}

	if( 
		!strncmp(code, _TYPEDEF, str_len(_TYPEDEF)) ||
		!strncmp(code, _STRUCT, str_len(_STRUCT)) ||
		!strncmp(code, _ENUM, str_len(_ENUM))
	){
		dbg_info("new type");
		return cparse_type(def,code);
	}
	
	dbg_info("function");
	return cparse_fn(def, code);
}


