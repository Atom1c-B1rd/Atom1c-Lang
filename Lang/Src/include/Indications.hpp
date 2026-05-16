#ifndef INDICATIONS_HPP
#define INDICATIONS_HPP

#include <cstdlib>
#include <cstring>
#include "Ast.hpp"
typedef enum {

    IND_MODULE,         // @module "nombre"
    IND_IMPORT,         // @import "lib"
    IND_EXPORT,         // @export nombre1, nombre2
    IND_INCLUDE,        // #pragma

    IND_TYPE_ALIAS,     // type Alias = Tipo
    IND_TYPE_DATA_BEGIN,// data Nombre
    IND_TYPE_ENUM_BEGIN,// enum Nombre
    IND_FIELD,          // campo de data (nombre, tipo)
    IND_CTOR,           // constructor de enum (nombre, tipos parámetros)
    IND_TYPE_END,       // fin de data/enum


    IND_FUNC_BEGIN,     // inicio de función (nombre, tipo función)
    IND_FUNC_END,       // fin de función
    IND_PARAM,          // parámetro $n (índice, tipo) – emitido al inicio de función


    IND_LET,            // let nombre: Tipo = expr (constante/mutable)
    IND_ASSIGN,         // nombre = expr (o +=, -=, etc.)


    IND_IF,             // if (condición) – inicia bloque then
    IND_ELSE,           // else
    IND_ENDIF,          // fin de if
    IND_MATCH,          // match (expr)
    IND_CASE,           // case patrón -> cuerpo
    IND_ENDMATCH,       // fin de match
    IND_LOOP,           // loop
    IND_BREAK,          // break
    IND_ENDLOOP,        // fin de loop


    IND_RETURN,         // retorno de función/bloque (valor en pila)
    IND_LITERAL,        // valor literal (entero, float, string, bool)
    IND_VAR_REF,        // referencia a variable local/global
    IND_ARG_REF,        // referencia a parámetro $n
    IND_CONSTRUCTOR,    // creación de constructor de enum (nombre, aridad)
    IND_LAMBDA,         // lambda ~> (cuerpo es una secuencia de indicaciones)
    IND_APPLY,          // aplicación $ (función en pila, argumento en pila)
    IND_CALL,           // llamada a función por nombre (nombre, aridad)
    IND_PRIMITIVE,      // operación primitiva (+, -, -EQ, putStrLn, etc.)
    IND_BLOCK,          // bloque do/where (contiene sub-indicaciones)


    IND_LABEL,          // etiqueta para saltos
    IND_JUMP,           // salto incondicional
    IND_JUMP_IF_FALSE,  // salto si falso (valor en pila)
    IND_NOP             // no operación
} IndicationKind;

typedef struct Indication Indication;
struct ASTType; 
typedef enum {
    PRIM_ADD, PRIM_SUB, PRIM_MUL, PRIM_DIV, PRIM_MOD,
    PRIM_EQ, PRIM_NEQ, PRIM_LT, PRIM_GT, PRIM_LE, PRIM_GE,
    PRIM_AND, PRIM_OR, PRIM_NOT,
    PRIM_NEG,
    PRIM_PUTSTRLN, PRIM_TOSTRING, PRIM_TONUMBER, PRIM_INPUT
} PrimitiveOpInd;

typedef struct {
    const char* name;               
    struct ASTType* type;           
    LiteralKind litKind;            
    union {
        int intValue;
        double floatValue;
        int boolValue;              
        const char* stringValue;    
    };
    PrimitiveOpInd primOp;             
    int index;                      
    int arity;                      
    bool constant;                  
    const char* label;              
    Indication* inner;              
    const char** stringList;
    int stringCount;
    struct ASTPattern* pattern; 
} IndicationData;

struct Indication {
    IndicationKind kind;
    int line;
    int column;
    IndicationData data;
    Indication* next;               
};
static inline void add_indication(Indication** head, Indication* ind) {
    if (!head || !ind) return;
    if (!*head) {
        *head = ind;
        return;
    }
    Indication* current = *head;
    while (current->next) {
        current = current->next;
    }
    current->next = ind;
}

void add_indication(Indication** head, Indication* ind);

Indication* make_ind_module(const char* name, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_MODULE;
    ind->line = line; ind->column = col;
    ind->data.name = name;
    return ind;
}

Indication* make_ind_import(const char* name,bool as,const char* asName, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_IMPORT;
    ind->line = line; ind->column = col;
    ind->data.name = name;
    ind->data.boolValue=as;
    return ind;
}

Indication* make_ind_export(const char** names, int count, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_EXPORT;
    ind->line = line; ind->column = col;
    ind->data.stringList = names;
    ind->data.stringCount = count;
    return ind;
}

Indication* make_ind_include(const char* pragma, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_INCLUDE;
    ind->line = line; ind->column = col;
    ind->data.name = pragma;
    return ind;
}

Indication* make_ind_type_alias(const char* name, struct ASTType* type, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_TYPE_ALIAS;
    ind->line = line; ind->column = col;
    ind->data.name = name;
    ind->data.type = type;
    return ind;
}

Indication* make_ind_type_data_begin(const char* name, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_TYPE_DATA_BEGIN;
    ind->line = line; ind->column = col;
    ind->data.name = name;
    return ind;
}

Indication* make_ind_type_enum_begin(const char* name, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_TYPE_ENUM_BEGIN;
    ind->line = line; ind->column = col;
    ind->data.name = name;
    return ind;
}

