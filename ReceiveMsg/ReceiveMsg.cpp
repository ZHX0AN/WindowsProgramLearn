// ReceiveMsg.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "ReceiveMsg.h"
#include "shellapi.h"
#include <string>
#include <tchar.h> 
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <strstream>


#pragma comment(lib, "Version.lib")



using namespace std;

//声明函数
VOID ShowDemoUI(HMODULE hModule);
INT_PTR CALLBACK DialogProc(_In_ HWND   hwndDlg, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
VOID HookWx();
VOID UnHookWx();
VOID RecieveMsg();
VOID RecieveMsgHook();
string Dec2Hex(DWORD i);
LPCWSTR String2LPCWSTR(string text);
LPCWSTR GetMsgByAddress(DWORD memAddress);
BOOL IsWxVersionValid();
wstring String2Wstring(string str);

//定义变量
DWORD wxBaseAddress = 0;
//HOOK标志
BOOL isWxHooked = FALSE;
//对话框句柄
HWND hWinDlg;
//跳回地址
DWORD jumBackAddress = 0;
//我们要提取的寄存器内容
DWORD r_esp = 0;
//此HOOK匹配的微信版本
const string wxVersoin = "2.9.5.41";
//我自己的微信ID
string myWxId = "";

CHAR originalCode[5] = { 0 };


//使用VS+Detours调试，必须一个没用的导出函数
VOID __declspec(dllexport) Test()
{
	//OutputDebugString(TEXT("开始调试"));
}



BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		OutputDebugString(TEXT("DLL_PROCESS_ATTACH"));
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
	string text = "wechat baseaddress:\t";
	text.append(Dec2Hex(wxBaseAddress));
	OutputDebugString(String2LPCWSTR(text));

	DialogBox(hModule, MAKEINTRESOURCE(IDD_MAIN), NULL, &DialogProc);
}
//把string 转换为 LPCWSTR
LPCWSTR String2LPCWSTR(string text)
{
	//原型：
	//typedef _Null_terminated_ CONST WCHAR *LPCWSTR, *PCWSTR;
	//typedef wchar_t WCHAR;

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
			OutputDebugString(TEXT("ID_START"));
			HookWx();

			HWND hEditor = GetDlgItem(hwndDlg, IDC_MSG);
			//SetWindowText(hEditor, TEXT("开始准备接收微信消息......\r\n"));
			break;
		}

		//停止接收
		if (wParam == IDC_BTN_STOP)
		{
			//OutputDebugString(TEXT("停止接收按钮被点击"));
			//UnHookWx();

			//HWND hFileHelper = GetDlgItem(hwndDlg, IDC_MSG);
			//SetWindowText(hFileHelper, TEXT("停止准备接收微信消息......"));
			break;
		}


		break;
	default:
		break;
	}
	return FALSE;
}

//Hook接收消息
VOID HookWx()
{
	////判断是否已经HOOK
	//if (isWxHooked == FALSE)
	//{
	isWxHooked = TRUE;

	//0x178C870计算公式
	//68DD7DFF    B9 70C81C6A     mov ecx,WeChatWi.6A1CC870
	//68DD7DFF = WechatDll.dll + 397DFF
	int hookAddress = wxBaseAddress + 0x397DFF;

	//string debugMsg = "Hook address:";
	//debugMsg.append(Dec2Hex(hookAddress));
	//OutputDebugString(String2LPCWSTR(debugMsg));

	//OutputDebugString(TEXT("hookAddress:"));
	//OutputDebugString((LPCWSTR)hookAddress);


	//跳回的地址
	jumBackAddress = hookAddress + 5;

	//组装跳转数据
	BYTE jmpCode[5] = { 0 };
	jmpCode[0] = 0xE9;

	//新跳转指令中的数据=跳转的地址-原地址（HOOK的地址）-跳转指令的长度
	*(DWORD*)&jmpCode[1] = (DWORD)RecieveMsgHook - hookAddress - 5;

	////保存当前位置的指令,在unhook的时候使用。
	//ReadProcessMemory(GetCurrentProcess(), (LPVOID)hookAddress, originalCode, 5, 0);

	//覆盖指令 B9 E8CF895C //mov ecx,0x5C89CFE8
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)hookAddress, jmpCode, 5, 0);

	OutputDebugString(TEXT("HookWx end"));
	//}
}

//UnHook接收消息
VOID UnHookWx()
{
	if (isWxHooked == TRUE)
	{
		//恢复指令
		//B9 E8CF895C
		//mov ecx,0x5C89CFE8
		//##################################################################################
		// 2019-06-20 修订
		// 感谢群里的Lost朋友（1580500224）发现，这个地址中的数据不是固定的。
		// 因此，在HOOK的时候，需要保存这个地址中的数据，然后在unhook的时候恢复。
		//##################################################################################
		//BYTE originalCode[5] = { 0xB9,0xE8,0xCF,0x89,0x5C };

		//恢复指令的地址
		int hookAddress = wxBaseAddress + 0x354AA3;

		//恢复指令
		WriteProcessMemory(GetCurrentProcess(), (LPVOID)hookAddress, originalCode, 5, 0);

		isWxHooked = FALSE;
	}
}

