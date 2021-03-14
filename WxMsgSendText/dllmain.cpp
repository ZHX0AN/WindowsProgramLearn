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

//3.1.0.72

//声明函数
VOID ShowDemoUI(HMODULE hModule);
INT_PTR CALLBACK DialogProc(_In_ HWND   hwndDlg, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
void SentTextMessage(HWND hwndDlg);
LPCWSTR String2LPCWSTR(string text);
string Dec2Hex(DWORD i);
WCHAR* CharToWChar(char* s);

VOID SentRoomMessageAt(HWND hwndDlg);
VOID UnLoadMyself();


////定义变量
DWORD wxBaseAddress = 0;
//DWORD g_callAddr = 0x3A0CA0;
//const int g_msgBuffer = 0x5A8;


/**
 * 发送文本消息
 */
#define SEND_MSG_HOOK_ADDRESS 0x3A0CA0	//HOOK消息的内存地址偏移
#define SEND_MSG_BUFFER 0x5A8	//HOOK消息的内存地址偏移



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



	DWORD funAddr = (DWORD)SentRoomMessageAt;

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
		if (wParam == BTN_SEND)
		{
			OutputDebugString(TEXT("发送消息按钮被点击"));
			SentTextMessage(hwndDlg);
		}

		if (wParam == BTN_SEND_AT)
		{
			OutputDebugString(TEXT("发送at消息 BTN_SEND_AT"));

			SentRoomMessageAt(hwndDlg);

			//SentAtTextMessage(hwndDlg);
		}
		else if (wParam == IDC_BTN_UNLOAD) {
			OutputDebugString(TEXT("卸载自己"));
			UnLoadMyself();
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
	//call WeChatWi.6D0FA7F0; 发送消息断点 ,6D0FA7F0 = wxBaseAddress + g_callAddr
	DWORD callFunAddr = wxBaseAddress + SEND_MSG_HOOK_ADDRESS;

	//组装wxid数据
	WCHAR wxid[50];
	UINT uINT = GetDlgItemText(hwndDlg, INPUT_WXID, wxid, 50);
	//if (uINT == 0)
	//{
	//	MessageBoxA(NULL, "请填写wxid", "错误", MB_OK | MB_ICONERROR);
	//	return;
	//}

	StructWxid structWxid = { 0 };
	structWxid.pWxid = wxid;
	structWxid.length = wcslen(wxid);
	structWxid.maxLength = wcslen(wxid) * 2;

	//取wxid的地址
	DWORD* asmWxid = (DWORD*)&structWxid.pWxid;

	//组装发送的文本数据
	WCHAR wxMsg[1024];
	uINT = GetDlgItemText(hwndDlg, INPUT_MSG, wxMsg, 1024);
	//if (uINT == 0)
	//{
	//	MessageBoxA(NULL, "请填写要发送的文本", "错误", MB_OK | MB_ICONERROR);
	//	return;
	//}

	StructWxid structMessage = { 0 };
	structMessage.pWxid = wxMsg;
	structMessage.length = wcslen(wxMsg);
	structMessage.maxLength = wcslen(wxMsg) * 2;
	DWORD* asmMsg = (DWORD*)&structMessage.pWxid;

	//定义一个缓冲区
	BYTE buff[SEND_MSG_BUFFER] = { 0 };

	//执行汇编调用
	__asm
	{
		push 0x1
		mov edi, 0x0
		push edi

		mov ebx, asmMsg
		push ebx

		mov edx, asmWxid
		lea ecx, buff

		call callFunAddr
		add esp, 0xC
	}

}

 class TEXT_WX
 {
 public:
	 wchar_t* pWxid = nullptr;
	 DWORD length = 0;
	 DWORD maxLength = 0;
	 DWORD fill1 = 0;
	 DWORD fill2 = 0;
	 wchar_t wxid[1024] = { 0 };

	 TEXT_WX(wstring wsWxid)
	 {
		 const wchar_t* temp = wsWxid.c_str();
		 wmemcpy(wxid, temp, wsWxid.length());
		 length = wsWxid.length();
		 maxLength = wsWxid.capacity();
		 fill1 = 0;
		 fill2 = 0;
		 pWxid = wxid;
	 }
 };

 class TEXT_WXID
 {
 public:
	 wchar_t* pWxid = nullptr;
	 DWORD length = 0;
	 DWORD maxLength = 0;
	 DWORD fill1 = 0;
	 DWORD fill2 = 0;
 };

 class ROOM_AT
 {
 public:
	 DWORD at_WxidList = 0;
	 DWORD at_end1 = 0;
	 DWORD at_end2 = 0;
 };
 
 VOID SentRoomMessageAt(HWND hwndDlg) {


	 HMODULE dllAdress = GetModuleHandleA("WeChatWin.dll");
	 DWORD callFunAddr = wxBaseAddress + SEND_MSG_HOOK_ADDRESS;

	 //122A8258  13A9A3F0  UNICODE "5847657683@chatroom"
	 //TEXT_WX wxId(L"5847657683@chatroom");


	 //122A8258  13A9A3F0  UNICODE "5847657683@chatroom"
	 TEXT_WX wxId(L"24377562166@chatroom");

	 //14139DEC  14139CF8  UNICODE "@马天佑 hahhaa"
	 TEXT_WX wxMsg(L"@张zhangjx 22222222");


	 //012CE028  13573FA0  UNICODE "wxid_k2d9oduqc9lc22"
	 WCHAR atIt[50] = L"wxid_4j4mqsuzdgie22";
	 TEXT_WXID wxAtId;
	 wxAtId.pWxid = atIt;
	 wxAtId.length = wcslen(atIt);
	 wxAtId.maxLength = wcslen(atIt) * 2;
	 wxAtId.fill1 = 0;
	 wxAtId.fill2 = 0;

	 ROOM_AT roomAt;
	 roomAt.at_WxidList = (DWORD)&wxAtId.pWxid;
	 roomAt.at_end1 = roomAt.at_WxidList + 5 * 4;
	 roomAt.at_end2 = roomAt.at_end1;

	 //定义一个缓冲区
	 BYTE buff[SEND_MSG_BUFFER] = { 0 };

	 WCHAR wxid2[50] = TEXT("1111111111111111111");

	 //执行汇编调用
	 __asm
	 {
		 //群号
		 //mov edx, asmWxid
		 lea edx, wxId

		 //传递参数
		 push 0x1

		 //mov eax, 0x0
		 lea eax, roomAt
		 push eax

		 //微信消息内容
		 //mov ebx, asmMsg
		 lea ebx, wxMsg

		 push ebx

		 lea ecx, buff

		 //调用函数
		 call callFunAddr

		 //平衡堆栈
		 add esp, 0xC

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

