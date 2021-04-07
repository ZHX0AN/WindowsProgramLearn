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

//3.1.0.72


//声明函数
VOID ShowDemoUI(HMODULE hModule);
INT_PTR CALLBACK DialogProc(_In_ HWND   hwndDlg, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
void SentTextMessage(HWND hwndDlg);
LPCWSTR String2LPCWSTR(string text);
string Dec2Hex(DWORD i);
WCHAR* CharToWChar(char* s);
wchar_t* AnsiToUnicode(const char* szStr);
VOID AddText(HWND hwnd, PCTSTR pszFormat, ...);
wchar_t* GetMsgByAddress2(DWORD memAddress);

VOID SentRoomMessageAt(HWND hwndDlg);
VOID UnLoadMyself();
BOOL JudgeSendStatus();

//接收发送的消息
BOOL JudgeSendStatus();
void InlinkHookJudgeSendStatus();
void JudgeSendStatusSendMsg(DWORD r_esi);

VOID getUserWxId();

////定义变量
DWORD wxBaseAddress = 0;
//DWORD g_callAddr = 0x3A0CA0;
//const int g_msgBuffer = 0x5A8;

HWND g_hwndDlg;

/**
 * 发送文本消息
 */
#define SEND_MSG_HOOK_ADDRESS 0x3A0CA0	
#define SEND_MSG_BUFFER 0x5A8	

#define SEND_MSG_SEND_STATUS_ADDRESS 0x37BD12
DWORD g_jumpBackAddress = 0;

#define USERINFO_WXID 0x18A3584	
char g_myWxId[30] = { 0 };


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



	DWORD funAddr = (DWORD)JudgeSendStatusSendMsg;

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
		g_hwndDlg = hwndDlg;

		getUserWxId();

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
		}else if(wParam == BTN_SEND_AT)
		{
			OutputDebugString(TEXT("发送at消息 BTN_SEND_AT"));

			SentRoomMessageAt(hwndDlg);

			//SentAtTextMessage(hwndDlg);
		}
		else if (wParam == IDC_BTN_UNLOAD) {
			OutputDebugString(TEXT("卸载自己"));
			UnLoadMyself();
		}
		else if (wParam == IDC_BTN_JUDGE) {

			JudgeSendStatus();
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

//获取当前用户wxid
VOID getUserWxId() {

	DWORD base = (DWORD)GetModuleHandle(TEXT("WeChatWin.dll"));

	//一级指针
	DWORD pWxidAddr = base + USERINFO_WXID;

	memcpy_s(g_myWxId, 30, (char*)(*(DWORD*)pWxidAddr), 30);

	wchar_t* wWxid = AnsiToUnicode(g_myWxId);

	AddText(GetDlgItem(g_hwndDlg, INPUT_MSG), TEXT("登录用户WxID: %s \r\n"), wWxid);
}



BOOL JudgeSendStatus() {


	DWORD hookAddress = wxBaseAddress + SEND_MSG_SEND_STATUS_ADDRESS;
	g_jumpBackAddress = hookAddress + 6;


	//组装跳转数据
	BYTE JmpCode[6] = { 0 };
	JmpCode[0] = 0xE9;
	JmpCode[6 - 1] = 0x90;

	//新跳转指令中的数据=跳转的地址-原地址（HOOK的地址）-跳转指令的长度
	*(DWORD*)&JmpCode[1] = (DWORD)InlinkHookJudgeSendStatus - hookAddress - 5;

	WriteProcessMemory(GetCurrentProcess(), (LPVOID)hookAddress, JmpCode, 6, 0);

	return true;
}

//InlineHook完成后，程序在Hook点跳转到这里执行。
//这里必须是裸函数
__declspec(naked) void InlinkHookJudgeSendStatus()
{
	//补充代码
	__asm
	{
		//补充被覆盖的代码
		mov ecx, dword ptr ss : [ebp - 0x24]
		add esp, 0x34

		//保存寄存器
		pushad


		//调用我们的处理函数
		push esi
		call JudgeSendStatusSendMsg
		add esp, 4

		//恢复寄存器
		popad

		//跳回去接着执行
		jmp g_jumpBackAddress
	}
}

void JudgeSendStatusSendMsg(DWORD r_esi)
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

	//610069e565280afcd67ec1edfef1c61d
	DWORD unknown1 = 0x178;

	//文件存储路径：wxid_4sy2barbyny712\FileStorage\File\2021-03\工作簿1(1).xlsx
	DWORD filePathOffset = 0x1A0;


	INT64 Svrid = *((INT64*)(r_esi + SvridOffset));
	DWORD msgType = *((DWORD*)(r_esi + msgTypeOffset));
	DWORD msgSubType = *((DWORD*)(r_esi + subTypeOffset));
	DWORD timestamp = *((DWORD*)(r_esi + timestampOffset));
	wchar_t* wToWxId = GetMsgByAddress2(r_esi + friendOffset);
	wchar_t* wContent = GetMsgByAddress2(r_esi + contentOffset);
	wchar_t* wAt = GetMsgByAddress2(r_esi + atFriendOffst);
	wchar_t* wFilePath = GetMsgByAddress2(r_esi + filePathOffset);
	wchar_t* wRoomSender = GetMsgByAddress2(r_esi + roomMsgSenderOffset);

	
	AddText(GetDlgItem(g_hwndDlg, INPUT_MSG), TEXT("Svrid: %I64d \r\n"), Svrid);
	AddText(GetDlgItem(g_hwndDlg, INPUT_MSG), TEXT("时间戳: %I32d \r\n"), timestamp);
	AddText(GetDlgItem(g_hwndDlg, INPUT_MSG), TEXT("信息类型: %d \r\n"), msgType);
	AddText(GetDlgItem(g_hwndDlg, INPUT_MSG), TEXT("信息子类型: %d \r\n"), msgSubType);
	AddText(GetDlgItem(g_hwndDlg, INPUT_MSG), TEXT("接收者: %s \r\n"), wToWxId);
	AddText(GetDlgItem(g_hwndDlg, INPUT_MSG), TEXT("消息内容: %s \r\n"), wContent);
	AddText(GetDlgItem(g_hwndDlg, INPUT_MSG), TEXT("AT好友: %s \r\n"), wAt);
	AddText(GetDlgItem(g_hwndDlg, INPUT_MSG), TEXT("文件路径: %s \r\n"), wFilePath);
	AddText(GetDlgItem(g_hwndDlg, INPUT_MSG), TEXT("群消息发送者: %s \r\n"), wRoomSender);

}



 VOID SentTextMessage(HWND hwndDlg)
{
	//call WeChatWi.6D0FA7F0; 发送消息断点 ,6D0FA7F0 = wxBaseAddress + g_callAddr
	DWORD callFunAddr = wxBaseAddress + SEND_MSG_HOOK_ADDRESS;

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


 VOID SentRoomMessageAt(HWND hwndDlg) {


	 HMODULE dllAdress = GetModuleHandleA("WeChatWin.dll");
	 DWORD callFunAddr = wxBaseAddress + SEND_MSG_HOOK_ADDRESS;


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

	 //组装wxid数据
	 WCHAR wxidDlg[50];
	 UINT uINT = GetDlgItemText(hwndDlg, INPUT_WXID, wxidDlg, 50);
	 if (uINT == 0)
	 {
		 MessageBoxA(NULL, "请填写wxid", "错误", MB_OK | MB_ICONERROR);
		 return;
	 }
	 TEXT_WX wxId(wxidDlg);




	 //组装发送的文本数据
	 WCHAR wxMsgDlg[1024];
	 uINT = GetDlgItemText(hwndDlg, INPUT_MSG, wxMsgDlg, 1024);
	 if (uINT == 0)
	 {
		 MessageBoxA(NULL, "请填写发送的文本数据", "错误", MB_OK | MB_ICONERROR);
		 return;
	 }
	 TEXT_WX wxMsg(wxMsgDlg);

	 
	 WCHAR wxAtDlg[50];
	 uINT = GetDlgItemText(hwndDlg, INPUT_AT, wxAtDlg, 50);
	 if (uINT == 0)
	 {
		 MessageBoxA(NULL, "请填写at Wxid", "错误", MB_OK | MB_ICONERROR);
		 return;
	 }

	 TEXT_WXID wxAtId;
	 wxAtId.pWxid = wxAtDlg;
	 wxAtId.length = wcslen(wxAtDlg);
	 wxAtId.maxLength = wcslen(wxAtDlg) * 2;
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

		 push 0x1

		 lea edi, roomAt
		 push edi


		 lea ebx, wxMsg
		 push ebx

		 lea ecx, buff

		 call callFunAddr
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
