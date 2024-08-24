#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include "../../thirdparty/json.hpp"
#include "../public.hpp"
#include "./usermodel.hpp"
#include "./friendmodel.hpp"
#include "./offlinemessagemodel.hpp"
#include "./groupmodel.hpp"
#include "./redis/redis.hpp"
#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>

using namespace std;
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;

//处理消息的事件回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr& conn, json& js, Timestamp time)>;

//业务类
class ChatService{
public:
    //单例模式
    static ChatService* instance();
    //登录
    void login(const TcpConnectionPtr& conn, json& js, Timestamp time);
    //注册
    void reg(const TcpConnectionPtr& conn, json& js, Timestamp time);
    //注销
    void loginout(const TcpConnectionPtr& conn, json& js, Timestamp time);
    //一对一聊天
    void oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time);
    //处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr& conn);
    //添加好友
    void addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time);

    //创建群组
    void createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);
    //加入群组
    void addGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);
    //群组聊天
    void chatGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);

    //服务器异常后，业务重置方法
    void reset();

    //获取消息对应的处理器
    MsgHandler getHandler(int msgId);

    //从redis中订阅消息
    void handleRedisSubscribeMessage(int userid, string msg);

private:
    ChatService();

    //存储消息id和其对应的业务处理方法
    unordered_map<int, MsgHandler> _msgHandlerMap;

    //存储在线用户的通信连接
    mutex _connMutex;
    unordered_map<int, TcpConnectionPtr> _userConnMap;

    //数据操作类对象
    UserModel _userModel;

    //离线消息操作类对象
    OfflineMsgModel _offlineMessageModel;

    //好友列表操作类对象
    FriendModel _friendModel;

    //群组操作类对象
    GroupModel _groupModel;

    Redis _redis;

};

#endif