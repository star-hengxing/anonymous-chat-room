#include<iostream>
#include<ctime>
#include<winsock2.h>
#include<thread>
#include<vector>
#include <ws2tcpip.h>
#include"server.h"
#include"users.h"

std::vector<SOCKET>clients;//存储每一个用户socket

//检查异常处理
void chat_server::CheckError(bool bool_execute, const char* ErrorMessage)
{
    if (bool_execute)
    {
        std::cout << ErrorMessage << std::endl;
        system("pause");
        exit(-1);
    }
}

//初始化
chat_server::chat_server()
{
    std::cout << "服务器ip:";
    std::cin >> server_ip;
    std::cout << std::endl;
    std::cout << "服务器端口:";
    std::cin >> server_port;
    //server_port = 2712;
    //strcpy(server_ip, "127.0.0.1");//服务器ip
    int error = WSAStartup(MAKEWORD(2, 2), &_wsadata);
    CheckError(error != 0, "初始化套接字库失败");
    CheckError(LOBYTE(_wsadata.wVersion) != 2 || HIBYTE(_wsadata.wHighVersion) != 2, "套接字库版本号不符");
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.S_un.S_addr = inet_addr(server_ip);
    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    CheckError(server_socket == INVALID_SOCKET, "启用套接字失败");
    error = bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr));
    CheckError(error == SOCKET_ERROR, "套接字绑定失败");
    error = listen(server_socket, SOMAXCONN);
    CheckError(error < 0, "设置监听状态失败");
    online_people = 0;
    system("cls");
    std::cout << "服务器已启动" << std::endl;
}

//清理环境
chat_server::~chat_server()
{
    closesocket(server_socket);
    //closesocket(client_socket);
    WSACleanup();
}

//等待客户端连接
void chat_server::wait_for_client_connect()
{
    while (1)
    {
        size = sizeof(sockaddr_in);
        client_socket = accept(server_socket, (sockaddr*)&client_addr, &size);
        CheckError(client_socket == SOCKET_ERROR, "客户端连接失败");
        clients.push_back(client_socket);
        online_people = clients.size();
        std::thread work_thread(&chat_server::create_client_thread, this, client_socket);
        if (work_thread.hardware_concurrency() == 1)
        {
            std::cout << "单核CPU运行多线程效率更低" << std::endl;
        }
        //分离线程，每一个线程单独对每一个客户端服务
        work_thread.detach();
    }
}

//工作线程
void chat_server::create_client_thread(const SOCKET& client)
{
    char recv_buff[MAX_SIZE];
    User user;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
    std::cout << "id " << user.id << "加入服务器" << std::endl;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_BLUE);
    std::cout << "当前服务器人数：" << online_people << std::endl;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY);
    int client_flag;//接受recv函数的返回信息
    while (1)
    {
        client_flag = recv(client, recv_buff, MAX_SIZE, 0);
        //判断指令
        if (strncmp(recv_buff, ROOT, 6) == 0)
        {
            std::cout << "id " << user.id << "取得权限" << std::endl;
            //开放权限
            //会给开放权限的用户单独开一个通道通信
            administrator(client);
        }
        else if (strncmp(recv_buff, "!quit", 6) == 0)
        {
            //当客户端发出!quit，说明客户端断开连接，服务器关闭该客户端socket
            closesocket(client);
            //销毁进程前让容器删除套接字
            for (auto&& temp = clients.begin(); temp != clients.end(); temp++)
            {
                if (*temp == client)
                {
                    clients.erase(temp);
                    break;
                }
            }
            //online_people--;
            online_people = clients.size();//在线人数-1
            break;
        }
        else if (client_flag == 0)
        {
            break;
        }
        else if (client_flag < 0)
        {
            std::cout << "服务器接受出错" << std::endl;
            std::cout << WSAGetLastError() << std::endl;
            system("pause");
            exit(-1);
        }
        else
        {
            if (sprintf(user.my_chat_message, "%s\n%s",
                user.show_speak_time_and_id(),
                recv_buff) < 0) {
                std::cout << "存储个人聊天信息失败!" << std::endl;
            }
            else
            {
                std::cout << user.my_chat_message << std::endl;
                //发言的信息转发给其他客户端
                send_message_to_other_user(client, user.my_chat_message);
            }
        }
    }
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
    printf("id %d 退出服务器\n", user.id);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |
        FOREGROUND_RED |
        FOREGROUND_GREEN |
        FOREGROUND_BLUE);
}

//转发用户聊天信息
bool chat_server::send_message_to_other_user(const SOCKET& speak_client, char* buffer)
{
    if (online_people > 1)
    {
        for (int i = 0; i < online_people; i++)
        {
            if (clients[i] != speak_client)//不再发回给发言客户端
            {
                if (send(clients[i], buffer, MAX_SIZE, 0) == SOCKET_ERROR)
                {
                    std::cout << "转发聊天信息至客户端失败！" << std::endl;
                }
            }
        }
    }
    else
    {
        return false;//没有用户要发送
    }
    return true;//发送成功
}

//转发服务器信息（仅限管理员）
void chat_server::send_server_information(const SOCKET& root)
{
    char send_buffer[MAX_SIZE];
    if (sprintf(send_buffer,
        "服务器IP：%s\n服务器端口：%d\n在线人数：%d\n",
        server_ip,
        server_port,
        online_people) < 0) {
        std::cout << "转换服务器信息失败!" << std::endl;
    }
    else
    {
        send(root, send_buffer, MAX_SIZE, 0);
    }
}

//远程管理服务端
void chat_server::administrator(const SOCKET& root)
{
    char recv_buffer[MAX_SIZE];
    while (1)
    {
        send(root, "#star:", MAX_SIZE, 0);
        recv(root, recv_buffer, MAX_SIZE, 0);
        if (strncmp(recv_buffer, "!info", 6) == 0)
        {
            //转发服务器信息
            std::cout << "管理员指令：!info" << std::endl;
            send_server_information(root);
        }
        else if (strncmp(recv_buffer, "!close", 7) == 0)
        {
            //关闭服务器
            std::cout << "管理员指令：!close" << std::endl;
            for (int i = 0; i < online_people; i++)
            {
                closesocket(clients[0]);
            }
            clients.clear();
            closesocket(server_socket);
            WSACleanup();
            exit(2);
        }
        else if (strncmp(recv_buffer, "!exit", 6) == 0)
        {
            //退出权限模式
            std::cout << "管理员指令：!exit" << std::endl;
            break;
        }
    }
}