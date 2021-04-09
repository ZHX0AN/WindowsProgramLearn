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


#define RECV_REVOKE_MSG_ADDRESS 0x2E6DE4
#define RECV_REVOKE_MSG_CALL_ADDRESS 0x2E7A70

DWORD g_recvRevokeCallAddress = 0;
DWORD g_recvRevokeBack = 0;
//DWORD g_recvRevokeHookAddr = 0;



//声明函数
VOID ShowDemoUI(HMODULE hModule);
INT_PTR CALLBACK DialogProc(_In_ HWND   hwndDlg, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
VOID AddText(HWND hwnd, PCTSTR pszFormat, ...);
wchar_t* GetMsgByAddress2(DWORD memAddress);
VOID RecvRevokeHookWx();
VOID RecvRevokeInlineHook();
VOID RecvRevokeDealMsg(DWORD r_edx);
VOID RecvRevokeDealMsg2(DWORD r_ecx, DWORD r_edx);

////定义变量
DWORD wxBaseAddress = 0;

HWND g_hwndDlg;

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
		g_hwndDlg = hwndDlg;

		break;

	case WM_CLOSE:
		//关闭窗口事件
		EndDialog(hwndDlg, 0);
		break;
	case WM_COMMAND:

		//发送消息
		if (wParam == IDOK)
		{
			OutputDebugString(TEXT("发送消息按钮被点击"));

			RecvRevokeHookWx();
		}
		else if (wParam == IDCANCEL) {
			OutputDebugString(TEXT("卸载自己"));

		}

		break;
	default:
		break;
	}
	return FALSE;
}

//Hook接收消息
VOID RecvRevokeHookWx()
{


	//WeChatWin.dll+354AA3 
	int hookAddress = wxBaseAddress + RECV_REVOKE_MSG_ADDRESS;

	g_recvRevokeBack = hookAddress + 5;

	g_recvRevokeCallAddress = wxBaseAddress + RECV_REVOKE_MSG_CALL_ADDRESS;


	//组装跳转数据
	BYTE jmpCode[5] = { 0 };
	jmpCode[0] = 0xE9;

	//新跳转指令中的数据=跳转的地址-原地址（HOOK的地址）-跳转指令的长度
	*(DWORD*)&jmpCode[1] = (DWORD)RecvRevokeInlineHook - hookAddress - 5;

	////保存当前位置的指令,在unhook的时候使用。
	//ReadProcessMemory(GetCurrentProcess(), (LPVOID)hookAddress, originalCode, 5, 0);

	//覆盖指令 B9 E8CF895C //mov ecx,0x5C89CFE8
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)hookAddress, jmpCode, 5, 0);
}


//跳转到这里，让我们自己处理消息
__declspec(naked) VOID RecvRevokeInlineHook()
{

	__asm
	{

		//保存寄存器
		pushad
		pushf


		//调用接收消息的函数
		//方法一
		//push edx
		//call RecvRevokeDealMsg
		//add esp, 4

		//方法二
		push edx
		push ecx
		call RecvRevokeDealMsg2
		add esp, 8

		popf
		popad

		call g_recvRevokeCallAddress
		jmp g_recvRevokeBack
	}
}

