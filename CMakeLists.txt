cmake_minimum_required (VERSION 2.8)
project (WeakCH)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/")

# LIBS
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/iti-framework/")
# add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/lib/metis-5.1.0")

# Compiler-Options
# -ggdb -D _GLIBCXX_DEBUG -DNDEBUG 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG -std=c++11 -fopenmp ${FLAGS} -Wall -Wextra")

# MAIN
add_executable(contraction src/contraction/contraction.cpp)
add_executable(buildmacrocode src/contraction/macro.cpp)

add_executable(macrocustomization src/customization/macro_customization.cpp)
add_executable(builder src/customization/builder.cpp)

add_executable(queries src/query/query.cpp)

# add_executable(slow-customization src/customization/slow_customization.cpp)
# add_executable(chranks src/chranks.cpp)