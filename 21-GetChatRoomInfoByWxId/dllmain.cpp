// dllmain.cpp : 定义 DLL 应用程序的入口点。
#define _CRT_SECURE_NO_WARNINGS
#include "pch.h"
#include "resource.h"
#include "shellapi.h"
#include <string>
#include <tchar.h> 
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <strstream>

using namespace std;



typedef struct _WXSTRING {
	wchar_t* pstr;
	int len;
	int maxLen;
	int fill1 = 0;
	int fill2 = 0;
} WxString, * PWxString;

//typedef struct _WXGROUPINFO {
//	char top[0x4];
//	WxString groupid;
//	char* member;
//	char un_0[0xc];
//	int un_1;
//	int un_2;
//	WxString un_3;
//	int un_4;
//	WxString admin;
//	char buff[0x118];
//} WxGroupInfo, * PWxGroupInfo;

typedef struct _WXGROUPINFO {
	char top[0x4];
	WxString groupid;
	char* member;
	char un_0[0xc];
	int un_1;
	int un_2;
	WxString un_3;
	int un_4;
	WxString admin;
	int un_5;
	WxString myNickname;
	char un_6[0x24];
	int number;
	char buff[0xD8];
} WxGroupInfo, * PWxGroupInfo;

//定义变量
DWORD wxBaseAddress = 0;
HWND hWinDlg;

//声明函数
VOID ShowDemoUI(HMODULE hModule);
VOID UnLoadMyself();
BOOL GetChatRoomInfo(HWND hwndDlg, WxGroupInfo* info);
string ReadAsciiString(DWORD address);
wchar_t* AnsiToUnicode(const char* szStr);
void GetUserInfoByWxid(wchar_t* userwxid);

wstring GetMsgByAddress(DWORD memAddress);


//窗口回调函数，处理窗口事件
INT_PTR CALLBACK DialogProc(_In_ HWND   hwndDlg, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	hWinDlg = hwndDlg;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		break;

	case WM_CLOSE:
		//关闭窗口事件
		EndDialog(hwndDlg, 0);
		break;
	case WM_COMMAND:

		if (wParam == IDOK)
		{
			WxGroupInfo info = { 0 };
			GetChatRoomInfo(hwndDlg, &info);

			string member = ReadAsciiString((DWORD)info.member);
			int number = info.number;

			char* memberList = info.member;

			const char flag[3] = "^G";
			char* wxidItem;
			char* buf;
			wxidItem = strtok_s(memberList, flag, &buf);
			while (wxidItem != NULL) {
				wchar_t* wTemp = AnsiToUnicode(wxidItem);
				GetUserInfoByWxid(wTemp);

				wxidItem = strtok_s(NULL, flag, &buf);
			}

			break;
		}
		else if (wParam == IDCANCEL) {
			UnLoadMyself();
		}

		break;
	default:
		break;
	}
	return FALSE;
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
		HANDLE hANDLE = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ShowDemoUI, hModule, NULL, 0);
		if (hANDLE != 0)
		{
			CloseHandle(hANDLE);
		}
		break;
	}
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

//显示操作的窗口
VOID ShowDemoUI(HMODULE hModule)
{
	//获取WeChatWin.dll的基址
	wxBaseAddress = (DWORD)GetModuleHandle(TEXT("WeChatWin.dll"));
	DialogBox(hModule, MAKEINTRESOURCE(IDD_MAIN), NULL, &DialogProc);
}




BOOL GetChatRoomInfo(HWND hwndDlg, WxGroupInfo* info)
{
	//call WeChatWi.6D0FA7F0; 发送消息断点 ,6D0FA7F0 = wxBaseAddress + g_callAddr
	//DWORD callFunAddr = wxBaseAddress + SEND_MSG_HOOK_ADDRESS;

	DWORD call_init = wxBaseAddress + 0x78F2A8C0 - 0x78A30000;
	DWORD call_query = wxBaseAddress + 0x78D2D6A0 - 0x78A30000;


	//组装wxid数据
	WCHAR wxid[50];
	UINT uINT = GetDlgItemText(hwndDlg, IDC_TEXT_ROOMID, wxid, 50);
	if (uINT == 0)
	{
		MessageBoxA(NULL, "请填写wxid", "错误", MB_OK | MB_ICONERROR);
		return 0;
	}

	wstring groupid(wxid);
	WxString id = { 0 };
	id.pstr = (wchar_t*)groupid.c_str();
	id.len = groupid.size();
	id.maxLen = groupid.size() * 2;


	int iRet = 0;
	__asm {
		pushad;
		pushfd;

		mov ecx, info;
		call call_init;

		mov eax, info;
		push eax;
		lea ebx, id;
		push ebx;
		call call_query;
		mov iRet, eax;

		popfd;
		popad;
	}
	return iRet == 1;

}


