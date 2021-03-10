#ifndef PCH_H
#define PCH_H

// 添加要在此处预编译的标头
#include "framework.h"

extern "C" _declspec(dllexport) BOOL InitWeChatSocket();
extern "C" _declspec(dllexport) BOOL InjectWechat(LPCSTR szDllPath);
extern "C" _declspec(dllexport) BOOL SendMsgText(LPCSTR lpJsonData);

#endif //PCH_H
