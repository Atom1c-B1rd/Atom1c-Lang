#include "Lexer.hpp"
#include "Parser.hpp"
#include "Analyzer.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
int CLI(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Uso: " << argv[0] << " <archivo>\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file)
    {
        std::cerr << "No se pudo abrir el archivo.\n";
        return 1;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    Lexer lexer(source);

    Parser parser(lexer);
    ASTProgram *ast = parser.parseProgram();
    std::cout << "Declaraciones en el programa: " << ast->declCount << std::endl;

    Analyzer analyzer;
    IndicationList list = analyzer.Analyze(ast);
    Indication *inds = list.Head();

    list.DebugPrint();
}