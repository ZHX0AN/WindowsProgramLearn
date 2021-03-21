// dllmain.cpp : 定义 DLL 应用程序的入口点。
#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
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
extern "C" _declspec(dllexport) BOOL SendMsgText(LPCSTR lpJsonData);
extern "C" _declspec(dllexport) BOOL SendWeChatData(DWORD dwClientId, LPCSTR szJsonData);
extern "C" _declspec(dllexport) BOOL InitWeChatSocket(VOID(*RECEIVE)(DWORD, LPSTR, DWORD), VOID(*ACCEPT)(DWORD), VOID(*CLOSE)(DWORD));

//extern "C" _declspec(dllexport) BOOL InjectWechat(LPCSTR szDllPath);
//extern "C" _declspec(dllexport) BOOL SendWeChatData(DWORD dwClientId, LPCSTR szJsonData);

typedef struct {
	VOID(*RECEIVE)(DWORD, LPSTR, DWORD);
	VOID(*ACCEPT)(DWORD);
	VOID(*CLOSE)(DWORD);
} CallBackFun;
CallBackFun* fun;

/****************** 回调 结束*****************/


/******************socket 开始*****************/
#define nSerPort 10080
#define nBufMaxSize 1024

DWORD WINAPI ThreadProc(PVOID lpParameter);
SOCKET ConnectSocket();

BOOL InitSocket();
SOCKET BindListen(int nBackLog);
SOCKET AcceptConnetion(SOCKET hSocket);
BOOL ClientConFun(SOCKET sd);
BOOL CloseConnect(SOCKET sd);
void MyTcpServeFun();
SOCKET g_hServer;
/******************socket 结束*****************/


/****************** 业务 开始*****************/

BOOL CallBackSendMsg(LPSTR data, DWORD size);

LPCSTR g_lpUserInfo;
BOOL SendMsgText(LPCSTR lpJsonData);

VOID GetWxMemoryUnicodeString(DWORD baseAddress, int nSize);
DWORD GetWxMemoryInt(HANDLE hProcess, DWORD baseAddress);
VOID GetContactInfo(DWORD roomAddress, rapidjson::Value* pVData);

//获取群组信息
BOOL GetChatRoomList();
VOID GetChatRoomInfo(DWORD roomAddress, rapidjson::Value* pVData);


list<int> nodeAddressList;
vector<DWORD> nodeAddressVector;
int nodeNumber = 0;
rapidjson::Document doc;

rapidjson::StringBuffer  g_sb;

BOOL GetUserInfo();
BOOL GetContactList();
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

//新线程，开启socket
DWORD WINAPI ThreadProc(PVOID lpParameter) {

	//OutputDebugString(TEXT("WeChatHelper ThreadProc... "));

	//1.建立流式套接字
	InitSocket();
	//2.建立连接
	g_hServer = ConnectSocket();

	DebugLog(TEXT("ClientConFun -> enter"));

	char buf[nBufMaxSize];
	int nRetByte;
	//接收来自客户端的数据
	do {
		nRetByte = recv(g_hServer, buf, nBufMaxSize, 0);
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

			char lpUserInfo[50] = "{\"data\":{},\"type\": 11028}";
			int nSize = sizeof(lpUserInfo);
			int nTemp = send(g_hServer, buf, nSize, 0);
			if (nTemp > 0) {
				continue;
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
	} while (nRetByte != 0);

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

SOCKET ConnectSocket() {

	int iRetValue = 0;
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN clientsock_in;
	clientsock_in.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	clientsock_in.sin_family = AF_INET;
	clientsock_in.sin_port = htons(10080);

	iRetValue = connect(clientSocket, (SOCKADDR*)&clientsock_in, sizeof(SOCKADDR));//开始连接

	//如果连接失败，睡眠一秒，直到成功
	while (iRetValue == SOCKET_ERROR) {
		Sleep(1000);
		iRetValue = connect(clientSocket, (SOCKADDR*)&clientsock_in, sizeof(SOCKADDR));//开始连接
	}

	return clientSocket;
}
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
		//case MT_DATA_FRIENDS_MSG:
		//	//获取联系人列表
		//	rs = GetContactList();
		//	if (!rs) {
		//		fun->RECEIVE(1, rsBuffer, sizeof(rsBuffer));
		//	}
		//	break;
		//case MT_DATA_CHATROOMS_MSG:
		//	rs = GetChatRoomList();
		//	if (!rs) {
		//		fun->RECEIVE(1, rsBuffer, sizeof(rsBuffer));
		//	}
		//	break;
	case MT_SEND_TEXTMSG:
		//发送文本消息
		SendMsgText(szJsonData);
		break;
		//case MT_SEND_CHATROOM_ATMSG:
		//	//发送at消息
		//	SendMsgText(szJsonData);
		//	break;
		//case MT_SEND_FILEMSG:
		//	//发送文件
		//	SendMsgText(szJsonData);
		//	break;
	default:
		DebugLog(TEXT("请输入正确的消息代码"));


	}


	return true;
}

