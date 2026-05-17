# Define a versão mínima do CMake
cmake_minimum_required(VERSION 3.10)

# Nome do teu projeto
project(DA_Project2)

# Usar C++17 (standard moderno e recomendado)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Diz ao compilador onde procurar pelos ficheiros .h (headers)
include_directories(.)

# Forma correta de apanhar todos os ficheiros .cpp nas respetivas pastas
file(GLOB SOURCES 
    "main.cpp"
    "cli/*.cpp"
    "core/*.cpp"
    "io/*.cpp"
    "data_structures/*.cpp"
    "algorithms/*.cpp"
)

# Cria o executável final com o nome "myProg"
add_executable(myProg ${SOURCES})