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
#include <strstream>

#pragma comment(lib, "Version.lib")

//3.1.0.72

using namespace std;

//声明函数
VOID ShowDemoUI(HMODULE hModule);
VOID HookWx();
VOID UnHookWx();
VOID RecieveMsg(DWORD dEsp);
VOID RecieveMsgHook(DWORD dEsp);
LPCWSTR GetMsgByAddress(DWORD memAddress);
string Dec2Hex(DWORD i);
LPCWSTR String2LPCWSTR(string text);
wstring String2Wstring(string str);
VOID UnLoadMyself();

//定义变量
DWORD wxBaseAddress = 0;
//HOOK标志
BOOL isWxHooked = FALSE;
HWND hWinDlg;
DWORD g_hookAddr = 0;
//跳回地址
DWORD jumBackAddress = 0;
//我们要提取的寄存器内容
//DWORD r_esp = 0;

const string wxVersoin = "3.1.0.72";
//我自己的微信ID
string myWxId = "";

CHAR originalCode[5] = { 0 };

DWORD g_moveAddr = 0x66553C50;
DWORD g_hookOffsetAddr = 0x3CD5A5;
DWORD g_jumBackOffsetAddr = 0x3CD5AB;
DWORD g_jumBackAddr = 0;


//使用VS+Detours调试，必须一个没用的导出函数
VOID __declspec(dllexport) Test()
{
	OutputDebugString(TEXT("__declspec(dllexport) Test()\r\n"));
}

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
		//接收消息
		if (wParam == IDC_BTN_START)
		{
			HookWx();

			HWND hEditor = GetDlgItem(hwndDlg, IDC_TEXT_MSG);
			SetWindowText(hEditor, TEXT("开始接收微信消息......\r\n"));
			break;
		}

		//停止接收
		if (wParam == IDC_BTN_STOP)
		{
			UnHookWx();
			//HWND hFileHelper = GetDlgItem(hwndDlg, IDC_MSG);
			//SetWindowText(hFileHelper, TEXT("停止准备接收微信消息......"));
			break;
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


//Hook接收消息
VOID HookWx()
{

	isWxHooked = TRUE;

	//WeChatWin.dll+354AA3 
	int hookAddress = wxBaseAddress + g_hookOffsetAddr;

	g_jumBackAddr = wxBaseAddress + g_jumBackOffsetAddr;


	//组装跳转数据
	BYTE jmpCode[5] = { 0 };
	jmpCode[0] = 0xE9;

	//新跳转指令中的数据=跳转的地址-原地址（HOOK的地址）-跳转指令的长度
	*(DWORD*)&jmpCode[1] = (DWORD)RecieveMsgHook - hookAddress - 5;

	//保存当前位置的指令,在unhook的时候使用。
	ReadProcessMemory(GetCurrentProcess(), (LPVOID)hookAddress, originalCode, 5, 0);

	//覆盖指令 B9 E8CF895C //mov ecx,0x5C89CFE8
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)hookAddress, jmpCode, 5, 0);
}

//UnHook接收消息
VOID UnHookWx()
{
	if (isWxHooked == TRUE)
	{
		//恢复指令

		//恢复指令的地址
		int hookAddress = wxBaseAddress + g_hookOffsetAddr;

		//恢复指令
		WriteProcessMemory(GetCurrentProcess(), (LPVOID)hookAddress, originalCode, 5, 0);

		isWxHooked = FALSE;
	}
}

//跳转到这里，让我们自己处理消息
__declspec(naked) VOID RecieveMsgHook(DWORD dEsp)
{

	__asm
	{
		//执行wx程序代码
		mov ecx, g_moveAddr
		push edi

		//提取esp寄存器内容，放在一个变量中
		mov dEsp, esp

		//保存寄存器
		pushad
		pushf
	}

	//调用接收消息的函数
	RecieveMsg(dEsp);

	
	//恢复现场
	__asm
	{
		popf
		popad

		//跳回 66A0D50B    FF50 08         call dword ptr ds:[eax+0x8]
		jmp g_jumBackAddr
	}
}

