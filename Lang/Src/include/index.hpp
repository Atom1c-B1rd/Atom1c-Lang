// main.cpp
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Analyzer.hpp"
#include "Indications.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

// Declaraciones de backends (debes implementarlos)
void interpret(Indication* inds);
void compile_to_bytecode(Indication* inds, const char* outFile);
void transpile_to_python(Indication* inds, const char* outFile);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <archivo_fuente> [--backend=interpreter|bytecode|python]\n";
        return 1;
    }

    // 1. Leer archivo fuente
    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "No se pudo abrir el archivo: " << argv[1] << "\n";
        return 1;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    // 2. Lexer
    Lexer lexer(source);

    // 3. Parser
    Parser parser(lexer);
    ASTProgram* ast = nullptr;
    try {
        ast = parser.parseProgram();
    } catch (const std::runtime_error& e) {
        std::cerr << "Error de parsing: " << e.what() << "\n";
        return 1;
    }

    // 4. Analyzer (semántico)
    Analyzer analyzer;
    IndicationList indList = analyzer.Analyze(ast);
    Indication* inds = indList.Head();

    // 5. Elegir backend (por defecto intérprete)
    std::string backend = "interpreter";
    if (argc >= 3) {
        std::string arg = argv[2];
        if (arg.rfind("--backend=", 0) == 0) {
            backend = arg.substr(10);
        }
    }

    // 6. Ejecutar backend
    if (backend == "interpreter") {
        interpret(inds);
    } else if (backend == "bytecode") {
        compile_to_bytecode(inds, "output.bin");
        std::cout << "Bytecode generado en output.bin\n";
    } else if (backend == "python") {
        transpile_to_python(inds, "output.py");
        std::cout << "Código Python generado en output.py\n";
    } else {
        std::cerr << "Backend desconocido: " << backend << "\n";
        return 1;
    }
    return 0;
}