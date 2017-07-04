cmake_minimum_required(VERSION 3.5)
project(sqlite_demo1)

add_custom_target(
        extern_lib
        COMMAND make sqlite3.c
        WORKING_DIRECTORY /home/zhouyao/workspace/jetbrains/ClionProjects/sqlite_source_code/sqlite-read-2017/bld
)
#add_executable(myexecutable myexcutable.c)
#target_link_libraries(myexecutable full_path_to_generated_library)


add_definitions(-DSQLITE_DEBUG -DSQLITE_MEMDEBUG)
#add_definitions(-DSQLITE_FORCE_OS_TRACE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -dl")

set(SOURCE_FILES demo_main.cpp  sqlite3.h /home/zhouyao/workspace/jetbrains/ClionProjects/sqlite_source_code/sqlite-read-2017/bld/sqlite3.c)
add_executable(sqlite_demo1 ${SOURCE_FILES} )

target_link_libraries(sqlite_demo1 ${CMAKE_DL_LIBS})
add_dependencies(sqlite_demo1 extern_lib)
