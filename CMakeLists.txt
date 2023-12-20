# CMakeList.txt: ConnectPool 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)


#指定c++编译版本
set(CMAKE_CXX_STANDARD 11)

# 判断系统是否为Windows

if(WIN32)
    message("Current system: WIN32")
    set(MYSQL_INCLUDE_DIR "C:\\Program Files\\MySQL\\MySQL Server 5.7\\include")
    set(MYSQL_LIBRARY_DIR "C:\\Program Files\\MySQL\\MySQL Server 5.7\\lib")
    set(MYSQL_LIB_NAME "libmysql.dll")
endif()

if(UNIX AND NOT APPLE)
    message("Current system: UNIX")
    set(MYSQL_INCLUDE_DIR "/usr/local/include")
    set(MYSQL_LIBRARY_DIR "/usr/local/lib")
    set(MYSQL_LIB_NAME "mysqlclient")

    set(CMAKE_C_COMPILER "/opt/rh/devtoolset-8/root/usr/bin/gcc")
    set(CMAKE_CXX_COMPILER "/opt/rh/devtoolset-8/root/usr/bin/g++")
endif()


project ("ConnectPool")
message("Current system: ${CMAKE_SYSTEM_NAME}")
aux_source_directory(. SRC)
include_directories(${MYSQL_INCLUDE_DIR})

set(LIB_JSON json11)

add_subdirectory(json11)

link_libraries(${LIB_JSON})
link_directories(${MYSQL_LIBRARY_DIR})

# 设置可执行文件生成路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
add_executable (ConnectPool ${SRC})

target_link_libraries(ConnectPool ${MYSQL_LIB_NAME})