struct GeneralStruct
{
	wchar_t* pstr;
	int iLen;
	int iMaxLen;
	int full1;
	int full2;
	GeneralStruct(wchar_t* pString)
	{
		pstr = pString;
		iLen = wcslen(pString);
		iMaxLen = iLen * 2;
		full1 = 0;
		full2 = 0;
	}
};


#define WxGetUserInfoWithNoNetworkCall1 0x574540		 //根据微信ID获取用户信息  1
#define WxGetUserInfoWithNoNetworkCall2 0x319E70		 //根据微信ID获取用户信息  1
#define WxGetUserInfoWithNoNetworkCall3 0x4E53C0		 //根据微信ID获取用户信息  1

void GetUserInfoByWxid(wchar_t* userwxid)
{
	DWORD WechatBase = (DWORD)GetModuleHandle(L"WeChatWin.dll");

	DWORD dwCall1 = WechatBase + WxGetUserInfoWithNoNetworkCall1;
	DWORD dwCall2 = WechatBase + WxGetUserInfoWithNoNetworkCall2;
	DWORD dwCall3 = WechatBase + WxGetUserInfoWithNoNetworkCall3;

	char buff[0x508] = { 0 };
	char* asmHeadBuff = buff;
	char* asmBuff = &buff[0x18];

	GeneralStruct pWxid(userwxid);
	char* asmWxid = (char*)&pWxid.pstr;

	__asm
	{
		pushad;
		mov edi, asmWxid;		//微信ID结构体	
		mov eax, asmBuff;		//缓冲区
		push eax;
		sub esp, 0x14;
		mov ecx, esp;
		push - 0x1;
		mov dword ptr ds : [ecx] , 0x0;
		mov dword ptr ds : [ecx + 0x4] , 0x0;
		mov dword ptr ds : [ecx + 0x8] , 0x0;
		mov dword ptr ds : [ecx + 0xC] , 0x0;
		mov dword ptr ds : [ecx + 0x10] , 0x0;
		push dword ptr ds : [edi] ;	//微信ID
		call dwCall1;				//call1
		call dwCall2;				//call2
		mov eax, asmHeadBuff;
		push eax;
		mov ecx, asmBuff;
		call dwCall3;
		popad
	}

	wstring nickname = GetMsgByAddress((DWORD)buff + 0x7C);

	//LPVOID lpWxid = *((LPVOID*)((DWORD)buff + 0x20));				//微信ID
	//LPVOID lpWxcount = *((LPVOID*)((DWORD)buff + 0x34));			//微信账号
	//LPVOID lpNickName = *((LPVOID*)((DWORD)buff + 0x7C));			//昵称

}


//卸载自己
VOID UnLoadMyself()
{
	HMODULE hModule = NULL;

	//GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS 会增加引用计数
	//因此，后面还需执行一次FreeLibrary
	//直接使用本函数（UnInject）地址来定位本模块
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPWSTR)&UnLoadMyself, &hModule);

	if (hModule != 0)
	{
		//减少一次引用计数
		FreeLibrary(hModule);

		//从内存中卸载
		FreeLibraryAndExitThread(hModule, 0);
	}
}


string ReadAsciiString(DWORD address) {
	string sValue = "";
	char cValue[0x1000] = { 0 };
	memcpy(cValue, (const void*)address, 0x1000);
	sValue = string(cValue);
	return sValue;
}



wchar_t* AnsiToUnicode(const char* szStr)
{
	int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szStr, -1, NULL, 0);
	if (nLen == 0)
	{
		return NULL;
	}
	wchar_t* pResult = new wchar_t[nLen];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szStr, -1, pResult, nLen);
	return pResult;
}


wstring GetMsgByAddress(DWORD memAddress)
{
	wstring tmp;
	DWORD msgLength = *(DWORD*)(memAddress + 4);
	if (msgLength > 0) {
		WCHAR* msg = new WCHAR[msgLength + 1]{ 0 };
		wmemcpy_s(msg, msgLength + 1, (WCHAR*)(*(DWORD*)memAddress), msgLength + 1);
		tmp = msg;
		delete[]msg;
	}
	return  tmp;

}
