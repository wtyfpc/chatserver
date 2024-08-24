#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.hpp"

class UserModel{
public:
    //向数据库中添加一个新的用户
    bool insert(User &user);
    //根据id查询用户
    User query(int id);
    //更新用户登录状态
    bool updateState(User &user);

    //重置用户的状态信息;
    void resetState();
};

#endif