// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "resource.h"
#include "Windows.h"
#include "shellapi.h"
#include <tchar.h>
#include <stdio.h>
#include <list>
#include <string>
#include <fstream>
#include <strstream>
#include <windowsx.h>
#include <StrSafe.h> 
#pragma comment(lib, "Version.lib")

using namespace std;

void inlineHook();
void InlinkHookJump();
void OutPutData(int, int);
VOID SaveUserToTxtFie();
BOOL IsWxVersionValid();

VOID ShowDemoUI(HMODULE hModule);
VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
INT_PTR CALLBACK DialogProc(_In_ HWND   hwndDlg, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
BOOL IsWxVersionValid();
VOID AddText(HWND hwnd, PCTSTR pszFormat, ...);
wchar_t* AnsiToUnicode(const char* szStr);
char* UnicodeToAnsi(const wchar_t* szStr);
DWORD BytesToDword(BYTE* bytes);
void execSql();


typedef int(__cdecl* sqlite3_callback)(void*, int, char**, char**);
typedef int(__cdecl* Sqlite3_exec)(
	DWORD,                /* The database on which the SQL executes */
	const char*,           /* The SQL to be executed */
	sqlite3_callback, /* Invoke this callback routine */
	void*,                 /* First argument to xCallback() */
	char**             /* Write error messages here */
	);
int __cdecl MyCallback(void* para, int nColumn, char** colValue, char** colName);//回调函数



//定时器ID
DWORD dwTimeId = 0;
//微信基址
DWORD dllAdress = 0;
//跳回地址
DWORD jumpBackAddress = 0;
//此HOOK匹配的微信版本
const string wxVersoin = "3.1.0.72";
//数据库的数量
DWORD dbCount = 0;

DWORD g_hookAddress = 0x515733;

#define SQLITE_DBHANDLER_EXEC 0x1

////定义变量
DWORD wxBaseAddress = 0;

HWND g_hwndDlg;

//定义一个结构体来存储 数据库句柄-->数据库名
struct DbNameHandle
{
	int DBHandler;
	char DBName[MAX_PATH];
};

//在内存中存储一个“数据库句柄-->数据库名”的链表，
list<DbNameHandle> dbList;

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
	DialogBox(hModule, MAKEINTRESOURCE(IDD_MAIN), NULL, &DialogProc);
}


INT_PTR CALLBACK DialogProc(_In_ HWND   hwndDlg, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{

		g_hwndDlg = hwndDlg;
		HANDLE hANDLE = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)inlineHook, 0, NULL, 0);
		if (hANDLE != 0)
		{
			CloseHandle(hANDLE);
		}
		break;
	}
		
	case WM_COMMAND:
		if (wParam == IDOK)
		{

			execSql();
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


void execSql() {


	//组装wxid数据
	WCHAR wHandle[20];
	UINT uINT = GetDlgItemText(g_hwndDlg, IDC_TEXT_HANDLE, wHandle, 20);
	if (uINT == 0)
	{
		MessageBoxA(NULL, "请填写句柄", "错误", MB_OK | MB_ICONERROR);
		return;
	}

	WCHAR wSql[500];
	uINT = GetDlgItemText(g_hwndDlg, IDC_TEXT_SQL, wSql, 500);
	if (uINT == 0)
	{
		MessageBoxA(NULL, "请填写sql", "错误", MB_OK | MB_ICONERROR);
		return;
	}

	char* handleStr = UnicodeToAnsi(wHandle);


	DWORD dHandle = BytesToDword((BYTE*)handleStr);
	char* sql = UnicodeToAnsi(wSql);

	//调用sqlite3_exec函数查询数据库
	Sqlite3_exec sqlite3_exec = (Sqlite3_exec)(wxBaseAddress + SQLITE_DBHANDLER_EXEC);
	DWORD i = sqlite3_exec(dHandle, sql, MyCallback, NULL, NULL);


	

}

int __cdecl MyCallback(void* para, int nColumn, char** colValue, char** colName)
{

	for (int i = 0; i < nColumn; i++)
	{

		char* colNameStr = *(colName + i);
		char* colValueStr = *(colValue + i);
	}

	return 0;
}

//开始InlineHook
void inlineHook()
{
	//循环等待获取微信基址
	while (TRUE)
	{
		dllAdress = (DWORD)GetModuleHandleA("WeChatWin.dll");
		if (dllAdress == 0)
		{
			Sleep(100);
		}
		else
		{
			break;
		}
	}
	if (IsWxVersionValid() == FALSE)
	{
		MessageBoxA(NULL, "微信版本不匹配", "错误", MB_OK);
		return;
	}

	//7B3F0FA3  |.  8B75 EC       mov esi,[local.5]
	//7B3F0FA6 | .  83C4 08       add esp, 0x8
	//0x‭430FA3‬
	DWORD hookAddress = dllAdress + g_hookAddress;

	jumpBackAddress = hookAddress + 6;

	//组装跳转数据
	BYTE JmpCode[6] = { 0 };
	JmpCode[0] = 0xE9;
	JmpCode[6 - 1] = 0x90;

	//新跳转指令中的数据=跳转的地址-原地址（HOOK的地址）-跳转指令的长度
	*(DWORD*)&JmpCode[1] = (DWORD)InlinkHookJump - hookAddress - 5;

	WriteProcessMemory(GetCurrentProcess(), (LPVOID)hookAddress, JmpCode, 6, 0);
}

//InlineHook完成后，程序在Hook点跳转到这里执行。
//这里必须是裸函数
__declspec(naked) void InlinkHookJump()
{
	//补充代码
	__asm
	{
		//补充被覆盖的代码
		mov esi, dword ptr ss : [ebp - 0x14]
		add esp, 0x8

		//保存寄存器
		pushad

		//参数2，数据库句柄
		push[ebp - 0x14]
		//参数1，数据库路径地址，ASCII
		push[ebp - 0x24]
		//调用我们的处理函数
		call OutPutData
		add esp, 8

		//恢复寄存器
		popad

		//跳回去接着执行
		jmp jumpBackAddress
	}
}

//把内存中HOOK到的数据存储在链表中，
//重置定时器，5秒钟后激活定时器
void OutPutData(int dbAddress, int dbHandle)
{
	DbNameHandle db = { 0 };
	db.DBHandler = dbHandle;
	_snprintf_s(db.DBName, MAX_PATH, "%s", (char*)dbAddress);
	dbList.push_back(db);


	//char转wchar_t
	wchar_t* path = AnsiToUnicode((char*)dbAddress);

	AddText(GetDlgItem(g_hwndDlg, IDC_TEXT_MSG), TEXT("句柄： 0x%08X ，地址：%s \r\n"), dbHandle, (char*)path);


	//定时器
	//dwTimeId = SetTimer(NULL, 1, 5000, TimerProc);
}

//定时器回调函数
//到时候，把内存中HOOK来的数据保存到一个文本文件中
VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if (dwTimeId == idEvent)
	{
		//关闭定时器
		KillTimer(NULL, 1);
		//把“数据库句柄-->数据库名”的链表保存到一个文本文件中
		SaveUserToTxtFie();
	}
}

