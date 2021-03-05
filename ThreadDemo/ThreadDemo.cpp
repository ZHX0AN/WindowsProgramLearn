// ThreadDemo.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "resource.h"
#include "ThreadDemo.h"
#include <windowsx.h>


INT_PTR CALLBACK DialogProc(_In_ HWND   hwndDlg, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
void AddText(HWND hwnd, PCTSTR pszFormat, ...);
DWORD WINAPI Thread1(LPVOID lp);
DWORD WINAPI Thread2(LPVOID lp);

HANDLE hThread;
DWORD  threadId;

HWND   g_hwndDlg;


/*
	Sleep() 自己睡眠
	SuspendThread(hThread) 挂起线程，可以挂起别的线程
	ResumeThread() 恢复线程
	WaitForMultipleObjct

*/


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	// MAKEINTRESOURCE：强制转换IDD_MAIN
	DialogBox(NULL, MAKEINTRESOURCE(IDD_MAIN), NULL, &DialogProc);
	return 0;
}


INT_PTR CALLBACK DialogProc(_In_ HWND   hwndDlg, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:

		g_hwndDlg = hwndDlg;
		break;
	case WM_COMMAND:


		if (wParam == IDC_BTN_START1)
		{
			hThread = CreateThread(NULL, 0, Thread1, 0, 0, &threadId);
		}
		else if (wParam == IDC_BTN_STOP1) {

			
		}
		else if (wParam == IDC_BTN_START2) {
			hThread = CreateThread(NULL, 0, Thread2, 0, 0, &threadId);
		}
		else if (wParam == IDC_BTN_STOP2) {

		}
		break;
	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;
	default:
		break;
	}
	return FALSE;
}



DWORD WINAPI Thread1(LPVOID lp)
{
	int i;
	for (i = 0; i < 5; i++)
	{
		AddText(GetDlgItem(g_hwndDlg, IDC_TEXT_MSG), TEXT("Thread1: %d \r\n"), i);
		Sleep(2000);
	}
	return 0;
}


DWORD WINAPI Thread2(LPVOID lp)
{
	int i;
	for (i = 0; i < 5; i++)
	{
		AddText(GetDlgItem(g_hwndDlg, IDC_TEXT_MSG), TEXT("Thread2: %d \r\n"), i);
		Sleep(2000);
	}
	return 0;
}




// Add a string to an edit control
void AddText(HWND hwnd, PCTSTR pszFormat, ...) {

	va_list argList;
	va_start(argList, pszFormat);

	TCHAR sz[20 * 1024];
	Edit_GetText(hwnd, sz, _countof(sz));
	_vstprintf_s(_tcschr(sz, TEXT('\0')), _countof(sz) - _tcslen(sz), pszFormat, argList);
	Edit_SetText(hwnd, sz);
	va_end(argList);
}