cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(darlang)

find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# darlib: support library for darlang AST and codegen
set(
  DARLIB_SOURCES

  src/lexer.cc
  src/parser.cc

  src/typing/typeable.cc
  src/typing/primitive_solver.cc
  src/typing/tuple_solver.cc
  src/typing/function_solver.cc
  src/typing/function_specializer.cc
  src/typing/type_transform.cc

  src/backend/llvm_typer.cc
  src/backend/llvm_backend.cc
)

add_library(darlib ${DARLIB_SOURCES})
set_property(TARGET darlib PROPERTY CXX_STANDARD 14)
llvm_map_components_to_libnames(llvm_libs support core)
target_link_libraries(darlib ${llvm_libs})
target_include_directories(darlib PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_compile_options(darlib PRIVATE -Wall)

# darlib_test
set(
  DARLIB_TEST_SOURCES

  src/typing/tuple_solver_test.cc
  src/darlib_test.cc
)
add_executable(darlib_test ${DARLIB_TEST_SOURCES})
set_property(TARGET darlib_test PROPERTY CXX_STANDARD 14)
target_include_directories(darlib_test PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(darlib_test darlib)

enable_testing()
add_test(NAME darlib_test COMMAND darlib_test)

# dac: the darlang compiler
set(DAC_SOURCES src/dac.cc)
add_executable(dac ${DAC_SOURCES})
set_property(TARGET dac PROPERTY CXX_STANDARD 14)
target_include_directories(dac PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(dac darlib)
