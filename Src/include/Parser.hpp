#ifndef PARSER_HPP
#define PARSER_HPP
#include "Lexer.hpp"
#include "Ast.hpp"
#include <vector>
#include <memory>
#include <stdexcept>
#include <sstream>

struct SourceLocation
{
    int line;
    int column;

    SourceLocation(int l = 0, int c = 0) : line(l), column(c) {}
};
class Parser
{
public:
    Parser(Lexer &lexer) : lexer(lexer)
    {
        currentToken = lexer.nextToken();
        peekToken = lexer.nextToken();
    }
    ASTProgram *parseProgram()
    {
        std::string moduleName;
        std::vector<ASTTopLevel *> decls;

        while (!check(TokenList::_EOF))
        {
            ASTTopLevel *decl = nullptr;
            SourceLocation loc = currentLocation();
            if (check(TokenList::Macro))
            {
                decl = (ASTTopLevel *)parseMacro();
            }
            else if (check(TokenList::Arroba))
            {
                decl = parseFlag();

                if (decl->base.kind == AST_Flag_Module)
                {
                    moduleName = ((ASTFlagModule *)decl)->moduleName;
                    free(decl);
                    decl = nullptr;
                }
            }
            else if (check(TokenList::LPike))
            {
                decl = parseTopDecl();
            }
            else
            {
                throw std::runtime_error("Error this entry: [" + currentToken.lexeme + "] Dont exist");
            }
            if (decl)
            {
                decls.push_back(decl);
            }
        }
        return build_program(moduleName.empty() ? nullptr : moduleName.c_str(), decls.data(), (int)decls.size(), 0, 0);
    }

private:
    Lexer &lexer;
    Token currentToken;
    Token peekToken;
    bool IsLapExpr = false;
    void advance()
    {
        currentToken = peekToken;
        peekToken = lexer.nextToken();
    }
    bool match(TokenList type)
    {
        if (check(type))
        {
            advance();
            return true;
        }
        return false;
    }
    bool check(TokenList type) const
    {
        return currentToken.type == type;
    }
    bool nextCheck(TokenList type) const
    {
        return peekToken.type == type;
    }
    Token consume(TokenList type, const std::string &errorMsg)
    {
        if (check(type))
        {
            Token tok = currentToken;
            advance();
            return tok;
        }
        throw std::runtime_error(errorMsg + " en línea " + std::to_string(currentToken.line) + "  pero me llego: " + TokenToString(currentToken.type));
    }
    SourceLocation currentLocation() const
    {
        SourceLocation loc;
        loc.line = currentToken.line;
        loc.column = currentToken.column;
        return loc;
    }

    bool isBinaryComparator(Token tok) const
    {
        TokenList t = tok.type;
        std::string lex = tok.lexeme;
        return t == TokenList::EqualEqual || t == TokenList::NotEqual ||
               t == TokenList::Less || t == TokenList::Greater ||
               t == TokenList::LessEqual || t == TokenList::GreaterEqual ||
               t == TokenList::And || t == TokenList::Or ||
               lex == "-LT" || lex == "-GT" || lex == "-EQ" || lex == "-NE" || lex == "-LE" || lex == "-GE";
    }
    bool isBinaryCalculator(Token tok) const
    {
        TokenList t = tok.type;
        std::string lex = tok.lexeme;
        return t == TokenList::Mult || t == TokenList::Slash ||
               t == TokenList::Percent || t == TokenList::Minus ||
               t == TokenList::Plus || lex == "*" || lex == "/" ||
               lex == "%" || lex == "-" || lex == "+";
    }
    bool canStartExpression(Token t) const
    {
        TokenList type = t.type;
        return type == TokenList::NumberLiteral ||
               type == TokenList::StringLiteral ||
               type == TokenList::BoolLiteral ||
               type == TokenList::Identifier ||
               type == TokenList::ArgRef ||
               type == TokenList::LParent ||
               type == TokenList::LambdaArrow ||
               type == TokenList::ArgRef ||
               type == TokenList::KwDo ||
               type == TokenList::KwIf ||
               type == TokenList::KwMatch;
    }

    BinaryOp tokenToBinaryOp(Token tok) const
    {
        TokenList t = tok.type;
        std::string lex = tok.lexeme;
        if (t == TokenList::Plus || lex == "+")
            return BIN_ADD;
        if (t == TokenList::Minus || lex == "-")
            return BIN_SUB;
        if (t == TokenList::Mult || lex == "*")
            return BIN_MUL;
        if (t == TokenList::Slash || lex == "/")
            return BIN_DIV;
        if (t == TokenList::Percent || lex == "%")
            return BIN_MOD;
        if (t == TokenList::EqualEqual || lex == "-EQ")
            return BIN_EQ;
        if (t == TokenList::NotEqual || lex == "-NE")
            return BIN_NEQ;
        if (t == TokenList::Less || lex == "-LT")
            return BIN_LT;
        if (t == TokenList::Greater || lex == "-GT")
            return BIN_GT;
        if (t == TokenList::LessEqual || lex == "-LE")
            return BIN_LE;
        if (t == TokenList::GreaterEqual || lex == "-GE")
            return BIN_GE;
        if (t == TokenList::And || lex == "and")
            return BIN_AND;
        if (t == TokenList::Or || lex == "or")
            return BIN_OR;
        throw std::runtime_error("Operador no soportado");
    }

