cmake_minimum_required(VERSION 2.8.8)
project(BrainfuckJIT)


#WARNING: ubuntu 14.10 keeps dling old version which
find_path(LLVM_DIR LLVM-Config.cmake D:/llvm-3.5.1.src/build/share/llvm/cmake/)
find_package(LLVM REQUIRED CONFIG)


message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
#message(STATUS "BIN: ${LLVM_TOOLS_BINARY_DIR}llvm-config")

#Get ldflags
execute_process(COMMAND "${LLVM_TOOLS_BINARY_DIR}/llvm-config" --ldflags
                  OUTPUT_VARIABLE LLVM_LDFLAGS
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

#get C preprocessor flags
#TODO: try cxxflags instead, since it's in C++11, cxx might suffice
execute_process(COMMAND "${LLVM_TOOLS_BINARY_DIR}/llvm-config" --cppflags
                  OUTPUT_VARIABLE LLVM_CPPFLAGS
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

message(STATUS "LDFLAGS?: ${LLVM_LDFLAGS}")
message(STATUS "CPPFLAGS?: ${LLVM_CPPFLAGS}")


# Set your project compile flags.
# E.g. if  using the C++ header files
# you will need to enable C++11 support
# for your compiler.

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

#Set Linker and CXX flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x ${LLVM_CPPFLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LLVM_LDFLAGS}")

#set source files and add them to the executable.
set(SOURCE_FILES src/main.cpp src/Compiler.cpp src/Compiler.h)
add_executable(BrainfuckJIT ${SOURCE_FILES})

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs engine bitreader interpreter jit core ipo instrumentation scalaropts native)


# Link against LLVM libraries
target_link_libraries(BrainfuckJIT ${llvm_libs})



