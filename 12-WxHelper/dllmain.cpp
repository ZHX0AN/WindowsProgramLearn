// dllmain.cpp : 定义 DLL 应用程序的入口点。
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <iostream>
#include <stdio.h>
#pragma comment(lib,"ws2_32.lib")

#include "pch.h"

#include "utils.h"


using std::cin;
using std::cout;
using std::endl;

#define nSerPort 10080
#define nBufMaxSize 1024

DWORD WINAPI ThreadProc(PVOID lpParameter);

BOOL InitSocket();
SOCKET ConnectSocket();
BOOL CloseConnect(SOCKET sd);


void debugLog(char* logStr) {
    cout << logStr << WSAGetLastError() << endl;

}

BOOL InitSocket() {


    WSADATA wsaData;
    SOCKET hServer;
    WORD wVerSion = MAKEWORD(2, 2);
    if (WSAStartup(wVerSion, &wsaData)) {
        //如果启动失败
        debugLog((char*)"initSocket->WSAStartup error");
        return FALSE;
    };

    return TRUE;


}


//关闭一个链接
BOOL CloseConnect(SOCKET sd) {

    //首先发送一个TCP FIN分段，向对方表明已经完成数据发送
    if (shutdown(sd, SD_SEND) == SOCKET_ERROR) {

        debugLog((char*)"CloseConnect - > ShutDown error");
        return FALSE;
    }
    char buf[nBufMaxSize];
    int nRetByte;

    do {
        nRetByte = recv(sd, buf, nBufMaxSize, 0);
        if (nRetByte == SOCKET_ERROR) {
            debugLog((char*)"closeconnect -> recv error");
            break;
        }
        else if (nRetByte > 0) {

            debugLog((char*)"closeconnect 错误的接收数据");
            break;
        }

    } while (nRetByte != 0);

    if (closesocket(sd) == SOCKET_ERROR) {
        debugLog((char*)"closeconnect -> closesocket error");
        return FALSE;
    }
    return TRUE;
}


SOCKET ConnectSocket() {

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    SOCKADDR_IN clientsock_in;
    clientsock_in.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    clientsock_in.sin_family = AF_INET;
    clientsock_in.sin_port = htons(10080);
    connect(clientSocket, (SOCKADDR*)&clientsock_in, sizeof(SOCKADDR));//开始连接

    return clientSocket;
}



VOID __declspec(dllexport) Test()
{
    //OutputDebugString(TEXT("开始调试"));
}





BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {

        OutputDebugString(TEXT("WeChatHelper "));
        HANDLE hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
        if (hThread) {
            CloseHandle(hThread);
        }

    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

//新线程，开启socket
DWORD WINAPI ThreadProc(PVOID lpParameter) {

    OutputDebugString(TEXT("WeChatHelper ThreadProc... "));
    //SocketClient();


    //1.建立流式套接字
    InitSocket();
    //2.建立连接
    SOCKET clientSocket = ConnectSocket();

    char receiveBuf[nBufMaxSize];
    int nRetByte;

    while (true) {
        //nRetByte：读取长度，直到遇到0
        nRetByte = recv(clientSocket, receiveBuf, nBufMaxSize, 0);
        if (nRetByte == SOCKET_ERROR) {
            //debugLog((char*)"AcceptConnetion 错误");
            OutputDebugString(TEXT("WeChatHelperERROR.. "));
            return FALSE;
        }
        else if (nRetByte != 0) {

            //int nSize = strlen(receiveBuf);

            string text = "receiveBuf长度：\t";
            text.append(Dec2Hex(nRetByte));
            OutputDebugString(String2LPCWSTR(text));


            //OutputDebugStringA((receiveBuf));

            OutputDebugString(TEXT("WeChatHelperYOU.. "));
        }

        //send(clientSocket, "hello world", strlen("hello world") + 1, 0);
        

    }

    return 0;

}

