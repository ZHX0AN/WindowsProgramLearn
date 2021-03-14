#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "pch.h"
#include "utils.h"
#include "string"
#include <vector>
#include "AddrOffset.h"
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace std;


//DWORD weChatWinAddress = 0;


//把string 转换为 LPCWSTR
LPCWSTR String2LPCWSTR(string text)
{
	size_t size = text.length();
	WCHAR* buffer = new WCHAR[size + 1];
	MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, buffer, size + 1);

	//确保以 '\0' 结尾
	buffer[size] = 0;
	return buffer;
}


DWORD BytesToDword(BYTE* bytes)
{
	DWORD a = bytes[0] & 0xFF;
	a |= ((bytes[1] << 8) & 0xFF00);
	a |= ((bytes[2] << 16) & 0xFF0000);
	a |= ((bytes[3] << 24) & 0xFF000000);
	return a;
}


/*
    UnicodeToUtf8
*/
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


string WcharToString(wchar_t* wchar)
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



//将int转成16进制字符串
string Dec2Hex(DWORD i)
{
    //定义字符串流
    stringstream ioss;
    //存放转化后字符
    string s_temp;
    //以十六制(大写)形式输出
    ioss.fill('0');
    ioss << setiosflags(ios::uppercase) << setw(8) << hex << i;
    //以十六制(小写)形式输出//取消大写的设置
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
