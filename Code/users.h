#ifndef USERS_H
#define USERS_H

#define MAX_MESSAGE 100

class User
{
public:
    int id;
    char my_chat_message[MAX_MESSAGE];//个人聊天信息，后期服务器转发给其他客户端
    User();
    char* show_speak_time_and_id();
};

#endif