    AssignOp tokenToAssignOp(TokenList type)
    {
        switch (type)
        {
        case TokenList::Assign:
            return ASSIGN_PLAIN;
        case TokenList::PlusAsing:
            return ASSIGN_PLUS;
        case TokenList::MinusAsing:
            return ASSIGN_MINUS;
        case TokenList::MultAsing:
            return ASSIGN_MULT;
        case TokenList::DivAsing:
            return ASSIGN_DIV;
        default:
            throw std::runtime_error("Operador de asignación inválido");
        }
    }
    ASTInclude *parseMacro()
    {
        SourceLocation loc = currentLocation();
        consume(TokenList::Macro, "Se esperaba '#'");
        Token pragmaTok = consume(TokenList::Identifier, "Se esperaba identificador después de #");
        return build_include(pragmaTok.lexeme.c_str(), pragmaTok.line, pragmaTok.column);
    }
    ASTTopLevel *parseFlag()
    {
        SourceLocation loc = currentLocation();
        consume(TokenList::Arroba, "Se esperaba '@");
        if (match(TokenList::KwModule))
        {
            Token name = consume(TokenList::Identifier, "Se esperaba nombre de modulo");
            return (ASTTopLevel *)build_flag_module(name.lexeme.c_str(), loc.line, loc.column);
        }
        if (match(TokenList::KwImport))
        {
            Token name = consume(TokenList::Identifier, "Se esperaba nombre de lib a importar");
            bool as = false;
            std::string asName;
            if (match(TokenList::KwAs))
            {
                as = true;
                asName = consume(TokenList::Identifier, "Se esperaba nombre de lib a importar").lexeme;
            }

            return (ASTTopLevel *)build_flag_import(name.lexeme.c_str(), as, asName.empty() ? nullptr : asName.c_str(), loc.line, loc.column);
        }
        if (match(TokenList::KwExport))
        {
            std::vector<const char *> names;
            while (check(TokenList::Identifier))
            {
                names.push_back(consume(TokenList::Identifier, "Se esperaba nombre a exportar").lexeme.c_str());
                if (!match(TokenList::Comma))
                    break;
            }
            return (ASTTopLevel *)build_flag_export(names.data(), names.size(), loc.line, loc.column);
        }
        else
        {
            throw std::runtime_error("Error this flag: [" + currentToken.lexeme + "] Dont exist");
        }
    }
    ASTTopLevel *parseTopDecl()
    {
        consume(TokenList::LPike, "Se esperaba '<'");
        if (check(TokenList::KwData))
        {
            return (ASTTopLevel *)parseDataDefinition();
        }
        if (check(TokenList::KwEnum))
        {
            return (ASTTopLevel *)parseEnumDefinition();
        }
        if (check(TokenList::KwType))
        {
            return (ASTTopLevel *)parseTypeAlias();
        }
        if (check(TokenList::Identifier))
        {
            return (ASTTopLevel *)parseFunction();
        }
        else
        {
            throw std::runtime_error("Error this declaration: [" + currentToken.lexeme + "] Dont exist");
        }
    }
    // Types
    ASTType *parseTypeFunction()
    {
        SourceLocation loc = currentLocation();
        std::vector<ASTType *> paramsTypes;
        do
        {
            ASTType *paramType = parseType();
            paramsTypes.push_back(paramType);
        } while (match(TokenList::ArrowR));
        if (paramsTypes.empty())
            throw std::runtime_error("Was expected a Return Type For the Function");
        ASTType *returnType = paramsTypes.back();
        int paramCount = (int)paramsTypes.size() - 1;
        if (paramCount < 0)
            paramCount = 0;
        return (ASTType *)build_type_function(paramsTypes.data(), paramCount, returnType, loc.line, loc.column);
    }
    ASTType *parseType()
    {
        SourceLocation loc = currentLocation();
        if (!check(TokenList::Identifier) || check(TokenList::TypeVar))
        {
            if (check(TokenList::KwNumber))
            {
                consume(TokenList::KwNumber, "Was Expected a Type Number");
                return (ASTType *)build_type_primitive(PRIM_NUMBER, loc.line, loc.column);
            }
            if (check(TokenList::KwString))
            {
                consume(TokenList::KwString, "Was Expected a Type String");
                return (ASTType *)build_type_primitive(PRIM_STRING, loc.line, loc.column);
            }
            if (check(TokenList::KwBool))
            {
                consume(TokenList::KwBool, "Was Expected a Type Bool");
                return (ASTType *)build_type_primitive(PRIM_BOOL, loc.line, loc.column);
            }
            if (check(TokenList::KwVoid))
            {
                consume(TokenList::KwVoid, "Was Expected a Type Void");
                return (ASTType *)build_type_primitive(PRIM_VOID, loc.line, loc.column);
            }
            if (check(TokenList::KwAny))
            {
                consume(TokenList::KwAny, "Was Expected a Type Any");
                return (ASTType *)build_type_primitive(PRIM_ANY, loc.line, loc.column);
            }
            if (check(TokenList::KwNull))
            {
                consume(TokenList::KwNull, "Was Expected a Type Null");
                return (ASTType *)build_type_primitive(PRIM_NULL, loc.line, loc.column);
            }
            if (check(TokenList::KwIO))
            {
                consume(TokenList::KwIO, "Was Expected a Type IO()");
                consume(TokenList::LParent, "Was Expected '(' in IO Type Definition");
                consume(TokenList::RParent, "Was Expected ')' in IO Type Definition");
                return (ASTType *)build_type_named("IO", nullptr, 1, loc.line, loc.column);
            }
            if (check(TokenList::KwMaybe))
            {
                consume(TokenList::KwMaybe, "Was Expected a Type Maybe");
                consume(TokenList::LPike, "Was Expected '<' at begin of Maybe Type Definition");
                std::vector<ASTType *> params;
                do
                {
                    ASTType *param = parseType();
                    params.push_back(param);
                    if (!check(TokenList::Pipe) || !check(TokenList::Or))
                    {
                        throw std::runtime_error("Only can use | or 'or' comparator in Maybe Definitions");
                        break;
                    }
                } while (!check(TokenList::RPike));
                consume(TokenList::RPike, "Was Expected '>' at end of Maybe Type Definition");
                return (ASTType *)build_type_named("Maybe", params.data(), params.size(), loc.line, loc.column);
            }
            if (check(TokenList::KwJust))
            {
                consume(TokenList::KwJust, "Was Expected a Type Just");
                consume(TokenList::Identifier, "Was Expecter a Identifier or Type after Just");
                return (ASTType *)build_type_named("Just", nullptr, 1, loc.line, loc.column);
            }
            if (check(TokenList::KwNothing))
            {
                consume(TokenList::KwNothing, "Was Expected a Type Nothing");
                return (ASTType *)build_type_primitive(PRIM_NOTHING, loc.line, loc.column);
            }
            if (check(TokenList::KwList))
            {
                consume(TokenList::KwList, "Was Expected a Type List");
                consume(TokenList::LPike, "Was Expected '<' at begin of List Type Definition");
                std::vector<ASTType *> params;
                do
                {
                    ASTType *param = parseType();
                    params.push_back(param);
                } while (!check(TokenList::RPike));
                consume(TokenList::RPike, "Was Expected '>' at end of List Type Definition");
                return (ASTType *)build_type_named("List", params.data(), params.size(), loc.line, loc.column);
            }
            else
            {
            }
        }
        else
        {
            throw std::runtime_error("Unknow Type");
        }
    }
    // Expressions
    ASTExpr *parseExpression()
    {
        if (check(TokenList::LambdaArrow))
        {
            return (ASTExpr *)parseLambdaExpression();
        }
        if (check(TokenList::KwDo))
        {
            return (ASTExpr *)parseDoBlockExpr();
        }
        if (check(TokenList::LParent))
        {
            return parseParenthesizeExpression();
        }
        if (check(TokenList::LPike))
        {
            return (ASTExpr *)parseFunctionCall();
        }
        else
        {
            if (isBinaryComparator(currentToken))
            {
                return parseCondition();
            }
            if (isBinaryCalculator(currentToken))
            {
                return parseBinaryCalc();
            }
            else
            {
                return parsePrimary();
            }
        }
    }
    ASTExpr *parsePrimary()
    {
        if (check(TokenList::Assign))
        {
            consume(TokenList::Assign, "Was Expected '=' in (Assing Primary Expression)");
        }
        if (check(TokenList::ArrowL))
        {
            consume(TokenList::ArrowL, "Was Expected '<-' in (Input Primary Expression)");
        }
        else
        {
            if (check(TokenList::BoolLiteral))
            {
                Token value = consume(TokenList::BoolLiteral, "Was Expected 'true or false' in (Bool Literal Expression)");
                bool val = (value.lexeme == "true");
                return (ASTExpr *)build_expr_literal(LIT_BOOL, val ? "true" : "false", value.line, value.column);
            }
            if (check(TokenList::NumberLiteral))
            {
                Token value = consume(TokenList::NumberLiteral, "Was Expected 'Number' in (Number Literal Expression)");
                LiteralKind kind = (value.lexeme.find('.') != std::string::npos) ? LIT_FLOAT : LIT_INT;
                return (ASTExpr *)build_expr_literal(kind, value.lexeme.c_str(), value.line, value.column);
            }
            if (check(TokenList::StringLiteral))
            {
                Token value = consume(TokenList::StringLiteral, "Was Expected 'String' in (String Literal Expression)");
                return (ASTExpr *)build_expr_literal(LIT_STRING, value.lexeme.c_str(), value.line, value.column);
            }
            if (check(TokenList::Identifier))
            {
                Token value = consume(TokenList::Identifier, "Was Expected 'Identifier' in (Identifier Expression)");
                ASTExpr *expr = (ASTExpr *)build_expr_var(value.lexeme.c_str(), value.line, value.column);
                return expr;
            }
            if (check(TokenList::ArgRef))
            {
                Token value = consume(TokenList::ArgRef, "Was Expected '$Var' in (ArgRef Expression)");
                int index = std::stoi(value.lexeme);
                return (ASTExpr *)build_expr_argref(index, value.line, value.column);
            }
            if (check(TokenList::LParent))
            {
                return parseExpression();
            }
            else
            {
                throw std::runtime_error("UnExpected Expression: " + currentToken.lexeme);
            }
        }
    }
    ASTExpr *parseUnary()
    {
        if (check(TokenList::Not))
        {
            consume(TokenList::Not, "Was Expected 'not' in (not Unary Expression)");
            ASTExpr *left = parseExpression();
            SourceLocation loc = currentLocation();
            return (ASTExpr *)build_expr_unary(UNARY_NOT, left, loc.line, loc.column);
        }
        else
        {
            return parsePrimary();
        }
    }
    ASTExpr *parseBinaryCalc()
    {
        if (check(TokenList::Plus))
        {

            consume(TokenList::Plus, "Was Expected '+' in (+ Binary Expression)");
            ASTExpr *left = parseExpression();
            ASTExpr *right = parseExpression();
            SourceLocation loc = currentLocation();
            return (ASTExpr *)build_expr_binary(BIN_ADD, left, right, loc.line, loc.column);
        }
        if (check(TokenList::Minus))
        {
            SourceLocation loc = currentLocation();
            consume(TokenList::Minus, "Was Expected '-' in (- Binary Expression)");
            ASTExpr *left = parseExpression();
            if (canStartExpression(currentToken))
            {
                ASTExpr *right = parseExpression();
                return (ASTExpr *)build_expr_binary(BIN_SUB, left, right, loc.line, loc.column);
            }
            else
            {
                return (ASTExpr *)build_expr_unary(UNARY_NEG, left, loc.line, loc.column);
            }
        }
        if (check(TokenList::Slash))
        {
            consume(TokenList::Slash, "Was Expected '/' in (/ Binary Expression)");
            ASTExpr *left = parseExpression();
            ASTExpr *right = parseExpression();
            SourceLocation loc = currentLocation();
            return (ASTExpr *)build_expr_binary(BIN_DIV, left, right, loc.line, loc.column);
        }
        if (check(TokenList::Mult))
        {
            consume(TokenList::Mult, "Was Expected '*' in (* Binary Expression)");
            ASTExpr *left = parseExpression();
            ASTExpr *right = parseExpression();
            SourceLocation loc = currentLocation();
            return (ASTExpr *)build_expr_binary(BIN_MUL, left, right, loc.line, loc.column);
        }
        if (check(TokenList::Percent))
        {
            consume(TokenList::Percent, "Was Expected '%' in (% Binary Expression)");
            ASTExpr *left = parseExpression();
            ASTExpr *right = parseExpression();
            SourceLocation loc = currentLocation();
            return (ASTExpr *)build_expr_binary(BIN_MOD, left, right, loc.line, loc.column);
        }
        else
        {
            return parseUnary();
        }
    }
    ASTExpr *parseCompare()
    {
        if (check(TokenList::Less))
        {
            consume(TokenList::Less, "Was Expected '-LT' in (-LT Compare expression)");
            ASTExpr *left = parseExpression();
            ASTExpr *right = parseExpression();
            SourceLocation loc = currentLocation();
            return (ASTExpr *)build_expr_binary(BIN_LT, left, right, loc.line, loc.column);
        }
        if (check(TokenList::LessEqual))
        {
            consume(TokenList::LessEqual, "Was Expected '-LE' in (-LE Compare expression)");
            ASTExpr *left = parseExpression();
            ASTExpr *right = parseExpression();
            SourceLocation loc = currentLocation();
            return (ASTExpr *)build_expr_binary(BIN_LE, left, right, loc.line, loc.column);
        }
        if (check(TokenList::Greater))
        {
            consume(TokenList::Greater, "Was Expected '-GT' in (-GT Compare expression)");
            ASTExpr *left = parseExpression();
            ASTExpr *right = parseExpression();
            SourceLocation loc = currentLocation();
            return (ASTExpr *)build_expr_binary(BIN_GT, left, right, loc.line, loc.column);
        }
        if (check(TokenList::GreaterEqual))
        {
            consume(TokenList::GreaterEqual, "Was Expected '-GE' in (-GE Compare expression)");
            ASTExpr *left = parseExpression();
            ASTExpr *right = parseExpression();
            SourceLocation loc = currentLocation();
            return (ASTExpr *)build_expr_binary(BIN_GE, left, right, loc.line, loc.column);
        }
        if (check(TokenList::EqualEqual))
        {
            consume(TokenList::EqualEqual, "Was Expected '-EQ' in (-EQ Compare expression)");
            ASTExpr *left = parseExpression();
            ASTExpr *right = parseExpression();
            SourceLocation loc = currentLocation();
            return (ASTExpr *)build_expr_binary(BIN_EQ, left, right, loc.line, loc.column);
        }
        if (check(TokenList::NotEqual))
        {
            consume(TokenList::NotEqual, "Was Expected '-NE' in (-NE Compare expression)");
            ASTExpr *left = parseExpression();
            ASTExpr *right = parseExpression();
            SourceLocation loc = currentLocation();
            return (ASTExpr *)build_expr_binary(BIN_NEQ, left, right, loc.line, loc.column);
        }
        else
        {
            return parseBinaryCalc();
        }
    }
    ASTExpr *parseCondition()
    {
        if (check(TokenList::And))
        {
            consume(TokenList::And, "Was Expected 'and' in (And Expression)");
            ASTExpr *left = parseExpression();
            ASTExpr *right = parseExpression();
            SourceLocation loc = currentLocation();
            return (ASTExpr *)build_expr_binary(BIN_AND, left, right, loc.line, loc.column);
        }
        if (check(TokenList::Or))
        {
            consume(TokenList::Or, "Was Expected 'or' in (Or Expression)");
            ASTExpr *left = parseExpression();
            ASTExpr *right = parseExpression();
            SourceLocation loc = currentLocation();
            return (ASTExpr *)build_expr_binary(BIN_OR, left, right, loc.line, loc.column);
        }
        else
        {
            return parseCompare();
        }
    }
    ASTPattern *parsePattern()
    {
        if (check(TokenList::Identifier))
        {
            Token r = consume(TokenList::Identifier, "Was Expected a 'Identifier' in (Identifier Pattern)");
            SourceLocation loc = currentLocation();
            return (ASTPattern *)build_pat_var(r.lexeme.c_str(), loc.line, loc.column);
        }
        if (check(TokenList::LPike))
        {
            return (ASTPattern *)(ASTExpr *)parseFunctionCall();
        }
        else
        {
            if (check(TokenList::StringLiteral) || check(TokenList::NumberLiteral) || check(TokenList::BoolLiteral))
            {
                SourceLocation loc = currentLocation();
                return (ASTPattern *)build_pat_literal(parseExpression(), loc.line, loc.column);
            }
            throw std::runtime_error("UnExpected Pattenr: " + currentToken.lexeme);
        }
    }
    MatchCase parseMatchCase()
    {
        MatchCase ret;
        ret.pattern = parsePattern();
        consume(TokenList::ArrowR, "Was Expected a '->' after CaseMatchPattern");
        ret.body = parseExpression();
        return ret;
    }
    ASTExpr *parseParenthesizeExpression()
    {

        consume(TokenList::LParent, "Was Expected a '(' to Begin Parent Expression");
        SourceLocation loc = currentLocation();
        if (check(TokenList::LParent))
        {
            consume(TokenList::LParent, "Was Expected a '(' to Begin If Condition");
            ASTExpr *cond = parseExpression();
            consume(TokenList::RParent, "Was Expected a ')' to End If Condition");
            ASTExpr *then = parseExpression();
            ASTExpr *elseThen = nullptr;
            if (check(TokenList::LParent))
            {
                consume(TokenList::LParent, "Was Expected a '(' to Begin Else");
                elseThen = parseExpression();
                consume(TokenList::RParent, "Was Expected a ')' to End Else");
            }
            consume(TokenList::RParent, "Was Expected a ')' to End Parent Expression");
            return (ASTExpr *)build_expr_if(cond, then, elseThen, loc.line, loc.column);
        }
        else if (check(TokenList::Identifier) || check(TokenList::ArgRef))
        {
            ASTExpr *cond = parseExpression();
            consume(TokenList::Colon, "Was Expected a ':' to Match Expression");
            std::vector<MatchCase> cases;
            while (!check(TokenList::RParent))
            {
                if (check(TokenList::LParent))
                {
                    consume(TokenList::LParent, "Was Expected a '(' to Begin Case Expression");
                    MatchCase matchCase = parseMatchCase();
                    cases.push_back(matchCase);
                    consume(TokenList::RParent, "Was Expected a ')' to End Case Expression");
                }
            }

            consume(TokenList::RParent, "Was Expected a ')' to End Parent Expression");
            return (ASTExpr *)build_expr_match(cond, cases.data(), cases.size(), loc.line, loc.column);
        }
        else
        {
            ASTExpr *expr = parseExpression();
            consume(TokenList::RParent, "Was Expected a ')' to End Parent Expression");
            return expr;
        }
    }
    ASTExprLambda *parseLambdaExpression()
    {
        consume(TokenList::LambdaArrow, "Was Expected a '~>' to lambdaExpression Begin");
        SourceLocation loc = currentLocation();
        ASTExpr *body = parseExpression();
        return build_expr_lambda(body, loc.line, loc.column);
    }
    ASTExprCall *parseFunctionCall()
    {

        consume(TokenList::LPike, "Was Expected a '<' to CallFunction Begin");
        SourceLocation loc = currentLocation();
        Token func;
        if (!check(TokenList::Identifier))
        {
            // esto puede parecer inecesario pero quiero hacerlo asi
            if (check(TokenList::KwPutStrLn))
            {
                func = consume(TokenList::KwPutStrLn, "Was Expected a 'putStrLn' Name for Call putStrLn Function ");
            }
            if (check(TokenList::KwToNumber))
            {
                func = consume(TokenList::KwToNumber, "Was Expected a 'toNumber' Name for Call toNumber Function ");
            }
            if (check(TokenList::KwToString))
            {
                func = consume(TokenList::KwToString, "Was Expected a 'toString' Name for Call toString Function ");
            }
        }
        else
        {
            func = consume(TokenList::Identifier, "Was Expected a 'Identifier' Name for CallFunction ");
        }
        std::vector<ASTExpr *> params;
        while (check(TokenList::Dollar))
        {
            consume(TokenList::Dollar, "Was Expected a '$' for Indicate 'Function parameter' ");
            ASTExpr *param = parseExpression();
            params.push_back(param);
        }
        consume(TokenList::AutoCloseTag, "Was Expected a '/>' End FunctionCall");
        if (check(TokenList::Semicolon))
            consume(TokenList::Semicolon, "Se esperaba ';'");
        return build_expr_call(func.lexeme.c_str(), params.data(), params.size(), loc.line, loc.column);
    }

