# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(tls_tcp_client VERSION 0.1.0 LANGUAGES C)

# Set C standard (adjust to your needs: 99, 11, 17, etc.)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Common warning flags for both GCC and Clang
add_compile_options(-Wall -Wextra -fsanitize=address -fno-omit-frame-pointer -g)
add_link_options(-fsanitize=address)

# Dependencies
find_package(OpenSSL REQUIRED)

add_executable(tls_tcp_client
    src/client.c
)

target_include_directories(tls_tcp_client PRIVATE include)

target_link_libraries(tls_tcp_client PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
)