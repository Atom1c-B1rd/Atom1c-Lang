#ifndef ANALYZER_HPP
#define ANALYZER_HPP

#include "AST.hpp"
#include "Indications.hpp"
#include <vector>
#include <cstdlib>
#include <cstring>
#include <string>
// ======================== Depth Guard ========================
struct AnalysisDepthGuard
{
    static int depth;
    const char *funcName;
    int kind;

    AnalysisDepthGuard(const char *fn, int k) : funcName(fn), kind(k)
    {
        ++depth;
        std::cout << ">>> " << funcName << " kind=" << kind << " depth=" << depth << std::endl;
        if (depth > 500)
        {
            std::cerr << "ERROR: Máxima profundidad de análisis alcanzada\n";
            exit(1);
        }
    }
    ~AnalysisDepthGuard()
    {
        std::cout << "<<< " << funcName << " kind=" << kind << " depth=" << depth << std::endl;
        --depth;
    }
};
int AnalysisDepthGuard::depth = 0;

#define ANALYZE_GUARD(fn, kind) AnalysisDepthGuard __guard(fn, kind)

// ======================== Helper para lista de indicaciones ========================
void PrintIndication(Indication *ind, int indent = 0);

class IndicationList
{
    std::vector<Indication *> inds;

public:
    Indication *First() const
    {
        return inds.empty() ? nullptr : inds[0];
    }
    void Add(Indication *ind)
    {
        if (ind)
            inds.push_back(ind);
    }

    size_t size() const { return inds.size(); } // <-- añadir

    void AppendList(Indication *head)
    {
        for (Indication *p = head; p; p = p->next)
            inds.push_back(p);
    }

    void AppendList(const IndicationList &other)
    {
        for (Indication *ind : other.inds)
        {
            if (ind)
                inds.push_back(ind);
        }
    }

    Indication *Head()
    {
        if (inds.empty())
            return nullptr;
        for (size_t i = 0; i < inds.size() - 1; ++i)
            inds[i]->next = inds[i + 1];
        inds.back()->next = nullptr;

        return inds.front();
    }

    bool Empty() const { return inds.empty(); }
    void DebugPrint() const
    {
        for (size_t i = 0; i < inds.size(); ++i)
        {
            std::cout << "[" << i << "] ";
            PrintIndication(inds[i]);
        }
    }
};

// ======================== Clase Analyzer ========================
class Analyzer
{
public:
    IndicationList Analyze(ASTProgram *prog);
    IndicationList Analyze(ASTTopLevel *decl);
    IndicationList Analyze(ASTStmt *stmt);
    IndicationList Analyze(ASTExpr *expr);
    IndicationList Analyze(ASTFlagModule *flag);
    IndicationList Analyze(ASTFlagImport *flag);
    IndicationList Analyze(ASTFlagExport *flag);
    IndicationList Analyze(ASTInclude *incl);

private:
    static void ExtractFuncParams(ASTType *type,
                                  ASTType ***outParams, int *outCount,
                                  ASTType **outReturn);

    static Indication *make_case_with_pattern(ASTPattern *pat, int line, int col);
};

// ======================== Implementación de funciones auxiliares ========================
inline void Analyzer::ExtractFuncParams(ASTType *type,
                                        ASTType ***outParams, int *outCount,
                                        ASTType **outReturn)
{
    if (type->base.kind == AST_Type_Function)
    {
        ASTTypeFunction *ftype = (ASTTypeFunction *)type;
        *outParams = ftype->paramTypes;
        *outCount = ftype->paramCount;
        *outReturn = ftype->returnType;
    }
    else
    {
        *outParams = nullptr;
        *outCount = 0;
        *outReturn = type;
    }
}

inline Indication *Analyzer::make_case_with_pattern(ASTPattern *pat, int line, int col)
{
    Indication *ind = (Indication *)malloc(sizeof(Indication));
    memset(ind, 0, sizeof(*ind));
    ind->kind = IND_CASE;
    ind->line = line;
    ind->column = col;
    ind->data.pattern = pat;
    return ind;
}

// ======================== Análisis de Flags e Include ========================
inline IndicationList Analyzer::Analyze(ASTFlagModule *flag)
{
    IndicationList list;
    list.Add(make_ind_module(flag->moduleName, flag->base.line, flag->base.column));
    return list;
}

