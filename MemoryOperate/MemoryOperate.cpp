// MemoryOperate.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <sdkddkver.h>
#include <Windows.h>
#include <TlHelp32.h>
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <shlwapi.h>
#include <atlconv.h>
#include <io.h>
#include <string>

#include <tchar.h>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>


// TODO:  在此处引用程序需要的其他头文件
#include <Windows.h>
#include <Psapi.h>

using namespace std;


const wchar_t* weChatName = L"WeChat.exe";


void readMemory() {

	DWORD weChatProcessID = 0;
	//1)	遍历系统中的进程，找到微信进程（CreateToolhelp32Snapshot、Process32Next）
	HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	//swprintf_s(buff, L"CreateToolhelp32Snapshot=%p", handle);
	//OutputDebugString(buff);

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
		std::cout << "no wechat" << endl;
		return;
	}
	std::cout << "weChatProcessID:" << weChatProcessID << endl;

	//2)	打开微信进程，获得HANDLE（OpenProcess）。
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, weChatProcessID);
	if (hProcess == NULL)
	{
		std::cout << "open wechat error" << endl;
		return;
	}
	std::cout << "hProcess:" << hProcess << endl;



	char originalCode[4] = { 0 };
	DWORD hookAddress = 0x012EEEF0;

	ReadProcessMemory(hProcess, (LPVOID)hookAddress, originalCode, 4, 0);

	std::cout << "code:" << originalCode << endl;

}

int main()
{
    std::cout << "Hello World!\n";
	readMemory();

	cin.get();
}
