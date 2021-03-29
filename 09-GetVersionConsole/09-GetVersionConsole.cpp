// 09-GetVersionConsole.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <tchar.h>
#include <string>
#include <iostream>
#include <WINDEF.H>
#include <strstream>

#define RTN_ERROR 13
#pragma comment(lib, "advapi32")

#pragma comment(lib,"version.lib")
using namespace std;



string GetFileVersion(const char* filePath)
{
    string asVer = "";
    VS_FIXEDFILEINFO* pVsInfo;
    unsigned int iFileInfoSize = sizeof(VS_FIXEDFILEINFO);
    int iVerInfoSize = GetFileVersionInfoSizeA(filePath, NULL);
    if (iVerInfoSize != 0)
    {
        char* pBuf = NULL;

        while (!pBuf)
        {
            pBuf = new char[iVerInfoSize];
        }
        if (GetFileVersionInfoA(filePath, 0, iVerInfoSize, pBuf))
        {
            if (VerQueryValueA(pBuf, "\\", (void**)&pVsInfo, &iFileInfoSize))
            {
                //主版本2.6.7.57
                //2
                int s_major_ver = (pVsInfo->dwFileVersionMS >> 16) & 0x0000FFFF;
                //6
                int s_minor_ver = pVsInfo->dwFileVersionMS & 0x0000FFFF;
                //7
                int s_build_num = (pVsInfo->dwFileVersionLS >> 16) & 0x0000FFFF;
                //57
                int s_revision_num = pVsInfo->dwFileVersionLS & 0x0000FFFF;

                sprintf_s(pBuf, iVerInfoSize, "%d.%d.%d.%d", s_major_ver, s_minor_ver, s_build_num, s_revision_num);
                asVer = pBuf;
            }
        }

        delete[] pBuf;
    }
    return asVer;
}


bool GetInstallPath(char* filePath) {

#define MY_BUFSIZE 132    // Arbitrary initial value. 

    char ansi[MY_BUFSIZE] = { 0 };

    // Dynamic allocation will be used.
    HKEY hKey;
    TCHAR szProductType[MY_BUFSIZE];
    memset(szProductType, 0, sizeof(szProductType));
    DWORD dwBufLen = MY_BUFSIZE;
    LONG lRet;

    // 下面是打开注册表, 只有打开后才能做其他操作 
    lRet = RegOpenKeyEx(HKEY_CURRENT_USER,  // 要打开的根键 
        TEXT("SOFTWARE\\Tencent\\WeChat"), // 要打开的子子键 
        0,       // 这个一定要为0 
        KEY_QUERY_VALUE,  //  指定打开方式,此为读 
        &hKey);    // 用来返回句柄 

    if (lRet != ERROR_SUCCESS)   // 判断是否打开成功 
        return ansi;
    //下面开始查询 
    lRet = RegQueryValueEx(hKey,  // 打开注册表时返回的句柄 
        TEXT("InstallPath"),  //要查询的名称,qq安装目录记录在这个保存 
        NULL,   // 一定为NULL或者0 
        NULL,
        (LPBYTE)szProductType, // 我们要的东西放在这里 
        &dwBufLen);
    if (lRet != ERROR_SUCCESS)  // 判断是否查询成功 
        return ansi;
    RegCloseKey(hKey);

    WideCharToMultiByte(CP_ACP, 0, szProductType, -1, ansi, sizeof(ansi), NULL, NULL);
    strcat(ansi, "\\WeChatWin.dll");
    //cout << ansi;

    memcpy_s(filePath, MAX_PATH, ansi, strlen(ansi));

    return true;
    //WinExec(ansi, SW_SHOW);

}



int main()
{
    char filepath[MAX_PATH] = { 0 };
    GetInstallPath(filepath);
    string version = GetFileVersion(filepath);


    //string strFilePath = filepath;
    //string version = GetFileVersion(strFilePath.c_str());
    cout << version << endl;

    getchar();
    return 0;
}
