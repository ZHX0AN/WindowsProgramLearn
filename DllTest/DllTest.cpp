// DllTest.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "DllTest.h"
#include "resource.h"
#include <string.h>
#include <TlHelp32.h>
#include <windowsx.h>
#include <StrSafe.h> 
#include <shlwapi.h>
#include<thread>

#pragma comment (lib,"shlwapi.lib")
#pragma comment (lib,"shell32.lib")

using namespace std;

void RunServer();
VOID AddText(HWND hwnd, PCTSTR pszFormat, ...);

typedef VOID(CALLBACK* ACCEPT)(DWORD);
typedef VOID(CALLBACK* RECEIVE)(DWORD, LPSTR, DWORD);
typedef VOID(CALLBACK* CLOSE)(DWORD);


// DLL导出函数
typedef DWORD(CALLBACK* InjectFn)(LPCSTR);
typedef BOOL(CALLBACK* InitFn)(ACCEPT, RECEIVE, CLOSE);
typedef BOOL(CALLBACK* VersionFn)(LPSTR);
typedef BOOL(CALLBACK* DestroyFn)();
typedef BOOL(CALLBACK* SendFn)(DWORD, LPCSTR);

HINSTANCE hModule;


VOID CALLBACK receive(DWORD clientId, LPSTR data, DWORD len)
{
	printf("new message: %s ", data);
}

VOID CALLBACK accept(DWORD clientId)
{
	printf("new client! ");
}

VOID CALLBACK close(DWORD clientId)
{
	printf("client closed! ");
}


INT_PTR CALLBACK DialogProc(_In_ HWND   hwndDlg, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	// MAKEINTRESOURCE：强制转换IDD_MAIN
	DialogBox(NULL, MAKEINTRESOURCE(IDD_MAIN), NULL, &DialogProc);
	return 0;
}


INT_PTR CALLBACK DialogProc(_In_ HWND   hwndDlg, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{


	}
		break;
	case WM_COMMAND:
	{

		if (wParam == IDC_BTN_CONTACT)
		{




		}
		else if (wParam == IDC_SEND_MSG) {

		}
		else if (wParam == IDC_BTN_SEND_FILE) {

			thread t1(RunServer);
			t1.detach();



		}
	}
		break;
	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;
	default:
		break;
	}
	return FALSE;
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


void RunServer() {


	hModule = LoadLibrary(TEXT("WxLoader.dll"));

	InitFn InitWeChatSocket = (InitFn)GetProcAddress(hModule, "InitWeChatSocket");
	VersionFn GetUserWeChatVersion = (VersionFn)GetProcAddress(hModule, "GetUserWeChatVersion");
	InjectFn InjectWechat = (InjectFn)GetProcAddress(hModule, "InjectWeChat");
	DestroyFn DestroyWeChat = (DestroyFn)GetProcAddress(hModule, "DestroyWeChat");
	SendFn SendWeChat = (SendFn)GetProcAddress(hModule, "SendWeChatData");

	DWORD in = 0;
	BOOL rs = FALSE;
	char szVersion[30] = { 0 };
	GetUserWeChatVersion(szVersion);

	rs = InitWeChatSocket(accept, receive, close);
	in = InjectWechat("WeChatHelper_3.1.0.41_full.dll");


	DWORD dwClientId = 1;
	//LPCSTR str = "{\"data\":{},\"type\":11028}";

	LPCSTR str = "{\"data\": {\"to_wxid\": \"wxid_4sy2barbyny712\",\"content\": \"你好，世界\" },\"type\": 11036}";

	//LPCSTR str = "{\"data\":{\"to_wxid\": \"24377562166@chatroom\",\"file\": \"C:\\temp\\工作簿1.xlsx\" },\"type\":11041}";
	SendWeChat(dwClientId, str);

	DestroyWeChat();

}