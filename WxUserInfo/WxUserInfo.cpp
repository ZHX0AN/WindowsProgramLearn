// WxUserInfo.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "WxUserInfo.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <windowsx.h>
#include <StrSafe.h> 
#include <sstream>
#include <string>
#include <tchar.h>

HWND g_hwndDialogDetail;


VOID AddText(HWND hwnd, PCTSTR pszFormat, ...);
VOID GetUserInfo(HWND hwnd);
BOOL GetUserInfoReal(HWND hwnd, DWORD dwPID);
INT_PTR CALLBACK DialogProc(_In_ HWND   hwndDlg, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

wchar_t* UTF8ToUnicode(const char* str);

const TCHAR weChatName[20] = TEXT("WeChat.exe");

HANDLE hProcess;
DWORD weChatBaseAdress = 0;
//微信ID
DWORD g_wxIdAddr = 0x18A3584;
//昵称
DWORD g_nameAddr = 0x18A35FC;
//电话
DWORD g_phoneNumberAddr = 0x18A3630;
//扫描手机类型
DWORD g_phoneTypeAddr = 0x18A3A50;
//头像
DWORD g_picBigAddr = 0x18A38C4;
DWORD g_picSmallAddr = 0x18A38DC;
//省市区
DWORD g_regionProvince = 0x18A36E8;
DWORD g_regionCity = 0x18A3700;
DWORD g_regionCountry = 0x18A37D8;

DWORD bytesToInt(BYTE* bytes)
{
	DWORD a = bytes[0] & 0xFF;
	a |= ((bytes[1] << 8) & 0xFF00);
	a |= ((bytes[2] << 16) & 0xFF0000);
	a |= ((bytes[3] << 24) & 0xFF000000);
	return a;
}


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

	GetUserInfoReal(hwnd, weChatProcessID);



}

