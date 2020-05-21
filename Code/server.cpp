#include<iostream>
#include<ctime>
#include<winsock2.h>
#include<thread>
#include<vector>
#include <ws2tcpip.h>
#include"server.h"
#include"users.h"

using namespace std;

vector<SOCKET>clients;//存储每一个用户socket

//初始化
chat_server::chat_server()
{
    server_port = 2712;
    strcpy(server_ip, "127.0.0.1");
    int error = WSAStartup(MAKEWORD(2, 2), &_wsadata);
    if (error == 0)
    {
        cout << "初始化套接字库成功！" << endl;
    }
    else
    {
        exit(error);
    }
    if (LOBYTE(_wsadata.wVersion) != 2 || HIBYTE(_wsadata.wHighVersion) != 2)
    {
        cout << "套接字库版本号不符！" << endl;
        WSACleanup();
    }
    else
    {
        cout << "套接字库版本正确！" << endl;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.S_un.S_addr = inet_addr(server_ip);
    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        cout << "套接字绑定失败！" << endl;
        WSACleanup();
        exit(-1);
    }
    else
    {
        cout << "套接字绑定成功！" << endl;
    }
    if (listen(server_socket, SOMAXCONN) < 0)
    {
        cout << "设置监听状态失败！" << endl;
        WSACleanup();
        exit(-1);
    }
    else
    {
        cout << "设置监听状态成功！" << endl;
    }
    online_people = 0;
    size = sizeof(sockaddr_in);
}

//清理环境
chat_server::~chat_server()
{
    closesocket(server_socket);
    closesocket(client_socket);
    WSACleanup();
}

//等待客户端连接
void chat_server::wait_for_client_connect()
{
    while (1)
    {
        client_socket = accept(server_socket, (sockaddr*)&client_addr, &size);
        if (client_socket == SOCKET_ERROR)
        {
            cout << "客户端连接失败！" << endl;
            WSACleanup();
            exit(-1);
        }
        clients.push_back(client_socket);
        online_people = clients.size();
        thread work_thread(&chat_server::create_client_thread,this,client_socket);
        if(work_thread.hardware_concurrency()==1)
        {
            cout<<"单核CPU运行多线程效率更低"<<endl;
        }
        //分离线程，每一个线程单独对每一个客户端服务
        work_thread.detach();
    }
}

//工作线程
void chat_server::create_client_thread(SOCKET client)
{
    User user;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
    cout << "id " << user.id << "加入服务器" << endl;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_BLUE);
    cout<<"当前服务器人数："<<online_people<<endl;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY);
    int client_flag;
    while (1)
    {
        char recv_buff[MAX_SIZE];
        client_flag = recv(client, recv_buff, MAX_SIZE, 0);
        //判断指令
        if (strncmp(recv_buff,ROOT,6)==0)
        {
            cout<<"id "<<user.id<<"取得权限"<<endl;
            //开放权限
            //会给开放权限的用户单独开一个通道通信
            administrator(client);
        }
        else if(strncmp(recv_buff,"!quit",6)==0)
        {
            break;
        }
        else if (client_flag == 0)
        {
            //客户端断开连接
            break;
        }
        else if (client_flag < 0)
        {
            cout<<"服务器接受出错"<<endl;
            cout<<WSAGetLastError()<<endl;
            system("pause");
            exit(-1);
        }
        else
        {
            if (sprintf(user.my_chat_message, "%s\n%s",
            user.show_speak_time_and_id(),
            recv_buff) < 0){
            cout << "存储个人聊天信息失败!" << endl;
            }
            else
            {
                cout << user.my_chat_message << endl;
                //发言的信息转发给其他客户端
                send_message_to_other_user(client, user.my_chat_message);
            }   
        }
    }
    //销毁进程前让容器删除最后一个元素
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
    printf("id %d 退出服务器\n",user.id);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | 
    FOREGROUND_RED |
    FOREGROUND_GREEN | 
    FOREGROUND_BLUE);
    closesocket(client);
    clients.pop_back();
}

//转发用户聊天信息
bool chat_server::send_message_to_other_user(SOCKET speak_client, char* buffer)
{
    if (online_people > 1)
    {
        for (int i = 0; i < online_people; i++)
        {
            if (clients[i] != speak_client)//不再发回给发言客户端
            {
                if (send(clients[i], buffer, MAX_SIZE, 0) == SOCKET_ERROR)
                {
                    cout << "转发聊天信息至客户端失败！" << endl;
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
void chat_server::send_server_information(SOCKET root)
{
    char send_buffer[MAX_SIZE];
    if (sprintf(send_buffer,
        "服务器IP：%s\n服务器端口：%d\n在线人数：%d\n",
        server_ip,
        server_port,
        online_people) < 0){
        cout << "转换服务器信息失败!" << endl;
    }
    else
    {
        send(root, send_buffer, MAX_SIZE, 0);
    }
}

//远程管理服务端
void chat_server::administrator(SOCKET root)
{
    char recv_buffer[MAX_SIZE];
    while(1)
    {
        send(root, "#star:", MAX_SIZE, 0);
        recv(root,recv_buffer,MAX_SIZE,0);
        if(strncmp(recv_buffer,"!info",6)==0)
        {
            cout<<"root"<<"行使了指令!info"<<endl;
            send_server_information(root);
        }
        else if(strncmp(recv_buffer,"!close",7)==0)
        {
            //关闭服务器
            
        }
        else if(strncmp(recv_buffer,"!exit",6)==0)
        {
            //退出权限模式
            cout<<"root"<<"退出权限模式"<<endl;
            break;
        }
    }
}