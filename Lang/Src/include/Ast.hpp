#ifndef AST_HPP
#define AST_HPP
#include "Lexer.hpp"  // Asumo que TokenList y Token están definidos aquí
#include <stdlib.h>
#include <string.h>

// ======================== Enumeración de tipos de nodo ========================
typedef enum {
    AST_Program,
    // Flags
    AST_Flag_Module,
    AST_Flag_Import,
    AST_Flag_Export,
    // Macro
    AST_Include,
    // Declaraciones
    AST_Decl_Func,
    AST_Decl_Data,
    AST_Decl_Type,
    AST_Decl_Enum,
    // Statements
    AST_Stmt_Block,
    AST_Stmt_Let,
    AST_Stmt_Assign,
    AST_Stmt_If,
    AST_Stmt_Match,
    AST_Stmt_Expr,
    AST_Stmt_Loop,
    AST_Stmt_Break,
    // Expresiones
    AST_Expr_Literal,
    AST_Expr_Var,
    AST_Expr_ArgRef,
    AST_Expr_Binary,
    AST_Expr_Apply,
    AST_Expr_Unary,
    AST_Expr_Call,
    AST_Expr_If,
    AST_Expr_Match,
    AST_Expr_Block,
    AST_Expr_Lambda,
    AST_Expr_Input,
    AST_Expr_Unit,
    // Patrones
    AST_Pat_Literal,
    AST_Pat_Var,
    AST_Pat_WildCard,
    AST_Pat_Init,
    // Tipos
    AST_Type_Named,
    AST_Type_Function,
    AST_Type_Primitive,
    AST_Type_Generic,
    AST_Type_Unit
} ASTNodeKind;

// ======================== Nodo base ========================
typedef struct {
    ASTNodeKind kind;
    int line;
    int column;
} ASTNode;

// Macro helper (usando enum normal, no enum class)
#define NEW_AST(type, kindValue) \
    type* node = (type*)malloc(sizeof(type)); \
    node->base.kind = kindValue;

// ======================== Forward declarations ========================
typedef struct ASTType ASTType;
typedef struct ASTPattern ASTPattern;
typedef struct ASTExpr ASTExpr;
typedef struct ASTStmt ASTStmt;
typedef struct ASTTopLevel ASTTopLevel;

// ======================== Tipos ========================
struct ASTType {
    ASTNode base;
};

typedef struct {
    ASTNode base;
    char* name;
    ASTType** typeArgs;
    int typeArgsCount;
} ASTTypeNamed;

typedef struct {
    ASTNode base;
    ASTType** paramTypes;
    int paramCount;
    ASTType* returnType;
} ASTTypeFunction;

typedef enum {
    PRIM_NUMBER,
    PRIM_STRING,
    PRIM_BOOL,
    PRIM_ANY,
    PRIM_VOID,
    PRIM_NULL,   
    PRIM_NOTHING,
} PrimitiveType;

typedef struct {
    ASTNode base;
    PrimitiveType primitiveKind; // 0=Number, 1=String, etc. O usar string
} ASTTypePrimitive;
typedef enum{
    Gen_Maybe,
}GenericType;
typedef struct {
    ASTNode base;
    char* name; // ej: 'a
} ASTTypeGeneric;

typedef struct {
    ASTNode base;
} ASTTypeUnit;

// Funciones constructoras de tipos
ASTTypeNamed* build_type_named(const char* name, ASTType** typeArgs, int count, int line, int col) {
    NEW_AST(ASTTypeNamed, AST_Type_Named);
    node->base.line = line; node->base.column = col;
    node->name = strdup(name);
    node->typeArgs = typeArgs;
    node->typeArgsCount = count;
    return node;
}

ASTTypeFunction* build_type_function(ASTType** params, int paramCount, ASTType* ret, int line, int col) {
    NEW_AST(ASTTypeFunction, AST_Type_Function);
    node->base.line = line; node->base.column = col;
    node->paramTypes = params;
    node->paramCount = paramCount;
    node->returnType = ret;
    return node;
}

ASTTypePrimitive* build_type_primitive(PrimitiveType primKind, int line, int col) {
    NEW_AST(ASTTypePrimitive, AST_Type_Primitive);
    node->base.line = line; node->base.column = col;
    node->primitiveKind = primKind;
    return node;
}

