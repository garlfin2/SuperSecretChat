cmake_minimum_required(VERSION 3.30)
project(Discord)

set(CMAKE_CXX_STANDARD 23)

set(COMPILE_FLAGS_RELEASE "-Wall -Werror -O3 -static -s -static-libgcc -mwindows")
set(COMPILE_FLAGS_DEBUG "-Wall -Werror -O0 -DDEBUG -static -static-libgcc -mwindows")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${COMPILE_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${COMPILE_FLAGS_RELEASE}")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${COMPILE_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${COMPILE_FLAGS_DEBUG}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(Secretest resources.o App/main.cpp Secretest/Windowing/Window.cpp Secretest/Networking/Socket.cpp App/ClientWindow.cpp)
add_executable(SecretestServer resources.o Server/main.cpp Secretest/Windowing/Window.cpp Secretest/Networking/Socket.cpp Server/ServerWindow.cpp)

target_link_libraries(Secretest PRIVATE "-lcomctl32 -lstdc++exp -lws2_32")
target_link_libraries(SecretestServer PRIVATE "-lcomctl32 -lstdc++exp -lws2_32")
