#include "../../include/server/model/groupmodel.hpp"
#include "../../include/server/db/db.h"

bool GroupModel::createGroup(Group &group){
    char sql[1024] = {0};
    sprintf(sql, "insert into allgroup(groupname, groupdesc) values('%s', '%s')", group.getName().c_str(), group.getDesc().c_str());
    MySQL mysql;
    if(mysql.connect()){
        if(mysql.update(sql)){
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

//加入群组
void GroupModel::addGroup(int userid, int groupid, string role){
    char sql[1024] = {0};
    sprintf(sql, "insert into groupuser values(%d, %d, '%s')", groupid, userid, role.c_str());
    MySQL mysql;
    if(mysql.connect()){
        mysql.update(sql);
    }
}

//查询用户群组信息
//查找userid用户所在的群组信息，包括群组名、各个群组的成员
vector<Group> GroupModel::queryGroups(int userid){
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.groupname, a.groupdesc from allgroup a inner join groupuser b on a.id = b.groupid where b.userid = %d", userid);
    
    //存储该用户所在的群组，以及每个群组里面的用户信息
    vector<Group> groupvec;
    
    MySQL mysql;
    if(mysql.connect()){
        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr){
            MYSQL_ROW row;
            //可能有多行数据
            while((row = mysql_fetch_row(res)) != nullptr){
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupvec.push_back(group);
            }
            mysql_free_result(res);
        }
    }
    //查询群组的用户信息，每个group是一个群组
    for(Group &group : groupvec){
        sprintf(sql, "select a.id, a.name, a.state, b.grouprole from user a inner join groupuser b on b.uerid = a.id where b.groupid = %d", group.getId());
        MySQL mysql;
        if(mysql.connect()){
            MYSQL_RES* res = mysql.query(sql);
            if(res != nullptr){
                MYSQL_ROW row;
                //可能有多行数据
                while((row = mysql_fetch_row(res)) != nullptr){
                    GroupUser groupUser;
                    groupUser.setId(atoi(row[0]));
                    groupUser.setName(row[1]);
                    groupUser.setState(row[2]);
                    groupUser.setRole(row[3]);
                    group.getUsers().push_back(groupUser);
                }
                mysql_free_result(res);
            }
        }
    }
    return groupvec;
}

//根据groupid查询群组用户id列表，排除userid，主要用于群发消息
vector<int> GroupModel::queryGroupUsers(int userid, int groupid){
    char sql[1024] = {0};
    sprintf(sql, "select userid from groupuser where groupid = %d and userid != %d", groupid, userid);
    
    vector<int> vec;
    
    MySQL mysql;
    if(mysql.connect()){
        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr){
            MYSQL_ROW row;
            //可能有多行数据
            while((row = mysql_fetch_row(res)) != nullptr){
                vec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return vec;
}