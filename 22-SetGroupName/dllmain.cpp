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
#include <windowsx.h>

using namespace std;


#define WxSetRoomName 0x2F93B0					//修改群名称

DWORD wxBaseAddress = 0;


//声明函数
VOID ShowDemoUI(HMODULE hModule);
INT_PTR CALLBACK DialogProc(_In_ HWND   hwndDlg, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
void SetRoomName(wchar_t* roomid, wchar_t* roomname);


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
			WCHAR wRoomId[50];
			UINT uINT = GetDlgItemText(hwndDlg, IDC_TEXT_ID, wRoomId, 50);
			if (uINT == 0)
			{
				MessageBoxA(NULL, "请填写wxid", "错误", MB_OK | MB_ICONERROR);
				return 0;
			}


			WCHAR wGroupName[50];
			uINT = GetDlgItemText(hwndDlg, IDC_TEXT_NAME, wGroupName, 50);
			if (uINT == 0)
			{
				MessageBoxA(NULL, "请填写群名称", "错误", MB_OK | MB_ICONERROR);
				return 0;
			}

			SetRoomName(wRoomId, wGroupName);
		}
		break;
	default:
		break;
	}
	return FALSE;
}

void SetRoomName(wchar_t* roomid, wchar_t* roomname) {
	DWORD winAddress = wxBaseAddress;
	DWORD dwCallAddr1 = winAddress + WxSetRoomName;

	GeneralStruct RoomWxid(roomid);

	GeneralStruct RoomName(roomname);

	__asm {
		pushad
		lea edx, RoomName
		lea ecx, RoomWxid
		call dwCallAddr1
		popad
	}
}