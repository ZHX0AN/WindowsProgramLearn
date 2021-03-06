﻿// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <windowsx.h>
#include <StrSafe.h> 
#include <sstream>
#include <string>
#include <tchar.h>
#include <sstream>
#include <iostream>
#include <CommCtrl.h>
#include<fstream>
#include<iomanip>

using namespace std;

DWORD wxBaseAddress = 0;
DWORD jumBackAddress = 0;
DWORD callAddress = 0;
void WriteDebugString(DWORD oldESP);
VOID OutPutDebugStr(DWORD oldESP);
VOID OpenDebugString(HMODULE hModule);
VOID AddText(HWND hwnd, PCTSTR pszFormat, ...);

HWND g_hwnd;
ofstream outFile;

//3.1.0.72

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
	{

		
		HANDLE hANDLE = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OpenDebugString, hModule, NULL, 0);
		if (hANDLE != 0)
		{
			CloseHandle(hANDLE);
		}
		break;
	}
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
	{
		
		break;
	}

    }
    return TRUE;
}


VOID OpenDebugString(HMODULE hModule)
{


	//获取WeChatWin.dll的基址
	while (wxBaseAddress == 0)
	{
		Sleep(100);
		wxBaseAddress = (DWORD)GetModuleHandle(TEXT("WeChatWin.dll"));
	}

	DWORD TLogLevel = 0;
	DWORD Xlogger_IsEnabled = 1;

	//日志级别
	DWORD TLogLevel_Address = wxBaseAddress + 0x181F184;
	//日志开启
	DWORD Xlogger_IsEnabled_Address = wxBaseAddress + 0x187B2D1;

	//修改内存
	*((int*)TLogLevel_Address) = TLogLevel;
	*((int*)Xlogger_IsEnabled_Address) = Xlogger_IsEnabled;

	// inline hook
	//6BA6E0E8 | .E8 F38B82FF   call WeChatWi.6B296CE0;  生成信息
	//WeChatWin.dll+A5E0E8 - E8 F38B82FF           - call WeChatWin.dll+286CE0

	DWORD hookAddress = wxBaseAddress + 0xEBF0C8;

	//返回地址
	jumBackAddress = hookAddress + 5;

	//Call的偏移
	DWORD offset = *((int*)((BYTE*)hookAddress + 1));
	callAddress = hookAddress + offset + 5;

	//组装指令
	BYTE jmpCode[5] = { 0 };
	jmpCode[0] = 0xE9;

	*((int*)&jmpCode[1]) = (DWORD)OutPutDebugStr - hookAddress - 5;

	//保存数据
	//BYTE originalCode[5] = { 0 };
	//ReadProcessMemory(GetCurrentProcess(), (LPCVOID)hookAddress, originalCode,5,0);

	//覆盖数据
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)hookAddress, jmpCode, 5, 0);
}

__declspec(naked) VOID OutPutDebugStr(DWORD oldESP)
{
	_asm
	{
		//补充被覆盖的代码
		call callAddress

		//保存现场
		mov oldESP, esp

		//添加参数
		push oldESP

		//调用函数
		call WriteDebugString

		//恢复堆栈
		mov esp, oldESP

		//跳回被HOOK指令的下一条指令
		jmp jumBackAddress
	}
}

VOID SaveToTxtFie(string str)
{

	if (str.length() ==0 ) {
		return;
	}

	ofstream outFile;
	outFile.open("C:\\temp\\carinfo.txt", std::ofstream::out | std::ofstream::app);

	outFile << str << endl;
	//outFile << "Year: "  << endl;
	//outFile << "was asking $"  << endl;
	//outFile << "Now asking $"  << endl;
	
	outFile.close(); //关闭文本文件
}

void WriteDebugString(DWORD oldESP)
{
	DWORD logAddress = *((int*)oldESP);
	char buffer[0x1000] = { 0 };
	ReadProcessMemory(GetCurrentProcess(), (LPCVOID)logAddress, buffer, 0x1000, 0);
	
	string str = buffer;

	SaveToTxtFie(str);
	//OutputDebugStringA(buffer);
}