//把“数据库句柄-->数据库名”的链表保存到一个文本文件中
VOID SaveUserToTxtFie()
{
	if (dbCount == dbList.size())
	{
		return;
	}
	wstring wxUserFileName = L"DB_Hander.txt";
	DWORD index = 0;

	//作为输出文件打开
	ofstream ofile;
	ofile.open(wxUserFileName, ios_base::trunc | ios_base::binary | ios_base::in);
	/*char const* const utf16head = "\xFF\xFE ";
	ofile.write(utf16head, 2);*/
	DWORD i = 0;
	for (auto& db : dbList)
	{
		i++;
		CHAR sOut[MAX_PATH + 11] = { 0 };
		_snprintf_s(sOut, MAX_PATH + 11, "%02d\t0x%08X\t%s\n", i, db.DBHandler, db.DBName);

		string strintStr = string(sOut);
		char const* pos = (char const*)strintStr.c_str();

		////写入文件
		ofile.write(pos, strintStr.length());
	}
	dbCount = i;
	ofile.flush();
	ofile.close();
	ShellExecute(NULL, NULL, L"notepad.exe", wxUserFileName.c_str(), L".\\", SW_SHOW);
}

//检查微信版本是否匹配
BOOL IsWxVersionValid()
{
	WCHAR VersionFilePath[MAX_PATH];
	if (GetModuleFileName((HMODULE)dllAdress, VersionFilePath, MAX_PATH) == 0)
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


VOID AddText(HWND hwnd, PCTSTR pszFormat, ...) {

	va_list argList;
	va_start(argList, pszFormat);

	TCHAR sz[7 * 1024];
	Edit_GetText(hwnd, sz, _countof(sz));
	_vstprintf_s(_tcschr(sz, TEXT('\0')), _countof(sz) - _tcslen(sz), pszFormat, argList);
	Edit_SetText(hwnd, sz);
	va_end(argList);
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


//wchar_t*ת转char*
char* UnicodeToAnsi(const wchar_t* szStr)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, szStr, -1, NULL, 0, NULL, NULL);
	if (nLen == 0)
	{
		return NULL;
	}
	char* pResult = new char[nLen];
	WideCharToMultiByte(CP_ACP, 0, szStr, -1, pResult, nLen, NULL, NULL);
	return pResult;
}


DWORD BytesToDword(BYTE* bytes)
{
	DWORD a = bytes[0] & 0xFF;
	a |= ((bytes[1] << 8) & 0xFF00);
	a |= ((bytes[2] << 16) & 0xFF0000);
	a |= ((bytes[3] << 24) & 0xFF000000);
	return a;
}
