cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(darlang)

set(
  DARLANG_SOURCES
  src/lexer.cc
  src/main.cc
)

add_executable(darlang ${DARLANG_SOURCES})
