// LoaderTest.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <iostream>
#include <windows.h> 
#include <string.h> 
#include <tchar.h>

using namespace std;


/*
 * 只支持3.1.0.72版本
 */
typedef VOID(*RECEIVE)(DWORD, LPSTR, DWORD);
typedef VOID(*ACCEPT)(DWORD);
typedef VOID(*CLOSE)(DWORD);
typedef BOOL(*InitFn)(RECEIVE, ACCEPT, CLOSE);

typedef BOOL(*lpInjectWechat) (LPCSTR);

//typedef BOOL(*lpSendWeChatData) (DWORD, LPCSTR);

typedef BOOL(*lpSendFn)(DWORD, LPCSTR);


VOID receive(DWORD clientId, LPSTR data, DWORD len)
{
    //std::cout << "new message! " << data << endl;
    std::cout << data << endl;
}
VOID accept(DWORD clientId)
{
    std::cout << "new client! " << endl;
}
VOID close(DWORD clientId)
{
    std::cout << "client closed! " << endl;
}


int _tmain(int argc, _TCHAR* argv[])
{

    //加载Dll
    HINSTANCE   hModule = LoadLibraryW(L"11-WxLoader.dll");

    //初始化
    BOOL rs = FALSE;
    InitFn InitWeChatSocket = (InitFn)GetProcAddress(hModule, "InitWeChatSocket");
    rs = InitWeChatSocket(receive, accept, close);
    std::cout << "初始化结果：" << rs << endl;


    //注入Dll
    lpInjectWechat InjectWechat = (lpInjectWechat)GetProcAddress(hModule, "InjectWechat");
    char DLLFileName[] = "C:\\project\\MyWeChat\\PcWxSingle\\Debug\\12-WxHelper.dll";
    std::cout << "注入DLL：" << InjectWechat(DLLFileName) << endl;

    ////休眠1秒，等Socket连接
    Sleep(1000);

    DWORD dwClientId = 1;
    lpSendFn SendWeChatData = (lpSendFn)GetProcAddress(hModule, "SendWeChatData");


    //获取个人信息
    printf("获取个人信息：\n");
    LPCSTR lpUserInfo = "{\"data\":{},\"type\": 11028}";
    SendWeChatData(dwClientId, lpUserInfo);

    Sleep(1000);



    while (true)
    {
        Sleep(5000);
    }
    cin.get();
}