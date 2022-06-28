cmake_minimum_required(VERSION 3.1)

project(
  ModernCMakeExample
  VERSION 1.0
  LANGUAGES CXX)


add_library(libareduce  src/ahash.cpp  src/anetlist.cpp  src/anreducer.cpp  src/awriter.cpp)

#set_target_properties(libareduce areduce_test PROPERTIES CXX_EXTENSIONS OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(areduce_test test/reducer_study.cpp)
target_link_libraries(areduce_test PRIVATE libareduce)

#target_compile_features(libareduce   PUBLIC cxx_std_14)
#target_compile_features(areduce_test PUBLIC cxx_std_14)
