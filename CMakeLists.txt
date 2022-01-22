cmake_minimum_required(VERSION 3.5)

project(scrypt_password_guesser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(scrypt_password_guesser main.cpp)
