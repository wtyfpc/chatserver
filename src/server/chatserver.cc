#include "../../include/server/chatserver.hpp"
#include "../../include/server/chatservice.hpp"
#include "../../thirdparty/json.hpp"

#include <functional>
#include <string>

using namespace std;
using namespace placeholders;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop* loop,
        const InetAddress& listenAddr,
        const string& nameArg)
        : _server(loop, listenAddr, nameArg)
        , _loop(loop){
    //注册连接回调函数
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));

    //注册消息回调函数
    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

    _server.setThreadNum(4);
}

void ChatServer::start(){
    _server.start();
}

void ChatServer::onConnection(const TcpConnectionPtr& conn){
    if(!conn->connected()){ //客户端断开连接
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}

void ChatServer::onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time){
    string buf = buffer->retrieveAllAsString();
    //数据的反序列化
    json js = json::parse(buf);

    MsgHandler handler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    handler(conn, js, time);
}