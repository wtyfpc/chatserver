#include "../../include/server/chatservice.hpp"
#include "../../include/public.hpp"

#include <map>
#include <muduo/base/Logging.h>
#include <vector>

using namespace std;
using namespace muduo;

ChatService *ChatService::instance() {
  static ChatService service;
  return &service;
}

ChatService::ChatService() {
  _msgHandlerMap.insert(
      {LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
  _msgHandlerMap.insert(
    {LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});
  _msgHandlerMap.insert(
      {REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
  _msgHandlerMap.insert(
      {ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
  _msgHandlerMap.insert(
      {ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});

  _msgHandlerMap.insert(
    {CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
  _msgHandlerMap.insert(
    {ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
  _msgHandlerMap.insert(
    {GROUP_CHAT_MSG, std::bind(&ChatService::chatGroup, this, _1, _2, _3)});

  if(_redis.connect()){
    _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));

  }

}

MsgHandler ChatService::getHandler(int msgId) {
  auto it = _msgHandlerMap.find(msgId);
  if (it == _msgHandlerMap.end()) {
    // 返回一个默认的处理器
    return [=](const TcpConnectionPtr &conn, json &js, Timestamp time) {
      LOG_ERROR << "can't find handler!";
    };
  } else {
    return it->second;
  }
}

// 登录业务 id password
// json格式：{"msgid": ,"id": ,"password": }
void ChatService::login(const TcpConnectionPtr &conn, json &js,
                        Timestamp time) {
  int id = js["id"]; // 相当于qq号登录
  string pwd = js["password"];

  User user = _userModel.query(id);
  if (user.getId() == id && user.getPwd() == pwd) {
    if (user.getState() == "online") { // 不允许重复登录
      json response;
      response["msgid"] = REG_MSG_ACK;
      response["errno"] = 2;
      response["errmsg"] = "用户已登录";
      conn->send(response.dump());
    } else { // 登录成功
      // 更新状态信息
      user.setState("online");
      _userModel.updateState(user); // 数据库中

      {
        lock_guard<mutex> lock(_connMutex);
        _userConnMap.insert({id, conn}); // 内存中
      }

      //用户登录成功后，向redis订阅channel
      _redis.subscribe(id);

      // 返回给用户一些信息
      json response;
      response["msgid"] = LOGIN_MSG_ACK;
      response["errno"] = 0;
      response["id"] = user.getId();
      response["name"] = user.getName();

      // 查询离线消息表中有没有发送给该客户端的消息
      vector<string> vec = _offlineMessageModel.query(id);
      if (!vec.empty()) {
        response["offlinemsg"] = vec;    // json可以序列化vector
        _offlineMessageModel.remove(id); // 删除该用户的离线消息
      }

      // 查询该用户的好友信息并返回
      vector<User> userVec = _friendModel.query(id);
      // if(userVec.empty()){
      //     LOG_INFO << "empty";
      // }
      if (!userVec.empty()) {
        vector<string> vec2;
        for (User &user : userVec) {
          json js;
          js["id"] = user.getId();
          js["name"] = user.getName();
          js["state"] = user.getState();
          vec2.push_back(js.dump());
        }
        response["friends"] = vec2;
        // LOG_INFO << "friend list";
      }

      conn->send(response.dump());
    }
  } else { // 登录失败
    // 还可以细分各种错误原因
    json response;
    response["msgid"] = REG_MSG_ACK;
    response["errno"] = 1;
    response["errmsg"] = "用户名或密码错误";
    conn->send(response.dump());
  }
}
//注销
void ChatService::loginout(const TcpConnectionPtr& conn, json& js, Timestamp time){
  int id = js["id"].get<int>();
  {
    lock_guard<mutex> lock(mutex);
    auto it = _userConnMap.find(id);
    if(it != _userConnMap.end()){
      _userConnMap.erase(it);
    }
  }

  //取消订阅
  _redis.unsubscribe(id);

  User user(id, "", "", "offline");
  _userModel.updateState(user);
}
// 处理注册业务 name password
// 业务操作的都是数据对象
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time) {
  string name = js["name"];
  string pwd = js["password"];

  User user;
  user.setName(name);
  user.setPwd(pwd);

  bool state = _userModel.insert(user);
  if (!state) { // 注册失败
    json response;
    response["msgid"] = REG_MSG_ACK;
    response["errno"] = 1;
    conn->send(response.dump());
  } else {
    json response;
    response["msgid"] = REG_MSG_ACK;
    response["errno"] = 0;
    response["id"] = user.getId();
    conn->send(response.dump());
  }
}

void ChatService::clientCloseException(const TcpConnectionPtr &conn) {
  // 由conn查找对应的用户id
  // 清除_userConnMap中的conn
  // 更改id对应的数据库表项
  User user;
  // 这样查找有很大的性能问题，但是没有办法，
  // 这种低效的做法是由顶层设计决定的，接口设计的有问题
  {
    lock_guard<mutex> lock(_connMutex);
    for (auto it = _userConnMap.begin(); it != _userConnMap.end(); it++) {
      if (it->second == conn) {
        user.setId(it->first);
        _userConnMap.erase(it);
        user.setState("offline");
        break;
      }
    }
  }

  //
  _redis.unsubscribe(user.getId());

  if (user.getId() != -1) {
    // LOG_INFO << "update user's state";
    // LOG_INFO << user.getId();
    _userModel.updateState(user);
  }
}

// 一对一聊天业务
// json格式：{"msgid": ,"id":1,"name":"zhang san","to":3,"msg":"XXXX"}
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js,
                          Timestamp time) {
  int to = js["toid"].get<int>();

  // 是否需要先在数据库中查找对方是否存在？

  {
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(to);
    // 服务器主动推送消息
    // 保证线程安全
    //在同一台服务器
    if (it != _userConnMap.end()) {
      it->second->send(js.dump());
      return;
    }
  }
  //查询是否在线，如果在线，说明他们不在一台服务器上
  User user = _userModel.query(to);
  if(user.getState() == "online"){
    _redis.publish(to, js.dump());
    return;
  }
  // TODO：存储离线消息
  // 为什么要存储这么多信息，只存储js里的具体消息不就行了吗？
  _offlineMessageModel.insert(to, js.dump());
}

// 业务重置方法
void ChatService::reset() {
  // 由于此时业务处理逻辑异常退出，所以需要重置各个用户的登录状态等信息
  _userModel.resetState();
  exit(0);
}

// 添加好友
//"msgid": ,"id": ,"friendid":
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js,
                            Timestamp time) {
  int userid = js["id"].get<int>();
  int friendid = js["friendid"].get<int>();

  // 或者可以先在用户表中查找要添加的好友id是否存在

  _friendModel.insert(userid, friendid);
}

// 创建群组
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    Group group(-1, name, desc);
    if(_groupModel.createGroup(group)){
        //群组创建人
        _groupModel.addGroup(userid, group.getId(), "creator");
    }
}
// 加入群组
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");
}
//群组聊天
void ChatService::chatGroup(const TcpConnectionPtr& conn, json& js, Timestamp time){
    //根据群组id查找该组内的所有用户
    //向除自己之外的所有用户发送消息，如果该用户不在线，需要把消息暂存起来

    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    auto vec = _groupModel.queryGroupUsers(userid, groupid);
    
    lock_guard<mutex> lock(_connMutex);// 不是太理解锁为什么要放在这里

    for(int id : vec){     
        auto it = _userConnMap.find(id);
        if(it != _userConnMap.end()){
            it->second->send(js.dump());
        }else{
          User user = _userModel.query(id);
          if(user.getState() == "online"){//对方在其它的服务器上，需要向redis中发布消息
            _redis.publish(id, js.dump());
          }else{
            // 存储离线消息
            _offlineMessageModel.insert(id, js.dump());
          }
        }
    }
}

//从redis中订阅消息
void ChatService::handleRedisSubscribeMessage(int userid, string msg){
  lock_guard<mutex> lock(_connMutex);
  auto it = _userConnMap.find(userid);
  if(it != _userConnMap.end()){
    it->second->send(msg);
    return;
  }
  //取回消息的过程中，发现对方下线，需要把消息存储在数据库中
  _offlineMessageModel.insert(userid, msg);
}