ASTTypeGeneric* build_type_generic(const char* name, int line, int col) {
    NEW_AST(ASTTypeGeneric, AST_Type_Generic);
    node->base.line = line; node->base.column = col;
    node->name = strdup(name);
    return node;
}

ASTTypeUnit* build_type_unit(int line, int col) {
    NEW_AST(ASTTypeUnit, AST_Type_Unit);
    node->base.line = line; node->base.column = col;
    return node;
}

// ======================== Patrones ========================
struct ASTPattern {
    ASTNode base;
};

typedef struct {
    ASTNode base;
    ASTExpr* literal; // LiteralExpr
} ASTPatLiteral;

typedef struct {
    ASTNode base;
    char* name;
} ASTPatVar;

typedef struct {
    ASTNode base;
} ASTPatWildCard;

typedef struct {
    ASTNode base;
    char* constructorName;
    ASTPattern** subpatterns;
    int subpatternCount;
} ASTPatInit;

// Constructores de patrones
ASTPatLiteral* build_pat_literal(ASTExpr* lit, int line, int col) {
    NEW_AST(ASTPatLiteral, AST_Pat_Literal);
    node->base.line = line; node->base.column = col;
    node->literal = lit;
    return node;
}

ASTPatVar* build_pat_var(const char* name, int line, int col) {
    NEW_AST(ASTPatVar, AST_Pat_Var);
    node->base.line = line; node->base.column = col;
    node->name = strdup(name);
    return node;
}

ASTPatWildCard* build_pat_wildcard(int line, int col) {
    NEW_AST(ASTPatWildCard, AST_Pat_WildCard);
    node->base.line = line; node->base.column = col;
    return node;
}

ASTPatInit* build_pat_init(const char* ctor, ASTPattern** subs, int count, int line, int col) {
    NEW_AST(ASTPatInit, AST_Pat_Init);
    node->base.line = line; node->base.column = col;
    node->constructorName = strdup(ctor);
    node->subpatterns = subs;
    node->subpatternCount = count;
    return node;
}

// ======================== Expresiones ========================
struct ASTExpr {
    ASTNode base;
};

typedef enum { LIT_INT, LIT_FLOAT, LIT_STRING, LIT_BOOL } LiteralKind;

typedef struct {
    ASTNode base;
    LiteralKind kind;
    char* value;
} ASTExprLiteral;

typedef struct {
    ASTNode base;
    char* name;
} ASTExprVar;

typedef struct {
    ASTNode base;
    int index;
} ASTExprArgRef;

typedef enum { 
    UNARY_NEG, //-
    UNARY_NOT // not
} UnaryOp;

typedef struct {
    ASTNode base;
    UnaryOp op;
    ASTExpr* operand;
} ASTExprUnary;

typedef enum {
    BIN_ADD, //+
    BIN_SUB, //-
    BIN_MUL, //*
    BIN_DIV, ///
    BIN_MOD, //%
    BIN_EQ, //-EQ
    BIN_NEQ, //-NE
    BIN_LT,  //-LT
    BIN_GT, //-GT
    BIN_LE, //-LE
    BIN_GE, //-GE
    BIN_AND, //and
    BIN_OR //or
} BinaryOp;

typedef struct {
    ASTNode base;
    BinaryOp op;
    ASTExpr* left;
    ASTExpr* right;
} ASTExprBinary;

typedef struct {
    ASTNode base;
    char* functionName;
    ASTExpr** arguments;
    int argCount;
} ASTExprCall;

typedef struct {
    ASTNode base;
    ASTExpr* condition;
    ASTExpr* thenExpr;
    ASTExpr* elseExpr;
} ASTExprIf;

typedef struct {
    ASTPattern* pattern;
    ASTExpr* body;
} MatchCase;

typedef struct {
    ASTNode base;
    ASTExpr* matchedExpr;
    MatchCase* cases;
    int caseCount;
} ASTExprMatch;

typedef struct {
    ASTNode base;
    ASTStmt** statements;
    int stmtCount;
} ASTStmtBlock;


typedef struct {
    ASTNode base;
    ASTExpr* doExpr;        // nullable
    ASTStmtBlock* WhereBlock;
    ASTExpr* finalExpr;     // nullable
} ASTExprBlock;

