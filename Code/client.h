#ifndef CLIENT_H
#define CLIENT_H

#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib")

#define MAX_BUFFER 100
#define MAX_RECONNECT_COUNT 5

class Client
{
private:
    SOCKET server_socket;
    SOCKADDR_IN server_addr;
    WSADATA _wsadata;
    char server_ip[15];
    int server_port;
public:
    Client();
    ~Client();
    void send_message_to_server();
    void from_server_recv_message();
    void CheckError(bool bool_execute,const char *ErrorMessage);
    void input_limit(char input[]);
};

#endif