Indication* make_ind_field(const char* name, struct ASTType* type, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_FIELD;
    ind->line = line; ind->column = col;
    ind->data.name = name;
    ind->data.type = type;
    return ind;
}

Indication* make_ind_ctor(const char* name, struct ASTType** paramTypes, int arity, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_CTOR;
    ind->line = line; ind->column = col;
    ind->data.name = name;
    ind->data.arity = arity;
    ind->data.type = (struct ASTType*)paramTypes; 
    return ind;
}

Indication* make_ind_type_end(int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_TYPE_END;
    ind->line = line; ind->column = col;
    return ind;
}

Indication* make_ind_func_begin(const char* name, struct ASTType* funcType, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_FUNC_BEGIN;
    ind->line = line; ind->column = col;
    ind->data.name = name;
    ind->data.type = funcType;
    return ind;
}

Indication* make_ind_func_end(const char* name, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_FUNC_END;
    ind->line = line; ind->column = col;
    ind->data.name = name;
    return ind;
}

Indication* make_ind_param(int index, struct ASTType* type, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_PARAM;
    ind->line = line; ind->column = col;
    ind->data.index = index;
    ind->data.type = type;
    return ind;
}

Indication* make_ind_let(const char* name, struct ASTType* type, bool constant, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_LET;
    ind->line = line; ind->column = col;
    ind->data.name = name;
    ind->data.type = type;
    ind->data.constant = constant;
    return ind;
}

Indication* make_ind_assign(const char* name, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_ASSIGN;
    ind->line = line; ind->column = col;
    ind->data.name = name;
    return ind;
}

Indication* make_ind_if(int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_IF;
    ind->line = line; ind->column = col;
    return ind;
}

Indication* make_ind_else(int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_ELSE;
    ind->line = line; ind->column = col;
    return ind;
}

Indication* make_ind_endif(int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_ENDIF;
    ind->line = line; ind->column = col;
    return ind;
}

Indication* make_ind_match(int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_MATCH;
    ind->line = line; ind->column = col;
    return ind;
}

Indication* make_ind_case(int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_CASE;
    ind->line = line; ind->column = col;
    return ind;
}

Indication* make_ind_endmatch(int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_ENDMATCH;
    ind->line = line; ind->column = col;
    return ind;
}

Indication* make_ind_loop(int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_LOOP;
    ind->line = line; ind->column = col;
    return ind;
}

Indication* make_ind_break(int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_BREAK;
    ind->line = line; ind->column = col;
    return ind;
}

Indication* make_ind_endloop(int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_ENDLOOP;
    ind->line = line; ind->column = col;
    return ind;
}

Indication* make_ind_return(int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_RETURN;
    ind->line = line; ind->column = col;
    return ind;
}

Indication* make_ind_literal_int(int value, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_LITERAL;
    ind->line = line; ind->column = col;
    ind->data.litKind = LIT_INT;
    ind->data.intValue = value;
    return ind;
}

Indication* make_ind_literal_float(double value, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_LITERAL;
    ind->line = line; ind->column = col;
    ind->data.litKind = LIT_FLOAT;
    ind->data.floatValue = value;
    return ind;
}

Indication* make_ind_literal_string(const char* value, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_LITERAL;
    ind->line = line; ind->column = col;
    ind->data.litKind = LIT_STRING;
    ind->data.stringValue = value;
    return ind;
}

Indication* make_ind_literal_bool(int value, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_LITERAL;
    ind->line = line; ind->column = col;
    ind->data.litKind = LIT_BOOL;
    ind->data.boolValue = value;
    return ind;
}

Indication* make_ind_var_ref(const char* name, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_VAR_REF;
    ind->line = line; ind->column = col;
    ind->data.name = name;
    return ind;
}

Indication* make_ind_arg_ref(int index, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_ARG_REF;
    ind->line = line; ind->column = col;
    ind->data.index = index;
    return ind;
}

Indication* make_ind_constructor(const char* name, int arity, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_CONSTRUCTOR;
    ind->line = line; ind->column = col;
    ind->data.name = name;
    ind->data.arity = arity;
    return ind;
}

Indication* make_ind_lambda(int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_LAMBDA;
    ind->line = line; ind->column = col;
    return ind;
}

Indication* make_ind_apply(int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_APPLY;
    ind->line = line; ind->column = col;
    return ind;
}

Indication* make_ind_call(const char* name, int arity, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_CALL;
    ind->line = line; ind->column = col;
    ind->data.name = name;
    ind->data.arity = arity;
    return ind;
}

Indication* make_ind_primitive(PrimitiveOpInd op, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_PRIMITIVE;
    ind->line = line; ind->column = col;
    ind->data.primOp = op;
    return ind;
}

Indication* make_ind_block(Indication* inner, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_BLOCK;
    ind->line = line; ind->column = col;
    ind->data.inner = inner;
    return ind;
}

Indication* make_ind_label(const char* label, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_LABEL;
    ind->line = line; ind->column = col;
    ind->data.label = label;
    return ind;
}

Indication* make_ind_jump(const char* label, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_JUMP;
    ind->line = line; ind->column = col;
    ind->data.label = label;
    return ind;
}

Indication* make_ind_jump_if_false(const char* label, int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_JUMP_IF_FALSE;
    ind->line = line; ind->column = col;
    ind->data.label = label;
    return ind;
}

Indication* make_ind_nop(int line, int col) {
    Indication* ind = (Indication*)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_NOP;
    ind->line = line; ind->column = col;
    return ind;
}




#endif