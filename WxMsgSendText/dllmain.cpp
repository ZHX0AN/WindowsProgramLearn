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

using namespace std;

//声明函数
VOID ShowDemoUI(HMODULE hModule);
INT_PTR CALLBACK DialogProc(_In_ HWND   hwndDlg, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
void SentTextMessage(HWND hwndDlg);
void SentAtTextMessage(HWND hwndDlg);
LPCWSTR String2LPCWSTR(string text);
string Dec2Hex(DWORD i);
WCHAR* CharToWChar(char* s);

//定义变量
DWORD wxBaseAddress = 0;
DWORD g_callAddr = 0x3A0C20;


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
	string text = "微信基址：\t";
	text.append(Dec2Hex(wxBaseAddress));
	OutputDebugString(String2LPCWSTR(text));

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
		if (wParam == BTN_SEND)
		{
			OutputDebugString(TEXT("发送消息按钮被点击"));
			SentTextMessage(hwndDlg);
		}

		if (wParam == BTN_SEND_AT)
		{
			OutputDebugString(TEXT("发送at消息 BTN_SEND_AT"));
			SentAtTextMessage(hwndDlg);
		}

		break;
	default:
		break;
	}
	return FALSE;
}

WCHAR* CharToWChar(char* s)
{
	int w_nlen = MultiByteToWideChar(CP_ACP, 0, s, -1, NULL, 0);
	WCHAR* ret = (WCHAR*)malloc(sizeof(WCHAR) * w_nlen);
	memset(ret, 0, sizeof(ret));
	MultiByteToWideChar(CP_ACP, 0, s, -1, ret, w_nlen);
	return ret;
}

//文本消息结构体
struct StructWxid
{
	//发送的文本消息指针
	wchar_t* pWxid;
	//字符串长度
	DWORD length;
	//字符串最大长度
	DWORD maxLength;

	//补充两个占位数据
	DWORD fill1;
	DWORD fill2;
};

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


VOID SentTextMessage(HWND hwndDlg)
{
	//6CED0E62    E8 89992200     call WeChatWi.6D0FA7F0; 发送消息断点 
	DWORD callAddress_SendText = wxBaseAddress + 0x3A0C20;

	//组装wxid数据
	WCHAR wxid[50];
	UINT uINT = GetDlgItemText(hwndDlg, INPUT_WXID, wxid, 50);
	if (uINT == 0)
	{
		MessageBoxA(NULL, "请填写wxid", "错误", MB_OK | MB_ICONERROR);
		return;
	}

	StructWxid structWxid = { 0 };
	structWxid.pWxid = wxid;
	structWxid.length = wcslen(wxid);
	structWxid.maxLength = wcslen(wxid) * 2;

	//structWxid.Init();
	//取wxid的地址
	DWORD* asmWxid = (DWORD*)&structWxid.pWxid;

	//组装发送的文本数据
	WCHAR wxMsg[1024];
	uINT = GetDlgItemText(hwndDlg, INPUT_MSG, wxMsg, 1024);
	if (uINT == 0)
	{
		MessageBoxA(NULL, "请填写要发送的文本", "错误", MB_OK | MB_ICONERROR);
		return;
	}

	StructWxid structMessage = { 0 };
	structMessage.pWxid = wxMsg;
	structMessage.length = wcslen(wxMsg);
	structMessage.maxLength = wcslen(wxMsg) * 2;
	DWORD* asmMsg = (DWORD*)&structMessage.pWxid;

	//定义一个缓冲区
	BYTE buff[0x5A8] = { 0 };

	//执行汇编调用
	__asm
	{
		push 0x1

		// 这儿放置被at的微信ID
		mov edi, 0x0
		push edi

		//微信消息内容
		mov ebx, asmMsg
		push ebx

		mov edx, asmWxid
		lea ecx, buff

		//调用函数
		call callAddress_SendText
		add esp, 0xC
	}

}

class ROOM_AT
{
public:
	DWORD at_WxidList = 0;
	DWORD at_end1 = 0;
	DWORD at_end2 = 0;
};

