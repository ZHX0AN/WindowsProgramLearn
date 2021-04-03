#include "pch.h"
#include "HiUtil.h"
#include "HiTools.h"
#include <iostream>
#include <Windows.h>


using namespace std;

HiUtil::HiUtil() : mConsoleOut(NULL), mConsoleOutBackup(NULL) {
	mBase = (DWORD)GetModuleHandle(L"WeChatWin.dll");
}

HiUtil::~HiUtil() {
}

DWORD HiUtil::Offset(DWORD offset) {
	return mBase + offset;
}

void HiUtil::OpenConsole() {
	if (!mConsoleOpen) {
		if (AllocConsole()) {
			mConsoleOutBackup = cout.rdbuf();
			mConsoleOut = freopen("CONOUT$", "w", stdout);
			char nt[100];
			sprintf_s(nt, "地狱微信日志-%lld-%d", GetTickCount64(), GetCurrentProcessId());
			SetConsoleTitleA(nt);
			Sleep(100);
			HWND find = FindWindowA(NULL, nt);
			if (find) {
				HMENU menu = GetSystemMenu(find, FALSE);
				if (menu) {
					if (RemoveMenu(menu, 0xF060, 0)) {
						sprintf_s(nt, "地狱微信日志-%lld-%d-已屏蔽关闭按钮", GetTickCount64(), GetCurrentProcessId());
						SetConsoleTitleA(nt);
					}
				}
			}
		}
		wcout.imbue(locale("", LC_CTYPE));
		mConsoleOpen = TRUE;
	}
}

void HiUtil::CloseConsole() {
	if (mConsoleOpen) {
		if (mConsoleOut != NULL && mConsoleOutBackup != NULL) {
			cout.rdbuf(mConsoleOutBackup);
			fclose(mConsoleOut);
		}
		FreeConsole();
		mConsoleOpen = FALSE;
	}
}

std::string HiUtil::GetWeChatVersion() {
	HMODULE module = (HMODULE)GetModuleHandle(L"WeChatWin.dll");
	char filename[MAX_PATH] = { 0 };
	::GetModuleFileNameA(module, filename, sizeof(filename) - 1);
	return HiTools::GetFileVersion(filename);
}