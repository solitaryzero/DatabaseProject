cmake_minimum_required(VERSION 2.6)
project (DatabaseProject)
add_compile_options(--std=c++14 -Wall -g -O3)

add_subdirectory(json11)

set(MODULES filesystem recordsystem indexsystem databasesystem)
set(NOEMPTY_MODULES "")
foreach(M ${MODULES})
    file(GLOB_RECURSE SOURCES ${M}/*.cpp)
    if(SOURCES)
        add_library(${M} STATIC ${SOURCES})
        target_link_libraries(${M} json11 ${NOEMPTY_MODULES})
        target_include_directories(${M} PUBLIC .)
        set(NOEMPTY_MODULES ${NOEMPTY_MODULES} ${M})
    endif()
endforeach()

find_package(FLEX)
find_package(BISON)
remove(NOEMPTY_MODULES databasesystem)

message(STATUS ${NOEMPTY_MODULES})

FLEX_TARGET(Lexer databasesystem/lexer.l ${CMAKE_CURRENT_BINARY_DIR}/mylexer.cc COMPILE_FLAGS -i)
BISON_TARGET(Parser databasesystem/yacc.y ${CMAKE_CURRENT_BINARY_DIR}/myyacc.cc COMPILE_FLAGS --defines=${CMAKE_CURRENT_BINARY_DIR}/myyacc.h)
target_sources(databasesystem PUBLIC ${BISON_Parser_OUTPUTS} ${FLEX_Lexer_OUTPUTS})
target_include_directories(databasesystem PUBLIC databasesystem)
set(NOEMPTY_MODULES ${NOEMPTY_MODULES} databasesystem)

message(STATUS ${NOEMPTY_MODULES})

file(GLOB TARGETS_CPP test/*.cpp)

foreach(TARGET_PATH ${TARGETS_CPP})
    string(REGEX REPLACE ".*/(.*)" "\\1" TARGET ${TARGET_PATH})
    string(REPLACE ".cpp" "" TARGET ${TARGET})
    message(STATUS TARGET ${TARGET})
    add_executable(${TARGET} ${TARGET_PATH})
    target_include_directories(${TARGET} PUBLIC .)
    if(NOEMPTY_MODULES)
        target_link_libraries(${TARGET} ${NOEMPTY_MODULES})
    endif()
    target_link_libraries(${TARGET} json11)
endforeach()
