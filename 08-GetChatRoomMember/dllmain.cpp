// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "resource.h"
#include "windows.h" 
#include "shellapi.h"
#include <string>
#include <tchar.h> 
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <atlconv.h>

using namespace std;

//微信通用结构体
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

//通过微信ID获取用户信息结构体
struct UserInfo
{
	wchar_t UserId[0x100];
	wchar_t UserNumber[0x100];
	wchar_t UserNickName[0x100];
};

//3.1.0.72

//声明函数
VOID ShowDemoUI(HMODULE hModule);
INT_PTR CALLBACK DialogProc(_In_ HWND   hwndDlg, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

LPCWSTR String2LPCWSTR(string text);
string Dec2Hex(DWORD i);
//WCHAR* CharToWChar(char* s);

void ShowChatRoomUser(wchar_t* chatroomwxid);
void GetUserInfoByWxid(wchar_t* userwxid);

VOID UnLoadMyself();

////定义变量
DWORD wxBaseAddress = 0;
//DWORD g_callAddr = 0x3A0CA0;
//const int g_msgBuffer = 0x5A8;


#define GET_ROOM_MEMBER_CALL_FUN1 0x506CF0
#define GET_ROOM_MEMBER_CALL_FUN2 0x36EAA0
#define GET_ROOM_MEMBER_CALL_FUN3 0x50D720
#define GET_ROOM_MEMBER_CALL_FUN4 0x31B030


BOOL APIENTRY DllMain(HMODULE hModule,
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
	string text = "微信基址：\t";
	text.append(Dec2Hex(wxBaseAddress));
	OutputDebugString(String2LPCWSTR(text));



	DWORD funAddr = (DWORD)ShowChatRoomUser;

	string funAddrtext = "funAddr：\t";
	funAddrtext.append(Dec2Hex(funAddr));
	OutputDebugString(String2LPCWSTR(funAddrtext));

	DialogBox(hModule, MAKEINTRESOURCE(IDD_MAIN), NULL, &DialogProc);
}


//窗口回调函数，处理窗口事件
INT_PTR CALLBACK DialogProc(_In_ HWND   hwndDlg, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		break;

	case WM_CLOSE:
		//关闭窗口事件
		EndDialog(hwndDlg, 0);
		break;
	case WM_COMMAND:

		//发送消息
		if (wParam == IDOK)
		{
			wchar_t chatroomwxid[50] = L"18315130315@chatroom";
			ShowChatRoomUser(chatroomwxid);
		}

		else if (wParam == IDC_BTN_UNLOAD) {
			UnLoadMyself();
		}
		break;
	default:
		break;
	}
	return FALSE;
}

#define WxGetRoomUserWxidCall1 0x4FA8C0			//获取群成员ID  1
#define WxGetRoomUserWxidCall2 0x36EBA0			//获取群成员ID  1
#define WxGetRoomUserWxidCall3 0x500EE0			//获取群成员ID  1
#define WxGetRoomUserWxidCall4 0x4FB2E0			//获取群成员ID  1


void ShowChatRoomUser(wchar_t* chatroomwxid)
{
	//准备缓冲区
	DWORD dwWxidArr = 0;	//保存微信ID数据的地址
	char buff[0x164] = { 0 };
	char userListBuff[0x174] = { 0 };
	//构造数据
	GeneralStruct pWxid(chatroomwxid);
	char* asmWxid = (char*)&pWxid.pstr;

	//调用call
	DWORD dwCall1 = wxBaseAddress + WxGetRoomUserWxidCall1;
	DWORD dwCall2 = wxBaseAddress + WxGetRoomUserWxidCall2;
	DWORD dwCall3 = wxBaseAddress + WxGetRoomUserWxidCall3;
	DWORD dwCall4 = wxBaseAddress + WxGetRoomUserWxidCall4;

	//获取群成员
	__asm
	{
		lea ecx, buff[16];
		call dwCall1;
		lea eax, buff[16];
		push eax;
		mov ebx, asmWxid;
		push ebx;
		call dwCall2;
		mov ecx, eax;
		call dwCall3;
		lea eax, buff;
		push eax;
		lea ecx, buff[16];
		call dwCall4;
		mov dwWxidArr, eax;
	}

	//拿到微信ID
	wchar_t test[0x100] = { 0 };
	wchar_t tempWxid[0x100] = { 0 };
	char tempWxidA[0x100] = { 0 };
	DWORD userList = *((DWORD*)dwWxidArr);		//userList这个里面的微信ID列表 3.1是ASCII格式的微信ID

	wchar_t test77[0x100] = L"11111111111111111";
	DWORD testTmp = dwWxidArr + 0xB4;
	int Len = *((int*)testTmp);				//取到微信ID的个数



	for (int i = 0; i < Len; i++)
	{
		DWORD temWxidAdd = userList + (i * 0x18);		//0x18是每个微信ID的间隔
		int flags = (int)(*((LPVOID*)(temWxidAdd + 0x14)));
		if (flags == 0xF)
		{
			sprintf_s(tempWxidA, "%s", (char*)temWxidAdd);
		}
		else
		{
			sprintf_s(tempWxidA, "%s", (char*)*((LPVOID*)temWxidAdd));
		}

		USES_CONVERSION;
		//2.再通过微信ID获取群成员信息
		GetUserInfoByWxid(A2W(tempWxidA));
	}

}

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

	LPVOID lpWxid = *((LPVOID*)((DWORD)buff + 0x20));				//微信ID
	LPVOID lpWxcount = *((LPVOID*)((DWORD)buff + 0x34));			//微信账号
	LPVOID lpNickName = *((LPVOID*)((DWORD)buff + 0x7C));			//昵称


	//组装结构体
	UserInfo* userinfo = new UserInfo;
	swprintf_s(userinfo->UserId, L"%s", (wchar_t*)lpWxid);
	swprintf_s(userinfo->UserNickName, L"%s", (wchar_t*)lpNickName);
	swprintf_s(userinfo->UserNumber, L"%s", (wchar_t*)lpWxcount);


	//COPYDATASTRUCT userinfodata;
	//userinfodata.dwData = WM_ShowChatRoomMembers;//保存一个数值, 可以用来作标志等
	//userinfodata.cbData = sizeof(UserInfo);// strlen(szSendBuf);//待发送的数据的长
	//userinfodata.lpData = userinfo;// szSendBuf;//待发送的数据的起始地址(可以为NULL)
	//SendMessage(hWnd, WM_COPYDATA, (WPARAM)hWnd, (LPARAM)&userinfodata);

	delete userinfo;

}


//将int转成16进制字符串
string Dec2Hex(DWORD i)
{
	//定义字符串流
	stringstream ioss;
	//存放转化后字符
	string s_temp;
	//以十六制(大写)形式输出
	ioss.fill('0');
	ioss << setiosflags(ios::uppercase) << setw(8) << hex << i;
	//以十六制(小写)形式输出//取消大写的设置
	//ioss << resetiosflags(ios::uppercase) << hex << i;
	ioss >> s_temp;
	return "0x" + s_temp;
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

		//卸载自己并退出
		FreeLibraryAndExitThread(hModule, 0);
	}
}


//把string 转换为 LPCWSTR
LPCWSTR String2LPCWSTR(string text)
{
	size_t size = text.length();
	WCHAR* buffer = new WCHAR[size + 1];
	MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, buffer, size + 1);

	//确保以 '\0' 结尾
	buffer[size] = 0;
	return buffer;
}


