#ifndef Lexer_HPP
#define Lexer_HPP
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>
#include <stdexcept>

enum class TokenList
{
    // ===== Bloques y agrupación =====
    LParent,   // (
    RParent,   // )
    LBrace,    // {
    RBrace,    // }
    LBracket,  // [
    RBracket,  // ]
    LPike,     // <
    RPike,     // >
    Semicolon, // ;
    Colon,     // :
    Dot,       // .
    Comma,     // ,

    // ===== Operadores básicos =====
    Plus,          // +
    Minus,         // -
    Mult,          // *
    Slash,         // /
    Percent,       // %
    Assign,        // =
    UnderScore,    // _
    Arroba,        // @
    Macro,         // #
    Dollar,        // $
    Pipe,          // |
                   // complex
    DoubleColon,   //::
    CloseTag,      // </
    AutoCloseTag,  // />
    ArrowL,        // <-
    ArrowR,        // ->
    LambdaArrow,   // ~>
    PlusAsing,     // +=
    MinusAsing,    // -=
    MultAsing,     // *=
    DivAsing,      // /=
                   // flag comparation
    EqualEqual,    // -EQ
    NotEqual,      // -NE
    Less,          // -LT
    Greater,       // -GT
    LessEqual,     // -LE
    GreaterEqual,  // -GE
    And,           //  and
    Or,            //  or
    Not,           //  not
                   // literales
    Identifier,    // nombres de variables, funciones, clases
    NumberLiteral, // 123, 3.14
    StringLiteral, // "hola"
    BoolLiteral,   // true, false
    ArgRef,        // $0, $1
                   // keywords
    KwImport,      // import
    KwModule,      // module
    KwExport,      // export
    KwLet,         // let
    KwDo,          // do
    KwWhere,       // where
    KwIf,          // if
    KwElse,        // else
    KwMatch,       // match
    KwCase,        // case
    KwLoop,        // loop
    KwBreak,       // break
    KwPutStrLn,    // putStrLn
    KwToString,    // toString
    KwToNumber,    // toNumber
    KwData,        // data
    KwType,        // type
    KwEnum,        // enum
    KwNothing,     // Nothing
    KwNull,        // Null
    KwAs,          // as
    KwContinue,    // continue
    // manejo de File
    KwFile,
    KwFileOpen,
    KwFileRead,
    KwFileWrite,
    // ===== Tipos básicos =====
    KwNumber, // Number
    KwString, // String
    KwBool,   // Bool
    KwAny,    // Any
    KwIO,     // IO
    KwVoid,   // Void
    KwMaybe,  // Maybe
    KwJust,   // Just
    KwList,   // List
    KwFileType,
    TypeVar,

    LineComment,  // --
    BlockComment, // -* *-
    _EOF
};

struct Token
{
    TokenList type;
    std::string lexeme;
    int line;
    int column;

    Token(TokenList t, const std::string &lex, int l, int c) : type(t), lexeme(lex), line(l), column(c) {}
    Token() : type(TokenList::Semicolon), lexeme(""), line(0), column(0) {}
};
class Lexer
{
public:
    Lexer(const std::string &source) : src(source), pos(0), line(1), column(1) {}
    /*
    Token nextToken()
    {
        skipWhiteSpaceAndComments();
        if (isAtEnd())
        {
            return Token(TokenList::_EOF, "End Of File", line, column);
        }

        char c = peek();


        // Números
        if (std::isdigit(c))
            return readNumber();

        // Identificadores / keywords
        if (std::isalpha(c))
            return readIdentifierOrKeyword();

        // Strings
        if (c == '"')
            return readString();

        // Símbolos y operadores
        return readSymbol();
    }
        */
    Token nextToken()
    {
        skipWhiteSpaceAndComments();
        if (isAtEnd())
        {
            Token tok(TokenList::_EOF, "End Of File", line, column);
            std::cout << (int)tok.type << " '" << tok.lexeme << "'\n"; // <-- imprime EOF
            return tok;
        }

        char c = peek();
        Token tok;

        if (std::isdigit(c))
            tok = readNumber();
        else if (std::isalpha(c))
            tok = readIdentifierOrKeyword();
        else if (c == '"')
            tok = readString();
        else
            tok = readSymbol();

        std::cout << (int)tok.type << " '" << tok.lexeme << "'\n"; // <-- imprime cada token
        return tok;
    }

