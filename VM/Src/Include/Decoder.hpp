#ifndef DECODER_HPP
#define DECODER_HPP
#include <vector>
#include <string>
enum class TokenDecorder
{
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

    //etc
};
typedef struct{
    TokenDecorder Type;
    size_t line;
}Decoder;
std::vector<Decoder> Decode(std::string Bytecode);

#endif