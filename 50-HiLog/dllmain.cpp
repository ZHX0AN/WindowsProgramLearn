// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "HiWeChat.h"
#include <iostream>


using namespace std;
HMODULE ntdll = GetModuleHandleW(L"ntdll.DLL");
HMODULE GModule = NULL;
HiUtil* Util = new HiUtil();
void* PvCookie = NULL;


DWORD WINAPI UnloadThreadProc(PVOID context) {
    FreeLibraryAndExitThread(*(HMODULE*)context, 0);
    return 0;
}

void UnLoad(HMODULE module) {
    HANDLE thread = CreateThread(NULL, 0, UnloadThreadProc, (PVOID) & module, NULL, NULL);
    if (thread) {
        CloseHandle(thread);
    }
}


//AddChatMsg
//7AF20000
//7B493420    E8 9F619300     call WeChatWi.7BDC95C4                   ; 日志HOOK

void PrintLogger(DWORD eax);

DWORD LoggerCallAddress = Util->Offset(0x7BDC95C4 - 0x7AF20000);
DWORD LoggerHookAddress = Util->Offset(0x7B493420 - 0x7AF20000);
DWORD LoggerReturnAddress = LoggerHookAddress + 5;
CHAR LoggerHookBackUp[5] = { 0 };

__declspec(naked) void HookLoggerCall() {
    __asm {
        call LoggerCallAddress;
        pushad;
        pushfd;

        push eax;
        call PrintLogger;
        add esp, 0x4;

        popfd;
        popad;
        jmp LoggerReturnAddress;
    }
}


void PrintLogger(DWORD eax) {
    if (eax == 0) {
        return;
    }
    string log = HiTools::ReadAsciiString(eax);
    cout << "WeChatWin.dll >> " << log.c_str() << endl;
    //OutputDebugStringA(log.c_str());
}

void HookLogger() {
    BYTE JMPCODE[5] = { 0 };
    JMPCODE[0] = 0xE9;
    *(DWORD*)&JMPCODE[1] = (DWORD)HookLoggerCall - LoggerHookAddress - 5;
    ReadProcessMemory(GetCurrentProcess(), (LPVOID)LoggerHookAddress, (LPVOID)LoggerHookBackUp, 5, 0);
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)LoggerHookAddress, (LPVOID)JMPCODE, 5, 0);
}

void UnHookLogger() {
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)LoggerHookAddress, (LPVOID)LoggerHookBackUp, 5, 0);
}



BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID reserved) {

    PfnLdrRegisterDllNotification PLdrRegisterDllNotification;
    PfnLdrUnregisterDllNotification PLdrUnregisterDllNotification;

    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: {
        GModule = hModule;
        Util->OpenConsole();
        HookLogger();
        break;
    }
    case DLL_PROCESS_DETACH: {
        Util->CloseConsole();
        UnHookLogger();
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}