    bool isAtEnd() const
    {
        return pos >= src.size();
    }

private:
    std::string src;
    size_t pos;
    int line;
    int column;

    char peek() const
    {
        if (isAtEnd())
            return '\0';
        return src[pos];
    }
    char advance()
    {
        char c = peek();
        pos++;
        if (c == '\n')
        {
            line++;
            column = 1;
        }
        else
        {
            column++;
        }
        return c;
    }
    bool match(char expected)
    {
        if (isAtEnd())
            return false;
        if (src[pos] != expected)
            return false;
        advance();
        return true;
    }
    void skipWhiteSpace()
    {
        while (!isAtEnd())
        {
            char c = peek();
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            {
                advance();
            }
            else
            {
                break;
            }
        }
    }
    void skipLineComment()
    {
        while (!isAtEnd() && peek() != '\n')
        {
            advance();
        }
    }
    void skipBlockComment()
    {
        int nesting = 1;
        while (!isAtEnd() && nesting > 0)
        {
            if (peek() == '-' && pos + 1 < src.size() && src[pos + 1] == '*')
            {
                advance();
                advance();
                nesting++;
            }
            else if (peek() == '*' && pos + 1 < src.size() && src[pos + 1] == '-')
            {
                advance();
                advance();
                nesting--;
            }
            else
            {
                advance();
            }
        }
        if (nesting > 0)
        {
            throw std::runtime_error("Bloque comentario no cerrado");
        }
    }
    void skipWhiteSpaceAndComments()
    {
        while (!isAtEnd())
        {
            skipWhiteSpace();
            if (isAtEnd())
                return;
            if (peek() == '-' && pos + 1 < src.size())
            {
                if (src[pos + 1] == '*')
                {
                    advance();
                    advance();
                    skipBlockComment();
                    continue;
                }
                if (src[pos + 1] == '-')
                {
                    advance();
                    advance();
                    skipLineComment();
                    continue;
                }
            }
            break;
        }
    }
    Token readNumber()
    {
        int startLine = line;
        int startCol = column;
        std::string number;
        bool hasDot = false;

        while (!isAtEnd() && (std::isdigit(peek()) || peek() == '.'))
        {
            if (peek() == '.')
            {
                if (hasDot)
                    break;
                hasDot = true;
            }
            number += advance();
        }

        return Token(TokenList::NumberLiteral, number, startLine, startCol);
    }

    Token readString()
    {
        int startLine = line;
        int startCol = column;
        advance();
        std::string str;

        bool scaped = false;
        while (!isAtEnd())
        {
            char c = advance();
            if (scaped)
            {
                switch (c)
                {
                case 'n':
                    str += '\n';
                    break;
                case 't':
                    str += '\t';
                    break;
                case 'r':
                    str += '\r';
                    break;
                case '\\':
                    str += '\\';
                    break;
                case '"':
                    str += '"';
                    break;
                default:
                    str += c;
                    break;
                }
                scaped = false;
            }
            else if (c == '\\')
            {
                scaped = true;
            }
            else if (c == '"')
            {
                return Token(TokenList::StringLiteral, str, startLine, startCol);
            }
            else
            {
                str += c;
            }
        }
        throw std::runtime_error("Cadena no terminada");
    }

    Token readIdentifierOrKeyword()
    {
        int startLine = line, startCol = column;
        std::string lexeme;
        while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_'))
            lexeme += advance();

        static std::unordered_map<std::string, TokenList> keywords = {
            {"import", TokenList::KwImport},
            {"module", TokenList::KwModule},
            {"export", TokenList::KwExport},
            {"let", TokenList::KwLet},
            {"do", TokenList::KwDo},
            {"where", TokenList::KwWhere},
            {"if", TokenList::KwIf},
            {"else", TokenList::KwElse},
            {"match", TokenList::KwMatch},
            {"case", TokenList::KwCase},
            {"loop", TokenList::KwLoop},
            {"break", TokenList::KwBreak},
            {"putStrLn", TokenList::KwPutStrLn},
            {"toString", TokenList::KwToString},
            {"toNumber", TokenList::KwToNumber},
            {"data", TokenList::KwData},
            {"type", TokenList::KwType},
            {"enum", TokenList::KwEnum},
            {"Number", TokenList::KwNumber},
            {"String", TokenList::KwString},
            {"Bool", TokenList::KwBool},
            {"Any", TokenList::KwAny},
            {"IO", TokenList::KwIO},
            {"Void", TokenList::KwVoid},
            {"Maybe", TokenList::KwMaybe},
            {"Just", TokenList::KwJust},
            {"Nothing", TokenList::KwNothing},
            {"null", TokenList::KwNull},
            {"List", TokenList::KwList},
            {"as", TokenList::KwAs},
            {"true", TokenList::BoolLiteral},
            {"false", TokenList::BoolLiteral},
            {"and", TokenList::And},
            {"or", TokenList::Or},
            {"not", TokenList::Not},
            {"continue", TokenList::KwContinue},
        };

        auto it = keywords.find(lexeme);
        if (it != keywords.end())
            return Token(it->second, lexeme, startLine, startCol);

        return Token(TokenList::Identifier, lexeme, startLine, startCol);
    }