inline IndicationList Analyzer::Analyze(ASTFlagImport *flag)
{
    IndicationList list;
    list.Add(make_ind_import(flag->importName,flag->as,flag->asName ,flag->base.line,flag->base.column));
    return list;
}

inline IndicationList Analyzer::Analyze(ASTFlagExport *flag)
{
    IndicationList list;
    list.Add(make_ind_export(flag->names, flag->count, flag->base.line, flag->base.column));
    return list;
}

inline IndicationList Analyzer::Analyze(ASTInclude *incl)
{
    IndicationList list;
    list.Add(make_ind_include(incl->pragma, incl->base.line, incl->base.column));
    return list;
}

// ======================== Análisis de Programa ========================
/*
inline IndicationList Analyzer::Analyze(ASTProgram *prog)
{
    IndicationList list;
    if (prog->moduleName)
    {
        list.Add(make_ind_module(prog->moduleName, prog->base.line, prog->base.column));
    }
    for (int i = 0; i < prog->declCount; ++i)
    {
        list.AppendList(Analyze(prog->declarations[i]));
    }
    return list;
}
*/
// En Analyzer::Analyze(ASTProgram* prog)
inline IndicationList Analyzer::Analyze(ASTProgram *prog)
{
    ANALYZE_GUARD("Program", 0);
    IndicationList list;
    if (prog->moduleName)
    {
        list.Add(make_ind_module(prog->moduleName, prog->base.line, prog->base.column));
    }
    std::cout << "[DEBUG] Analizando " << prog->declCount << " declaraciones top-level\n";
    for (int i = 0; i < prog->declCount; ++i)
    {
        ASTTopLevel *decl = prog->declarations[i];
        std::cout << "[DEBUG]   Decl " << i << " kind = " << (int)decl->base.kind << "\n";
        if (decl->base.kind == AST_Decl_Func)
        {
            ASTDeclFunc *f = (ASTDeclFunc *)decl;
            std::cout << "[DEBUG]     Funcion: " << f->name << "\n";
        }
        IndicationList sub = Analyze(decl);
        std::cout << "[DEBUG]   Sub-indicaciones: " << sub.size() << "\n";
        list.AppendList(sub);
    }
    std::cout << "[DEBUG] Total indicaciones: " << list.size() << "\n";
    return list;
}
// ======================== Análisis de Declaraciones Top-Level ========================
inline IndicationList Analyzer::Analyze(ASTTopLevel *decl)
{
    ANALYZE_GUARD("TopLevel", (int)decl->base.kind);
    switch (decl->base.kind)
    {
    case AST_Decl_Func:
    {
        auto *d = (ASTDeclFunc *)decl;
        IndicationList list;
        int line = d->base.line, col = d->base.column;
        list.Add(make_ind_func_begin(d->name, d->funcType, line, col));

        ASTType **params;
        int paramCount;
        ASTType *retType;
        ExtractFuncParams(d->funcType, &params, &paramCount, &retType);
        for (int i = 0; i < paramCount; ++i)
        {
            list.Add(make_ind_param(i, params[i], line, col));
        }

        list.AppendList(Analyze(d->body));
        list.Add(make_ind_func_end(d->name, line, col));
        return list;
    }
    case AST_Decl_Data:
    {
        auto *d = (ASTDeclData *)decl;
        IndicationList list;
        int line = d->base.line, col = d->base.column;
        list.Add(make_ind_type_data_begin(d->name, line, col));
        for (int i = 0; i < d->fieldCount; ++i)
        {
            list.Add(make_ind_field(d->fields[i].name, d->fields[i].type, line, col));
        }
        list.Add(make_ind_type_end(line, col));
        return list;
    }
    case AST_Decl_Enum:
    {
        auto *d = (ASTDeclEnum *)decl;
        IndicationList list;
        int line = d->base.line, col = d->base.column;
        list.Add(make_ind_type_enum_begin(d->name, line, col));
        for (int i = 0; i < d->constructorCount; ++i)
        {
            EnumConstructor &ctor = d->constructors[i];
            list.Add(make_ind_ctor(ctor.name, ctor.paramTypes, ctor.paramCount, line, col));
        }
        list.Add(make_ind_type_end(line, col));
        return list;
    }
    case AST_Decl_Type:
    {
        auto *d = (ASTDeclType *)decl;
        IndicationList list;
        list.Add(make_ind_type_alias(d->name, d->aliasedType, d->base.line, d->base.column));
        return list;
    }
    default:
        return IndicationList();
    }
}

