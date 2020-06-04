#include<iostream>
#include<cstring>
#include<thread>
#include"client.h"
#include"users.h"

//检查异常处理
void Client::CheckError(bool bool_execute, const char* ErrorMessage)
{
    if (bool_execute)
    {
        std::cout << ErrorMessage << std::endl;
        system("pause");
        exit(-1);
    }
}

//初始化
Client::Client()
{
    server_port = 2712;//端口可以任由系统分配
    strcpy(server_ip, "127.0.0.1");
    int error = WSAStartup(MAKEWORD(2, 2), &_wsadata);
    CheckError(error != 0, "初始化套接字库失败");
    CheckError(LOBYTE(_wsadata.wVersion) != 2 || HIBYTE(_wsadata.wHighVersion) != 2, "套接字库版本号不符");
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.S_un.S_addr = inet_addr(server_ip);
    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    CheckError(server_socket == INVALID_SOCKET, "启用套接字失败");
    for (int i = 0; i < 10; i++)
    {
        if (connect(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
        {
            std::cout << "服务器连接失败，正在重新连接" << std::endl;
            std::cout << WSAGetLastError() << std::endl;
            if (i == 9)
            {
                std::cout << "服务器多次连接失败，程序自动关闭" << std::endl;
                exit(-1);
            }
        }
        else
        {
            if (i > 0)
            {
                std::cout << "重新连接服务器成功" << std::endl;
            }
            else
            {
                std::cout << "连接到服务器成功" << std::endl;
            }
            break;
        }
    }
    std::cout << "输入!quit退出聊天室" << std::endl;
}

//清理环境
Client::~Client()
{
    closesocket(server_socket);
    WSACleanup();
}

void Client::send_message_to_server()
{
    User user;
    char send_buffer[MAX_BUFFER];
    //客户端另开线程专门接受服务器的转发
    std::thread recv_server_thread(&Client::from_server_recv_message, this);
    if (recv_server_thread.hardware_concurrency() == 1)
    {
        std::cout << "单核CPU运行多线程效率更低" << std::endl;
    }
    //recv_server_thread.detach();
    while (1)
    {
        std::cin.getline(send_buffer, MAX_BUFFER);
        if (strncmp(send_buffer, "!quit", 6) == 0)
        {
            send(server_socket, "!quit", 6, 0);
            recv_server_thread.join();
            break;
        }
        else if (strlen(send_buffer) > MAX_BUFFER)
        {
            std::cout << "输入字符超过最大限度！" << std::endl;
            continue;
        }
        send(server_socket, send_buffer, MAX_BUFFER, 0);
    }
}

void Client::from_server_recv_message()
{
    int server_flag;
    char recv_buffer[MAX_BUFFER];
    while (1)
    {
        server_flag = recv(server_socket, recv_buffer, MAX_BUFFER, 0);
        if (server_flag < 0)
        {
            std::cout << "服务器转发失败" << std::endl;
            std::cout << WSAGetLastError() << std::endl;
            system("pause");
            exit(-1);
        }
        else if (server_flag == 0)
        {
            std::cout << "服务器断开连接" << std::endl;
            break;
        }
        else
        {
            std::cout << recv_buffer << std::endl;
        }
    }
}