class  TEXT_WXID
{
public:
	wchar_t* pWxid = nullptr;
	DWORD length = 0;
	DWORD maxLength = 0;
	DWORD fill1 = 0;
	DWORD fill2 = 0;
};


/**
 * at没调试成功
 */
VOID SentAtTextMessage(HWND hwndDlg)
{
	OutputDebugString(L"in SentAtTextMessage...");
	string text = "";

	//6841A1CB    E8 506A2900     call WeChatWi.686B0C20
	DWORD callAddress_SendText = wxBaseAddress + g_callAddr;

	text = "Call地址:";
	text.append(Dec2Hex(callAddress_SendText));
	OutputDebugString(String2LPCWSTR(text));

	//组装wxid数据
	WCHAR wxid[50];
	UINT uINT = GetDlgItemText(hwndDlg, INPUT_WXID, wxid, 50);
	if (uINT == 0)
	{
		MessageBoxA(NULL, "请填写wxid", "错误", MB_OK | MB_ICONERROR);
		return;
	}


	StructWxid structWxid = { 0 };
	structWxid.pWxid = wxid;
	structWxid.length = wcslen(wxid);
	structWxid.maxLength = wcslen(wxid) * 2;

	text = "微信ID长度:";
	text.append(Dec2Hex(structWxid.length));
	OutputDebugString(String2LPCWSTR(text));

	//structWxid.Init();
	//取wxid的地址
	DWORD* asmWxid = (DWORD*)&structWxid.pWxid;


	//组装发送的文本数据
	WCHAR wxMsg[1024];
	uINT = GetDlgItemText(hwndDlg, INPUT_MSG, wxMsg, 1024);
	if (uINT == 0)
	{
		MessageBoxA(NULL, "请填写要发送的文本", "错误", MB_OK | MB_ICONERROR);
		return;
	}

	StructWxid structMessage = { 0 };
	structMessage.pWxid = wxMsg;
	structMessage.length = wcslen(wxMsg);
	structMessage.maxLength = wcslen(wxMsg) * 2;
	DWORD* asmMsg = (DWORD*)&structMessage.pWxid;

	// 获取at消息
	WCHAR wxidAt2[50];
	UINT uINTAt = GetDlgItemText(hwndDlg, INPUT_AT, wxidAt2, 50);

	TEXT_WXID wxAtId;
	wxAtId.pWxid = wxidAt2;
	wxAtId.length = wcslen(wxidAt2);
	wxAtId.maxLength = wcslen(wxidAt2) * 2;
	wxAtId.fill1 = 0;
	wxAtId.fill2 = 0;

	ROOM_AT roomAt;
	roomAt.at_WxidList = (DWORD)&wxAtId.pWxid;
	roomAt.at_end1 = roomAt.at_WxidList + 5 * 4;
	roomAt.at_end2 = roomAt.at_end1;

	//定义一个缓冲区
	BYTE buff[0x87C] = { 0 };

	OutputDebugString(L"before asm");

	//执行汇编调用
	__asm
	{

		//lea edx, asmWxid

		////传递参数
		//push 0x1

		//lea eax, roomAt
		//push eax

		////微信消息内容
		//lea ebx, asmMsg

		//push ebx
		//lea ecx, buff

		////调用函数
		//call callAddress_SendText

		////平衡堆栈
		//add esp, 0xC

		push 0x1

		// 这儿放置被at的微信ID
		mov edi, roomAt
		push edi

		//微信消息内容
		mov ebx, asmMsg
		push ebx

		mov edx, asmWxid
		lea ecx, buff

		//调用函数
		call callAddress_SendText
		// 因为前面push了3个参数，堆栈要平衡
		add esp, 0xC
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

string WcharToString(WCHAR* wchar)
{
	WCHAR* wText = wchar;
	// WideCharToMultiByte的运用
	DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, NULL, 0, NULL, FALSE);
	// psText为char*的临时数组，作为赋值给std::string的中间变量
	char* psText = new char[dwNum];
	// WideCharToMultiByte的再次运用
	WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, psText, dwNum, NULL, FALSE);
	// std::string赋值
	return psText;
}