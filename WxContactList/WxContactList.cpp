// MemoryOper.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#define _CRT_SECURE_NO_WARNINGS
#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/pointer.h"

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

using namespace rapidjson;
using namespace std;

const wchar_t* weChatName = L"WeChat.exe";
HANDLE hProcess;
void PrintAddress(HANDLE hProcess, DWORD baseAddress);
BOOL ListProcessModules(DWORD dwPID);
DWORD GetWxMemoryInt(HANDLE hProcess, DWORD baseAddress);
VOID GetWxMemoryUnicodeString(IN DWORD baseAddress, IN int size);

VOID GetRoomInfo(DWORD roomAddress, rapidjson::Value* pVData);
string WcharToString(WCHAR* wchar);

DWORD addressOffsetPoint = 0x1886B38;
DWORD address1 = 0x28;
DWORD address2 = 0x8C;

DWORD weChatBaseAdress = 0;

//Rapid
rapidjson::Document doc;
//rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();


////构建data数据
//rapidjson::Value vData(kArrayType);
////构建userinfo数据
//rapidjson::Value vUserInfo(kObjectType);


// wxid：0x10 长度0x14
// 联系人账号：0x30，长度0x34
// 联系人昵称：0x8C，长度0x90
// 联系人备注：0x78，长度0x7C
struct ContactStruct
{
	wchar_t wxid[50];
	wchar_t account[50];
	wchar_t nickname[100];
	wchar_t remark[500];
};

list<ContactStruct> contactList;


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
	std::cout << "hProcess:" << hProcess << endl;
	if (hProcess == NULL)
	{
		std::cout << "open wechat error" << endl;
		return 0;
	}
	//std::cout << "hProcess:" << hProcess << endl;

	char content[4] = { 0 };
	//DWORD hookAddress = 0x040D87A0;
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
	_tprintf(TEXT("contractCount: 0x%08X \n"), contractCount);
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



	/*
	 * 构建RapidJson数据
	 */

	doc.SetObject();
	doc.AddMember("type", 11030, doc.GetAllocator());

	//增加data中数据


	rapidjson::Value vData(kArrayType);

	// 打印群数据
	GetRoomInfo(header1, &vData);
	GetRoomInfo(header2, &vData);
	GetRoomInfo(header3, &vData);


	//构建data数据
	
	doc.AddMember("data", vData, doc.GetAllocator());

	//StringBuffer sb;
	//PrettyWriter<StringBuffer> writer(sb);
	//doc.Accept(writer);    // Accept() traverses the DOM and generates Handler events.
	//std::cout << sb.GetString() << std::endl;

	cin.get();
}

