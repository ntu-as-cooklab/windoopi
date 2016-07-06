cmake_minimum_required (VERSION 2.6)

project(WindooPi)

# Dirs

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
link_directories(${CMAKE_BINARY_DIR}/lib)

# Flags

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast -s")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -W -pedantic -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Ofast -s")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -W -pedantic -Wextra")

if (WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
endif (WIN32)

# Include headers

include_directories("include")

# Sources

file(GLOB WINDOOPI_SRC "src/*.cpp")
add_executable(windoopi ${WINDOOPI_SRC})

if (WIN32)
    target_link_libraries(windoopi fftw3.a portaudio.a winmm setupapi winpthread.a stdc++.a gcc.a winpthread.a)
endif (WIN32)

if (UNIX)
target_link_libraries(windoopi wiringPi fftw3 portaudio m)
endif (UNIX)

install(TARGETS windoopi DESTINATION /usr/bin/)
