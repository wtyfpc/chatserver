#include "../../../include/server/model/offlinemessagemodel.hpp"
#include "../../../include/server/db/db.h"

vector<string> OfflineMsgModel::query(int userid){
    char sql[1024] = {0};
    sprintf(sql, "select message from offlinemessage where userid = %d", userid);
    vector<string> message;
    MySQL mysql;
    if(mysql.connect()){
        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr){
            MYSQL_ROW row;
            //可能有多行数据
            while((row = mysql_fetch_row(res)) != nullptr){
                message.emplace_back(row[0]);
            }
            mysql_free_result(res);
        }
    }
    return message;
}
void OfflineMsgModel::insert(int userid, string msg){
    char sql[1024] = {0};
    sprintf(sql, "insert into offlinemessage values(%d, '%s')", userid, msg.c_str());
    MySQL mysql;
    if(mysql.connect()){
        mysql.update(sql);
    }
}
void OfflineMsgModel::remove(int userid){
    char sql[1024] = {0};
    sprintf(sql, "delete from offlinemessage where userid = %d", userid);
    MySQL mysql;
    if(mysql.connect()){
        mysql.update(sql);
    }
}