// MemoryOperate.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#pragma execution_character_set("utf-8")
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


#include <Windows.h>
#include <Psapi.h>

using namespace std;
const wchar_t* weChatName = L"WeChat.exe";
void GetAddress(HANDLE hProcess, DWORD baseAddress);




void readMemory() {

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

	char content[4] = { 0 };
	DWORD hookAddress = 0x040D87A0;

	cout << "头地址";
	GetAddress(hProcess, hookAddress);

}




void GetAddress(HANDLE hProcess, DWORD baseAddress) {

	char content[4] = { 0 };
	//读4个字节出来
	ReadProcessMemory(hProcess, (LPVOID)baseAddress, content, 4, 0);

	//10进制数值转成16进制显示
	cout << "0x";

	stringstream ioss;
	string s_temp;

	for (int i = 3; i >=0 ; i--) {
		ioss.clear();
		s_temp = "";
		DWORD di = content[i];

		ioss.fill('0');
		ioss << setiosflags(ios::uppercase) << setw(2) << hex << di;
		//以十六制(小写)形式输出//取消大写的设置
		//ioss << resetiosflags(ios::uppercase) << hex << i;
		ioss >> s_temp;
		cout << s_temp;
	}

}


int main()
{
    std::cout << "Hello World!\n";
	readMemory();

	cin.get();
}