// ======================== Análisis de Statements ========================
inline IndicationList Analyzer::Analyze(ASTStmt *stmt)
{
    ANALYZE_GUARD("Stmt", (int)stmt->base.kind);
    switch (stmt->base.kind)
    {
    case AST_Stmt_Block:
    {
        auto *s = (ASTStmtBlock *)stmt;
        std::cout << ">>> Block: Stmt=" << stmt->base.kind << std::endl;
        IndicationList list;
        for (int i = 0; i < s->stmtCount; ++i)
        {

            list.AppendList(Analyze(s->statements[i]));
        }

        return list;
    }
    case AST_Stmt_Let:
    {
        auto *s = (ASTStmtLet *)stmt;
        IndicationList list;
        if (s->initializer)
            list.AppendList(Analyze(s->initializer));
        list.Add(make_ind_let(s->name, s->type, s->constant, s->base.line, s->base.column));
        return list;
    }
    case AST_Stmt_Assign:
    {
        auto *s = (ASTStmtAssign *)stmt;
        IndicationList list;
        list.AppendList(Analyze(s->rhs));
        int line = s->base.line, col = s->base.column;
        if (s->op != ASSIGN_PLAIN)
        {
            list.Add(make_ind_var_ref(s->var, line, col));
            PrimitiveOpInd op;
            switch (s->op)
            {
            case ASSIGN_PLUS:
                op = PRIM_ADD;
                break;
            case ASSIGN_MINUS:
                op = PRIM_SUB;
                break;
            case ASSIGN_MULT:
                op = PRIM_MUL;
                break;
            case ASSIGN_DIV:
                op = PRIM_DIV;
                break;
            default:
                op = PRIM_ADD;
            }
            list.Add(make_ind_primitive(op, line, col));
        }
        list.Add(make_ind_assign(s->var, line, col));
        return list;
    }
    case AST_Stmt_If:
    {
        auto *s = (ASTStmtIf *)stmt;
        IndicationList list;
        list.AppendList(Analyze(s->condition));
        int line = s->base.line, col = s->base.column;
        list.Add(make_ind_if(line, col));
        for (int i = 0; i < s->thenCount; ++i)
            list.AppendList(Analyze(s->thenBranch[i]));
        if (s->elseCount > 0)
        {
            list.Add(make_ind_else(line, col));
            for (int i = 0; i < s->elseCount; ++i)
                list.AppendList(Analyze(s->elseBranch[i]));
        }
        list.Add(make_ind_endif(line, col));
        return list;
    }
    case AST_Stmt_Match:
    {
        auto *s = (ASTStmtMatch *)stmt;
        IndicationList list;
        list.AppendList(Analyze(s->matchedExpr));
        int line = s->base.line, col = s->base.column;
        list.Add(make_ind_match(line, col));
        for (int i = 0; i < s->caseCount; ++i)
        {
            list.Add(make_case_with_pattern(s->cases[i].pattern, line, col));
            list.AppendList(Analyze(s->cases[i].body));
        }
        list.Add(make_ind_endmatch(line, col));
        return list;
    }
    case AST_Stmt_Expr:
    {
        auto *s = (ASTStmtExpr *)stmt;
        return Analyze(s->expr);
    }
    case AST_Stmt_Loop:
    {
        auto *s = (ASTStmtLoop *)stmt;
        IndicationList list;
        int line = s->base.line, col = s->base.column;
        list.Add(make_ind_loop(line, col));
        for (int i = 0; i < s->bodyCount; ++i)
            list.AppendList(Analyze(s->body[i]));
        list.Add(make_ind_endloop(line, col));
        return list;
    }
    case AST_Stmt_Break:
    {
        auto *s = (ASTStmtBreak *)stmt;
        IndicationList list;
        list.Add(make_ind_break(s->base.line, s->base.column));
        return list;
    }
    default:
        return IndicationList();
    }
}

