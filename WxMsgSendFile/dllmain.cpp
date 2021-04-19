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
#include<thread>

using namespace std;

//3.1.0.72

//声明函数
VOID ShowDemoUI(HMODULE hModule);
INT_PTR CALLBACK DialogProc(_In_ HWND   hwndDlg, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
void SentTextMessage(HWND hwndDlg);
LPCWSTR String2LPCWSTR(string text);
string Dec2Hex(DWORD i);
WCHAR* CharToWChar(char* s);
VOID UnLoadMyself();
VOID ThreadSendMessage(HWND   hwndDlg);
void SendFileMessage(wchar_t* wxid, wchar_t* filepath);

//定义变量
DWORD wxBaseAddress = 0;

BOOL flag = true;

/**
 * 发送文件
 */
#define WxFileMessage1 0x574180				 //发送文件消息   1
#define WxFileMessage2 0x5741C0				 //发送文件消息   1
#define WxFileMessage3 0x66CB0				 //发送文件消息   1
#define WxFileMessage4 0x2B7600				 //发送文件消息   1
#define WxFileMessageParam 0x1550368		 //发送文件消息    1

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


	//DWORD funAdd = (DWORD)SentTextMessage;
	DWORD funAdd = (DWORD)SentTextMessage;
	string funAddtext = "funAdd：\t";
	funAddtext.append(Dec2Hex(funAdd));
	OutputDebugString(String2LPCWSTR(funAddtext));

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


			thread t1(ThreadSendMessage, hwndDlg);
			t1.detach();

		
			
		}
		else if(wParam == IDC_BTN_UNLOAD) {
			UnLoadMyself();
		}

		break;
	default:
		break;
	}
	return FALSE;
}


VOID ThreadSendMessage(HWND   hwndDlg) {

	int i = 0;

	//循环多少次
	DWORD num = 0;
	WCHAR numString[20] = { 0 };
	UINT uINT = GetDlgItemText(hwndDlg, IDC_TEXT_NUM, numString, 20);
	if (uINT == 0)
	{
		MessageBoxA(NULL, "请填写循环次数", "错误", MB_OK | MB_ICONERROR);
		return ;
	}

	int number = _ttoi(numString);

	//间隔多少毫秒
	WCHAR timeString[20] = { 0 };
	uINT = GetDlgItemText(hwndDlg, IDC_TEXT_TIME, timeString, 20);
	if (uINT == 0)
	{
		MessageBoxA(NULL, "请填写循环间隔时间", "错误", MB_OK | MB_ICONERROR);
		return ;
	}
	int timeSecond = _ttoi(timeString);



	string numberLog = "循环：";
	numberLog.append(Dec2Hex((DWORD)number));
	numberLog.append(" 次, 间隔（毫秒）");
	numberLog.append(Dec2Hex((DWORD)timeSecond));
	OutputDebugString(String2LPCWSTR(numberLog));

	for (i = 0; i < number; i++) {

		SentTextMessage(hwndDlg);
		//SendMessageFile2(hwndDlg);
		//SendMessageFile3(hwndDlg);
		
		Sleep(timeSecond);

		string threadLog = "threadLog：\t";
		threadLog.append(Dec2Hex((DWORD)i));
		OutputDebugString(String2LPCWSTR(threadLog));
	}
}


VOID SentTextMessage(HWND hwndDlg)
{
	//构造需要的数据
	//组装wxid数据
	wchar_t wxid[50] = { 0 };
	UINT uINT = GetDlgItemText(hwndDlg, IDC_TEXT_WXID, wxid, 50);
	if (uINT == 0)
	{
		MessageBoxA(NULL, "请填写wxid", "错误", MB_OK | MB_ICONERROR);
		return;
	}

	wchar_t filepath[1024] = { 0 };
	uINT = GetDlgItemText(hwndDlg, IDC_TEXT_PATH, filepath, 1024);
	if (uINT == 0)
	{
		MessageBoxA(NULL, "请填写文件路径", "错误", MB_OK | MB_ICONERROR);
		return;
	}

	//SendFileMessage(wxid, wxMsg);


	//构造需要的地址
	DWORD dwBase = wxBaseAddress;
	DWORD dwCall1 = dwBase + WxFileMessage1;
	DWORD dwCall2 = dwBase + WxFileMessage2;
	DWORD dwCall3 = dwBase + WxFileMessage3;	//组合数据
	DWORD dwCall4 = dwBase + WxFileMessage4;	//发送消息
	DWORD dwParams = dwBase + WxFileMessageParam;

	char buff[0x45C] = { 0 };

	//构造需要的数据
	GeneralStruct wxidStruct(wxid);
	GeneralStruct filePathStruct(filepath);

	//取出需要的数据的地址
	char* pFilePath = (char*)&filePathStruct.pstr;
	char* pWxid = (char*)&wxidStruct.pstr;

	__asm {
		pushad;
		sub esp, 0x14;
		lea eax, buff;
		mov ecx, esp;
		push eax;
		call dwCall2;

		push 0;
		sub esp, 0x14;
		mov ecx, esp;
		push - 0x1;
		push dwParams;
		call dwCall1;

		sub esp, 0x14;
		mov ecx, esp;
		mov ebx, pFilePath;
		push ebx;
		call dwCall2;

		sub esp, 0x14;
		mov eax, pWxid;
		mov ecx, esp;
		push eax;
		call dwCall2;

		lea eax, buff;
		push eax;
		call dwCall3;

		mov ecx, eax;
		call dwCall4;
		popad;
	}


	//OutputDebugString(String2LPCWSTR("end asm....."));
}


void SendFileMessage(wchar_t* wxid, wchar_t* filepath)
{
	if (GetFileAttributesW(filepath) == INVALID_FILE_ATTRIBUTES)
	{
		//__OutputDebugStringW(L"%s文件不存在", filepath);
		return;
	}

	//构造需要的地址
	DWORD dwBase = wxBaseAddress;
	DWORD dwCall1 = dwBase + WxFileMessage1;
	DWORD dwCall2 = dwBase + WxFileMessage2;
	DWORD dwCall3 = dwBase + WxFileMessage3;	//组合数据
	DWORD dwCall4 = dwBase + WxFileMessage4;	//发送消息
	DWORD dwParams = dwBase + WxFileMessageParam;

	char buff[0x45C] = { 0 };

	//构造需要的数据
	GeneralStruct wxidStruct(wxid);
	GeneralStruct filePathStruct(filepath);

	//取出需要的数据的地址
	char* pFilePath = (char*)&filePathStruct.pstr;
	char* pWxid = (char*)&wxidStruct.pstr;

	__asm {
		pushad;
		sub esp, 0x14;
		lea eax, buff;
		mov ecx, esp;
		push eax;
		call dwCall2;

		push 0;
		sub esp, 0x14;
		mov ecx, esp;
		push - 0x1;
		push dwParams;
		call dwCall1;

		sub esp, 0x14;
		mov ecx, esp;
		mov ebx, pFilePath;
		push ebx;
		call dwCall2;

		sub esp, 0x14;
		mov eax, pWxid;
		mov ecx, esp;
		push eax;
		call dwCall2;

		lea eax, buff;
		push eax;
		call dwCall3;

		mov ecx, eax;
		call dwCall4;
		popad;
	}
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
