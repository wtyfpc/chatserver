# chatserver
a cluster chat server implemented based on the muduo library

db.sql是建表语句
thirdparty目录包含了引用的第三方库，用于解析json
test目录包含了一些简单的测试文件
src目录是项目的实现代码
include目录包含一些头文件

编译：
    进入到build目录，先执行 cmake ..，再执行make
    编译好的二进制文件存放在bin目录下

涉及到的技术：
Nginx MySQL Redis的发布订阅 muduo网络库 cmake的使用

开发环境：Ubuntu20.04，gcc9.4.0
本机需要安装MySQL、Redis
安装muduo网络库、hiredis库、nginx