// ======================== Análisis de Expresiones ========================
inline IndicationList Analyzer::Analyze(ASTExpr *expr)
{
    std::cout << "!!! Entered Analyze(Expr) with kind=" << (int)expr->base.kind << std::endl;
    ANALYZE_GUARD("Expr", (int)expr->base.kind);

    // Literal
    if (expr->base.kind == AST_Expr_Literal)
    {
        auto *e = (ASTExprLiteral *)expr;
        IndicationList list;
        int line = e->base.line, col = e->base.column;
        switch (e->kind)
        {
        case LIT_INT:
            list.Add(make_ind_literal_int(std::atoi(e->value), line, col));
            break;
        case LIT_FLOAT:
            list.Add(make_ind_literal_float(std::atof(e->value), line, col));
            break;
        case LIT_STRING:
            list.Add(make_ind_literal_string(e->value, line, col));
            break;
        case LIT_BOOL:
            list.Add(make_ind_literal_bool(std::strcmp(e->value, "true") == 0 ? 1 : 0, line, col));
            break;
        }
        return list;
    }
    // Variable
    else if (expr->base.kind == AST_Expr_Var)
    {
        auto *e = (ASTExprVar *)expr;
        IndicationList list;
        list.Add(make_ind_var_ref(e->name, e->base.line, e->base.column));
        return list;
    }
    // Argumento $n
    else if (expr->base.kind == AST_Expr_ArgRef)
    {
        auto *e = (ASTExprArgRef *)expr;
        IndicationList list;
        list.Add(make_ind_arg_ref(e->index, e->base.line, e->base.column));
        return list;
    }
    // Unario
    else if (expr->base.kind == AST_Expr_Unary)
    {
        auto *e = (ASTExprUnary *)expr;
        IndicationList list;
        list.AppendList(Analyze(e->operand));
        PrimitiveOpInd op = (e->op == UNARY_NEG) ? PRIM_NEG : PRIM_NOT;
        list.Add(make_ind_primitive(op, e->base.line, e->base.column));
        return list;
    }
    // Binario
    else if (expr->base.kind == AST_Expr_Binary)
    {
        auto *e = (ASTExprBinary *)expr;
        IndicationList list;
        list.AppendList(Analyze(e->left));
        list.AppendList(Analyze(e->right));
        PrimitiveOpInd op;
        switch (e->op)
        {
        case BIN_ADD:
            op = PRIM_ADD;
            break;
        case BIN_SUB:
            op = PRIM_SUB;
            break;
        case BIN_MUL:
            op = PRIM_MUL;
            break;
        case BIN_DIV:
            op = PRIM_DIV;
            break;
        case BIN_MOD:
            op = PRIM_MOD;
            break;
        case BIN_EQ:
            op = PRIM_EQ;
            break;
        case BIN_NEQ:
            op = PRIM_NEQ;
            break;
        case BIN_LT:
            op = PRIM_LT;
            break;
        case BIN_GT:
            op = PRIM_GT;
            break;
        case BIN_LE:
            op = PRIM_LE;
            break;
        case BIN_GE:
            op = PRIM_GE;
            break;
        case BIN_AND:
            op = PRIM_AND;
            break;
        case BIN_OR:
            op = PRIM_OR;
            break;
        default:
            op = PRIM_ADD;
        }
        list.Add(make_ind_primitive(op, e->base.line, e->base.column));
        return list;
    }
    // Llamada a función <nombre $ ... />
    else if (expr->base.kind == AST_Expr_Call)
    {
        auto *e = (ASTExprCall *)expr;
        IndicationList list;
        for (int i = 0; i < e->argCount; ++i)
            list.AppendList(Analyze(e->arguments[i]));
        list.Add(make_ind_call(e->functionName, e->argCount, e->base.line, e->base.column));
        return list;
    }
    // Aplicación $
    else if (expr->base.kind == AST_Expr_Apply)
    {
        auto *e = (ASTExprApply *)expr;
        IndicationList list;
        list.AppendList(Analyze(e->func));
        list.AppendList(Analyze(e->arg));
        list.Add(make_ind_apply(e->base.line, e->base.column));
        return list;
    }
    // If expresivo
    else if (expr->base.kind == AST_Expr_If)
    {
        auto *e = (ASTExprIf *)expr;
        IndicationList list;
        list.AppendList(Analyze(e->condition));
        int line = e->base.line, col = e->base.column;
        list.Add(make_ind_if(line, col));
        list.AppendList(Analyze(e->thenExpr));
        list.Add(make_ind_else(line, col));
        list.AppendList(Analyze(e->elseExpr));
        list.Add(make_ind_endif(line, col));
        return list;
    }
    // Match expresivo
    else if (expr->base.kind == AST_Expr_Match)
    {
        auto *e = (ASTExprMatch *)expr;
        std::cout << "===== ENTRANDO A AST_Expr_Match =====" << std::endl;
        std::cout << "matchedExpr kind: " << (int)e->matchedExpr->base.kind << std::endl;
        std::cout << "caseCount: " << e->caseCount << std::endl;
        IndicationList list;
        try
        {
            list.AppendList(Analyze(e->matchedExpr));
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Excepción en matchedExpr: " << ex.what() << std::endl;
        }
        int line = e->base.line, col = e->base.column;
        list.Add(make_ind_match(line, col));
        std::cout << "Preparando para procesar " << e->caseCount << " casos\n";
        std::cout << "Dirección de e: " << (void *)e << ", caseCount: " << e->caseCount << std::endl;
        for (int i = 0; i < e->caseCount; ++i)
        {
            std::cout << "Procesando caso " << i
                      << " con cuerpo kind=" << (int)e->cases[i].body->base.kind << std::endl;
            list.Add(make_case_with_pattern(e->cases[i].pattern, line, col));
            IndicationList bodyList = Analyze(e->cases[i].body);
            std::cout << "Cuerpo analizado, generó " << bodyList.size() << " indicaciones" << std::endl;
            list.AppendList(bodyList);
        }
        list.Add(make_ind_endmatch(line, col));
        return list;
    }
    // Bloque do/where
    // Lambda ~>
    else if (expr->base.kind == AST_Expr_Lambda)
    {
        auto *e = (ASTExprLambda *)expr;
        IndicationList list;
        Indication *lam = make_ind_lambda(e->base.line, e->base.column);
        lam->data.inner = Analyze(e->body).First();
        list.Add(lam);
        return list;
    }
    // putStrLn
    // Input (<-)
    else if (expr->base.kind == AST_Expr_Input)
    {
        auto *e = (ASTExprInput *)expr;
        IndicationList list;
        list.AppendList(Analyze(e->prompt));
        list.Add(make_ind_primitive(PRIM_INPUT, e->base.line, e->base.column));
        return list;
    }
    else
    {
        std::cout << "!!! Unknown Expr kind: " << (int)expr->base.kind << std::endl;
        return IndicationList();
    }
}