typedef struct {
    ASTNode base;
    ASTExpr* body;
} ASTExprLambda;

typedef struct {
    ASTNode base;
    ASTExpr* func;
    ASTExpr* arg;
} ASTExprApply;

typedef struct {
    ASTNode base;
    ASTExpr* prompt; 
} ASTExprInput;

// Constructores de expresiones

ASTExprApply* build_expr_apply(ASTExpr* func, ASTExpr* arg, int line, int col) {
    NEW_AST(ASTExprApply, AST_Expr_Apply);
    node->base.line = line; node->base.column = col;
    node->func = func;
    node->arg = arg;
    return node;
}

ASTExprLiteral* build_expr_literal(LiteralKind kind, const char* value, int line, int col) {
    NEW_AST(ASTExprLiteral, AST_Expr_Literal);
    node->base.line = line; node->base.column = col;
    node->kind = kind;
    node->value = strdup(value);
    return node;
}

ASTExprVar* build_expr_var(const char* name, int line, int col) {
    NEW_AST(ASTExprVar, AST_Expr_Var);
    node->base.line = line; node->base.column = col;
    node->name = strdup(name);
    return node;
}

ASTExprArgRef* build_expr_argref(int index, int line, int col) {
    NEW_AST(ASTExprArgRef, AST_Expr_ArgRef);
    node->base.line = line; node->base.column = col;
    node->index = index;
    return node;
}

ASTExprUnary* build_expr_unary(UnaryOp op, ASTExpr* operand, int line, int col) {
    NEW_AST(ASTExprUnary, AST_Expr_Unary);
    node->base.line = line; node->base.column = col;
    node->op = op;
    node->operand = operand;
    return node;
}

ASTExprBinary* build_expr_binary(BinaryOp op, ASTExpr* left, ASTExpr* right, int line, int col) {
    NEW_AST(ASTExprBinary, AST_Expr_Binary);
    node->base.line = line; node->base.column = col;
    node->op = op;
    node->left = left;
    node->right = right;
    return node;
}

ASTExprCall* build_expr_call(const char* name, ASTExpr** args, int argCount, int line, int col) {
    NEW_AST(ASTExprCall, AST_Expr_Call);
    node->base.line = line; node->base.column = col;
    node->functionName = strdup(name);
    node->arguments = args;
    node->argCount = argCount;
    return node;
}

ASTExprIf* build_expr_if(ASTExpr* cond, ASTExpr* thenE, ASTExpr* elseE, int line, int col) {
    NEW_AST(ASTExprIf, AST_Expr_If);
    node->base.line = line; node->base.column = col;
    node->condition = cond;
    node->thenExpr = thenE;
    node->elseExpr = elseE;
    return node;
}

ASTExprMatch* build_expr_match(ASTExpr* matched, MatchCase* cases, int caseCount, int line, int col) {
    NEW_AST(ASTExprMatch, AST_Expr_Match);
    node->base.line = line; node->base.column = col;
    node->matchedExpr = matched;
    node->caseCount = caseCount;
    // ¡Aquí debe copiar el array!
    if (caseCount > 0 && cases) {
        node->cases = (MatchCase*)malloc(sizeof(MatchCase) * caseCount);
        memcpy(node->cases, cases, sizeof(MatchCase) * caseCount);
    } else {
        node->cases = nullptr;
    }
    return node;
}

ASTExprBlock* build_expr_block(ASTExpr* doE, ASTStmtBlock* WhereBlock, ASTExpr* finalE, int line, int col) {
    NEW_AST(ASTExprBlock, AST_Expr_Block);
    node->base.line = line; node->base.column = col;
    node->doExpr = doE;
    node->WhereBlock=WhereBlock;
    node->finalExpr = finalE;
    return node;
}

ASTExprLambda* build_expr_lambda(ASTExpr* body, int line, int col) {
    NEW_AST(ASTExprLambda, AST_Expr_Lambda);
    node->base.line = line; node->base.column = col;
    node->body = body;
    return node;
}

ASTExprInput* build_expr_input(ASTExpr* prompt, int line, int col) {
    NEW_AST(ASTExprInput, AST_Expr_Input);
    node->base.line = line; node->base.column = col;
    node->prompt = prompt;
    return node;
}

