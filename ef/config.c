#include <ef/type.h>
#include <ef/str.h>
#include <ef/substr.h>
#include <ef/dstr.h>
#include <ef/rbhash.h>
#include <ef/vector.h>
#include <ef/fconfig.h>
#include <ef/file.h>

/*-file 'fconfig format'
 * technical specifications\n
 * lvalue  format ([a-zA-Z_][a-zA-Z_0-9]*)\n
 * rvalue  format ([*a-zA-Z_][a-zA-Z_0-9]*)\n
 * long    format (number(base 8(01) | base 10(1234) | base 16(0xff)))\n
 * double  format ([0-9].[0-9])\n
 * string  format ([^ \\t\\n] | ['"][^'"]['"])\n
 * vector  format (\\[ rvalue (, rvalue)* \\])\n
 * comment format (//.*)\n
 * op      format (= | +=)\n
 * lvalue op rvalue\n
 * @^2 'Examples'
 * @^3 'assign'
 * @^5 'number'
 * @{ 
 * name  = 1234  //long base 10
 * name2 = 12.34 //double
 * name3 = 0xf   //long base 16
 * name4 = 0666  //long base 8
 * @}
 * @^5 'reference'
 * copy name in bar\n
 * @{ bar = *name @}
 * @^5 'assign string'
 * @{ 
 * str0 = /usr 
 * str1 = 'hello world'
 * str2 = 'abc "cde" efg'
 * str3 = "abc \"cde\" efg"
 * @}
 * @^5 'vector'
 * @{
 * v   = [] //empty vector
 * vec = [ 1, 'a', b, *name ]
 * @}
 * @^3 'add'
 * required same type for add value\n
 * @{
 * name += 123    //name(1234) + 123 = 1347
 * name += *name3 //name(1347) + name3(0xf) = 1363
 * str0 += /share //str0(/usr) + /share = /usr/share
 * v += [ 1 , 2]  //v(empty) = v[1,2]
 * v += [ 3, 4]   //v(1,2) + (3,4) = v(1,2,3,4)
 * @}
*/

/*-visual side*/

#define CHUNKSIZE 4096
#define RB_SIZE 32
#define RB_MIN  12

#define RX_az(CH) ((CH) >= 'a' && (CH) <= 'z')
#define RX_AZ(CH) ((CH) >= 'A' && (CH) <= 'Z')
#define RX_09(CH) ((CH) >= '0' && (CH) <= '9')
#define RX_W(CH) (RX_az(CH) || RX_AZ(CH) || RX_09(CH) || ((CH)=='_'))

typedef struct fconfig{
	rbhash_t* var;
	char* beginParse;
	char* error;
}fconfig_t;

__private int fc_linen(const char* data, const char* parse){
	int linen=1;
	while( *(data=str_chr(data, '\n')) && data < parse ){
		++linen;
		++data;
	}
	return linen;
}

__private int fc_chn(const char* data, const char* parse){
	const char* sl = parse;
	while( sl > data && *sl != '\n' ) --sl;
	if( *sl != '\n' ){
		//defensive
		sl = data;
	}
	else{
		++sl;
	}
	return parse - sl;
}

__private int fc_lvalue(substr_s* tk, fconfig_t* fc, const char** parse){
	if( !(RX_AZ(**parse) || RX_az(**parse) || **parse != '_') ){
		fc->error = str_printf("wrong varibale at line:%d ch:%d\n", fc_linen(fc->beginParse, *parse), fc_chn(fc->beginParse, *parse));
		mem_link(fc, fc->error);
		return -1;
	}
	
	tk->begin = *parse;
	while( **parse && RX_W(**parse) ){
		++(*parse);
	}
	tk->end = *parse;

	return 0;
}

__private int fc_rvalue_is_double(const char* parse){
	while( *parse && RX_09(*parse) ) ++parse;
	return *parse == '.' ? 1 : 0;
}

__private int fc_rvalue_base(const char* parse){
	if( *parse == '0' && RX_09(parse[1]) ) return 8;
	if( *parse == '0' && parse[1] == 'x' ) return 16;
	return 10;
}

__private void fc_assign(fconfig_t* fc, fconfigVar_s* var, fconfigVar_s* ptr){
	var->type = ptr->type;
	switch( var->type ){
		case FCVAR_LONG  : var->fclong = ptr->fclong; break;
		case FCVAR_DOUBLE: var->fcdouble = ptr->fcdouble; break;
		case FCVAR_NULL  : var->fcnull = ptr->fcnull; break;
		case FCVAR_STR   : var->fcstr = ds_dup(ptr->fcstr, ds_len(ptr->fcstr)); mem_link(var, var->fcstr); break;
		case FCVAR_VECTOR: 
			var->fcvector = vector_new(fconfigVar_s*, vector_count(ptr->fcvector));
			mem_link(var, var->fcvector);
			vector_foreach(ptr->fcvector, i){
				fconfigVar_s* vv = NEW(fconfigVar_s);
				mem_link(var->fcvector, vv);
				fc_assign(fc, vv, var->fcvector[i]);
				vector_push(var->fcvector, vv);
			}
		break;
	}
}

