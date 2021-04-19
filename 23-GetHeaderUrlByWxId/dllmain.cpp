// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "resource.h"
#include "shellapi.h"
#include <string>
#include <tchar.h> 
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <windowsx.h>

using namespace std;


#define WxGetHeadCall1  0x36E6A0				//获取头像
#define WxGetHeadCall2  0x536E20				//获取头像

DWORD wxBaseAddress = 0;


//声明函数
VOID ShowDemoUI(HMODULE hModule);
INT_PTR CALLBACK DialogProc(_In_ HWND   hwndDlg, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
void GetHeaderUrl(HWND hwnd, wchar_t* wxid);
VOID AddText(HWND hwnd, PCTSTR pszFormat, ...);

wstring GetMsgByAddress(DWORD memAddress);
wstring GetNicknameByWxid(wstring userwxid);



typedef struct _WXSTRING {
	wchar_t* pstr;
	int len;
	int maxLen;
	int fill1 = 0;
	int fill2 = 0;
} WxString, * PWxString;

typedef struct _WXUSERINFO {
	char top[0x8];
	WxString wxid;
	WxString account;
	WxString vData;
	int un_0;
	int un_1;
	char un_2[0x18];
	WxString name;
	char un_3[0x28];
	int un_4;
	WxString nameUpper;
	WxString nameUn;
	char un_5[0x28];
	WxString pic;
	WxString img;
	char* md5;
	char un_6[0xC];
	int un_7;
	int addSource;
	char un_8[0x2AC];
} WxUserInfo, * PWxUserInfo;


bool GetUserInfoByWxid(wstring wxid, WxUserInfo* info);


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
			WCHAR wWxId[50];
			UINT uINT = GetDlgItemText(hwndDlg, IDC_TEXT_WXID, wWxId, 50);
			if (uINT == 0)
			{
				MessageBoxA(NULL, "请填写wxid", "错误", MB_OK | MB_ICONERROR);
				return 0;
			}

			//获取头url
			//GetHeaderUrl(hwndDlg, wWxId);

			//第一版
			//WxUserInfo info = { 0 };
			//GetUserInfoByWxid(wWxId, &info);

			//wstring w = info.wxid.pstr;
			//wstring name = info.name.pstr;

			//第二版
			GetNicknameByWxid(wWxId);

		}
		break;
	default:
		break;
	}
	return FALSE;
}

void GetHeaderUrl(HWND hwnd, wchar_t* wxid) {
	
	
	// 获取微信基址
	DWORD winAddress = wxBaseAddress;
	// 开始获取好友详情数据
	DWORD r_esp = 0;
	DWORD call1 = winAddress + WxGetHeadCall1;
	DWORD call2 = winAddress + WxGetHeadCall2;

	char buff[0x88] = { 0 };//如何确定这个缓冲区
	DWORD head = (DWORD)buff;

	GeneralStruct stWxid(wxid);

	__asm {
		pushad
		mov r_esp, esp
		lea ecx, buff
		mov eax, ecx
		push eax
		lea eax, stWxid
		push eax
		call call1
		mov ecx, eax
		call call2
		mov esp, r_esp
		popad
	}


	wstring smallHead = GetMsgByAddress(head + 0x14);
	wstring	bigHead = GetMsgByAddress(head + 0x28);

	AddText(GetDlgItem(hwnd, IDC_TEXT_MSG), TEXT("小头像: %s\r\n"), smallHead.c_str());
	AddText(GetDlgItem(hwnd, IDC_TEXT_MSG), TEXT("大头像: %s\r\n"), bigHead.c_str());


}

VOID AddText(HWND hwnd, PCTSTR pszFormat, ...) {

	va_list argList;
	va_start(argList, pszFormat);

	TCHAR sz[7 * 1024];
	Edit_GetText(hwnd, sz, _countof(sz));
	_vstprintf_s(_tcschr(sz, TEXT('\0')), _countof(sz) - _tcslen(sz), pszFormat, argList);
	Edit_SetText(hwnd, sz);
	va_end(argList);
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



//第一版,不行
bool GetUserInfoByWxid(wstring wxid, WxUserInfo* info) {
	WxString temp = { 0 };
	temp.pstr = (wchar_t*)wxid.c_str();
	temp.len = wxid.size();
	temp.maxLen = wxid.size() * 2;

	//DWORD call_1 = Util->Offset(0x7B2241C0 - 0x7ACB0000);
	//DWORD call_2 = Util->Offset(0x7AD11900 - 0x7ACB0000);
	//DWORD call_3 = Util->Offset(0x7AFC9E70 - 0x7ACB0000);

	DWORD winAddress = wxBaseAddress;

	DWORD call_1 = winAddress + 0x7B2241C0 - 0x7ACB0000;
	DWORD call_2 = winAddress + 0x7AD11900 - 0x7ACB0000;
	DWORD call_3 = winAddress + 0x7AFC9E70 - 0x7ACB0000;


	int ret = 0;
	__asm {
		pushad;
		pushfd;

		mov edi, info;
		push edi;
		sub esp, 0x14;
		lea eax, temp;
		mov ecx, esp;
		push eax;

		call call_1;
		call call_2;
		call call_3;

		mov ret, eax;

		popfd;
		popad;
	}
	return ret == 1;
}


//第二版
#define WxGetNativeUserInfoByWxidCall1 0x574540
#define WxGetNativeUserInfoByWxidCall2 0x319E70
#define WxGetNativeUserInfoByWxidCall3 0x4E53C0

wstring GetNicknameByWxid(wstring userwxid)
{
	DWORD dwCall1 = wxBaseAddress + WxGetNativeUserInfoByWxidCall1;
	DWORD dwCall2 = wxBaseAddress + WxGetNativeUserInfoByWxidCall2;
	DWORD dwCall3 = wxBaseAddress + WxGetNativeUserInfoByWxidCall3;

	char buff[0x508] = { 0 };
	char* asmHeadBuff = buff;
	char* asmBuff = &buff[0x18];

	//GeneralStruct pWxid = { 0 };
	//pWxid.pstr = (wchar_t*)userwxid.c_str();
	//pWxid.iLen = userwxid.size();
	//pWxid.iMaxLen = userwxid.size();

	GeneralStruct pWxid = { (wchar_t*)userwxid.c_str() };

	__asm
	{
		pushad;
		lea edi, pWxid;
		mov eax, asmBuff;
		push eax;
		sub esp, 0x14;
		mov ecx, esp;
		push - 0x1;
		mov dword ptr ds : [ecx] , 0x0;
		mov dword ptr ds : [ecx + 0x4] , 0x0;
		mov dword ptr ds : [ecx + 0x8] , 0x0;
		mov dword ptr ds : [ecx + 0xC] , 0x0;
		mov dword ptr ds : [ecx + 0x10] , 0x0;
		push dword ptr ds : [edi] ;
		call dwCall1;
		call dwCall2;
		lea eax, asmHeadBuff;
		push eax;
		mov ecx, asmBuff;
		call dwCall3;
		popad
	}

	wstring nickname = GetMsgByAddress((DWORD)buff + 0x7C);
	return nickname;
}