VOID RecvRevokeDealMsg(DWORD r_edx)
{
	/*地址跟接收消息一样*/

	DWORD SvridOffset = 0x28;
	//时间戳
	DWORD timestampOffset = 0x3C;

	//消息类型
	DWORD msgTypeOffset = 0x30;
	//发送人，好友或群ID
	DWORD friendOffset = 0x40;
	//群消息，消息发送者
	DWORD roomMsgSenderOffset = 0x164;
	//消息内容
	DWORD contentOffset = 0x68;

	//子类型，如果是文件，sub=6
	DWORD subTypeOffset = 0xEC;

	//好友消息：<msgsource />
	//群消息: 01023B78  <msgsource>..<silence>0</silence>..<membercount>2</membercount> < / msgsource>
	//群at消息: <msgsource>..<atuserlist>wxid_4sy2barbyny712</atuserlist>..<silence>0</silence>..<membercount>2</membercount>.</msgsource>
	DWORD atFriendOffst = 0x1b8;

	//文件存储路径：wxid_4sy2barbyny712\FileStorage\File\2021-03\工作簿1(1).xlsx
	DWORD filePathOffset = 0x1A0;


	INT64 Svrid = *((INT64*)(r_edx + SvridOffset));
	DWORD msgType = *((DWORD*)(r_edx + msgTypeOffset));
	DWORD msgSubType = *((DWORD*)(r_edx + subTypeOffset));
	DWORD timestamp = *((DWORD*)(r_edx + timestampOffset));
	wchar_t* wToWxId = GetMsgByAddress2(r_edx + friendOffset);
	wchar_t* wContent = GetMsgByAddress2(r_edx + contentOffset);
	wchar_t* wAt = GetMsgByAddress2(r_edx + atFriendOffst);
	wchar_t* wFilePath = GetMsgByAddress2(r_edx + filePathOffset);
	wchar_t* wRoomSender = GetMsgByAddress2(r_edx + roomMsgSenderOffset);

	AddText(GetDlgItem(g_hwndDlg, IDC_TEXT_INFO), TEXT("Svrid: %I64d \r\n"), Svrid);
	AddText(GetDlgItem(g_hwndDlg, IDC_TEXT_INFO), TEXT("时间戳: %I32d \r\n"), timestamp);
	AddText(GetDlgItem(g_hwndDlg, IDC_TEXT_INFO), TEXT("信息类型: %d \r\n"), msgType);
	AddText(GetDlgItem(g_hwndDlg, IDC_TEXT_INFO), TEXT("信息子类型: %d \r\n"), msgSubType);
	AddText(GetDlgItem(g_hwndDlg, IDC_TEXT_INFO), TEXT("接收者: %s \r\n"), wToWxId);
	AddText(GetDlgItem(g_hwndDlg, IDC_TEXT_INFO), TEXT("消息内容: %s \r\n"), wContent);
	AddText(GetDlgItem(g_hwndDlg, IDC_TEXT_INFO), TEXT("AT好友: %s \r\n"), wAt);
	AddText(GetDlgItem(g_hwndDlg, IDC_TEXT_INFO), TEXT("文件路径: %s \r\n"), wFilePath);
	AddText(GetDlgItem(g_hwndDlg, IDC_TEXT_INFO), TEXT("群消息发送者: %s \r\n"), wRoomSender);




}

