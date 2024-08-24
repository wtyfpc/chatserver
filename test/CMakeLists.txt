cmake_minimum_required(VERSION 3.0)
project(main)

#配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

#配置头文件搜索路径
#include_directories()

#配置库文件搜索路径
#link_directories()

#设置需要编译的源文件列表
set(SRC_LIST ./testmuduo.cpp)

#设置可执行文件的输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#把 .指定路径下的所有源文件名字放入变量名SRC_LSIT
#aux_source_directory(. SRC_LIST)

#生成可执行文件
add_executable(server ${SRC_LIST})

#add_library() 生成库文件

#server这个目标程序需要链接这三个库文件
#三个库的链接顺序不可以乱
target_link_libraries(server muduo_net muduo_base pthread)

#可能是项目太小的原因，暂时看不出cmake的优越之处