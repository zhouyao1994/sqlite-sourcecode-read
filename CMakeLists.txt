cmake_minimum_required(VERSION 3.5)
project(sqlite_demo1)

#set(CMAKE_CXX_STANDARD 11
#set (ttt "-SQLITE_DEBUG")
#set(CMAKE_CXX_FLAGS_DEBUG "-SQLITE_DEBUG=1 -SQLITE_ENABLE_IOTRACE")
#add_definitions(-DSQLITE_DEBUG -DSQLITE_ENABLE_EXPLAIN_COMMENTS -DSQLITE_ENABLE_SELECTTRACE -DSQLITE_ENABLE_WHERETRACE)
add_definitions(-DSQLITE_DEBUG -DSQLITE_MEMDEBUG)
#add_definitions(-DSQLITE_FORCE_OS_TRACE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -dl")

set(SOURCE_FILES main.cpp src/sqlite3-all.c)
add_executable(sqlite_demo1 ${SOURCE_FILES} )

target_link_libraries(sqlite_demo1 ${CMAKE_DL_LIBS})