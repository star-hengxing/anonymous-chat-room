#ifndef USERS_H
#define USERS_H

class User
{
public:
    int id;
    bool user_online;//用户是否在线
    char my_chat_message[100];//个人聊天信息，后期服务器转发给其他客户端
    User();
    char* show_speak_time_and_id();
};

#endif
