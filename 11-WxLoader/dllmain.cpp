// dllmain.cpp : 定义 DLL 应用程序的入口点。
#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/pointer.h"

#include <WinSock2.h>
#include <iostream>
#include <stdio.h>

#include "pch.h"
#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
#include <TlHelp32.h>
#include "AddrOffset.h"
#include "utils.h"
#include <list>
#include <vector>

#pragma comment(lib,"ws2_32.lib")

using namespace rapidjson;
using namespace std;



/****************** 回调 开始*****************/
extern "C" _declspec(dllexport) VOID RECEIVE(DWORD, LPSTR, DWORD);
extern "C" _declspec(dllexport) VOID ACCEPT(DWORD);
extern "C" _declspec(dllexport) VOID CLOSE(DWORD);

extern "C" _declspec(dllexport) BOOL InjectWechat(LPCSTR szDllPath);
extern "C" _declspec(dllexport) BOOL SendWeChatData(DWORD dwClientId, LPCSTR szJsonData);
extern "C" _declspec(dllexport)  BOOL InitWeChatSocket(VOID(*RECEIVE)(DWORD, LPSTR, DWORD), VOID(*ACCEPT)(DWORD), VOID(*CLOSE)(DWORD));

typedef struct {
	VOID(*RECEIVE)(DWORD, LPSTR, DWORD);
	VOID(*ACCEPT)(DWORD);
	VOID(*CLOSE)(DWORD);
} CallBackFun;
CallBackFun* fun;

/****************** 回调 结束*****************/


/******************socket 开始*****************/
#define nSerPort 10080
#define nBufMaxSize 10240

DWORD WINAPI ThreadProc(PVOID lpParameter);


BOOL InitSocket();
SOCKET BindListen(int nBackLog);
SOCKET AcceptConnetion(SOCKET hSocket);
BOOL ClientConFun(SOCKET sd);
BOOL CloseConnect(SOCKET sd);
void MyTcpServeFun();
SOCKET g_hClient;
/******************socket 结束*****************/


/****************** 业务 开始*****************/

BOOL CallBackSendMsg(LPSTR data, DWORD size);

LPCSTR g_lpUserInfo;

BOOL SendMsgText(LPCSTR lpJsonData);

list<int> nodeAddressList;
vector<DWORD> nodeAddressVector;
int nodeNumber = 0;
rapidjson::Document doc;

rapidjson::StringBuffer  g_sb;

BOOL GetUserInfo();

/****************** 业务 结束*****************/



VOID __declspec(dllexport) Test()
{
	//DebugLog(TEXT("开始调试"));
}


BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		DebugLog(TEXT("Loader DLL_PROCESS_ATTACH... "));
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