__private int fc_plus(fconfig_t* fc, fconfigVar_s* var, fconfigVar_s* rval){
	switch( var->type ){
		case FCVAR_LONG  : var->fclong += rval->fclong; break;
		case FCVAR_DOUBLE: var->fcdouble += rval->fcdouble; break;
		case FCVAR_NULL  : var->fcnull = rval->fcnull; break;
		case FCVAR_STR   : ds_cat(&var->fcstr, rval->fcstr, ds_len(rval->fcstr)); break;
		case FCVAR_VECTOR: 
			vector_foreach(rval->fcvector, i){
				fconfigVar_s* vv = NEW(fconfigVar_s);
				mem_link(var->fcvector, vv);
				fc_assign(fc, vv, rval->fcvector[i]);
				vector_push(var->fcvector, vv);
			}
		break;
	}
	return 0;
}

__private int fc_rvalue(fconfig_t* fc, fconfigVar_s* var, const char** parse){
	const char* p = *parse;

	switch( *p ){
		case '\'':
		case '\"':
			var->type = FCVAR_STR;
			var->fcstr = ds_new(32);
			mem_link(var, var->fcstr);
			p = ds_between(&var->fcstr, p);
			if( p[-1] != '\'' && p[-1] != '\"' ){
				fc->error = str_printf("unterminated string at line:%d ch:%d\n", fc_linen(fc->beginParse, *parse), fc_chn(fc->beginParse, *parse));
				mem_link(fc, fc->error);
				return -1;
			}
		break;

		case '0' ... '9':
			if( fc_rvalue_is_double(p) ){
				var->type = FCVAR_DOUBLE;
				var->fcdouble = strtod(p, (char**)&p);
			}
			else{
				var->type = FCVAR_LONG;
				var->fclong = strtol(p, (char**)&p, fc_rvalue_base(p));
			}
		break;

		case '[':
			var->type = FCVAR_VECTOR;
			var->fcvector = vector_new(fconfigVar_s*, 8);	
			mem_link(var, var->fcvector);
			
			p = str_skip_h(p+1);
			while( *p && *p != ']' ){
				fconfigVar_s* rv = NEW(fconfigVar_s);
				mem_link(var->fcvector, rv);
				if( fc_rvalue(fc, rv, &p) ){
					dbg_error("wrong rval");
					return -1;
				}
				vector_push(var->fcvector, rv);
				p = str_skip_h(p);
				if( *p == ',' ) p = str_skip_h(p+1);
			}
			if( *p == ']' ) ++p;
		break;
		
		case '*':{
			++p;
			substr_s tk;
			if( fc_lvalue(&tk, fc, &p) ){
				dbg_error("fc_lvalue error");
				return -1;
			}
			fconfigVar_s* ptr = rbhash_find(fc->var, tk.begin, substr_len(&tk));
			if( !var ){
				dbg_error("rbhash");
				fc->error = str_printf("var %.*s declared at line:%d ch:%d not exists\n", substr_format(&tk), fc_linen(fc->beginParse, *parse), fc_chn(fc->beginParse, *parse));
				mem_link(fc, fc->error);
				return -1;
			}
			fc_assign(fc, var, ptr);
			break;
		}

		default:
			var->type = FCVAR_STR;
			var->fcstr = ds_new(32);
			mem_link(var, var->fcstr);
			while( *p && *p != ' ' && *p != '\t' && *p != '\n' && *p != ',' && *p != ']' ){
				ds_push(&var->fcstr, *p++);
			}
		break;
	}

	*parse = p;
	return 0;
}

