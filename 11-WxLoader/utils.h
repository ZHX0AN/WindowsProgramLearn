#pragma once
#include <string>
using namespace std;

wchar_t* UTF8ToUnicode(const char* str);
DWORD getWeChatWinAddr();
LPCWSTR String2LPCWSTR(string text);
string Dec2Hex(DWORD i);
