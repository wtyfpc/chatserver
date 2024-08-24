#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include "group.hpp"
#include <vector>
#include <string>

using namespace std;

class GroupModel{
public:
    //创建群组
    bool createGroup(Group& group);

    //加入群组
    void addGroup(int userid, int groupid, string role);

    //查询用户群组信息
    vector<Group> queryGroups(int userid);
    
    //根据groupid查询群组用户id列表，排除userid，
    //主要用户userid用户给群组中的其它用户发送消息
    vector<int> queryGroupUsers(int userid, int groupid);
};
#endif