    ASTExprBlock *parseDoBlockExpr()
    {

        consume(TokenList::KwDo, "Was Expected a 'do' for Indicate 'Function parameter' ");
        SourceLocation loc = currentLocation();
        consume(TokenList::LParent, "Was Expected a '(' to 'Do Expression' Begin");
        ASTExpr *DoExpr = nullptr;
        if (!check(TokenList::RParent))
            DoExpr = parseExpression();
        consume(TokenList::RParent, "Was Expected a ')' to 'Do Expression' Finalise");
        ASTStmtBlock *WhereBlock = parseWhereBlock();
        ASTExpr *WhereExpr = parseExpression();
        consume(TokenList::Semicolon, "Was Expected a ';' to 'Where Expression' Finalise");
        return build_expr_block(DoExpr, WhereBlock, WhereExpr, loc.line, loc.column);
    }
    // Stmts
    ASTStmtBlock *parseWhereBlock()
    {

        consume(TokenList::KwWhere, "Was Expected a 'where' to 'where' Begin");
        SourceLocation loc = currentLocation();
        consume(TokenList::LBrace, "Was Expected a '{' to 'whereBlock' Begin");
        std::vector<ASTStmt *> Stmts;
        while (!check(TokenList::RBrace))
        {
            ASTStmt *Stmt = parseStmt();
            Stmts.push_back(Stmt);
        }
        consume(TokenList::RBrace, "Was Expected a '}' to 'whereBlock' Finalyse");
        return build_stmt_block(Stmts.data(), Stmts.size(), loc.line, loc.column);
    }
    bool isAsingFunction(Token tok)
    {
        TokenList t = tok.type;
        std::string lex = tok.lexeme;
        return t == TokenList::PlusAsing || t == TokenList::DivAsing || t == TokenList::MinusAsing ||
               t == TokenList::MultAsing || lex == "=*" || lex == "=/" ||
               lex == "=%" || lex == "=-" || lex == "=" ||
               lex == "/=" || lex == "*=" || lex == "+=" || lex == "-=";
    }
    ASTStmtLet *parseLetStmt()
    {
        consume(TokenList::KwLet, "Was Expected a 'Let' for let Definition Statement");
        SourceLocation loc = currentLocation();
        std::string NameVar = consume(TokenList::Identifier, "Was Expected an Identifier for the Var").lexeme;
        consume(TokenList::Colon, "Was Expected a ':' to Define Variabel Type");
        ASTType *type = parseType();
        ASTExpr *init = nullptr;
        bool constant = false;
        if (check(TokenList::Assign))
        {
            consume(TokenList::Assign, "Was Expected a '=' for Assing Value");
            init = parseExpression();
        }
        else if (check(TokenList::ArrowR))
        {
            consume(TokenList::ArrowR, "Was Expected a '->' for Assing Value");
            constant = true;
            init = parseExpression();
        }
        else if (check(TokenList::ArrowL))
        {
            consume(TokenList::ArrowL, "Was Expected a '<-' for Assing Value");
            init = parseExpression();
        }
        else
        {
        }
        consume(TokenList::Semicolon, "Was Expected a ';' for finallise Let Stmt");
        return build_stmt_let(NameVar.c_str(), type, constant, init, loc.line, loc.column);
    }
    ASTStmtAssign *parseAssingStmt()
    {
        SourceLocation loc = currentLocation();
        Token varTok = consume(TokenList::Identifier, "Was Expected a identeifier for to assing");
        AssignOp op;
        if (isAsingFunction(currentToken))
        {
            if (match(TokenList::Assign))
            {
            }
            else if (match(TokenList::PlusAsing))
            {
            }
            else if (match(TokenList::MinusAsing))
            {
            }
            else if (match(TokenList::MultAsing))
            {
            }
            else if (match(TokenList::DivAsing))
            {
            }
        }
        else
        {
            throw std::runtime_error("UnKnown Assingment OP");
        }
        ASTExpr *rhs = parseExpression();
        consume(TokenList::Semicolon, "Was Expected a ';' for finallise Assing Stmt");
        return build_stmt_assign(varTok.lexeme.c_str(), op, rhs, varTok.line, varTok.column);
    }
    ASTStmtIf *parseIfStmt()
    {
        SourceLocation loc = currentLocation();
        consume(TokenList::KwIf, "Se esperaba 'if'");
        consume(TokenList::LParent, "Se esperaba '(' después de 'if'");
        ASTExpr *cond = parseExpression();
        consume(TokenList::RParent, "Se esperaba ')' después de la condición");
        consume(TokenList::LBrace, "Se esperaba '{'");
        std::vector<ASTStmt *> thenStmts;
        while (!check(TokenList::RBrace))
        {
            ASTStmt *stmt = parseStmt();
            thenStmts.push_back(stmt);
        }
        consume(TokenList::RBrace, "Se esperaba '}'");

        std::vector<ASTStmt *> elseStmts;
        if (match(TokenList::KwElse))
        {
            if (match(TokenList::KwIf))
            {
                ASTStmt *elseIfStmt = (ASTStmt *)parseIfStmt();
                elseStmts.push_back(elseIfStmt);
            }
            else
            {
                consume(TokenList::LBrace, "Se esperaba '{' después de 'else'");
                while (!check(TokenList::RBrace))
                {
                    ASTStmt *elseStmt = parseStmt();
                    elseStmts.push_back(elseStmt);
                }
                consume(TokenList::RBrace, "Se esperaba '}'");
            }
        }
        return build_stmt_if(cond, thenStmts.data(), thenStmts.size(), elseStmts.empty() ? nullptr : elseStmts.data(), elseStmts.size(), loc.line, loc.column);
    }