// ======================== Statements ========================
struct ASTStmt {
    ASTNode base;
};

typedef struct {
    ASTNode base;
    char* name;
    ASTType* type;
    bool constant;
    ASTExpr* initializer;
} ASTStmtLet;

typedef struct {
    ASTNode base;
    ASTExpr* condition;
    ASTStmt** thenBranch;
    int thenCount;
    ASTStmt** elseBranch; // puede ser NULL
    int elseCount;
} ASTStmtIf;

typedef struct {
    ASTNode base;
    ASTExpr* matchedExpr;
    MatchCase* cases;
    int caseCount;
} ASTStmtMatch;

typedef struct {
    ASTNode base;
    ASTExpr* expr;
} ASTStmtExpr;

typedef struct {
    ASTNode base;
    ASTStmt** body;
    int bodyCount;
} ASTStmtLoop;

typedef struct {
    ASTNode base;
} ASTStmtBreak;

typedef enum {
    ASSIGN_PLAIN,   // =
    ASSIGN_PLUS,    // +=
    ASSIGN_MINUS,   // -=
    ASSIGN_MULT,    // *=
    ASSIGN_DIV      // /=
} AssignOp;

typedef struct {
    ASTNode base;
    char* var;          
    AssignOp op;        
    ASTExpr* rhs;       
} ASTStmtAssign;

// Constructores de statements

ASTStmtBlock* build_stmt_block(ASTStmt** stmts, int count, int line, int col) {
    NEW_AST(ASTStmtBlock, AST_Stmt_Block);
    node->base.line = line; node->base.column = col;
    node->statements = stmts;
    node->stmtCount = count;
    return node;
}

ASTStmtAssign* build_stmt_assign(const char* var, AssignOp op, ASTExpr* rhs, int line, int col) {
    NEW_AST(ASTStmtAssign, AST_Stmt_Assign);
    node->base.line = line; node->base.column = col;
    node->var = strdup(var);
    node->op = op;
    node->rhs = rhs;
    return node;
}

ASTStmtLet* build_stmt_let(const char* name, ASTType* type,bool constant, ASTExpr* init, int line, int col) {
    NEW_AST(ASTStmtLet, AST_Stmt_Let);
    node->base.line = line; node->base.column = col;
    node->name = strdup(name);
    node->type = type;
    node->constant=constant;
    node->initializer = init;
    return node;
}

ASTStmtIf* build_stmt_if(ASTExpr* cond, ASTStmt** thenStmts, int thenCnt, ASTStmt** elseStmts, int elseCnt, int line, int col) {
    NEW_AST(ASTStmtIf, AST_Stmt_If);
    node->base.line = line; node->base.column = col;
    node->condition = cond;
    node->thenBranch = thenStmts;
    node->thenCount = thenCnt;
    node->elseBranch = elseStmts;
    node->elseCount = elseCnt;
    return node;
}

ASTStmtMatch* build_stmt_match(ASTExpr* matched, MatchCase* cases, int caseCount, int line, int col) {
    NEW_AST(ASTStmtMatch, AST_Stmt_Match);
    node->base.line = line; node->base.column = col;
    node->matchedExpr = matched;
    node->cases = cases;
    node->caseCount = caseCount;
    return node;
}

ASTStmtExpr* build_stmt_expr(ASTExpr* expr, int line, int col) {
    NEW_AST(ASTStmtExpr, AST_Stmt_Expr);
    node->base.line = line; node->base.column = col;
    node->expr = expr;
    return node;
}

ASTStmtLoop* build_stmt_loop(ASTStmt** body, int bodyCount, int line, int col) {
    NEW_AST(ASTStmtLoop, AST_Stmt_Loop);
    node->base.line = line; node->base.column = col;
    node->body = body;
    node->bodyCount = bodyCount;
    return node;
}

ASTStmtBreak* build_stmt_break(int line, int col) {
    NEW_AST(ASTStmtBreak, AST_Stmt_Break);
    node->base.line = line; node->base.column = col;
    return node;
}

// ======================== Declaraciones Top-Level ========================
struct ASTTopLevel {
    ASTNode base;
};

