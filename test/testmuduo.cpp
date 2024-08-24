/*
muduo网络库给用户提供了两个主要的类：
    TcpServer：用于编写服务器程序
    TcpClient：用于编写客户端程序
epoll+线程池
把网络IO的代码和业务代码区分开
               用户的连接和断开    用户的可读写事件
*/
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional>
#include <string>

using namespace std;
using namespace placeholders;
using namespace muduo;
using namespace muduo::net;
/*
1.组合TcpServer对象；
2.创建EventLoop事件循环对象的指针；
3.明确TcpServer构造函数需要的参数；
4.在当前服务器类的构造函数注册处理连接、数据读写的回调函数；
5.设置合适的服务端线程数量，muduo库会自己划分IO线程和worker线程；
*/
class ChatServer{
public:
    ChatServer(EventLoop* loop, const InetAddress& listenAddr,
               const string& nameArg)
               : _server(loop, listenAddr, nameArg)
               , _loop(loop){
        //给服务器注册用户连接的创建和断开回调，
        //由于不知道函数调用的时机，所以需要使用回调函数
        _server.setConnectionCallback(bind(&ChatServer::onConnection, this, _1));

        //给服务注册用户读写事件回调
        _server.setMessageCallback(bind(&ChatServer::onMessage, this, _1, _2, _3));
    
        //设置服务器端的线程数量，1个IO线程，3个worker线程
        _server.setThreadNum(4);
    }
    //开启事件循环
    void start(){
        _server.start();
    }
private:
    //专门处理用户的连接创建和断开
    void onConnection(const TcpConnectionPtr& conn){
        if(conn->connected()){
            cout << conn->peerAddress().toIpPort() << "->" <<
                    conn->localAddress().toIpPort() << endl;
        }else{
            conn->shutdown();
        }
    }
    //专门处理用户的读写事件
    void onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time){
        string buf = buffer->retrieveAllAsString();
        cout << "buf = " << buf << ", time = " << time.toFormattedString() << endl;
        conn->send(buf);
    }
    TcpServer _server;
    EventLoop *_loop;
};

int main(){
    EventLoop loop;
    InetAddress addr("127.0.0.1", 6000);

    ChatServer server(&loop, addr, "ChatServer");
    server.start();

    loop.loop(); //epoll_wait。以阻塞方式等待新用户连接等操作

    return 0;
}