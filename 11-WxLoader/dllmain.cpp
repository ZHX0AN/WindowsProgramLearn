// dllmain.cpp : 定义 DLL 应用程序的入口点。
#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <WinSock2.h>
#include <iostream>
#include <stdio.h>

#include "pch.h"
#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
#include <TlHelp32.h>
#include "string"
#include "utils.h"

#pragma comment(lib,"ws2_32.lib")

//using namespace rapidjson;
using namespace std;

#define nSerPort 10080
#define nBufMaxSize 1024



BOOL InitWeChatSocket();

void debugLog(char* logStr);
BOOL InitSocket();
SOCKET BindListen(int nBackLog);
SOCKET AcceptConnetion(SOCKET hSocket);
BOOL ClientConFun(SOCKET sd);
BOOL CloseConnect(SOCKET sd);
void MyTcpServeFun();

SOCKET g_hClient;

static DWORD  g_WinBaseDddress = 0;
static HANDLE g_hProcess = 0;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {

        OutputDebugString(TEXT("Loader DLL_PROCESS_ATTACH... "));

        //InitWeChat();
        //InjectWeChat(szDllPath2);
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

	OutputDebugString(TEXT("ThreadProc...\n "));

    //初始化
    InitSocket();
    //业务处理
    MyTcpServeFun();
    //关闭webSocket链接
    WSACleanup();



	return 0;

}



BOOL InitWeChatSocket() {

	OutputDebugString(TEXT("Loader InjectWeChat... "));

	//开启socket服务
	HANDLE hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
    if (hThread) {
        CloseHandle(hThread);
    }
	return TRUE;
}


//注入dll，获取WeChatWin地址
BOOL InjectWechat(LPCSTR szDllPath) {

	string text = "szDllPath：\t";
	text.append(szDllPath);
	OutputDebugString(String2LPCWSTR(text));


	//wchar_t buff[0x100] = { 0 };
	DWORD weChatProcessID = 0;
	HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	//swprintf_s(buff, L"CreateToolhelp32Snapshot=%p", handle);

	PROCESSENTRY32 processentry32 = { 0 };
	processentry32.dwSize = sizeof(PROCESSENTRY32);

	BOOL next = Process32Next(handle, &processentry32);
	while (next == TRUE)
	{
		if (wcscmp(processentry32.szExeFile, L"WeChat.exe") == 0)
		{
			weChatProcessID = processentry32.th32ProcessID;
			break;
		}
		next = Process32Next(handle, &processentry32);
	}
	if (weChatProcessID == 0)
	{
		//MessageBox(NULL, L"没找到微信", L"错误", MB_OK);
		return 0;
	}

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, weChatProcessID);
	if (hProcess == NULL)
	{
		//MessageBox(NULL, L"打开微信进程失败", L"错误", MB_OK);
		return 0;
	}

	g_hProcess = hProcess;

	//获取win 地址

	HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
	MODULEENTRY32 me32;

	//给进程所引用的模块信息设定一个快照 
	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, weChatProcessID);
	if (hModuleSnap == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	me32.dwSize = sizeof(MODULEENTRY32);

	if (!Module32First(hModuleSnap, &me32))
	{
		CloseHandle(hModuleSnap);
		return FALSE;
	}

	do
	{
		if (_tcscmp(TEXT("WeChatWin.dll"), me32.szModule) == 0) {
			g_WinBaseDddress = (DWORD)me32.modBaseAddr;
			break;
		}

	} while (Module32Next(hModuleSnap, &me32));




	/*
	 * 注入dll
	 */
	DWORD strSize = strlen(szDllPath) + 1;

	LPVOID allocAddress = VirtualAllocEx(g_hProcess, NULL, strSize, MEM_COMMIT, PAGE_READWRITE);
	if (NULL == allocAddress)
	{
		//MessageBox(NULL, L"分配内存空间失败", L"错误", MB_OK);
		return 0;
	}
	//swprintf_s(buff, L"VirtualAllocEx=%p", allocAddress);

	BOOL result = WriteProcessMemory(g_hProcess, allocAddress, szDllPath, strSize, NULL);
	if (result == FALSE)
	{
		//MessageBox(NULL, L"写入内存失败", L"错误", MB_OK);
		return 0;
	}


	HMODULE hMODULE = GetModuleHandle(L"Kernel32.dll");
	FARPROC fARPROC = GetProcAddress(hMODULE, "LoadLibraryA");
	if (NULL == fARPROC)
	{
		//MessageBox(NULL, L"查找LoadLibraryA失败", L"错误", MB_OK);
		return 0;
	}

	HANDLE hANDLE = CreateRemoteThread(g_hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)fARPROC, allocAddress, 0, NULL);
	if (NULL == hANDLE)
	{
		//MessageBox(NULL, L"启动远程线程失败", L"错误", MB_OK);
		return 0;
	}



	return  TRUE;
}

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

