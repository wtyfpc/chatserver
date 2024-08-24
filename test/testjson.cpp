#include "json.hpp"
using json = nlohmann::json;

#include <iostream>
#include <vector>
#include <map>
using namespace std;

//序列化实例1
string func1(){
    json js;
    js["msg_type"] = 2;
    js["from"] = "zhangsan";
    js["to"] = "lisi";
    js["msg"] = "hello";
    string serial = js.dump();
    return serial;
}
//序列化实例2
string func2(){
    json js;
    js["id"] = {1, 2, 3, 4, 5};
    js["msg"]["zhangsan"] = "hello";
    js["msg"]["lisi"] = "world";
    string serial = js.dump();
    return serial;
}
//序列化实例3
string func3(){
    vector<int> vec = {1, 2, 3, 4};
    json js;
    js["vector"] = vec;
    return js.dump();
}
int main(){
    // string recvbuf = func3();
    // //json对象重载了输出运算符
    // json buf = json::parse(recvbuf);
    // //cout << buf["msg"] << endl;
    // auto vec = buf["vector"];
    // for(auto i : vec){
    //     cout << i << ' ';
    // }
    // cout << endl;

    //string buf = R"({"msgid": 12})";
    string buf = "{\"msgid\": 12";
    json js = json::parse(buf);
    cout << js["msgid"] << endl;
    return 0;
}