__private int fc_op(fconfig_t* fc, substr_s* lvalue, const char** parse){
	const char* p = *parse;

	p = str_skip_h(p);
	if( *p == '=' ){
		dbg_info("assign");
		p = str_skip_h(p+1);
		fconfigVar_s* var = NEW(fconfigVar_s);
		mem_link(fc->var, var);
		if( fc_rvalue(fc, var, &p) ) return -1;
		dbg_info("add %.*s", substr_format(lvalue));
		if( rbhash_add_unique(fc->var, lvalue->begin, substr_len(lvalue), var) ){
			dbg_error("not unique key");
			fc->error = str_printf("var '%.*s' already exists at line:%d ch:%d\n", substr_format(lvalue), fc_linen(fc->beginParse, p), fc_chn(fc->beginParse, p));
			mem_link(fc, fc->error);
			return -1;
		}
		dbg_info("ok");
	}
	else if( *p == '+' ){
		dbg_info("plus");
		if( p[1] != '=' ){
			fc->error = str_printf("miss = at line:%d ch:%d\n", fc_linen(fc->beginParse, p), fc_chn(fc->beginParse, p));
			mem_link(fc, fc->error);
			return -1;
		}
		p = str_skip_h(p+2);
		fconfigVar_s* var = rbhash_find(fc->var, lvalue->begin, substr_len(lvalue));
		if( !var ){
			fc->error = str_printf("var %.*s declared at line:%d ch:%d not exists\n", substr_format(lvalue), fc_linen(fc->beginParse, *parse), fc_chn(fc->beginParse, *parse));
			mem_link(fc, fc->error);
			return -1;
		}
		__mem_free fconfigVar_s* tmp = NEW(fconfigVar_s);
		if( fc_rvalue(fc, tmp, &p) ) return -1;
		if( tmp->type != var->type ){
			fc->error = str_printf("plus with different type at line:%d ch:%d\n", fc_linen(fc->beginParse, *parse), fc_chn(fc->beginParse, *parse));
			mem_link(fc, fc->error);
			return -1;
		}
		fc_plus(fc, var, tmp);
	}
	else{
		dbg_info("null");
		fconfigVar_s* var = NEW(fconfigVar_s);
		mem_link(fc->var, var);
		var->type = FCVAR_NULL;
		var->fcnull = NULL;
		dbg_info("add %.*s", substr_format(lvalue));
		if( rbhash_add_unique(fc->var, lvalue->begin, substr_len(lvalue), var) ){
			fc->error = str_printf("var '%.*s' already exists at line:%d ch:%d\n", substr_format(lvalue), fc_linen(fc->beginParse, p), fc_chn(fc->beginParse, p));
			mem_link(fc, fc->error);
			return -1;
		}
	}
	
	if( *p ){
		p = str_skip_h(p);
		if( p[0] == '/' && p[1] == '/' ){
			p = str_next_line(p);
		}
		else if( *p == '\n' ){
			++p;
		}
		else{
			fc->error = str_printf("wrong lvalue at line:%d ch:%d\n", fc_linen(fc->beginParse, p), fc_chn(fc->beginParse, p));
			mem_link(fc, fc->error);
			return -1;
		}
	}

	*parse = p;
	return 0;
}

__private void fc_parse(fconfig_t* fc, const char* parse){
	parse = str_skip_hn(parse);
	while( *parse ){
		//find lvalue
		dbg_info("\n%s", parse);
		if( parse[0] == '/' && parse[1] == '/' ){
			parse = str_next_line(parse+2);
			continue;
		}
		substr_s lvalue;
		if( fc_lvalue(&lvalue, fc, &parse) ) return;
		dbg_info("%.*s", substr_format(&lvalue));

		if( fc_op(fc, &lvalue, &parse) ) return;
		parse = str_skip_hn(parse);
	}
}

fconfig_t* fconfig_load(const char* file, size_t maxVarLen){
	fconfig_t* fc = NEW(fconfig_t);
	fc->error  = NULL;
	fc->var = rbhash_new(RB_SIZE, RB_MIN, maxVarLen, hash_fasthash);
	mem_link(fc, fc->var);	

	__fd_close int fd = fd_open(file, "r", 0);
	if( !fd ){
		fc->error = str_printf("error on open file '%s': %s\n", file, str_errno());
		mem_link(fc, fc->error);
		return fc;
	}
	__mem_free char* data = fd_slurp(NULL, fd, CHUNKSIZE, 1);
	iassert(data);

	fc->beginParse = data;
	dbg_info("%s", data);
	fc_parse(fc, data);
	fc->beginParse = NULL;
	
	return fc;
}

const char* fconfig_error(fconfig_t* fc){
	return fc->error;
}

fconfigVar_s* fconfig_get(fconfig_t*  fc, const char* name, size_t len){
	if( !len ) len = strlen(name);
	fconfigVar_s* ret = rbhash_find(fc->var, name, len);
	return ret;
}

int fconfig_add(fconfig_t* fc, const char* name, size_t len, fconfigVar_s* value){
	iassert(value);
	if( rbhash_add_unique(fc->var, name, len, value) ){
		return -1;
	}
	mem_link(fc->var, value);
	return 0;
}

















