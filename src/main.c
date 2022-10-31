#include <stdio.h>
#include <Windows.h>
#include <direct.h>
#include <TlHelp32.h>
#include <CommCtrl.h>
#include "DLL_INJECT.h"
#include "resource.h"

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
) {
	HWND hwnd;
	MSG msg;
	WNDCLASS wnd;

	wnd.style = CS_HREDRAW | CS_VREDRAW;
	wnd.lpfnWndProc = WndProc;
	wnd.cbClsExtra = 0;
	wnd.cbWndExtra = 0;
	wnd.hInstance = hInstance;
	wnd.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
	wnd.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wnd.lpszMenuName = NULL;
	wnd.lpszClassName = SZCLASSNAME;

	if (!RegisterClass(&wnd)) {
		MessageBoxA(NULL, "注册窗口失败", "错误", MB_OK);
		return 1;
	}

	hwnd = CreateWindow(
		SZCLASSNAME,
		TEXT("aaa"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		250,
		200,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hwnd) {
		MessageBoxA(NULL, "创建窗口失败", "错误", MB_OK);
		UnregisterClass(SZCLASSNAME, hInstance);
		return 1;
	}

	ShowWindow(hwnd, nCmdShow);

	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return  0;
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_CREATE:
		DialogBox(NULL,MAKEINTRESOURCE(IDD_DIALOG1), hwnd, Dlgproc);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
};

INT_PTR CALLBACK Dlgproc(
	HWND hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam
) {
	switch (msg)
	{
	case WM_COMMAND: {
		UINT l = LOWORD(wparam);
		UINT h = HIWORD(wparam);

		if (l == ID_INJECT && h == BN_CLICKED) {
			INJECT();
			return 0;
		}
		if (l == IDCANCEL) {
			EndDialog(hwnd, 0);
			PostQuitMessage(0);
		}
		return 0;
	}
		default:
			return 0;
	}
	return 0;
};

VOID INJECT(VOID) {
	static char DLLPATH[0x100];
	_getcwd(DLLPATH, MAX_PATH);
	sprintf_s(DLLPATH, sizeof(DLLPATH), "%s%s", DLLPATH, "\\wxhook.dll");
	printf(DLLPATH);

	DWORD PID = ProcessNameFindPID(WXNAME);
	if (PID == 0) {
		MessageBoxA(NULL, "未找到进程", "ERROR", MB_OK);
		return;
	}
	printf("PID %d\n", PID);
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	if (hProcess == NULL) {
		MessageBoxA(NULL, "打开进程失败", "ERROR", MB_OK);
		return;
	}
	LPVOID v = VirtualAllocEx(hProcess, NULL, strlen(DLLPATH), MEM_COMMIT, PAGE_READWRITE);
	if (v == NULL) {
		MessageBoxA(NULL, "分配内存失败", "ERROR", MB_OK);
		return;
	}
	if (WriteProcessMemory(hProcess, v, DLLPATH, strlen(DLLPATH), NULL) == 0) {
		MessageBoxA(NULL, "写入内存失败", "ERROR", MB_OK);
		return;
	}

	printf("%p\n", v);

	HMODULE hModule = GetModuleHandle("kernel32.dll");
	if (hModule == NULL) {
		MessageBoxA(NULL, "获取内核失败", "ERROR", MB_OK);
		return;
	}

	FARPROC LOADLIBRARYA = GetProcAddress(hModule, "LoadLibraryA");
	if (LOADLIBRARYA == NULL) {
		MessageBoxA(NULL, "获取动态库加载器失败", "ERROR", MB_OK);
		return;
	}

	CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LOADLIBRARYA, v, 0, NULL);
	printf("%s\n", DLLPATH);

}

DWORD ProcessNameFindPID(char* NAME) {
	HANDLE Processes = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,NULL);
	PROCESSENTRY32 t32;
	t32.dwSize = sizeof(t32);
	do
	{
		printf("%s\n", t32.szExeFile);
		if (strcmp(NAME, t32.szExeFile) == 0) {
			return t32.th32ProcessID;
		}
	} while (Process32Next(Processes, &t32));
	return 0;
}

VOID FREE(
	HANDLE hProcess,
	LPVOID lpAddress,
	SIZE_T dwSize,
	DWORD dwFreeType
) {
	VirtualFreeEx(hProcess, lpAddress, dwSize, dwFreeType);
}