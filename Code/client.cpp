#include<iostream>
#include<cstring>
#include<thread>
#include"client.h"
#include"users.h"

using namespace std;

//初始化
Client::Client()
{
    server_port = 2712;
    strcpy(server_ip, "127.0.0.1");
    int error = WSAStartup(MAKEWORD(2, 2), &_wsadata);
    if (error != 0)
    {
        exit(error);
    }
    if (LOBYTE(_wsadata.wVersion) != 2 || HIBYTE(_wsadata.wHighVersion) != 2)
    {
        cout << "套接字库版本号不符！" << endl;
        WSACleanup();
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.S_un.S_addr = inet_addr(server_ip);
    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    for (int i = 0; i < 10; i++)
    {
        if (connect(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
        {
            cout << "服务器连接失败，正在重新连接" << endl;
            Sleep(2000);
            if (i == 9)
            {
                cout << "服务器多次连接失败，程序自动关闭" << endl;
                exit(-1);
            }
        }
        else
        {
            if(i>0)
            {
                cout<<"重新连接服务器成功"<<endl;
            }
            break;
        }
    }
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
    thread recv_server_thread(&Client::from_server_recv_message,this);
    if(recv_server_thread.hardware_concurrency()==1)
    {
        cout<<"单核CPU运行多线程效率更低"<<endl;
    }
    //recv_server_thread.detach();
    while (1)
    {
        cin.getline(send_buffer, MAX_BUFFER);
        if (strncmp(send_buffer, "!quit",6) == 0)
        {
            send(server_socket,"!quit" , 6, 0);
            recv_server_thread.join();
            break;
        }
        else if (strlen(send_buffer) > MAX_BUFFER)
        {
            cout << "输入字符超过最大限度！" << endl;
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
        server_flag=recv(server_socket, recv_buffer, MAX_BUFFER, 0);
        if(server_flag<0)
        {
            cout<<"服务器转发失败"<<endl;
            cout<<WSAGetLastError()<<endl;
            system("pause");
            exit(-1);
        }
        else if(server_flag==0)
        {
            cout<<"服务器断开连接"<<endl;
            break;
        }
        else
        {
            cout << recv_buffer << endl;   
        }
    }
}