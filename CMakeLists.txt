cmake_minimum_required(VERSION 3.15)
project(MixerDeterministico VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Otimizações para a biblioteca principal
if(MSVC)
    add_compile_options(/O2 /arch:AVX2)
else()
    add_compile_options(-O3 -march=native -Wall -Wextra -pthread)
endif()

include_directories(include)

# --- BIBLIOTECA CORE ---
# Separamos o core para poder linkar tanto no executável principal quanto nos testes
add_library(mixer_core 
    src/audio_engine.cpp
)

# --- EXECUTÁVEL PRINCIPAL ---
add_executable(mixer_app src/main.cpp)
target_link_libraries(mixer_app PRIVATE mixer_core)

# --- TESTES UNITÁRIOS (GoogleTest) ---
enable_testing()

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(unit_tests test/unit_tests.cpp)
target_link_libraries(unit_tests PRIVATE mixer_core gtest_main)

include(GoogleTest)
gtest_discover_tests(unit_tests)