VOID GetRoomInfo(DWORD roomAddress, rapidjson::Value* pVData) {

	for (int i = 0; i < nodeAddressVector.size(); ++i) {
		if (roomAddress == nodeAddressVector[i]) {
			return;
		}
	}
	nodeAddressVector.push_back(roomAddress);

	nodeNumber++;
	_tprintf(TEXT("------------  %d ---------------\n"), nodeNumber);

	//三个分支
	DWORD header1 = GetWxMemoryInt(hProcess, roomAddress);
	DWORD header2 = GetWxMemoryInt(hProcess, roomAddress + 4);
	DWORD header3 = GetWxMemoryInt(hProcess, roomAddress + 8);



	//群：70,2
	//好友： 70:3

	//好友：
	//0xc,70:3,C8:1,
	//0x30 微信账号 
	//0x44 账号？
	//0x78 好友备注
	//0x8C 微信昵称
	//0x11c 头像 - 大图片
	//0x130 头像 - 小图片
	//0x1bc 签名
	//0x29c 头像 - 小图片？


	//群：
	//0x30 微信账号
	//0x44 0000000 ？
	//0x78 群备注
	//0x8c 群名称
	//0x11c 00000000
	//0x130 头像 - 小图片
	//0x1bc 00000000


		//联系人账号 wchat_t
	TCHAR wWxId[50] = { 0 };
	DWORD wxIdTemp = GetWxMemoryInt(hProcess, roomAddress + 0x30);
	int wxIdSizeTemp = GetWxMemoryInt(hProcess, roomAddress + 0x34);
	ReadProcessMemory(hProcess, (LPVOID)wxIdTemp, wWxId, 100, 0);
	_tprintf(TEXT("wxid: %s \n"), wWxId);

                                        
	//联系人账号 wchat_t
	TCHAR acount[50] = {0};
	DWORD accountTemp = GetWxMemoryInt(hProcess, roomAddress + 0x44);
	int accountSizeTemp = GetWxMemoryInt(hProcess, roomAddress + 0x48);
	ReadProcessMemory(hProcess, (LPVOID)accountTemp, acount, 100, 0);
	_tprintf(TEXT("account: %s \n"), acount);

	//昵称
	wchar_t nickname[50] = {0};
	DWORD nicknameTemp = GetWxMemoryInt(hProcess, roomAddress + 0x8C);
	int nicknameSizeTemp = GetWxMemoryInt(hProcess, roomAddress + 0x90);
	ReadProcessMemory(hProcess, (LPVOID)nicknameTemp, nickname, 100, 0);
	_tprintf(TEXT("%s \n"), nickname);

	//头像
	wchar_t avatar[200] = {0};
	DWORD avatarTemp = GetWxMemoryInt(hProcess, roomAddress + 0x130);
	int avatarSizeTemp = GetWxMemoryInt(hProcess, roomAddress + 0x134);
	ReadProcessMemory(hProcess, (LPVOID)avatarTemp, avatar, 400, 0);
	_tprintf(TEXT("samall: %s \n"), avatar);

	//大头像
	wchar_t avatarBig[200] = { 0 };
	DWORD avatarBigTemp = GetWxMemoryInt(hProcess, roomAddress + 0x11C);
	int avatarSizeBigTemp = GetWxMemoryInt(hProcess, roomAddress + 0x120);
	ReadProcessMemory(hProcess, (LPVOID)avatarTemp, avatar, 400, 0);
	_tprintf(TEXT("big: %s \n"), avatar);

	//备注
	wchar_t remark[100] = {0};
	DWORD remarkTemp = GetWxMemoryInt(hProcess, roomAddress + 0x78);
	int remarkSizeTemp = GetWxMemoryInt(hProcess, roomAddress + 0x7c);
	ReadProcessMemory(hProcess, (LPVOID)remarkTemp, remark, 200, 0);
	_tprintf(TEXT("%s \n"), remark);


	//GetWxMemoryUnicodeString(GetWxMemoryInt(hProcess, roomAddress + 0x30), GetWxMemoryInt(hProcess, roomAddress + 0x34));
	//GetWxMemoryUnicodeString(GetWxMemoryInt(hProcess, roomAddress + 0x8C), GetWxMemoryInt(hProcess, roomAddress + 0x90));
	//GetWxMemoryUnicodeString(GetWxMemoryInt(hProcess, roomAddress + 0x130), GetWxMemoryInt(hProcess, roomAddress + 0x134));

	


	//构建userinfo数据
	rapidjson::Value vUserInfo(kObjectType);

	//wxid
	Value vWxid((char*)wWxId, doc.GetAllocator());
	vUserInfo.AddMember("wxid", vWxid, doc.GetAllocator());


	//accout
	Value vAcount((char*)acount, doc.GetAllocator());
	vUserInfo.AddMember("account", vAcount, doc.GetAllocator());

	//nickname
	Value vNickName((char*)nickname, doc.GetAllocator());
	vUserInfo.AddMember("nickname", vNickName, doc.GetAllocator());

	//头像
	Value vAvatar((char*)avatar, doc.GetAllocator());
	vUserInfo.AddMember("avatar", vAvatar, doc.GetAllocator());

	//备注
	Value vRemark((char*)remark, doc.GetAllocator());
	vUserInfo.AddMember("remark", vRemark, doc.GetAllocator());

	pVData->PushBack(vUserInfo, doc.GetAllocator());

	GetRoomInfo(header1, pVData);
	GetRoomInfo(header2, pVData);
	GetRoomInfo(header3, pVData);


}


VOID GetWxMemoryUnicodeString(IN DWORD baseAddress, IN int size) {

	//BYTE content[100] = { 0 };
	////ReadProcessMemory(hProcess, (LPVOID)baseAddress, content, nSize * 2, 0);
	//ReadProcessMemory(hProcess, (LPVOID)baseAddress, content,1000, 0);
	//WCHAR* ch = (WCHAR*)content;
	//cout << WcharToString(ch) << endl;

	wchar_t content[1000] = { 0 };
	ReadProcessMemory(hProcess, (LPVOID)baseAddress, content, 1000, 0);

	//_tprintf(TEXT("%s \n"), content);
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


string WcharToString(WCHAR* wchar)
{
	WCHAR* wText = wchar;
	// WideCharToMultiByte的运用
	DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, NULL, 0, NULL, FALSE);
	// psText为char*的临时数组，作为赋值给std::string的中间变量
	char* psText = new char[dwNum];
	// WideCharToMultiByte的再次运用
	WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, psText, dwNum, NULL, FALSE);
	// std::string赋值
	return psText;
}
