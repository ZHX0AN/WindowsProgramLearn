// InjectDllTools.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "InjectDllTools.h"
#include "resource.h"
#include <string.h>
#include <TlHelp32.h>
#include <windowsx.h>
#include <StrSafe.h> 
#include <shlwapi.h>
#pragma comment (lib,"shlwapi.lib")
#pragma comment (lib,"shell32.lib")

VOID Dlg_PopulateProcessList(HWND hwnd);
VOID ShowProcessInfo(HWND hwnd, DWORD dwProcessID);
void AddText(HWND hwnd, PCTSTR pszFormat, ...);
BOOL ProcessNext(HANDLE m_hSnapshot, PPROCESSENTRY32 ppe);
BOOL GetProcessCmdLine(DWORD PID, LPTSTR szCmdLine, DWORD Size);
PVOID GetModulePreferredBaseAddr(DWORD dwProcessId, PVOID pvModuleRemote);
int HowManyHeaps(HANDLE m_hSnapshot);
VOID TcharToChar(const TCHAR* tchar, char* _char);

VOID InjectDll(HWND   hwndDlg);

INT_PTR CALLBACK DialogProc(_In_ HWND   hwndDlg, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

const int			 s_cchAddress = sizeof(PVOID) * 2;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	// MAKEINTRESOURCE：强制转换IDD_MAIN
	DialogBox(NULL, MAKEINTRESOURCE(IDD_MAIN), NULL, &DialogProc);
	return 0;
}


//收信息
//char DllFileName[] = "C:\\workspaces\\thrid\\WxDllHookTest\\Debug\\WxDllHookTest.dll";
////char DllFileName[] = "C:\\workspaces\\pc\\demo\\C_inject\\Debug\\SQLite_CPP2.dll";
//
//DWORD strSize = strlen(DllFileName) + 1;

INT_PTR CALLBACK DialogProc(_In_ HWND   hwndDlg, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		Dlg_PopulateProcessList(hwndDlg);
		break;
	case WM_COMMAND:


		if (wParam == BTN_INJECT)
		{
			InjectDll(hwndDlg);
			//获取下拉框中内容
			HWND hwndCB = GetDlgItem(hwndDlg, LIST_PROCESSLIST);
			DWORD dwProcessId = (DWORD)ComboBox_GetItemData(hwndCB, ComboBox_GetCurSel(hwndCB));
			ShowProcessInfo(GetDlgItem(hwndDlg, IDC_DLL_INFO), dwProcessId);
		}
		else if (wParam == BTN_INFO) {
			//获取下拉框中内容
			HWND hwndCB = GetDlgItem(hwndDlg, LIST_PROCESSLIST);
			DWORD dwProcessId = (DWORD)ComboBox_GetItemData(hwndCB, ComboBox_GetCurSel(hwndCB));
			ShowProcessInfo(GetDlgItem(hwndDlg, IDC_DLL_INFO), dwProcessId);

		}
		else if (wParam == LIST_PROCESSLIST) {
			DWORD dw = ComboBox_GetCurSel(hwndDlg);
			DWORD processId = (DWORD)ComboBox_GetItemData(hwndDlg, dw); // Process ID
			ShowProcessInfo(GetDlgItem(hwndDlg, IDC_DLL_INFO), processId);
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


VOID InjectDll(HWND   hwndDlg) {

	//获取下拉框中内容
	HWND hwndCB = GetDlgItem(hwndDlg, LIST_PROCESSLIST);
	DWORD dwProcessId = (DWORD)ComboBox_GetItemData(hwndCB, ComboBox_GetCurSel(hwndCB));

	TCHAR buff[MAX_PATH] = { 0 };

	// CreateToolhelp32Snapshot：给线程拍照
	HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	swprintf_s(buff, TEXT("CreateToolhelp32Snapshot=%p"), handle);

	PROCESSENTRY32 processentry32 = { 0 };
	processentry32.dwSize = sizeof(PROCESSENTRY32);


	//2)	打开进程，获得HANDLE（OpenProcess）。
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, dwProcessId);
	if (hProcess == NULL)
	{
		MessageBox(NULL, TEXT("打开程序失败"), TEXT("错误"), MB_OK);
		return;
	}


	//获取dll路径
	DWORD strSize = 255;
	TCHAR DllFile[255] = {0};
	GetWindowText(GetDlgItem(hwndDlg, IDC_TEXT_DLLPATH), DllFile, 255);
	char dllFileNew[255] = { 0 };
	TcharToChar(DllFile, dllFileNew);

	//3)	在目标程序进程中为DLL文件路径字符串申请内存空间（VirtualAllocEx）。
	LPVOID allocAddress = VirtualAllocEx(hProcess, NULL, sizeof(dllFileNew), MEM_COMMIT, PAGE_READWRITE);
	if (NULL == allocAddress)
	{
		MessageBox(NULL, TEXT("分配内存空间失败"), TEXT("错误"), MB_OK);
		return;
	}

	//4)	把DLL文件路径字符串写入到申请的内存中（WriteProcessMemory）
	BOOL result = WriteProcessMemory(hProcess, allocAddress, dllFileNew, sizeof(dllFileNew), NULL);
	if (result == FALSE)
	{
		MessageBox(NULL, TEXT("写入内存失败"), TEXT("错误"), MB_OK);
		return;
	}

	//5)	从Kernel32.dll中获取LoadLibraryA的函数地址（GetModuleHandle、GetProcAddress）
	HMODULE hMODULE = GetModuleHandle(TEXT("Kernel32.dll"));
	FARPROC fARPROC = GetProcAddress(hMODULE, "LoadLibraryA");
	if (NULL == fARPROC)
	{
		MessageBox(NULL, TEXT("查找LoadLibrary失败"), TEXT("错误"), MB_OK);
		return;
	}
	//6)	在微信中启动内存中指定了文件名路径的DLL（CreateRemoteThread）。
	//也就是调用DLL中的DllMain（以DLL_PROCESS_ATTACH为参数）。
	HANDLE hANDLE = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)fARPROC, allocAddress, 0, NULL);
	if (NULL == hANDLE)
	{
		MessageBox(NULL, TEXT("启动远程线程失败"), TEXT("错误"), MB_OK);
		return;
	}
}