#include <iostream>
#include <string>

inline const char *IndicationKindName(IndicationKind k)
{
    switch (k)
    {
    case IND_MODULE:
        return "MODULE";
    case IND_IMPORT:
        return "IMPORT";
    case IND_EXPORT:
        return "EXPORT";
    case IND_INCLUDE:
        return "INCLUDE";
    case IND_TYPE_ALIAS:
        return "TYPE_ALIAS";
    case IND_TYPE_DATA_BEGIN:
        return "DATA_BEGIN";
    case IND_TYPE_ENUM_BEGIN:
        return "ENUM_BEGIN";
    case IND_FIELD:
        return "FIELD";
    case IND_CTOR:
        return "CTOR";
    case IND_TYPE_END:
        return "TYPE_END";
    case IND_FUNC_BEGIN:
        return "FUNC_BEGIN";
    case IND_FUNC_END:
        return "FUNC_END";
    case IND_PARAM:
        return "PARAM";
    case IND_LET:
        return "LET";
    case IND_ASSIGN:
        return "ASSIGN";
    case IND_IF:
        return "IF";
    case IND_ELSE:
        return "ELSE";
    case IND_ENDIF:
        return "ENDIF";
    case IND_MATCH:
        return "MATCH";
    case IND_CASE:
        return "CASE";
    case IND_ENDMATCH:
        return "ENDMATCH";
    case IND_LOOP:
        return "LOOP";
    case IND_BREAK:
        return "BREAK";
    case IND_ENDLOOP:
        return "ENDLOOP";
    case IND_RETURN:
        return "RETURN";
    case IND_LITERAL:
        return "LITERAL";
    case IND_VAR_REF:
        return "VAR_REF";
    case IND_ARG_REF:
        return "ARG_REF";
    case IND_CONSTRUCTOR:
        return "CONSTRUCTOR";
    case IND_LAMBDA:
        return "LAMBDA";
    case IND_APPLY:
        return "APPLY";
    case IND_CALL:
        return "CALL";
    case IND_PRIMITIVE:
        return "PRIMITIVE";
    case IND_BLOCK:
        return "BLOCK";
    case IND_LABEL:
        return "LABEL";
    case IND_JUMP:
        return "JUMP";
    case IND_JUMP_IF_FALSE:
        return "JUMP_IF_FALSE";
    case IND_NOP:
        return "NOP";
    default:
        return "UNKNOWN";
    }
}