//注入dll，获取WeChatWin地址
BOOL InjectWechat(LPCSTR szDllPath) {

	string text = "szDllPath：\t";
	text.append(szDllPath);
	DebugLog(String2LPCWSTR(text));


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


/************* Socket 开始**************/


//新线程，开启socket
DWORD WINAPI ThreadProc(PVOID lpParameter) {

	DebugLog(TEXT("ThreadProc...\n "));

	//初始化
	InitSocket();
	//业务处理
	MyTcpServeFun();

	////关闭webSocket链接
	//WSACleanup();

	return 0;

}

BOOL InitSocket() {

	WSADATA wsaData;
	SOCKET hServer;
	WORD wVerSion = MAKEWORD(2, 2);
	if (WSAStartup(wVerSion, &wsaData)) {
		//如果启动失败
		DebugLog((char*)"initSocket->WSAStartup error");
		return FALSE;
	};

	return TRUE;
}

//先创建socket，绑定本地地址，然后开始监听
SOCKET BindListen(int nBackLog) {

	//流式套接字SOCK_STREAM，  TCP协议 IPPROTO_TCP
	SOCKET hServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (hServer == INVALID_SOCKET) {

		DebugLog((char*)"bind listen->socket error");
		return INVALID_SOCKET;
	}

	sockaddr_in addrServer;
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(nSerPort);
	addrServer.sin_addr.s_addr = htonl(INADDR_ANY);

	//绑定
	int nRet = bind(hServer, (sockaddr*)&addrServer, sizeof(addrServer));
	if (nRet == SOCKET_ERROR) {
		DebugLog((char*)"socket 绑定失败");
		closesocket(hServer);
		return INVALID_SOCKET;
	}

	//在socket上进行监听
	if (listen(hServer, 10) == SOCKET_ERROR) {
		closesocket(hServer);
		WSACleanup();
		DebugLog((char*)"listen错误");
		return INVALID_SOCKET;
	}

	return hServer;
}

SOCKET AcceptConnetion(SOCKET hSocket) {

	sockaddr_in saConnAddr;
	int nSize = sizeof(saConnAddr);

	SOCKET hClient = accept(hSocket, (LPSOCKADDR)&saConnAddr, &nSize);
	if (hClient == INVALID_SOCKET) {
		DebugLog((char*)"AcceptConnetion 错误");
		return INVALID_SOCKET;
	}


	return hClient;
}

BOOL ClientConFun(SOCKET sd) {

	DebugLog(TEXT("ClientConFun -> enter"));

	char buf[nBufMaxSize];
	int nRetByte;
	//接收来自客户端的数据
	do {
		nRetByte = recv(sd, buf, nBufMaxSize, 0);
		if (nRetByte == SOCKET_ERROR) {
			DebugLog((char*)"AcceptConnetion 错误\n");
			return FALSE;
		}
		else if (nRetByte != 0) {

			buf[nRetByte] = 0;
			//cout << "接收到一条数据:" << buf << endl;

			DebugLog(TEXT("ClientConFun -> 接收到一条数据\n"));
			DebugLog(buf);

			CallBackSendMsg(buf, sizeof(buf));

			//char lpUserInfo[50] = "{\"data\":{},\"type\": 11028}";
			//int nSize = sizeof(lpUserInfo);
			//int nTemp = send(sd, buf, nSize, 0);
			//if (nTemp > 0) {
			//	continue;
			//}
			//else if (nTemp == SOCKET_ERROR) {
			//	DebugLog((char*)"ClientConFun -> send 错误 \n");
			//	return FALSE;
			//}
			//else
			//{
			//	//send返回0，由于此时send < nRetByte，也就是数据还没发送出去，表示客户端被意外被关闭了
			//	DebugLog((char*)"ClientConFun -> send -> close 错误 \n");
			//	return FALSE;
			//}
		}
	} while (nRetByte != 0);

	return TRUE;
}

//发送消息到客户端
BOOL SocketSendMsg(SOCKET sd, char* buf, int nSize) {

	int nTemp = send(sd, buf, nSize, 0);
	if (nTemp > 0) {
		return TRUE;
	}
	else if (nTemp == SOCKET_ERROR) {
		DebugLog((char*)"ClientConFun -> send 错误 \n");
		return FALSE;
	}
	else
	{
		//send返回0，由于此时send < nRetByte，也就是数据还没发送出去，表示客户端被意外被关闭了
		DebugLog((char*)"ClientConFun -> send -> close 错误 \n");
		return FALSE;
	}

}

//关闭一个链接
BOOL CloseConnect(SOCKET sd) {

	//首先发送一个TCP FIN分段，向对方表明已经完成数据发送
	if (shutdown(sd, SD_SEND) == SOCKET_ERROR) {

		DebugLog((char*)"CloseConnect - > ShutDown error");
		return FALSE;
	}
	char buf[nBufMaxSize];
	int nRetByte;

	do {
		nRetByte = recv(sd, buf, nBufMaxSize, 0);
		if (nRetByte == SOCKET_ERROR) {
			DebugLog((char*)"closeconnect -> recv error");
			break;
		}
		else if (nRetByte > 0) {

			DebugLog((char*)"closeconnect 错误的接收数据");
			break;
		}

	} while (nRetByte != 0);

	if (closesocket(sd) == SOCKET_ERROR) {
		DebugLog((char*)"closeconnect -> closesocket error");
		return FALSE;
	}
	return TRUE;
}

void MyTcpServeFun() {

	SOCKET hSocket = BindListen(1);
	if (hSocket == INVALID_SOCKET) {
		DebugLog((char*)"MyTcpSerFun -> bindListen error");
		return;
	}
	while (true) {

		SOCKET hClient = AcceptConnetion(hSocket);

		if (hClient == INVALID_SOCKET) {
			DebugLog((char*)"MyTcpSerFun -> acceptconnect error");
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
		DebugLog((char*)"mytcpSerFun --> closesocket");
		return;
	}
}

/************* Socket 结束**************/


/****************** 回调 开始*****************/

BOOL InitWeChatSocket(VOID(*RECEIVE)(DWORD, LPSTR, DWORD), VOID(*ACCEPT)(DWORD), VOID(*CLOSE)(DWORD)) {

	fun = (CallBackFun*)malloc(sizeof(CallBackFun));
	fun->RECEIVE = RECEIVE;
	fun->ACCEPT = ACCEPT;
	fun->CLOSE = CLOSE;


	//开启socket服务
	HANDLE hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
	//if (hThread) {
	//	CloseHandle(hThread);
	//}

	return TRUE;

}

/****************** 回调 结束*****************/




/****************** 2、业务代码 开始*****************/

//发送消息给回调函数
BOOL CallBackSendMsg(LPSTR data, DWORD size = 0) {

	if (size == 0) {
		size = sizeof(data);
	}
	fun->RECEIVE(1, data, sizeof(data));

	return true;
}

//获取消息处理handler
BOOL SendWeChatData(DWORD dwClientId, LPCSTR szJsonData) {

	char buffer[200] = { 0 };
	int nSize = strlen(szJsonData) + 1;
	memcpy(buffer, szJsonData, nSize);


	//RapidJson
	Document doc;
	doc.SetObject();
	Document::AllocatorType& allocator = doc.GetAllocator(); //获取分配器


	if (doc.ParseInsitu(buffer).HasParseError()) {
		DebugLog(TEXT("json格式错误"));
		char result[50] = "{\"type\": -1,msg: \"json格式错误\",\"data\":{}}";
		fun->RECEIVE(1, result, sizeof(result));
		return false;
	}

	char rsBuffer[30] = "{\"type\": -1,\"data\":{}}";
	int nType = doc["type"].GetInt();

	BOOL rs = false;
	switch (nType) {

	case MT_DATA_OWNER_MSG:
		//获取个人消息
		rs = GetUserInfo();
		if (!rs) {
			CallBackSendMsg(rsBuffer);
		}
		break;
	case MT_SEND_TEXTMSG:
		//发送文本消息
		SendMsgText(szJsonData);
		break;
	default:
		DebugLog(TEXT("请输入正确的消息代码"));


	}


	return true;
}

//调用socket，发送消息到客户端
BOOL SendMsgText(LPCSTR lpJsonData) {

	int nSize = strlen(lpJsonData) + 1;
	return SocketSendMsg(g_hClient, (char*)lpJsonData, nSize);
}

//获取登录用户信息
BOOL GetUserInfo() {

	DebugLog(TEXT("GetUserInfo... "));

	//wxid
	BYTE pWxIdAddr[4] = { 0 };
	ReadProcessMemory(g_hProcess, (LPVOID)(g_WinBaseDddress + USERINFO_WXID), pWxIdAddr, 4, 0);
	DWORD wxIdAddr = BytesToDword(pWxIdAddr);
	char wxId[50] = { 0 };
	ReadProcessMemory(g_hProcess, (LPVOID)wxIdAddr, wxId, 50, 0);

	//nickname
	char nickname[100] = { 0 };
	ReadProcessMemory(g_hProcess, (LPVOID)(g_WinBaseDddress + USERINFO_NICKNAME), nickname, 100, 0);

	//头像Big
	BYTE pPicBig[4] = { 0 };
	ReadProcessMemory(g_hProcess, (LPVOID)(g_WinBaseDddress + USERINFO_BIG_PIC), pPicBig, 4, 0);
	DWORD picBigAddr = BytesToDword(pPicBig);
	BYTE picBig[200] = { 0 };
	ReadProcessMemory(g_hProcess, (LPVOID)picBigAddr, picBig, 200, 0);


	/*
	 * 构建json数据
	 */

	rapidjson::Document* ptr_doc = new rapidjson::Document();
	ptr_doc->Parse("{}");
	rapidjson::Document::AllocatorType& allocator = ptr_doc->GetAllocator();


	rapidjson::Value vType(rapidjson::kObjectType);
	rapidjson::Value vData(rapidjson::kObjectType);
	ptr_doc->AddMember("type", 11028, allocator);//对象添加

	ptr_doc->AddMember("data", vData, allocator);

	//wxid
	Value vWxid(wxId, doc.GetAllocator());
	//nickname
	Value vNickName((nickname), doc.GetAllocator());
	//avatar
	Value vAvatar((char*)picBig, doc.GetAllocator());

	(*ptr_doc)["data"].AddMember("wxid", vWxid, allocator);//data对象添加
	(*ptr_doc)["data"].AddMember("nickname", vNickName, allocator);//data对象添加
	(*ptr_doc)["data"].AddMember("avatar", vAvatar, allocator);//data对象添加


	rapidjson::StringBuffer  buffer;
	rapidjson::Writer<rapidjson::StringBuffer>  writer(buffer);
	ptr_doc->Accept(writer);


	//发送消息给回调函数
	size_t size = buffer.GetSize();
	CallBackSendMsg((char*)buffer.GetString(), size);

	return true;

}



/******************业务代码 结束*****************/