VOID Dlg_PopulateProcessList(HWND hwnd) {


	HWND hwndList = GetDlgItem(hwnd, LIST_PROCESSLIST);
	SetWindowRedraw(hwndList, FALSE);
	ComboBox_ResetContent(hwndList);

	// CreateToolhelp32Snapshot：给线程拍照
	HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	PROCESSENTRY32 pe32 = { 0 };
	pe32.dwSize = sizeof(PROCESSENTRY32);

	BOOL processNext = Process32Next(handle, &pe32);
	while (processNext == TRUE)
	{
		TCHAR sz[1024];
		processNext = Process32Next(handle, &pe32);

		StringCchPrintf(sz, _countof(sz), TEXT("%s     (0x%08X)"), pe32.szExeFile, pe32.th32ProcessID);

		OutputDebugString(sz);
		OutputDebugString(L"\n\t");
		int n = ComboBox_AddString(hwndList, sz);

		ComboBox_SetItemData(hwndList, n, pe32.th32ProcessID);
	}

	SetWindowRedraw(hwndList, TRUE);
	InvalidateRect(hwndList, NULL, FALSE);
}

VOID ShowProcessInfo(HWND hwnd, DWORD dwProcessID) {

	SetWindowText(hwnd, TEXT(""));   // Clear the output box
	HANDLE m_hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, dwProcessID);

	// Show Process details
	PROCESSENTRY32 pe = { sizeof(pe) };
	PPROCESSENTRY32 ppe = &pe;
	BOOL fOk = Process32First(m_hSnapshot, ppe);

	if (fOk && (ppe->th32ProcessID == 0)) {
		fOk = ProcessNext(m_hSnapshot, ppe); // Remove the "[System Process]" (PID = 0)
	}

	for (; fOk; fOk = ProcessNext(m_hSnapshot, &pe)) {
		if (pe.th32ProcessID == dwProcessID) {
			TCHAR szCmdLine[1024];
			if (GetProcessCmdLine(dwProcessID, szCmdLine, _countof(szCmdLine))) {
				AddText(hwnd, TEXT("Command line: %s %s\r\n"), pe.szExeFile, szCmdLine);
			}
			else {
				AddText(hwnd, TEXT("Filename: %s\r\n"), pe.szExeFile);
			}
			AddText(
				hwnd,
				TEXT("   PID=%08X, ParentPID=%08X, ") TEXT("PriorityClass=%d, Threads=%d, Heaps=%d\r\n"),
				pe.th32ProcessID,
				pe.th32ParentProcessID,
				pe.pcPriClassBase,
				pe.cntThreads,
				HowManyHeaps(m_hSnapshot)
			);

			break;   // No need to continue looping
		}
	}


	// Show Modules in the Process
	// Number of characters to display an address
	AddText(
		hwnd,
		TEXT("\r\nModules Information:\r\n") TEXT("  Usage  %-*s(%-*s)  %10s  Module\r\n"),
		s_cchAddress,
		TEXT("BaseAddr"),
		s_cchAddress,
		TEXT("ImagAddr"),
		TEXT("Size")
	);

	MODULEENTRY32 me = { sizeof(me) };
	fOk = Module32First(m_hSnapshot, &me);
	for (; fOk; fOk = Module32Next(m_hSnapshot, &me)) {
		if (me.ProccntUsage == 65535) {
			// Module was implicitly loaded and cannot be unloaded
			AddText(hwnd, TEXT("  Fixed"));
		}
		else {
			AddText(hwnd, TEXT("  %5d"), me.ProccntUsage);
		}

		// Try to format the size in kb.
		TCHAR szFormattedSize[64];
		if (StrFormatKBSize(me.modBaseSize, szFormattedSize, _countof(szFormattedSize)) == NULL)
		{
			StringCchPrintf(szFormattedSize, _countof(szFormattedSize), TEXT("%10u"), me.modBaseSize);
		}

		PVOID pvPreferredBaseAddr = GetModulePreferredBaseAddr(pe.th32ProcessID, me.modBaseAddr);
		if (me.modBaseAddr == pvPreferredBaseAddr) {
			AddText(
				hwnd,
				TEXT("  %p %*s   %10s  %s\r\n"),
				me.modBaseAddr,
				s_cchAddress,
				TEXT(""),
				szFormattedSize,
				me.szExePath
			);
		}
		else {
			AddText(
				hwnd,
				TEXT("  %p(%p)  %10s  %s\r\n"),
				me.modBaseAddr,
				pvPreferredBaseAddr,
				szFormattedSize,
				me.szExePath
			);
		}
	}

	// Show threads in the process
	//AddText(hwnd, TEXT("\r\nThread Information:\r\n")
	//	TEXT("      TID     Priority\r\n"));
	//THREADENTRY32 te = { sizeof(te) };
	//fOk = ThreadFirst(&te);
	//for (; fOk; fOk = th.ThreadNext(&te)) {
	//	if (te.th32OwnerProcessID == dwProcessID) {
	//		int nPriority = te.tpBasePri + te.tpDeltaPri;
	//		if ((te.tpBasePri < 16) && (nPriority > 15)) nPriority = 15;
	//		if ((te.tpBasePri > 15) && (nPriority > 31)) nPriority = 31;
	//		if ((te.tpBasePri < 16) && (nPriority < 1)) nPriority = 1;
	//		if ((te.tpBasePri > 15) && (nPriority < 16)) nPriority = 16;
	//		AddText(hwnd, TEXT("   %08X       %2d\r\n"),
	//			te.th32ThreadID, nPriority);
	//	}
	//}
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

