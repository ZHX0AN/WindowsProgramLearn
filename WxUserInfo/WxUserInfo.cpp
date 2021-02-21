// WxUserInfo.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "WxUserInfo.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <windowsx.h>
#include <StrSafe.h> 


HWND g_hwndDialogDetail;


VOID AddText(HWND hwnd, PCTSTR pszFormat, ...);
VOID GetUserInfo(HWND hwnd);
BOOL ListProcessModules(HWND hwnd, DWORD dwPID);
INT_PTR CALLBACK DialogProc(_In_ HWND   hwndDlg, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

wchar_t* UTF8ToUnicode(const char* str);

const TCHAR weChatName[20] = TEXT("WeChat.exe");

HANDLE hProcess;
DWORD weChatBaseAdress = 0;

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
		break;
	case WM_COMMAND:


		if (wParam == IDOK)
		{
			GetUserInfo(hwndDlg);
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

VOID GetUserInfo(HWND hwnd) {

	DWORD weChatProcessID = 0;
	//1)	遍历系统中的进程，找到微信进程（CreateToolhelp32Snapshot、Process32Next）
	HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	PROCESSENTRY32 processentry32 = { 0 };
	processentry32.dwSize = sizeof(PROCESSENTRY32);

	BOOL next = Process32Next(handle, &processentry32);
	while (next == TRUE)
	{
		if (wcscmp(processentry32.szExeFile, L"WeChat.exe") == 0)
		{
			weChatProcessID = processentry32.th32ProcessID;
			break;
		}
		next = Process32Next(handle, &processentry32);
	}
	if (weChatProcessID == 0)
	{
		AddText(GetDlgItem(hwnd, IDC_TEXT_INFO), TEXT("未找到WeChat\r\n"));
		return ;
	}

	AddText(GetDlgItem(hwnd, IDC_TEXT_INFO), TEXT("WeChatProcessID: 0x%08X\r\n"), weChatProcessID);
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, weChatProcessID);
	if (hProcess == NULL)
	{
		AddText(GetDlgItem(hwnd, IDC_TEXT_INFO), TEXT("打开WeChat失败\r\n"));
		return ;
	}

	//获取UserInfo

	ListProcessModules(hwnd, weChatProcessID);



}

BOOL ListProcessModules(HWND hwnd, DWORD dwPID) {

	HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
	MODULEENTRY32 me32;

	//给进程所引用的模块信息设定一个快照 
	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
	if (hModuleSnap == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	me32.dwSize = sizeof(MODULEENTRY32);

	if (!Module32First(hModuleSnap, &me32))
	{
		CloseHandle(hModuleSnap);
		return FALSE;
	}

	do
	{
		if (_tcscmp(TEXT("WeChatWin.dll"), me32.szModule) == 0) {
			//_tprintf(TEXT("MODULE NAME: %s \n"), me32.szModule);
			//_tprintf(TEXT("File Path = %s \n"), me32.szExePath);
			//_tprintf(TEXT("Base Address = 0x%08X \n"), (DWORD)me32.modBaseAddr);
			weChatBaseAdress = (DWORD)me32.modBaseAddr;

			AddText(GetDlgItem(hwnd, IDC_TEXT_INFO), TEXT("Module Name: %S\r\n"), me32.szModule);
			AddText(GetDlgItem(hwnd, IDC_TEXT_INFO), TEXT("File Path: %S\r\n"), me32.szExePath);
			AddText(GetDlgItem(hwnd, IDC_TEXT_INFO), TEXT("Base Address: 0x%08X\r\n"), weChatBaseAdress);


			break;
		}

	} while (Module32Next(hModuleSnap, &me32));

	DWORD nameAddr = 0x18A259C;
	DWORD phoneNumberAddr = 0x18A25D0;

	DWORD phoneTypeAddr = 0x18A29F0;


	BYTE phoneNumber[20] = { 0 };
	ReadProcessMemory(hProcess, (LPVOID)(weChatBaseAdress + phoneNumberAddr), phoneNumber,  11, 0);
	TCHAR dBuf[30] = {0};
	int nRet = MultiByteToWideChar(CP_ACP, 0, (char*)phoneNumber, 11, dBuf, 30);
	AddText(GetDlgItem(hwnd, IDC_TEXT_INFO), TEXT("手机号: %s\r\n"), dBuf);


	char name[100] = { 0 };
	ReadProcessMemory(hProcess, (LPVOID)(weChatBaseAdress + nameAddr), name, 100, 0);
	TCHAR* content = UTF8ToUnicode(name);
	AddText(GetDlgItem(hwnd, IDC_TEXT_INFO), TEXT("微信名字: %s\r\n"), content);


	BYTE phoneType[50] = { 0 };
	ReadProcessMemory(hProcess, (LPVOID)(weChatBaseAdress + phoneTypeAddr), phoneType, 50, 0);
	TCHAR tempBuf[100] = { 0 };
	nRet = MultiByteToWideChar(CP_ACP, 0, (char*)phoneType, 50, tempBuf, 100);
	AddText(GetDlgItem(hwnd, IDC_TEXT_INFO), TEXT("手机类型: %s\r\n"), tempBuf);



	//AddText(GetDlgItem(hwnd, IDC_TEXT_INFO), TEXT("微信名字: %s\r\n"), weChatBaseAdress + nameAddr);
	//AddText(GetDlgItem(hwnd, IDC_TEXT_INFO), TEXT("手机类型: %s\r\n"), weChatBaseAdress + phoneTypeAddr);

	HWND hwndEdit = GetDlgItem(hwnd, IDC_TEXT_INFO);


	CloseHandle(hModuleSnap);

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

wchar_t* UTF8ToUnicode(const char* str)
{
	int    textlen = 0;
	wchar_t* result;
	textlen = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	result = (wchar_t*)malloc((textlen + 1) * sizeof(wchar_t));
	if (result != 0)
	{
		memset(result, 0, (textlen + 1) * sizeof(wchar_t));
	}
	MultiByteToWideChar(CP_UTF8, 0, str, -1, (LPWSTR)result, textlen);
	return    result;
}