//调用socket，发送消息到客户端
BOOL SendMsgText(LPCSTR lpJsonData) {

	int nSize = strlen(lpJsonData) + 1;
	return SocketSendMsg(g_hServer, (char*)lpJsonData, nSize);
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

////获取联系人
//BOOL GetContactList() {
//
//	//初始化
//	g_sb.Clear();
//	nodeAddressList.clear();
//	nodeAddressVector.clear();
//	nodeNumber = 0;
//
//	//size_t s2 = g_sb.GetSize();
//	//char temp2[5000] = { 0 };
//	//memcpy(temp2, (char*)g_sb.GetString(), s2);
//
//	char content[4] = { 0 };
//	DWORD linkPointer = GetWxMemoryInt(g_hProcess, g_WinBaseDddress + CONTACT_ADDR) + CONTACT_OFFSET1 + CONTACT_OFFSET2;
//
//
//	//群链表地址
//	DWORD headerAddress = GetWxMemoryInt(g_hProcess, linkPointer);
//
//	//节点数量
//	DWORD contractCount = GetWxMemoryInt(g_hProcess, linkPointer + 4);
//	nodeAddressList.push_front(headerAddress);
//	nodeAddressVector.push_back(headerAddress);
//
//
//	//三个分支
//	DWORD header1 = GetWxMemoryInt(g_hProcess, headerAddress);
//	DWORD header2 = GetWxMemoryInt(g_hProcess, headerAddress + 4);
//	DWORD header3 = GetWxMemoryInt(g_hProcess, headerAddress + 8);
//
//
//	/*
//     * 构建RapidJson数据
//    */
//	doc.SetObject();
//	doc.AddMember("type", 11030, doc.GetAllocator());
//	rapidjson::Value vData(kArrayType);
//
//
//	// 打印群数据
//	GetContactInfo(header1, &vData);
//	GetContactInfo(header2, &vData);
//	GetContactInfo(header3, &vData);
//
//	doc.AddMember("data", vData, doc.GetAllocator());
//
//
//	//无空格
//	/*rapidjson::StringBuffer  sb;*/
//	rapidjson::Writer<rapidjson::StringBuffer>  writer(g_sb);
//
//	////json有空格
//	//StringBuffer sb;
//	//PrettyWriter<StringBuffer> writer(sb);
//	doc.Accept(writer);
//
//	//发送消息给回调函数
//	size_t size = g_sb.GetSize();
//	CallBackSendMsg((char*)g_sb.GetString(), size);
//
//	return true;
//}
//
//VOID GetContactInfo(DWORD roomAddress, rapidjson::Value* pVData) {
//
//	for (int i = 0; i < nodeAddressVector.size(); ++i) {
//		if (roomAddress == nodeAddressVector[i]) {
//			return;
//		}
//	}
//	nodeAddressVector.push_back(roomAddress);
//
//	nodeNumber++;
//
//	//三个分支
//	DWORD header1 = GetWxMemoryInt(g_hProcess, roomAddress);
//	DWORD header2 = GetWxMemoryInt(g_hProcess, roomAddress + 4);
//	DWORD header3 = GetWxMemoryInt(g_hProcess, roomAddress + 8);
//
//
//	//联系人账号 wchat_t
//	wchar_t acount[50] = { 0 };
//	DWORD accountTemp = GetWxMemoryInt(g_hProcess, roomAddress + 0x30);
//	int accountSizeTemp = (GetWxMemoryInt(g_hProcess, roomAddress + 0x34) + 1 ) * 2;
//	ReadProcessMemory(g_hProcess, (LPVOID)accountTemp, acount, accountSizeTemp, 0);
//	//_tprintf(TEXT("%s \n"), acount);
//
//	//昵称
//	wchar_t nickname[50] = { 0 };
//	DWORD nicknameTemp = GetWxMemoryInt(g_hProcess, roomAddress + 0x8C);
//	int nicknameSizeTemp = (GetWxMemoryInt(g_hProcess, roomAddress + 0x90) + 1) *2 ;
//	ReadProcessMemory(g_hProcess, (LPVOID)nicknameTemp, nickname, nicknameSizeTemp, 0);
//	//_tprintf(TEXT("%s \n"), nickname);
//
//
//	//头像
//	wchar_t avatar[200] = { 0 };
//	DWORD avatarTemp = GetWxMemoryInt(g_hProcess, roomAddress + 0x130);
//	int avatarSizeTemp = GetWxMemoryInt(g_hProcess, roomAddress + 0x134) * 2 + 2;
//	ReadProcessMemory(g_hProcess, (LPVOID)avatarTemp, avatar, avatarSizeTemp, 0);
//
//
//
//	//备注
//	wchar_t remark[200] = { 0 };
//	DWORD remarkTemp = GetWxMemoryInt(g_hProcess, roomAddress + 0x78);
//	int remarkSizeTemp = GetWxMemoryInt(g_hProcess, roomAddress + 0x7c) * 2 + 2;
//	ReadProcessMemory(g_hProcess, (LPVOID)remarkTemp, remark, remarkSizeTemp, 0);
//
//
//
//	//构建userinfo数据
//	rapidjson::Value vUserInfo(kObjectType);
//
//
//	//wxid
//	
//	Value vWxid(UnicodeToUtf8(acount), doc.GetAllocator());
//	vUserInfo.AddMember("wxid", vWxid, doc.GetAllocator());
//
//	//nickname
//	char* nickNameUtf8 = UnicodeToUtf8(nickname);
//	Value vNickName(nickNameUtf8, doc.GetAllocator());
//	vUserInfo.AddMember("nickname", vNickName, doc.GetAllocator());
//
//	//头像
//	char* avatarUtf8 = UnicodeToUtf8(avatar);
//	Value vAvatar(avatarUtf8, doc.GetAllocator());
//	vUserInfo.AddMember("avatar", vAvatar, doc.GetAllocator());
//
//	//备注
//	char* remarkUtf8 = UnicodeToUtf8(remark);
//	Value vRemark(remarkUtf8, doc.GetAllocator());
//	vUserInfo.AddMember("remark", vRemark, doc.GetAllocator());
//
//	pVData->PushBack(vUserInfo, doc.GetAllocator());
//
//	GetContactInfo(header1, pVData);
//	GetContactInfo(header2, pVData);
//	GetContactInfo(header3, pVData);
//
//
//}
//
//DWORD GetWxMemoryInt(HANDLE hProcess, DWORD baseAddress) {
//
//	BYTE content[4] = { 0 };
//	ReadProcessMemory(g_hProcess, (LPVOID)baseAddress, content, 4, 0);
//	return (DWORD)BytesToDword(content);
//}
//
//VOID GetWxMemoryUnicodeString(DWORD baseAddress, int nSize) {
//
//
//	TCHAR content[2000] = { 0 };
//	ReadProcessMemory(g_hProcess, (LPVOID)baseAddress, content, 2000, 0);
//
//	//_tprintf(TEXT("%s \n"), content);
//}
//
//
////获取群组信息
//BOOL GetChatRoomList() {
//
//	//初始化
//	g_sb.Clear();
//	nodeAddressList.clear();
//	nodeAddressVector.clear();
//	nodeNumber = 0;
//
//	//nodeAddressVector.swap(vector<DWORD>());
//
//
//	//list<int> nodeAddressList;
//	//vector<DWORD> nodeAddressVector;
//
//
//
//	char content[4] = { 0 };
//	DWORD linkPointer = GetWxMemoryInt(g_hProcess, g_WinBaseDddress + CHATROOM_ADDR) + CHATROOM_OFFSET1 + CHATROOM_OFFSET2;
//	
//	//群链表地址
//	DWORD headerAddress = GetWxMemoryInt(g_hProcess, linkPointer);
//
//	//节点数量
//	DWORD contractCount = GetWxMemoryInt(g_hProcess, linkPointer + 4);
//	nodeAddressList.push_front(headerAddress);
//	nodeAddressVector.push_back(headerAddress);
//
//
//	//三个分支
//	DWORD header1 = GetWxMemoryInt(g_hProcess, headerAddress);
//	DWORD header2 = GetWxMemoryInt(g_hProcess, headerAddress + 4);
//	DWORD header3 = GetWxMemoryInt(g_hProcess, headerAddress + 8);
//
//
//	/*
//	 * 构建RapidJson数据
//	*/
//	doc.SetObject();
//	doc.AddMember("type", MT_DATA_CHATROOMS_MSG, doc.GetAllocator());
//	rapidjson::Value vData(kArrayType);
//
//
//	// 打印群数据
//	GetChatRoomInfo(header1, &vData);
//	GetChatRoomInfo(header2, &vData);
//	GetChatRoomInfo(header3, &vData);
//
//	doc.AddMember("data", vData, doc.GetAllocator());
//
//
//	//无空格
//	/*rapidjson::StringBuffer  sb;*/
//	rapidjson::Writer<rapidjson::StringBuffer>  writer(g_sb);
//
//	////json有空格
//	//StringBuffer sb;
//	//PrettyWriter<StringBuffer> writer(sb);
//	doc.Accept(writer);
//
//	//发送消息给回调函数
//	size_t size = g_sb.GetSize();
//	CallBackSendMsg((char*)g_sb.GetString(), size);
//
//	return true;
//}
//
//VOID GetChatRoomInfo(DWORD roomAddress, rapidjson::Value* pVData) {
//
//	for (int i = 0; i < nodeAddressVector.size(); ++i) {
//		if (roomAddress == nodeAddressVector[i]) {
//			return;
//		}
//	}
//	nodeAddressVector.push_back(roomAddress);
//
//	nodeNumber++;
//
//	//三个分支
//	DWORD header1 = GetWxMemoryInt(g_hProcess, roomAddress);
//	DWORD header2 = GetWxMemoryInt(g_hProcess, roomAddress + 4);
//	DWORD header3 = GetWxMemoryInt(g_hProcess, roomAddress + 8);
//
//    
//	//0x10 群号
//	//0x28 群号
//	//0x3c 好友id list
//	//0x6c 群主wxid
//	//0x84 在群中的昵称
//
//	//群号 wchat_t
//	wchar_t wxid[50] = { 0 };
//	DWORD dwTemp = GetWxMemoryInt(g_hProcess, roomAddress + 0x10);
//	int nSize = (GetWxMemoryInt(g_hProcess, roomAddress + 0x14) + 1) * 2;
//	ReadProcessMemory(g_hProcess, (LPVOID)dwTemp, wxid, nSize, 0);
//
//
//	//群主wxid
//	wchar_t owner[50] = { 0 };
//	dwTemp = GetWxMemoryInt(g_hProcess, roomAddress + 0x6C);
//	nSize = (GetWxMemoryInt(g_hProcess, roomAddress + 0x70) + 1) * 2;
//	ReadProcessMemory(g_hProcess, (LPVOID)dwTemp, owner, nSize, 0);
//
//
//	//在群中的昵称
//	wchar_t nickname[200] = { 0 };
//	dwTemp = GetWxMemoryInt(g_hProcess, roomAddress + 0x84);
//	nSize = GetWxMemoryInt(g_hProcess, roomAddress + 0x88) * 2 + 2;
//	ReadProcessMemory(g_hProcess, (LPVOID)dwTemp, nickname, nSize, 0);
//
//	//群成员list
//	//char memberList[3000] = { 0 };
//	LPSTR memberList = (LPSTR)new char[300000];
//	ZeroMemory(memberList, 300000);
//	dwTemp = GetWxMemoryInt(g_hProcess, roomAddress + 0x3C);
//	ReadProcessMemory(g_hProcess, (LPVOID)dwTemp, memberList, 300000, 0);
//
//
//
//	//构建vRoomInfo数据
//	rapidjson::Value vRoomInfo(kObjectType);
//
//	//wxid
//	char* wxidUtf8 = UnicodeToUtf8(wxid);
//	Value vWxid(wxidUtf8, doc.GetAllocator());
//	vRoomInfo.AddMember("wxid", vWxid, doc.GetAllocator());
//
//	//群主
//	char* ownerUtf8 = UnicodeToUtf8(owner);
//	Value vOwnerUtf8(ownerUtf8, doc.GetAllocator());
//
//
//	const char flag[3] = "^G";
//	char* wxidItem;
//
//
//	/* 获取第一个子字符串 */
//	wxidItem = strtok(memberList, flag);
//
//	/* 继续获取其他的子字符串 */
//	Value array1(kArrayType);
//	while (wxidItem != NULL) {
//
//		Value string_object(kObjectType);
//		size_t nSize = strlen(wxidItem);
//		string_object.SetString(wxidItem, nSize, doc.GetAllocator());
//		array1.PushBack(string_object, doc.GetAllocator());
//		wxidItem = strtok(NULL, flag);
//	}
//
//
//
//	vRoomInfo.AddMember("manager_wxid", vOwnerUtf8, doc.GetAllocator());
//
//	//在群中的昵称
//	char* nicknameUtf8 = UnicodeToUtf8(nickname);
//	Value vAvatar(nicknameUtf8, doc.GetAllocator());
//	vRoomInfo.AddMember("nickname", vAvatar, doc.GetAllocator());
//
//	//成员id list
//
//	//处理数组，变成
//
//	Value vRemark(memberList, doc.GetAllocator());
//	vRoomInfo.AddMember("member_list", vRemark, doc.GetAllocator());
//
//	pVData->PushBack(vRoomInfo, doc.GetAllocator());
//
//	GetChatRoomInfo(header1, pVData);
//	GetChatRoomInfo(header2, pVData);
//	GetChatRoomInfo(header3, pVData);
//
//
//}
//



/******************业务代码 结束*****************/
