#pragma once
#include <string>

#define		WX_VERSION		"3.1.0.72"



class HiUtil {
public:
	HiUtil();
	~HiUtil();

	DWORD Offset(DWORD offset);

	void OpenConsole();
	void CloseConsole();

private:
	DWORD mBase = NULL;

	FILE* mConsoleOut;
	std::streambuf* mConsoleOutBackup;
	BOOL mConsoleOpen = FALSE;

public:
	static std::string GetWeChatVersion();
};