//跳转到这里，让我们自己处理消息
__declspec(naked) VOID RecieveMsgHook()
{

	__asm
	{

		//68DD7DFF    B9 70C81C6A     mov ecx,WeChatWi.6A1CC870
		mov ecx, 0x6A1CC870

		//提取esp寄存器内容，放在一个变量中
		mov r_esp, esp

		//保存寄存器
		pushad
		pushf
	}

	//调用接收消息的函数
	RecieveMsg();

	//恢复现场
	__asm
	{
		popf
		popad

		//跳回被HOOK指令的下一条指令
		jmp jumBackAddress
	}
}

VOID RecieveMsg()
{

	wstring receivedMessage = TEXT("");
	BOOL isFriendMsg = FALSE;
	//[[esp]]
	//信息块位置
	DWORD** msgAddress = (DWORD**)r_esp;

	//消息类型[[esp]]+0x30
	//[01文字] [03图片] [31转账XML信息] [22语音消息] [02B视频信息]
	DWORD msgType = *((DWORD*)(**msgAddress + 0x30));
	receivedMessage.append(TEXT("msg type:"));
	switch (msgType)
	{
	case 0x01:
		receivedMessage.append(TEXT("text"));
		break;
		//case 0x03:
		//	receivedMessage.append(TEXT("pic"));
		//	break;

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
		//case 0x31:
		//	//共享实时位置
		//	//文件
		//	//转账
		//	//链接
		//	//receivedMessage.append(TEXT("共享实时位置、文件、转账、链接"));
		//	break;
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
	msgSource.append(GetMsgByAddress(**msgAddress + 0x198));

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
		receivedMessage.append(TEXT("friend wxid：\r\n"))
			.append(GetMsgByAddress(**msgAddress + 0x40))
			.append(TEXT("\r\n\r\n"));
	}
	else
	{
		receivedMessage.append(TEXT("chatroom number：\r\n"))
			.append(GetMsgByAddress(**msgAddress + 0x40))
			.append(TEXT("\r\n\r\n"));

		receivedMessage.append(TEXT("msg sender: \r\n"))
			.append(GetMsgByAddress(**msgAddress + 0x144))
			.append(TEXT("\r\n\r\n"));

		receivedMessage.append(TEXT("info: \r\n"));
		receivedMessage += msgSource;
		receivedMessage.append(TEXT("\r\n\r\n"));
	}



	receivedMessage.append(TEXT("content: \r\n"))
		.append(GetMsgByAddress(**msgAddress + 0x68))
		.append(TEXT("\r\n\r\n"));


	receivedMessage.append(TEXT("unkunow msg:\r\n"))
		.append(GetMsgByAddress(**msgAddress + 0x198))
		.append(TEXT("\r\n\r\n"));


	//文本框输出信息
	SetWindowText(GetDlgItem(hWinDlg, IDC_MSG), receivedMessage.c_str());
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

//检查微信版本是否匹配
BOOL IsWxVersionValid()
{
	WCHAR VersionFilePath[MAX_PATH];
	if (GetModuleFileName((HMODULE)wxBaseAddress, VersionFilePath, MAX_PATH) == 0)
	{
		return FALSE;
	}

	string asVer = "";
	VS_FIXEDFILEINFO* pVsInfo;
	unsigned int iFileInfoSize = sizeof(VS_FIXEDFILEINFO);
	int iVerInfoSize = GetFileVersionInfoSize(VersionFilePath, NULL);
	if (iVerInfoSize != 0) {
		char* pBuf = new char[iVerInfoSize];
		if (GetFileVersionInfo(VersionFilePath, 0, iVerInfoSize, pBuf)) {
			if (VerQueryValue(pBuf, TEXT("\\"), (void**)&pVsInfo, &iFileInfoSize)) {
				//主版本2.6.7.57
				//2
				int s_major_ver = (pVsInfo->dwFileVersionMS >> 16) & 0x0000FFFF;
				//6
				int s_minor_ver = pVsInfo->dwFileVersionMS & 0x0000FFFF;
				//7
				int s_build_num = (pVsInfo->dwFileVersionLS >> 16) & 0x0000FFFF;
				//57
				int s_revision_num = pVsInfo->dwFileVersionLS & 0x0000FFFF;

				//把版本变成字符串
				strstream wxVer;
				wxVer << s_major_ver << "." << s_minor_ver << "." << s_build_num << "." << s_revision_num;
				wxVer >> asVer;
			}
		}
		delete[] pBuf;
	}

	//版本匹配
	if (asVer == wxVersoin)
	{
		return TRUE;
	}

	//版本不匹配
	return FALSE;
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