VOID RecieveMsg(DWORD dEsp)
{


	//消息类型
	DWORD msgTypeOffset = 0x30;
	//发送人，好友或群ID
	DWORD friendOffset = 0x40;
	//群消息，消息发送者
	DWORD roomMsgSenderOffset = 0x164;
	//消息
	DWORD msgOffset = 0x68;

	//好友消息：<msgsource />
	//群消息: 01023B78  <msgsource>..<silence>0</silence>..<membercount>2</membercount> < / msgsource>
	//群at消息: <msgsource>..<atuserlist>wxid_4sy2barbyny712</atuserlist>..<silence>0</silence>..<membercount>2</membercount>.</msgsource>
	DWORD atFriendOffst = 0x1b8;

	//610069e565280afcd67ec1edfef1c61d
	DWORD unknown1 = 0x178;

	//文件存储路径：wxid_4sy2barbyny712\FileStorage\File\2021-03\工作簿1(1).xlsx
	DWORD filePathOffset = 0x1A0;

	wstring receivedMessage = TEXT("");
	BOOL isFriendMsg = FALSE;
	
	//[[esp]]
	DWORD** msgAddress = (DWORD**)dEsp;

	//消息类型[[esp]]+0x30
	//[01文字] [03图片] [31转账XML信息] [22语音消息] [02B视频信息]
	DWORD msgType = *((DWORD*)(**msgAddress + msgTypeOffset));
	receivedMessage.append(TEXT("消息类型:"));
	switch (msgType)
	{
	case 0x01:
		receivedMessage.append(TEXT("text"));
		break;

	case 0x03:
		receivedMessage.append(TEXT("pic"));
		break;

		//case 0x22:
		//	receivedMessage.append(TEXT("voice"));
		//	break;
		//case 0x25:
		//	receivedMessage.append(TEXT("好友确认"));
		//	break;
		//case 0x28:
		//	receivedMessage.append(TEXT("POSSIBLEFRIEND_MSG"));
		//	break;
		//case 0x2A:
		//	receivedMessage.append(TEXT("名片"));
		//	break;
		//case 0x2B:
		//	receivedMessage.append(TEXT("视频"));
		//	break;
		//case 0x2F:
		//	//石头剪刀布
		//	receivedMessage.append(TEXT("表情"));
		//	break;
		//case 0x30:
		//	receivedMessage.append(TEXT("位置"));
		//	break;
		case 0x31:
			//共享实时位置
			//文件
			//转账
			//链接

			receivedMessage.append(TEXT("共享实时位置、文件、转账、链接"));
			break;
		//case 0x32:
		//	receivedMessage.append(TEXT("VOIPMSG"));
		//	break;
		//case 0x33:
		//	//receivedMessage.append(TEXT("微信初始化"));
		//	break;
		//case 0x34:
		//	receivedMessage.append(TEXT("VOIPNOTIFY"));
		//	break;
		//case 0x35:
		//	receivedMessage.append(TEXT("VOIPINVITE"));
		//	break;
		//case 0x3E:
		//	//receivedMessage.append(TEXT("小视频"));
		//	break;
		//case 0x270F:
		//	receivedMessage.append(TEXT("SYSNOTICE"));
		//	break;
		//case 0x2710:
		//	//系统消息
		//	//红包
		//	//receivedMessage.append(TEXT("红包、系统消息"));
		//	break;
		//case 0x2712:
		//	//receivedMessage.append(TEXT("撤回消息"));
		//	break;
	default:
		wostringstream oss;
		oss.fill('0');
		oss << setiosflags(ios::uppercase) << setw(8) << hex << msgType;
		receivedMessage.append(TEXT("unknow:0x"));
		receivedMessage.append(oss.str());
		break;
	}
	receivedMessage.append(TEXT("\r\n"));



	//判断是群消息还是好友消息
	//相关信息
	wstring msgSource2 = TEXT("<msgsource />\n");
	wstring msgSource = TEXT("");
	msgSource.append(GetMsgByAddress(**msgAddress + atFriendOffst));

	if (msgSource.length() <= msgSource2.length())
	{
		//receivedMessage.append(TEXT("receive from friend:\r\n"));
		isFriendMsg = TRUE;
	}
	else
	{
		//receivedMessage.append(TEXT("receive from chatroom:\r\n"));
		isFriendMsg = FALSE;
	}


	//好友消息
	if (isFriendMsg == TRUE)
	{
		receivedMessage.append(TEXT("发送人ID：\r\n"))
			.append(GetMsgByAddress(**msgAddress + friendOffset))
			.append(TEXT("\r\n\r\n"));
	}
	else
	{
		receivedMessage.append(TEXT("群ID：\r\n"))
			.append(GetMsgByAddress(**msgAddress + friendOffset))
			.append(TEXT("\r\n\r\n"));

		receivedMessage.append(TEXT("发送人ID: \r\n"))
			.append(GetMsgByAddress(**msgAddress + roomMsgSenderOffset))
			.append(TEXT("\r\n\r\n"));

		receivedMessage.append(TEXT("at消息: \r\n"));
		receivedMessage += msgSource;
		receivedMessage.append(TEXT("\r\n\r\n"));
	}

	receivedMessage.append(TEXT("消息内容: \r\n"))
		.append(GetMsgByAddress(**msgAddress + msgOffset))
		.append(TEXT("\r\n\r\n"));


	receivedMessage.append(TEXT("文件存储路径: \r\n"))
		.append(GetMsgByAddress(**msgAddress + filePathOffset))
		.append(TEXT("\r\n\r\n"));

	receivedMessage.append(TEXT("unkunow:\r\n"))
		.append(GetMsgByAddress(**msgAddress + unknown1))
		.append(TEXT("\r\n\r\n"));


	//文本框输出信息
	SetWindowText(GetDlgItem(hWinDlg, IDC_TEXT_MSG), receivedMessage.c_str());
}


//读取内存中的字符串
//存储格式
//xxxxxxxx:字符串地址（memAddress）
//xxxxxxxx:字符串长度（memAddress +4）
LPCWSTR GetMsgByAddress(DWORD memAddress)
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

//将string转换成wstring  
wstring String2Wstring(string str)
{
	wstring result;
	//获取缓冲区大小，并申请空间，缓冲区大小按字符计算  
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
	TCHAR* buffer = new TCHAR[len + 1];
	//多字节编码转换成宽字节编码  
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, len);
	buffer[len] = '\0';             //添加字符串结尾  
	//删除缓冲区并返回值  
	result.append(buffer);
	delete[] buffer;
	return result;
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


//将int转成16进制字符串
string Dec2Hex(DWORD i)
{
	//定义字符串流
	stringstream ioss;

	//存放转化后字符
	string s_temp;

	//以8位十六制(大写)形式输出
	ioss.fill('0');
	ioss << setiosflags(ios::uppercase) << setw(8) << hex << i;
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

		//从内存中卸载
		FreeLibraryAndExitThread(hModule, 0);
	}
}