#pragma once
#include <Windows.h>
#include <string>
#include "AddrOffset.h"
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace std;

/*
	UTF8ToUnicode
*/
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

char* UnicodeToUtf8(const wchar_t* unicode)
{
	int len;
	len = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, NULL, 0, NULL, NULL);
	char* szUtf8 = (char*)malloc(len + 1);
	if (szUtf8 != 0) {
		memset(szUtf8, 0, len + 1);
	}
	WideCharToMultiByte(CP_UTF8, 0, unicode, -1, szUtf8, len, NULL, NULL);
	return szUtf8;
}

//��string ת��Ϊ LPCWSTR
LPCWSTR String2LPCWSTR(string text)
{
	size_t size = text.length();
	WCHAR* buffer = new WCHAR[size + 1];
	MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, buffer, size + 1);

	//ȷ���� '\0' ��β
	buffer[size] = 0;
	return buffer;
}

//��intת��16�����ַ���
string Dec2Hex(DWORD i)
{
	//�����ַ�����
	stringstream ioss;
	//���ת�����ַ�
	string s_temp;
	//��ʮ����(��д)��ʽ���
	ioss.fill('0');
	ioss << setiosflags(ios::uppercase) << setw(8) << hex << i;
	//��ʮ����(Сд)��ʽ���//ȡ����д������
	//ioss << resetiosflags(ios::uppercase) << hex << i;
	ioss >> s_temp;
	return "0x" + s_temp;
}




wchar_t* StrToWchar(std::string str)
{
	int strSize = (int)(str.length() + 1);
	wchar_t* wStr = new wchar_t[strSize];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wStr, strSize);
	return wStr;
	delete[] wStr;
}

void DebugLog(char* logStr) {
	//cout << logStr << WSAGetLastError() << endl;
	//OutputDebugString(String2LPCWSTR(logStr));

}

void DebugLog(LPCWSTR logStr) {
	//OutputDebugString((LPCWSTR)WSAGetLastError());
	//OutputDebugString(logStr);

}

//��ȡ�ڴ��е��ַ���
//�洢��ʽ
//xxxxxxxx:�ַ�����ַ��memAddress��
//xxxxxxxx:�ַ������ȣ�memAddress +4��
LPCWSTR GetMsgByAddress(DWORD memAddress)
{
	//��ȡ�ַ�������
	DWORD msgLength = *(DWORD*)(memAddress + 4);
	if (msgLength == 0)
	{
		WCHAR* msg = new WCHAR[1];
		msg[0] = 0;
		return msg;
	}

	WCHAR* msg = new WCHAR[msgLength + 1];
	ZeroMemory(msg, msgLength + 1);

	//��������
	wmemcpy_s(msg, msgLength + 1, (WCHAR*)(*(DWORD*)memAddress), msgLength + 1);
	return msg;

}


// wchar_t תUTF8
WCHAR* GetMsgByAddress2(DWORD memAddress)
{
	//��ȡ�ַ�������
	DWORD msgLength = *(DWORD*)(memAddress + 4);
	if (msgLength == 0)
	{
		WCHAR* msg = new WCHAR[1];
		msg[0] = 0;
		return msg;
	}

	WCHAR* msg = new WCHAR[msgLength + 1];
	ZeroMemory(msg, msgLength + 1);

	//��������
	wmemcpy_s(msg, msgLength + 1, (WCHAR*)(*(DWORD*)memAddress), msgLength + 1);
	return msg;
}