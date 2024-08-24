#ifndef GROUPUSER_H
#define GROUPUSER_H

#include "user.hpp"

//群组用户，多了一个role信息
class GroupUser : public User{
public:
    void setRole(string role){
        this->role = role;
    }
    string getRole(){
        return role;
    }
private:
    string role;
};

#endif