cmake_minimum_required(VERSION 3.0.2)

#project name
project(https-coro)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

add_compile_options(-Wall)
add_compile_options(-Werror)
add_compile_options(-Wpedantic)
add_compile_options(-Wfatal-errors)
add_compile_options(-pipe)
add_compile_options(-fPIC)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -march=native -mtune=native")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DNDEBUG -march=native -mtune=native")

if(UNIX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
elseif(WIN32)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
endif(UNIX)

find_package(Threads REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_search_module(OPENSSL REQUIRED openssl)

message("OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
message("OpenSSL libraries: ${OPENSSL_LIBRARIES}")
include_directories(${OPENSSL_INCLUDE_DIR})

#set(Boost_DEBUG 1)
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_DEBUG_LIBS        OFF)  # ignore debug libs and
set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
set(Boost_NO_SYSTEM_PATHS       OFF)
set(BOOST_COMPONENTS)

list(APPEND BOOST_COMPONENTS system coroutine)
find_package(Boost COMPONENTS ${BOOST_COMPONENTS})

#build file list
file(GLOB HPP_FILES "${CMAKE_CURRENT_LIST_DIR}/*.hpp")
file(GLOB CPP_FILES "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
set(ALL_FILES ${HPP_FILES} ${CPP_FILES} ${MOC_LIST})

add_executable(${PROJECT_NAME} ${ALL_FILES})

#linking
target_link_libraries(${PROJECT_NAME} Threads::Threads)
target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES})
target_link_libraries(${PROJECT_NAME} Boost::coroutine)
