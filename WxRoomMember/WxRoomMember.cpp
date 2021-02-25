// MemoryOper.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <sstream>
#include <string>
#include <tchar.h>
#include <iomanip>
#include <list>
#include <vector>

using namespace std;
const wchar_t* weChatName = L"WeChat.exe";
HANDLE hProcess;
void PrintAddress(HANDLE hProcess, DWORD baseAddress);
BOOL ListProcessModules(DWORD dwPID);
DWORD GetWxMemoryInt(HANDLE hProcess, DWORD baseAddress);
VOID GetWxMemoryUnicodeString(DWORD baseAddress, int nSize);
VOID GetRoomInfo(DWORD roomAddress, DWORD log);
void CharToTchar(const char* _char, TCHAR* tchar);


DWORD addressOffsetPoint = 0x1886B38;
DWORD address1 = 0x840;
DWORD address2 = 0x88;

DWORD weChatBaseAdress = 0;

list<int> nodeAddressList;
vector<DWORD> nodeAddressVector;

int nodeNumber = 0;

int bytesToInt(byte* bytes)
{
	int a = bytes[0] & 0xFF;
	a |= ((bytes[1] << 8) & 0xFF00);
	a |= ((bytes[2] << 16) & 0xFF0000);
	a |= ((bytes[3] << 24) & 0xFF000000);
	return a;
}


int main()
{

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
		return 0;
	}
	std::cout << "ProcessID:" << weChatProcessID << endl;
	//weChatProcessID = 0x1680;
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, weChatProcessID);
	if (hProcess == NULL)
	{
		std::cout << "open wechat error" << endl;
		return 0;
	}
	//std::cout << "hProcess:" << hProcess << endl;

	char content[4] = { 0 };
	DWORD hookAddress = 0x040D87A0;
	ListProcessModules(weChatProcessID);

	DWORD linkPointer = GetWxMemoryInt(hProcess, weChatBaseAdress + addressOffsetPoint) + address1 + address2;
	// 打印链表指针
	_tprintf(TEXT("LinkPointer: 0x%08X \n"), linkPointer);
	cout << "-----------------------------------------" << endl;


	//群链表地址
	DWORD headerAddress = GetWxMemoryInt(hProcess, linkPointer);
	_tprintf(TEXT("headerAddress: 0x%08X \n"), headerAddress);
	//节点数量
	DWORD contractCount = GetWxMemoryInt(hProcess, linkPointer + 4);
	_tprintf(TEXT("Number: %d \n"), contractCount);
	nodeAddressList.push_front(headerAddress);
	nodeAddressVector.push_back(headerAddress);

	cout << "-----------------------------------------" << endl;

	//三个分支
	DWORD header1 = GetWxMemoryInt(hProcess, headerAddress);
	DWORD header2 = GetWxMemoryInt(hProcess, headerAddress + 4);
	DWORD header3 = GetWxMemoryInt(hProcess, headerAddress + 8);
	_tprintf(TEXT("header1: 0x%08X \n"), header1);
	_tprintf(TEXT("header2: 0x%08X \n"), header2);
	_tprintf(TEXT("header3: 0x%08X \n"), header3);
	cout << "-----------------------------------------" << endl;


	// 打印群数据
	GetRoomInfo(header1, headerAddress);




	cin.get();
}

VOID GetRoomInfo(DWORD roomAddress, DWORD log) {

	for (int i = 0; i < nodeAddressVector.size(); ++i) {
		if (roomAddress == nodeAddressVector[i]) {
			return;
		}
	}
	nodeAddressVector.push_back(roomAddress);

	nodeNumber++;
	_tprintf(TEXT("------------  %d ---------------\n"), nodeNumber);
	//_tprintf(TEXT("0x%08X"), roomAddress);
	//_tprintf(TEXT("->"));
	//_tprintf(TEXT("0x%08X"), log);


	//三个分支
	DWORD header1 = GetWxMemoryInt(hProcess, roomAddress);
	DWORD header2 = GetWxMemoryInt(hProcess, roomAddress + 4);
	DWORD header3 = GetWxMemoryInt(hProcess, roomAddress + 8);



	//群号
	GetWxMemoryUnicodeString(GetWxMemoryInt(hProcess, roomAddress + 0x10), GetWxMemoryInt(hProcess, roomAddress + 0x14));
	//群主
	GetWxMemoryUnicodeString(GetWxMemoryInt(hProcess, roomAddress + 0x6C), GetWxMemoryInt(hProcess, roomAddress + 0x70));

	//群成员
	//GetWxMemoryUnicodeString(GetWxMemoryInt(hProcess, roomAddress + 0x6C), GetWxMemoryInt(hProcess, roomAddress + 0x70));


	char content[10000] = { 0 };
	DWORD addr = GetWxMemoryInt(hProcess, roomAddress + 0x3C);
	ReadProcessMemory(hProcess, (LPVOID)addr, content, 10000, 0);
	printf("%s", content);

	GetRoomInfo(header1, log);
	GetRoomInfo(header2, log);
	GetRoomInfo(header3, log);

}


VOID GetWxMemoryUnicodeString(DWORD baseAddress, int nSize = 4) {

	TCHAR content[100] = { 0 };
	ReadProcessMemory(hProcess, (LPVOID)baseAddress, content, nSize * 2, 0);

	_tprintf(TEXT("%s \n"), content);
}


BOOL ListProcessModules(DWORD dwPID) {

	HANDLE                hModuleSnap = INVALID_HANDLE_VALUE;
	MODULEENTRY32        me32;

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
			_tprintf(TEXT("MODULE NAME: %s \n"), me32.szModule);
			_tprintf(TEXT("File Path = %s \n"), me32.szExePath);
			_tprintf(TEXT("Base Address = 0x%08X \n"), (DWORD)me32.modBaseAddr);
			weChatBaseAdress = (DWORD)me32.modBaseAddr;
			break;
		}

	} while (Module32Next(hModuleSnap, &me32));

	CloseHandle(hModuleSnap);

}


DWORD GetWxMemoryInt(HANDLE hProcess, DWORD baseAddress) {

	BYTE content[4] = { 0 };
	ReadProcessMemory(hProcess, (LPVOID)baseAddress, content, 4, 0);

	int i = bytesToInt(content);
	return (DWORD)bytesToInt(content);
}


void PrintAddress(HANDLE hProcess, DWORD baseAddress) {

	BYTE content[4] = { 0 };
	//读4个字节出来
	ReadProcessMemory(hProcess, (LPVOID)baseAddress, content, 4, 0);

	//10进制数值转成16进制显示
	cout << "0x";

	stringstream ioss;
	string s_temp;

	for (int i = 3; i >= 0; i--) {
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


void CharToTchar(const char* _char, TCHAR* tchar)
{
	int iLength;

	iLength = MultiByteToWideChar(CP_ACP, 0, _char, strlen(_char) + 1, NULL, 0);
	MultiByteToWideChar(CP_ACP, 0, _char, strlen(_char) + 1, tchar, iLength);
}