#pragma once
#include <string>
#include <iostream>
#include <Windows.h>

using namespace std;

wchar_t* UTF8ToUnicode(const char* str);
char* UnicodeToUtf8(const wchar_t* unicode);
DWORD getWeChatWinAddr();
LPCWSTR String2LPCWSTR(string text);
string Dec2Hex(DWORD i);
wchar_t* StrToWchar(std::string str);

LPCWSTR GetMsgByAddress(DWORD memAddress);
WCHAR* GetMsgByAddress2(DWORD memAddress);

void DebugLog(char* logStr);
void DebugLog(LPCWSTR logStr);