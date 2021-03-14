#pragma once
#include <string>
#include <iostream>
#include <Windows.h>

using namespace std;

LPCWSTR String2LPCWSTR(string text);
string Dec2Hex(DWORD i);
DWORD BytesToDword(BYTE* bytes);
char* UnicodeToUtf8(const wchar_t* unicode);
wchar_t* UTF8ToUnicode(const char* str);
DWORD getWeChatWinAddr();
string WcharToString(wchar_t* wchar);


DWORD GetWxMemoryInt(HANDLE hProcess, DWORD baseAddress);

wchar_t* StrToWchar(std::string str);