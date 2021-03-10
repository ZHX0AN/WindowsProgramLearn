// 10-LoaderTest.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <iostream>
#include <windows.h> 
#include <string.h> 

using namespace std;


//微信ID的结构体
struct UserInfoStructRPC
{
    TCHAR wxid[50];
    TCHAR nickname[100];
    TCHAR avatar[200];
};


//定义函数指针
typedef BOOL(*lpInitWeChatSocket) ();
typedef BOOL(*lpInjectWechat) (LPCSTR);
typedef string(*lpGetUserInfo) ();
typedef string(*lpGetContactList) ();
typedef BOOL(*lpSendMsgText) (LPCSTR);



int main()
{
    //声明定义函数指针变量
    lpInitWeChatSocket InitWeChatSocket;
    lpInjectWechat InjectWechat;
    lpSendMsgText SendMsgText;

    //lpGetUserInfo GetUserInfo;
    //lpGetContactList GetContactList;
   

    //动态加载dll到内存
    HINSTANCE   hModule = LoadLibraryW(L"11-WxLoader.dll");

    //获取函数地址
    //GetUserInfo = (lpGetUserInfo)GetProcAddress(hModule, "GetUserInfo");
    //GetContactList = (lpGetUserInfo)GetProcAddress(hModule, "GetContactList");


    InitWeChatSocket = (lpInitWeChatSocket)GetProcAddress(hModule, "InitWeChatSocket");
    InitWeChatSocket();


    InjectWechat = (lpInjectWechat)GetProcAddress(hModule, "InjectWechat");
    char DLLFileName[] = "C:\\workspaces\\MyWeChat\\PcWxSingle\\Debug\\12-WxHelper.dll";
    std::cout << InjectWechat(DLLFileName) << endl;


    //获取个人信息
    //string userInfo = GetUserInfo();
    //cout << userInfo << endl;


    //获取联系人
    //cout << GetContactList();


    //LPCSTR str = "{\"data\": {\"to_wxid\": \"wxid_4sy2barbyny712\",\"content\": \"你好，世界\" },\"type\": 11036}";

    //char str[] = "{\"data\": {\"to_wxid\": \"wxid_4sy2barbyny712\",\"content\": \"123aaa\" },\"type\": 11036}";


    Sleep(3000);

    SendMsgText = (lpSendMsgText)GetProcAddress(hModule, "SendMsgText");

    LPCSTR str2 = "hello111";
    SendMsgText(str2);


    LPCSTR str = "{\"data\": {\"to_wxid\": \"wxid_4j4mqsuzdgie22\",\"content\": \"123aaa\" },\"type\": 1}";
    SendMsgText(str);



    getchar();
}