#ifndef SERVER_H
#define SERVER_H

#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4996)

#define MAX_SIZE 100
#define MAX_ONLINE 4    
const char ROOT[]="!star";  //获得权限指令

class chat_server
{
private:
    WSADATA _wsadata;
    int online_people;
    char server_ip[15];
    int server_port;
    SOCKET server_socket;
    SOCKADDR_IN server_addr;
    SOCKET client_socket;
    SOCKADDR_IN client_addr;
    int size;
public:
    chat_server();
    ~chat_server();
    void wait_for_client_connect();
    void create_client_thread(SOCKET client);
    void send_server_information(SOCKET root);
    bool send_message_to_other_user(SOCKET speak_client, char* buffer);
    void administrator(SOCKET root);
};

#endif