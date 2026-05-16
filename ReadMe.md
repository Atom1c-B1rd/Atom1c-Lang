# Atom1c-Lang

A hypertext-inspired programming language with functional roots and two syntactic modes.

Functions are defined as XML-style tags with type signatures in the tag name.
Outside `where` blocks, the language uses Lisp-style prefix notation.
Inside `where` blocks, it shifts to an imperative JS/Haskell hybrid.

Built from scratch in C/C++ with a complete Lexer, Parser, and AST.

---

## Syntax at a glance

**Hello World:**
```
@module main

<main::IO()>
  <putStrLn $ "Hello World"/>
</main>
```

**Functions** — type signature in the tag, positional args with `$n`:
```
<suma::Number->Number->Number>
  + $0 $1
</suma>
```

**Fibonacci** — Lisp-style prefix notation:
```
<fib::Number->Number>
  ((-LT $0 2) 1 (* $0 (<fib $ (- $0 1)/>) ))
</fib>
```

**Pattern matching:**
```
<describirColor::Color->Texto>
  ($0
   (Rojo-> "Es rojo")
   (Verde-> "Es verde")
   (Azul-> "Es azul")
   ((Personalizado texto)-> (+ "Personalizado: " texto))
  )
</describirColor>
```

**Lambdas** with `~>`:
```
<applyTwice::(~a->~a)->~a->~a>
  ~> <$0 $ (<$0 $ $1/>) />
</applyTwice>
```

**Block mode** — inside `where`, imperative syntax with `let`, `if`, `match`, `loop`:
```
<main::IO()>
  do()
  where {
    let nombre:Texto = <- $ "Ingrese su nombre: ";
    let edad:Number  = <- $ "Ingrese su edad: ";

    if((-LT edad 18)){
      putStrLn $ "Eres menor de edad";
    }else{
      putStrLn $ "Eres mayor de edad";
    };

    loop {
      let entrada:Texto = <- $ "Escribe 'salir' para terminar: ";
      if((-EQ entrada "salir")){ break; };
      putStrLn $ (+ "Escribiste: " entrada);
    };
  };
  putStrLn $ "¡Programa finalizado!"
</main>
```

---

## Type system

**Primitives:** `Number`, `String`, `Bool`, `Void`, `Any`

**Type aliases:**
```
<type Texto::String />
```

**Data types:**
```
<data Persona>
  nombre:Texto;
  edad:Number;
</data>
```

**Enums with constructors:**
```
<enum Color>
  Rojo $ Verde $ Azul $ Personalizado Texto
</enum>
```

**Generics** with `~a`:
```
<id::~a -> ~a>
  $0
</id>
```

**Maybe:**
```
let maybeValor:Maybe<Number> = Just $ 42;
match(maybeValor){
  case Nothing -> putStrLn $ "No hay valor";
  case (Just n) -> putStrLn $ (+ "El valor es: " (<toString $ n/>));
};
```

---

## Two syntactic modes

| Context | Style | Constructs |
|---|---|---|
| Outside `where` | Lisp-style prefix | function calls, pattern match, if-then-else, lambdas |
| Inside `where` | JS/Haskell hybrid | `let`, `if/else`, `match/case`, `loop`, `break` |

---

## Module system

```
@module MyModule
@export suma, fib, main
@import OtherModule
@import OtherModule as OM
```

---

## Implementation

Written in C/C++ from scratch:

- **Lexer** — full tokenizer with 60+ token types, nested block comments, string escape sequences, positional error tracking
- **Parser** — recursive descent parser producing a typed AST
- **AST** — complete node hierarchy: programs, declarations, statements, expressions, patterns, types
- **VM** — bytecode virtual machine (in progress)
- **Interpreter / Compiler** — planned

Build system: CMake

---

## Project structure

```
Atom1c-Lang/
├── Lang/
│   ├── Src/include/
│   │   ├── Lexer.hpp       # Tokenizer
│   │   ├── Parser.hpp      # Recursive descent parser
│   │   ├── Ast.hpp         # Full AST node definitions
│   │   ├── Analyzer.hpp    # Semantic analysis
│   │   └── Interpreter/    # In progress
│   └── main.cpp
├── VM/
│   ├── Src/Include/
│   │   ├── VM.hpp          # Virtual machine
│   │   ├── Kernel.hpp
│   │   └── Decoder.hpp     # Bytecode decoder
│   └── main.cpp
└── examples/
    ├── helloworld.atl
    ├── example.atl
    └── fulltest.atl        # Full language feature test
```

---

## Status

- [x] Lexer
- [x] Parser
- [x] AST
- [x] Examples running
- [ ] Interpreter / VM complete
- [ ] Standard library
- [ ] JSON config for tooling

---

Built by [Guillermo Lescano](https://github.com/Atom1c-B1rd)
