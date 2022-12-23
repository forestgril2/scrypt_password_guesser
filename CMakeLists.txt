cmake_minimum_required(VERSION 3.5)

project(scrypt_password_guesser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Set the linker flags to use static libraries
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

add_executable(scrypt_password_guesser main.cpp)