//先创建socket，绑定本地地址，然后开始监听
SOCKET BindListen(int nBackLog) {

    //流式套接字SOCK_STREAM，  TCP协议 IPPROTO_TCP
    SOCKET hServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (hServer == INVALID_SOCKET) {

        debugLog((char*)"bind listen->socket error");
        return INVALID_SOCKET;
    }

    sockaddr_in addrServer;
    addrServer.sin_family = AF_INET;
    addrServer.sin_port = htons(10080);
    addrServer.sin_addr.s_addr = htonl(INADDR_ANY);

    //绑定
    int nRet = bind(hServer, (sockaddr*)&addrServer, sizeof(addrServer));
    if (nRet == SOCKET_ERROR) {
        debugLog((char*)"socket 绑定失败");
        closesocket(hServer);
        return INVALID_SOCKET;
    }

    //在socket上进行监听
    if (listen(hServer, 10) == SOCKET_ERROR) {
        closesocket(hServer);
        WSACleanup();
        debugLog((char*)"listen错误");
        return INVALID_SOCKET;
    }

    return hServer;
}

SOCKET AcceptConnetion(SOCKET hSocket) {

    sockaddr_in saConnAddr;
    int nSize = sizeof(saConnAddr);

    SOCKET hClient = accept(hSocket, (LPSOCKADDR)&saConnAddr, &nSize);
    if (hClient == INVALID_SOCKET) {
        debugLog((char*)"AcceptConnetion 错误");
        return INVALID_SOCKET;
    }


    return hClient;
}

BOOL ClientConFun(SOCKET sd) {


    char buf[nBufMaxSize];
    int nRetByte;
    //接收来自客户端的数据
    do {
        nRetByte = recv(sd, buf, nBufMaxSize, 0);
        if (nRetByte == SOCKET_ERROR) {
            debugLog((char*)"AcceptConnetion 错误");
            return FALSE;
        }
        else if (nRetByte != 0) {

            buf[nRetByte] = 0;
            cout << "接收到一条数据:" << buf << endl;
            int nSend = 0;
            while (nSend < nRetByte) {

                int nTemp = send(sd, &buf[nSend], nRetByte - nSend, 0);
                if (nTemp > 0) {
                    nSend += nTemp;
                }
                else if (nTemp == SOCKET_ERROR) {
                    debugLog((char*)"ClientConFun -> send 错误");
                    return FALSE;
                }
                else
                {
                    //send返回0，由于此时send < nRetByte，也就是数据还没发送出去，表示客户端被意外被关闭了
                    debugLog((char*)"ClientConFun -> send -> close 错误");
                    return FALSE;
                }
            }
        }
    } while (nRetByte != 0);

    return TRUE;
}

BOOL SocketSendMsg(SOCKET sd, char* buf, int nSize) {

    int nTemp = send(sd, buf, nSize, 0);
    if (nTemp > 0) {
        return TRUE;
    }
    else if (nTemp == SOCKET_ERROR) {
        debugLog((char*)"ClientConFun -> send 错误");
        return FALSE;
    }
    else
    {
        //send返回0，由于此时send < nRetByte，也就是数据还没发送出去，表示客户端被意外被关闭了
        debugLog((char*)"ClientConFun -> send -> close 错误");
        return FALSE;
    }

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


void MyTcpServeFun() {

    SOCKET hSocket = BindListen(1);
    if (hSocket == INVALID_SOCKET) {
        debugLog((char*)"MyTcpSerFun -> bindListen error");
        return;
    }
    while (true) {

        SOCKET hClient = AcceptConnetion(hSocket);
        
        if (hClient == INVALID_SOCKET) {
            debugLog((char*)"MyTcpSerFun -> acceptconnect error");
            break;
        }

        g_hClient = hClient;

        //客户端处理
        if (ClientConFun(hClient) == FALSE) {
            //break;
        }

        //关闭一个客户端连接
        if (CloseConnect(hClient) == FALSE) {
            //break;
        }

    }

    if (closesocket(hSocket) == SOCKET_ERROR) {
        debugLog((char*)"mytcpSerFun --> closesocket");
        return;
    }
}


BOOL SendMsgText(LPCSTR lpJsonData) {

    int nSize = strlen(lpJsonData) ;
    return SocketSendMsg(g_hClient, (char *)lpJsonData, nSize);
}