typedef struct {
    ASTNode base;
    char* name;
    ASTType* funcType;
    ASTExpr* body;
} ASTDeclFunc;

typedef struct {
    char* name;
    ASTType* type;
} Field;

typedef struct {
    ASTNode base;
    char* name;
    Field* fields;
    int fieldCount;
} ASTDeclData;

typedef struct {
    ASTNode base;
    char* name;
    ASTType* aliasedType;
} ASTDeclType;

typedef struct {
    char* name;
    ASTType** paramTypes; 
    int paramCount;
} EnumConstructor;

typedef struct {
    ASTNode base;
    char* name;
    EnumConstructor* constructors;
    int constructorCount;
} ASTDeclEnum;

// Constructores
ASTDeclEnum* build_decl_enum(const char* name, EnumConstructor* ctors, int count, int line, int col) {
    NEW_AST(ASTDeclEnum, AST_Decl_Enum);
    node->base.line = line; node->base.column = col;
    node->name = strdup(name);
    node->constructors = ctors;
    node->constructorCount = count;
    return node;
}

ASTDeclFunc* build_decl_func(const char* name, ASTType* funcType, ASTExpr* body, int line, int col) {
    NEW_AST(ASTDeclFunc, AST_Decl_Func);
    node->base.line = line; node->base.column = col;
    node->name = strdup(name);
    node->funcType = funcType;
    node->body = body;
    return node;
}

ASTDeclData* build_decl_data(const char* name, Field* fields, int count, int line, int col) {
    NEW_AST(ASTDeclData, AST_Decl_Data);
    node->base.line = line; node->base.column = col;
    node->name = strdup(name);
    node->fields = fields;
    node->fieldCount = count;
    return node;
}

ASTDeclType* build_decl_type(const char* name, ASTType* aliased, int line, int col) {
    NEW_AST(ASTDeclType, AST_Decl_Type);
    node->base.line = line; node->base.column = col;
    node->name = strdup(name);
    node->aliasedType = aliased;
    return node;
}

// ======================== Directivas ========================
typedef struct {
    ASTNode base;
    char* moduleName;
} ASTFlagModule;

typedef struct {
    ASTNode base;
    const char* importName;
    bool as;
    const char* asName;
} ASTFlagImport;

typedef struct {
    ASTNode base;
    const char** names;
    int count;
} ASTFlagExport;

typedef struct {
    ASTNode base;
    char* pragma;
} ASTInclude;

ASTFlagModule* build_flag_module(const char* name, int line, int col) {
    NEW_AST(ASTFlagModule, AST_Flag_Module);
    node->base.line = line; node->base.column = col;
    node->moduleName = strdup(name);
    return node;
}

ASTFlagImport* build_flag_import(const char* name, bool as,const char* asName, int line, int col) {
    NEW_AST(ASTFlagImport, AST_Flag_Import);
    node->base.line = line; node->base.column = col;
    node->importName = strdup(name);
    node->as=as;
    node->asName=asName;
    return node;
}

ASTFlagExport* build_flag_export(const char** names, int count, int line, int col){ 
    NEW_AST(ASTFlagExport, AST_Flag_Export);
    node->base.line = line; node->base.column = col;
    node->names = names;
    node->count = count;
    return node;
}

ASTInclude* build_include(const char* pragma, int line, int col) {
    NEW_AST(ASTInclude, AST_Include);
    node->base.line = line; node->base.column = col;
    node->pragma = strdup(pragma);
    return node;
}

// ======================== Programa ========================
typedef struct {
    ASTNode base;
    char* moduleName; // del @module, puede ser NULL
    ASTTopLevel** declarations;
    int declCount;
} ASTProgram;

ASTProgram* build_program(const char* moduleName, ASTTopLevel** decls, int count, int line, int col) {
    NEW_AST(ASTProgram, AST_Program);
    node->base.line = line; node->base.column = col;
    node->moduleName = moduleName ? strdup(moduleName) : NULL;
    node->declarations = decls;
    node->declCount = count;
    return node;
}

typedef struct {
    ASTNode base;
} ASTExprUnit;

ASTExprUnit* build_expr_unit(int line, int col) {
    NEW_AST(ASTExprUnit, AST_Expr_Unit);
    node->base.line = line; node->base.column = col;
    return node;
}
#endif