    Token readSymbol()
    {
        int startLine = line, startCol = column;
        char c = advance();

        switch (c)
        {
        case '(':
            return Token(TokenList::LParent, "(", startLine, startCol);
        case ')':
            return Token(TokenList::RParent, ")", startLine, startCol);
        case '{':
            return Token(TokenList::LBrace, "{", startLine, startCol);
        case '}':
            return Token(TokenList::RBrace, "}", startLine, startCol);
        case '[':
            return Token(TokenList::LBracket, "[", startLine, startCol);
        case ']':
            return Token(TokenList::RBracket, "]", startLine, startCol);
        case ',':
            return Token(TokenList::Comma, ",", startLine, startCol);
        case '#':
            return Token(TokenList::Macro, "#", startLine, startCol);
        case ';':
            return Token(TokenList::Semicolon, ";", startLine, startCol);
        case '~':
            if (match('>'))
            {
                return Token(TokenList::LambdaArrow, "~>", startLine, startCol);
            }

            if (!isAtEnd() && std::isalpha(peek()))
            {
                std::string varName;
                while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_'))
                {
                    varName += advance();
                }
                return Token(TokenList::TypeVar, varName, startLine, startCol);
            }
            throw std::runtime_error("Carácter inesperado después de '~'");
        case ':':
            return match(':') ? Token(TokenList::DoubleColon, "::", startLine, startCol)
                              : Token(TokenList::Colon, ":", startLine, startCol);
        case '.':
            return Token(TokenList::Dot, ".", startLine, startCol);
        case '_':
            return Token(TokenList::UnderScore, "_", startLine, startCol);
        case '@':
            return Token(TokenList::Arroba, "@", startLine, startCol);
        case '$':
            if (!isAtEnd() && std::isdigit(peek()))
            {
                std::string num;
                while (!isAtEnd() && std::isdigit(peek()))
                    num += advance();
                return Token(TokenList::ArgRef, num, startLine, startCol);
            }
            return Token(TokenList::Dollar, "$", startLine, startCol);
        case '+':
            return match('=') ? Token(TokenList::PlusAsing, "+=", startLine, startCol)
                              : Token(TokenList::Plus, "+", startLine, startCol);
        case '*':
            return match('=') ? Token(TokenList::MultAsing, "*=", startLine, startCol)
                              : Token(TokenList::Mult, "*", startLine, startCol);
        case '/':
            return match('>') ? Token(TokenList::AutoCloseTag, "/>", startLine, startCol) : match('=') ? Token(TokenList::DivAsing, "/=", startLine, startCol)
                                                                                                       : Token(TokenList::Slash, "/", startLine, startCol);
        case '%':
            return Token(TokenList::Percent, "%", startLine, startCol);
        case '=':
            return Token(TokenList::Assign, "=", startLine, startCol);
        case '|':
            return Token(TokenList::Pipe, "|", startLine, startCol);
        case '<':
            if (match('-'))
                return Token(TokenList::ArrowL, "<-", startLine, startCol);
            if (match('/'))
                return Token(TokenList::CloseTag, "</", startLine, startCol);
            return Token(TokenList::LPike, "<", startLine, startCol);
        case '>':
            return Token(TokenList::RPike, ">", startLine, startCol);
        case '-':
            if (match('>'))
                return Token(TokenList::ArrowR, "->", startLine, startCol);
            return readComparation(startLine, startCol);
        }

        throw std::runtime_error(std::string("Carácter inesperado: ") + c);
    }