    ASTStmtMatch *parseMatchStmt()
    {
        SourceLocation loc = currentLocation();
        consume(TokenList::KwMatch, "Se esperaba 'match'");
        consume(TokenList::LParent, "Se esperaba '(' después de 'match'");
        ASTExpr *expr = parseExpression();
        consume(TokenList::RParent, "Se esperaba ')'");
        consume(TokenList::LBrace, "Se esperaba '{'");

        std::vector<MatchCase> cases;
        while (!check(TokenList::RBrace) && !check(TokenList::_EOF))
        {
            consume(TokenList::KwCase, "Se esperaba 'case'");
            ASTPattern *pat = parsePattern();
            consume(TokenList::ArrowR, "Se esperaba '->'");
            ASTExpr *body = parseExpression();
            consume(TokenList::Semicolon, "Se esperaba ';'");
            cases.push_back({pat, body});
        }

        consume(TokenList::RBrace, "Se esperaba '}'");
        return build_stmt_match(expr, cases.data(), cases.size(), loc.line, loc.column);
    }

    ASTStmt *parseStmt()
    {

        if (check(TokenList::KwLet))
        {
            return (ASTStmt *)parseLetStmt();
        }
        if (check(TokenList::KwIf))
        {
            return (ASTStmt *)parseIfStmt();
        }
        if (check(TokenList::KwMatch))
        {
            return (ASTStmt *)parseMatchStmt();
        }
        if (check(TokenList::Identifier) && isAsingFunction(peekToken))
        {
            return (ASTStmt *)parseAssingStmt();
        }
        if (check(TokenList::LPike))
        {
            return (ASTStmt *)parseFunctionCall();
        }
        else
        {
            throw std::runtime_error("UnExpected Statement:" + currentToken.lexeme);
        }
    }
    // Delcs
    ASTDeclFunc *parseFunction()
    {
        SourceLocation loc = currentLocation();
        std::string StartfunctionName = consume(TokenList::Identifier, "Was Expected an Identifier for the Function").lexeme;
        consume(TokenList::DoubleColon, "Was Expected a '::' after Name of Function");
        ASTType *funcType = parseTypeFunction();
        consume(TokenList::RPike, "Was Expected a '>' after Array of Types");
        ASTExpr *body = nullptr;
        if (!check(TokenList::CloseTag))
        {
            body = parseBodyFunction();
        }
        consume(TokenList::CloseTag, "Was Expected a '</' after Body Function");
        std::string EndfunctionName = consume(TokenList::Identifier, "Was Expected an Identifier for Close Function").lexeme;
        consume(TokenList::RPike, "Was Expected a '>' after Name of Function");
        if (StartfunctionName != EndfunctionName)
        {
            throw std::runtime_error("Names of Functions not are corelative[\"" + StartfunctionName + "\"!=\"" + EndfunctionName + "\"]");
        }
        return build_decl_func(StartfunctionName.c_str(), funcType, body, loc.line, loc.column);
    }
    ASTExpr *parseBodyFunction()
    {
        if (!check(TokenList::Identifier))
        {
            return parseExpression();
        }
        else
        {
            throw std::runtime_error("UnExpected Identifier in Body Function");
        }
    }
    ASTDeclEnum *parseEnumDefinition()
    {
        Token name = consume(TokenList::Identifier, "Was Expected 'identifier' for Enum Declaration");
        consume(TokenList::RPike, "Was Expected '>' after Enum 'identifier'");

        std::vector<EnumConstructor> ctors;
        do
        {
            Token ctorName;
            if (!check(TokenList::Dollar) && !check(TokenList::CloseTag) && !check(TokenList::RPike) && !check(TokenList::_EOF))
            {
                ctorName = currentToken;
                advance();
            }
            else
            {
                throw std::runtime_error("Was Expected a Name for Constructor");
            }

            std::vector<ASTType *> paramTypes;
            while (!check(TokenList::Dollar) && !check(TokenList::CloseTag) && !check(TokenList::RPike) && !check(TokenList::_EOF))
            {
                paramTypes.push_back(parseType());
            }

            EnumConstructor ctor;
            ctor.name = strdup(ctorName.lexeme.c_str());
            ctor.paramTypes = paramTypes.empty() ? nullptr : (ASTType **)malloc(sizeof(ASTType *) * paramTypes.size());
            for (size_t i = 0; i < paramTypes.size(); ++i)
            {
                ctor.paramTypes[i] = paramTypes[i];
            }
            ctor.paramCount = paramTypes.size();
            ctors.push_back(ctor);
        } while (match(TokenList::Dollar));

        consume(TokenList::CloseTag, "Was Expected '</' for close Enum Declaration");
        consume(TokenList::KwEnum, "Was Expected 'enum' for close Enum Declaration");
        consume(TokenList::RPike, "Was Expected'>' for Finallyse Enum Declaration");

        return build_decl_enum(name.lexeme.c_str(), ctors.data(), ctors.size(), name.line, name.column);
    }
    ASTDeclData *parseDataDefinition()
    {
        Token name = consume(TokenList::Identifier, "Was Expected 'identifier' for Data Declaration");
        consume(TokenList::RPike, "Was Expected '>' after Data 'identifier'");

        std::vector<Field> fields;
        while (!check(TokenList::CloseTag) && !check(TokenList::_EOF))
        {
            Token fieldName = consume(TokenList::Identifier, "Was expected 'identifier' for FieldName");
            consume(TokenList::Colon, "WasExpected ':' For define Type in Field");
            ASTType *fieldType = parseType();
            consume(TokenList::Semicolon, "Was Expected';' after field");

            Field f;
            f.name = strdup(fieldName.lexeme.c_str());
            f.type = fieldType;
            fields.push_back(f);
        }

        consume(TokenList::CloseTag, "Was Expected '</' for close Data Declaration");
        consume(TokenList::KwData, "Was Expected 'data' for close Data Declaration");
        consume(TokenList::RPike, "Was Expected '>' for Finallyse Data Declaration");

        return build_decl_data(name.lexeme.c_str(), fields.data(), fields.size(), name.line, name.column);
    }
    ASTDeclType *parseTypeAlias()
    {
        Token name = consume(TokenList::Identifier, "Was Expected a name for the alias");
        consume(TokenList::DoubleColon, "Was Expected a '::' before type Ident");
        ASTType *aliasedType = parseType();
        consume(TokenList::AutoCloseTag, "Was Expected '/>' to finallyse Type Declaration");
        return build_decl_type(name.lexeme.c_str(), aliasedType, name.line, name.column);
    }
};

#endif