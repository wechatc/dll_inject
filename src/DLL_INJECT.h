#pragma once
#include <Windows.h>
#ifndef DLL_INJECT
#define DLL_INJECT

#define WXNAME (char *)"WeChat.exe"
//#define DLLPATH (CHAR *)"F:\\C\\dllProject\\wxhook\\Debug\\wxhook.dll"
#define SZCLASSNAME TEXT("Test")
#define ID_BTN_1 (HMENU)150

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK Dlgproc(
	HWND,
	UINT,
	WPARAM,
	LPARAM
);

VOID INJECT(VOID);

VOID FREE(
	HANDLE hProcess,
	LPVOID lpAddress,
	SIZE_T dwSize,
	DWORD  dwFreeType
);

DWORD ProcessNameFindPID(char*);

#endif // !DLL_INJECT
