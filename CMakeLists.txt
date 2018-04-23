cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(darlang)

set(
  DARLANG_SOURCES
  src/lexer.cc
  src/parser.cc
  src/main.cc
)

add_executable(darlang ${DARLANG_SOURCES})