    Token readComparation(int startLine, int startCol)
    {
        std::string lexeme("-");
        if (!isAtEnd() && std::isalpha(peek()))
        {
            while (!isAtEnd() && std::isalpha(peek()))
                lexeme += advance();

            if (lexeme == "-EQ")
                return Token(TokenList::EqualEqual, lexeme, startLine, startCol);
            if (lexeme == "-NE")
                return Token(TokenList::NotEqual, lexeme, startLine, startCol);
            if (lexeme == "-LT")
                return Token(TokenList::Less, lexeme, startLine, startCol);
            if (lexeme == "-GT")
                return Token(TokenList::Greater, lexeme, startLine, startCol);
            if (lexeme == "-LE")
                return Token(TokenList::LessEqual, lexeme, startLine, startCol);
            if (lexeme == "-GE")
                return Token(TokenList::GreaterEqual, lexeme, startLine, startCol);
        }
        return match('=') ? Token(TokenList::MinusAsing, "-=", startLine, startCol)
                          : Token(TokenList::Minus, "-", startLine, startCol);
    }
};
std::string TokenToString(TokenList type)
{
    switch (type)
    {
    case TokenList::LParent:
        return "LParent '('";
    case TokenList::RParent:
        return "RParent ')'";
    case TokenList::LBrace:
        return "LBrace '{'";
    case TokenList::RBrace:
        return "RBrace '}'";
    case TokenList::LBracket:
        return "LBracket '['";
    case TokenList::RBracket:
        return "RBracket ']'";
    case TokenList::LPike:
        return "LPike '<'";
    case TokenList::RPike:
        return "RPike '>'";
    case TokenList::Semicolon:
        return "Semicolon ';'";
    case TokenList::Colon:
        return "Colon ':'";
    case TokenList::Dot:
        return "Dot '.'";
    case TokenList::Dollar:
        return "Dollar '$'";
    case TokenList::Comma:
        return "Comma ','";

    case TokenList::Plus:
        return "Plus '+'";
    case TokenList::Minus:
        return "Minus '-'";
    case TokenList::Mult:
        return "Mult '*'";
    case TokenList::Slash:
        return "Slash '/'";
    case TokenList::Percent:
        return "Percent '%'";
    case TokenList::Assign:
        return "Assign '='";

    case TokenList::DoubleColon:
        return "DoubleColon '::'";
    case TokenList::CloseTag:
        return "CloseTag '</'";
    case TokenList::AutoCloseTag:
        return "AutoCloseTag '/>'";
    case TokenList::ArrowL:
        return "ArrowL '<-'";
    case TokenList::ArrowR:
        return "ArrowR '->'";
    case TokenList::LambdaArrow:
        return "LambdaArrow '~>'";

    case TokenList::PlusAsing:
        return "PlusAssign '+='";
    case TokenList::MinusAsing:
        return "MinusAssign '-='";
    case TokenList::MultAsing:
        return "MultAssign '*='";
    case TokenList::DivAsing:
        return "DivAssign '/='";

    case TokenList::EqualEqual:
        return "EqualEqual '-EQ'";
    case TokenList::NotEqual:
        return "NotEqual '-NE'";
    case TokenList::Less:
        return "Less '-LT'";
    case TokenList::Greater:
        return "Greater '-GT'";
    case TokenList::LessEqual:
        return "LessEqual '-LE'";
    case TokenList::GreaterEqual:
        return "GreaterEqual '-GE'";

    case TokenList::And:
        return "And 'and'";
    case TokenList::Or:
        return "Or 'or'";
    case TokenList::Not:
        return "Not 'not'";

    case TokenList::Identifier:
        return "Identifier";
    case TokenList::NumberLiteral:
        return "NumberLiteral";
    case TokenList::StringLiteral:
        return "StringLiteral";
    case TokenList::BoolLiteral:
        return "BoolLiteral";
    case TokenList::ArgRef:
        return "ArgRef '$n'";

    case TokenList::KwImport:
        return "KwImport 'import'";
    case TokenList::KwModule:
        return "KwModule 'module'";
    case TokenList::KwExport:
        return "KwExport 'export'";
    case TokenList::KwLet:
        return "KwLet 'let'";
    case TokenList::KwIf:
        return "KwIf 'if'";
    case TokenList::KwElse:
        return "KwElse 'else'";
    case TokenList::KwMatch:
        return "KwMatch 'match'";
    case TokenList::KwCase:
        return "KwCase 'case'";

    case TokenList::_EOF:
        return "EOF";

    default:
        return "UnknownToken";
    }
}
#endif