BOOL ProcessNext(HANDLE m_hSnapshot, PPROCESSENTRY32 ppe) {

	BOOL fOk = Process32Next(m_hSnapshot, ppe);
	if (fOk && (ppe->th32ProcessID == 0))
		fOk = ProcessNext(m_hSnapshot, ppe); // Remove the "[System Process]" (PID = 0)
	return(fOk);
}

BOOL GetProcessCmdLine(DWORD PID, LPTSTR szCmdLine, DWORD Size) {

	// Sanity checks
	if ((PID <= 0) || (szCmdLine == NULL))
		return(FALSE);

	// Check if we can get information for this process
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, PID);
	if (hProcess == NULL)
		return(FALSE);

	BOOL bReturn = GetProcessCmdLine((DWORD)hProcess, szCmdLine, Size);

	// Don't forget to release the process handle
	CloseHandle(hProcess);

	return(bReturn);
}


PVOID GetModulePreferredBaseAddr(DWORD dwProcessId, PVOID pvModuleRemote) {

	PVOID pvModulePreferredBaseAddr = NULL;
	IMAGE_DOS_HEADER idh;
	IMAGE_NT_HEADERS inth;

	// Read the remote module's DOS header
	Toolhelp32ReadProcessMemory(dwProcessId, pvModuleRemote, &idh, sizeof(idh), NULL);

	// Verify the DOS image header
	if (idh.e_magic == IMAGE_DOS_SIGNATURE) {
		// Read the remote module's NT header
		Toolhelp32ReadProcessMemory(dwProcessId, (PBYTE)pvModuleRemote + idh.e_lfanew, &inth, sizeof(inth), NULL);

		// Verify the NT image header
		if (inth.Signature == IMAGE_NT_SIGNATURE) {
			// This is valid NT header, get the image's preferred base address
			pvModulePreferredBaseAddr = (PVOID)inth.OptionalHeader.ImageBase;
		}
	}
	return(pvModulePreferredBaseAddr);
}


int HowManyHeaps(HANDLE m_hSnapshot) {

	int nHowManyHeaps = 0;
	HEAPLIST32 hl = { sizeof(hl) };
	for (BOOL fOk = Heap32ListFirst(m_hSnapshot, &hl); fOk; fOk = Heap32ListNext(m_hSnapshot, &hl))
		nHowManyHeaps++;
	return(nHowManyHeaps);
}


//将TCHAR转为char   
//*tchar是TCHAR类型指针，*_char是char类型指针   
VOID TcharToChar(const TCHAR* tchar, char* _char)
{
	int iLength;
	//获取字节长度   
	iLength = WideCharToMultiByte(CP_ACP, 0, tchar, -1, NULL, 0, NULL, NULL);
	//将tchar值赋给_char    
	WideCharToMultiByte(CP_ACP, 0, tchar, -1, _char, iLength, NULL, NULL);
}