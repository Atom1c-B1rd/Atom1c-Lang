cmake_minimum_required(VERSION 3.10)

project(Atom1c-Lang)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Buscar todos los .cpp dentro de Src
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/target)
file(GLOB_RECURSE SRC_FILES 
    Src/*.cpp
)

# Crear ejecutable
add_executable(${PROJECT_NAME}
    main.cpp
    ${SRC_FILES}
)

# Incluir headers
target_include_directories(${PROJECT_NAME} PRIVATE
    Src/Include
)