VOID RecvRevokeDealMsg2(DWORD r_ecx, DWORD r_edx)
{

	DWORD SvridOffset = 0x28;
	//时间戳
	DWORD timestampOffset = 0x3C;


	//消息类型
	DWORD msgTypeOffset = 0x30;
	//发送人，好友或群ID
	DWORD friendOffset = 0x40;
	//群消息，消息发送者
	DWORD roomMsgSenderOffset = 0x164;
	//消息内容
	DWORD contentOffset = 0x68;

	//子类型，如果是文件，sub=6
	DWORD subTypeOffset = 0xEC;

	//好友消息：<msgsource />
	//群消息: 01023B78  <msgsource>..<silence>0</silence>..<membercount>2</membercount> < / msgsource>
	//群at消息: <msgsource>..<atuserlist>wxid_4sy2barbyny712</atuserlist>..<silence>0</silence>..<membercount>2</membercount>.</msgsource>
	DWORD atFriendOffst = 0x1b8;

	//文件存储路径：wxid_4sy2barbyny712\FileStorage\File\2021-03\工作簿1(1).xlsx
	DWORD filePathOffset = 0x1A0;


	INT64 Svrid = *((INT64*)(r_edx + SvridOffset));
	DWORD msgType = *((DWORD*)(r_edx + msgTypeOffset));
	DWORD msgSubType = *((DWORD*)(r_edx + subTypeOffset));
	DWORD timestamp = *((DWORD*)(r_edx + timestampOffset));
	wchar_t* wToWxId = GetMsgByAddress2(r_edx + friendOffset);
	wchar_t* wContent = GetMsgByAddress2(r_edx + contentOffset);
	wchar_t* wAt = GetMsgByAddress2(r_edx + atFriendOffst);
	wchar_t* wFilePath = GetMsgByAddress2(r_edx + filePathOffset);
	wchar_t* wRoomSender = GetMsgByAddress2(r_edx + roomMsgSenderOffset);

	AddText(GetDlgItem(g_hwndDlg, IDC_TEXT_INFO), TEXT("Svrid: %I64d \r\n"), Svrid);
	AddText(GetDlgItem(g_hwndDlg, IDC_TEXT_INFO), TEXT("时间戳: %I32d \r\n"), timestamp);
	AddText(GetDlgItem(g_hwndDlg, IDC_TEXT_INFO), TEXT("信息类型: %d \r\n"), msgType);
	AddText(GetDlgItem(g_hwndDlg, IDC_TEXT_INFO), TEXT("信息子类型: %d \r\n"), msgSubType);
	AddText(GetDlgItem(g_hwndDlg, IDC_TEXT_INFO), TEXT("接收者: %s \r\n"), wToWxId);
	AddText(GetDlgItem(g_hwndDlg, IDC_TEXT_INFO), TEXT("消息内容: %s \r\n"), wContent);
	AddText(GetDlgItem(g_hwndDlg, IDC_TEXT_INFO), TEXT("AT好友: %s \r\n"), wAt);
	AddText(GetDlgItem(g_hwndDlg, IDC_TEXT_INFO), TEXT("文件路径: %s \r\n"), wFilePath);
	AddText(GetDlgItem(g_hwndDlg, IDC_TEXT_INFO), TEXT("群消息发送者: %s \r\n"), wRoomSender);

	AddText(GetDlgItem(g_hwndDlg, IDC_TEXT_INFO), TEXT(" *************** 下面是撤回消息 ************** \r\n"));


	DWORD recv_SvridOffset = 0x28;
	DWORD recv_TimestampOffset = 0x28;
	DWORD recv_ContentOffset = 0x70;
	DWORD recv_MsgFromWxidOffset = 0x98;//消息来源，群ID或好友ID

	INT64 recv_SvridOff = *((INT64*)(r_ecx + recv_SvridOffset));
	DWORD recv_timestamp = *((DWORD*)(r_ecx + recv_TimestampOffset));
	wchar_t* recv_wContent = GetMsgByAddress2(r_ecx + recv_ContentOffset);
	wchar_t* recv_wMsgFromWxidWxid = GetMsgByAddress2(r_ecx + recv_MsgFromWxidOffset);
	AddText(GetDlgItem(g_hwndDlg, IDC_TEXT_INFO), TEXT("Svrid: %I64d \r\n"), recv_SvridOff);
	AddText(GetDlgItem(g_hwndDlg, IDC_TEXT_INFO), TEXT("时间戳: %I32d \r\n"), recv_timestamp);
	AddText(GetDlgItem(g_hwndDlg, IDC_TEXT_INFO), TEXT("消息内容: %s \r\n"), recv_wContent);
	AddText(GetDlgItem(g_hwndDlg, IDC_TEXT_INFO), TEXT("撤回消息者: %s \r\n"), recv_wMsgFromWxidWxid);



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

// wchar_t 转UTF8
wchar_t* GetMsgByAddress2(DWORD memAddress)
{
	//获取字符串长度
	DWORD msgLength = *(DWORD*)(memAddress + 4);
	if (msgLength == 0)
	{
		WCHAR* msg = new WCHAR[1];
		msg[0] = 0;
		return msg;
	}

	WCHAR* msg = new WCHAR[msgLength + 1];
	ZeroMemory(msg, msgLength + 1);

	//复制内容
	wmemcpy_s(msg, msgLength + 1, (WCHAR*)(*(DWORD*)memAddress), msgLength + 1);
	return msg;
}