BOOL GetUserInfoReal(HWND hwnd, DWORD dwPID) {

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
			weChatBaseAdress = (DWORD)me32.modBaseAddr;

			//AddText(GetDlgItem(hwnd, IDC_TEXT_INFO), TEXT("Module Name: %S\r\n"), me32.szModule);
			//AddText(GetDlgItem(hwnd, IDC_TEXT_INFO), TEXT("File Path: %S\r\n"), me32.szExePath);
			AddText(GetDlgItem(hwnd, IDC_TEXT_INFO), TEXT("Base Address: 0x%08X\r\n"), weChatBaseAdress);

			break;
		}

	} while (Module32Next(hModuleSnap, &me32));


	//wxid
	BYTE pWxIdAddr[4] = {0};
	ReadProcessMemory(hProcess, (LPVOID)(weChatBaseAdress + g_wxIdAddr), pWxIdAddr, 4, 0);
	DWORD wxIdAddr = bytesToInt(pWxIdAddr);
	BYTE wxId[50] = { 0 };
	ReadProcessMemory(hProcess, (LPVOID)wxIdAddr, wxId, 50, 0);
	TCHAR tempBuf[200] = { 0 };
	int nRet = MultiByteToWideChar(CP_ACP, 0, (char*)wxId, 100, tempBuf, 200);
	AddText(GetDlgItem(hwnd, IDC_TEXT_INFO), TEXT("微信ID: %s\r\n"), tempBuf);


	BYTE phoneNumber[20] = { 0 };
	ReadProcessMemory(hProcess, (LPVOID)(weChatBaseAdress + g_phoneNumberAddr), phoneNumber,  11, 0);
	TCHAR dBuf[30] = {0};
	nRet = MultiByteToWideChar(CP_ACP, 0, (char*)phoneNumber, 11, dBuf, 30);
	AddText(GetDlgItem(hwnd, IDC_TEXT_INFO), TEXT("手机号: %s\r\n"), dBuf);


	char name[100] = { 0 };
	ReadProcessMemory(hProcess, (LPVOID)(weChatBaseAdress + g_nameAddr), name, 100, 0);
	TCHAR* content = UTF8ToUnicode(name);
	AddText(GetDlgItem(hwnd, IDC_TEXT_INFO), TEXT("微信名字: %s\r\n"), content);


	BYTE phoneType[50] = { 0 };
	ReadProcessMemory(hProcess, (LPVOID)(weChatBaseAdress + g_phoneTypeAddr), phoneType, 50, 0);
	TCHAR phoneTypeBuf[100] = { 0 };
	nRet = MultiByteToWideChar(CP_ACP, 0, (char*)phoneType, 50, phoneTypeBuf, 100);
	AddText(GetDlgItem(hwnd, IDC_TEXT_INFO), TEXT("手机类型: %s\r\n"), phoneTypeBuf);

	//头像Big
	BYTE pPicBig[4] = { 0 };
	ReadProcessMemory(hProcess, (LPVOID)(weChatBaseAdress + g_picBigAddr), pPicBig, 4, 0);
	DWORD picBigAddr = bytesToInt(pPicBig);
	BYTE picBig[200] = { 0 };
	ReadProcessMemory(hProcess, (LPVOID)picBigAddr, picBig, 200, 0);
	TCHAR picBigBuf[200] = { 0 };
	nRet = MultiByteToWideChar(CP_ACP, 0, (char*)picBig, 200, picBigBuf, 400);
	AddText(GetDlgItem(hwnd, IDC_TEXT_INFO), TEXT("头像Big: %s\r\n"), picBigBuf);

	//头像Small
	BYTE pPicSmall[4] = { 0 };
	ReadProcessMemory(hProcess, (LPVOID)(weChatBaseAdress + g_picSmallAddr), pPicSmall, 4, 0);
	DWORD picSmallAddr = bytesToInt(pPicSmall);
	BYTE picSmall[200] = { 0 };
	ReadProcessMemory(hProcess, (LPVOID)picSmallAddr, picSmall, 200, 0);
	TCHAR picSmallBuf[200] = { 0 };
	nRet = MultiByteToWideChar(CP_ACP, 0, (char*)picSmall, 200, picSmallBuf, 400);
	AddText(GetDlgItem(hwnd, IDC_TEXT_INFO), TEXT("头像Small: %s\r\n"), picSmallBuf);


	//国家
	BYTE regionCountry[50] = { 0 };
	ReadProcessMemory(hProcess, (LPVOID)(weChatBaseAdress + g_regionCountry), regionCountry, 50, 0);
	TCHAR regionCountryBuf[100] = { 0 };
	nRet = MultiByteToWideChar(CP_ACP, 0, (char*)regionCountry, 11, regionCountryBuf, 30);

	//省
	BYTE regionProvince[50] = { 0 };
	ReadProcessMemory(hProcess, (LPVOID)(weChatBaseAdress + g_regionProvince), regionProvince, 50, 0);
	TCHAR regionProvinceBuf[100] = { 0 };
	nRet = MultiByteToWideChar(CP_ACP, 0, (char*)regionProvince, 11, regionProvinceBuf, 30);

	//市
	BYTE regionCity[50] = { 0 };
	ReadProcessMemory(hProcess, (LPVOID)(weChatBaseAdress + g_regionCity), regionCity, 50, 0);
	TCHAR regionCityBuf[100] = { 0 };
	nRet = MultiByteToWideChar(CP_ACP, 0, (char*)regionCity, 11, regionCityBuf, 30);
	AddText(GetDlgItem(hwnd, IDC_TEXT_INFO), TEXT("国家：%s   省：%s   市: %s\r\n"), regionCountryBuf, regionProvinceBuf, regionCityBuf);

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

DWORD GetWxMemoryInt(HANDLE hProcess, DWORD baseAddress) {

	BYTE content[4] = { 0 };
	ReadProcessMemory(hProcess, (LPVOID)baseAddress, content, 4, 0);

	int i = bytesToInt(content);
	return (DWORD)bytesToInt(content);
}