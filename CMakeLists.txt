cmake_minimum_required(VERSION 3.15)
project(atomic-dsp-graph VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configurações de Compilador (Otimização Máxima)
if(MSVC)
    add_compile_options(/O2 /arch:AVX2)
else()
    add_compile_options(-O3 -march=native -Wall -Wextra -pthread)
endif()

# Diretórios de Include
include_directories(include)

# Executável Principal
add_executable(mixer_app src/main.cpp)

# Testes
enable_testing()
add_executable(unit_tests test/test_mixer.cpp)
add_test(NAME CoreTests COMMAND unit_tests)