inline void PrintIndication(Indication *ind, int indent)
{
    if (!ind)
        return;
    std::string pad(indent * 2, ' ');
    std::cout << pad << "[" << IndicationKindName(ind->kind) << "]";
    switch (ind->kind)
    {
    case IND_MODULE:
    case IND_IMPORT:
    case IND_INCLUDE:
        std::cout << " " << (ind->data.name ? ind->data.name : "");
        break;
    case IND_EXPORT:
    {
        std::cout << " [";
        for (int i = 0; i < ind->data.stringCount; ++i)
        {
            if (i > 0)
                std::cout << ", ";
            std::cout << ind->data.stringList[i];
        }
        std::cout << "]";
        break;
    }
    case IND_TYPE_ALIAS:
    case IND_TYPE_DATA_BEGIN:
    case IND_TYPE_ENUM_BEGIN:
    case IND_FIELD:
    case IND_CTOR:
    case IND_FUNC_BEGIN:
    case IND_FUNC_END:
    case IND_VAR_REF:
    case IND_CONSTRUCTOR:
    case IND_CALL:
        std::cout << " " << (ind->data.name ? ind->data.name : "");
        if (ind->kind == IND_CALL || ind->kind == IND_CONSTRUCTOR)
            std::cout << "/" << ind->data.arity;
        break;
    case IND_PARAM:
        std::cout << " $" << ind->data.index;
        break;
    case IND_LET:
        std::cout << " " << (ind->data.name ? ind->data.name : "") << (ind->data.constant ? " (const)" : "");
        break;
    case IND_ASSIGN:
        std::cout << " " << (ind->data.name ? ind->data.name : "");
        break;
    case IND_LITERAL:
        std::cout << " ";
        switch (ind->data.litKind)
        {
        case LIT_INT:
            std::cout << ind->data.intValue;
            break;
        case LIT_FLOAT:
            std::cout << ind->data.floatValue;
            break;
        case LIT_STRING:
            std::cout << "\"" << ind->data.stringValue << "\"";
            break;
        case LIT_BOOL:
            std::cout << (ind->data.boolValue ? "true" : "false");
            break;
        }
        break;
    case IND_ARG_REF:
        std::cout << " $" << ind->data.index;
        break;
    case IND_PRIMITIVE:
    {
        static const char *primNames[] = {
            "ADD", "SUB", "MUL", "DIV", "MOD",
            "EQ", "NEQ", "LT", "GT", "LE", "GE",
            "AND", "OR", "NOT", "NEG",
            "PUTSTRLN", "TOSTRING", "TONUMBER", "INPUT"};
        std::cout << " " << primNames[ind->data.primOp];
        break;
    }
    case IND_LAMBDA:
    case IND_APPLY:
    case IND_RETURN:
    case IND_IF:
    case IND_ELSE:
    case IND_ENDIF:
    case IND_MATCH:
    case IND_ENDMATCH:
    case IND_LOOP:
    case IND_ENDLOOP:
    case IND_BREAK:
    case IND_LABEL:
    case IND_JUMP:
    case IND_JUMP_IF_FALSE:
    case IND_NOP:
    case IND_TYPE_END:
    case IND_CASE:
        std::cout << " [case]";
        break;
    case IND_BLOCK:
        break;
    }
    std::cout << " (line " << ind->line << ")\n";
    if (ind->kind == IND_BLOCK || ind->kind == IND_LAMBDA)
    {
        PrintIndication(ind->data.inner);
    }
    else if (ind->kind == IND_CASE)
    {
    }
}

inline void PrintIndicationList(Indication *head)
{
    for (Indication *p = head; p; p = p->next)
    {
        PrintIndication(p);
    }
}

#endif