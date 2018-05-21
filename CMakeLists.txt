cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(darlang)

find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

set(
  DARLANG_SOURCES
  src/lexer.cc
  src/parser.cc
  src/typing/typeable.cc
  src/typing/solver.cc
  src/typing/type_transform.cc
  src/backend/llvm_typer.cc
  src/backend/llvm_backend.cc
  src/main.cc
)

add_executable(darlang ${DARLANG_SOURCES})

set_property(TARGET darlang PROPERTY CXX_STANDARD 17)

llvm_map_components_to_libnames(llvm_libs support core)

target_link_libraries(darlang ${llvm_libs})

target_include_directories(darlang PUBLIC ${CMAKE_SOURCE_DIR}/src)

target_compile_options(darlang PRIVATE -Wall)
