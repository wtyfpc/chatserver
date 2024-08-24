#include "../../include/server/chatserver.hpp"
#include "../../include/server/chatservice.hpp"
#include <iostream>
#include <signal.h>

using namespace std;

//处理 Ctrl+C结束后，服务器的状态信息
void resetHandler(int signum){
    ChatService::instance()->reset();
    exit(0);
}
int main(int argc, char* argv[]){
    if(argc != 3){
        cerr << "example: ./chatServer 127.0.0.1 6000" << endl;
        exit(-1);
    }
    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);

    signal(SIGINT, resetHandler);

    EventLoop loop;
    InetAddress addr(ip, port);
    ChatServer server(&loop, addr, "chatserver");
    server.start();
    loop.loop();
    return 0;
}