#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include <TlHelp32.h>
#include "dll.h"

static TCHAR *szClassName = SZClASSNAME;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPSTR lpCmdLine, int nCmdShow) {
    HWND hwnd;
    MSG msg;
    WNDCLASS wndclass;

    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szClassName;

    if (RegisterClass(&wndclass))
        MessageBoxW(hwnd, L"创建失败", L"ERROR", MB_CANCELTRYCONTINUE);

    hwnd = CreateWindow(
            szClassName,
            WINDOWNAME,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            200,
            150,
            NULL,
            NULL,
            hInstance,
            NULL
    );

    // 显示窗口
    ShowWindow(hwnd, nCmdShow);

    // 更新窗口
    UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lParam) {
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rect;

    switch (msg) {

        case WM_CREATE: {
            CreateWindow(WC_BUTTON, _T("点击"), WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE, 100, 100, 50, 20, hwnd,
                         BTN_ID_INJECT,
                         NULL, NULL);
        }

        case WM_COMMAND: {
            UINT ID = LOWORD(wparam);
            UINT nCode = HIWORD(wparam);
            if (ID == 200 && nCode == BN_CLICKED)
                InjectDll();
        }

        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);
            GetClientRect(hwnd, &rect);
            DrawText(hdc, TEXT("C Is Best!"), -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER
            );
            EndPaint(hwnd, &ps);
            return 0;

        case WM_CLOSE:
            if (MessageBoxW(hwnd, L"确认关闭?", L"确认", MB_YESNO) == IDYES)
                DestroyWindow(hwnd);
            else
                return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

    }

    return DefWindowProc(hwnd, msg, wparam, lParam);
}

// 获取微信句柄 (有了句柄才能操作微信内存)
// 通过微信进程名获取微信PID
DWORD ProcessNameFindPID(CHAR *NAME) {
    // 获取整个系统进程快照
    HANDLE HProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    // 定义PROCESSENTRY32结构体 这个结构体里面有我们需要的参数
    PROCESSENTRY32 processentry32;
    // 必须初始化dwSize变量
    processentry32.dwSize = sizeof(processentry32);
    do {
        // 判断szExeFile的进程名是否和形参*NAME一样 如果一样返回0
        if (strcmp(NAME, processentry32.szExeFile) == 0) {
            return processentry32.th32ProcessID;
        }
    } while (Process32Next(HProcess, &processentry32));
    return 0;
}

// 在微信内部申请一块内存放dll路径 然后通过PID打开微信进程获取到句柄
VOID InjectDll() {
    char *path = "F://C//dllProject//testdll//cmake-build-debug//libtestdlla.dll";
    // 先获取到微信的句柄
    DWORD PID = ProcessNameFindPID(WECHAT);
    if (PID == 0) {
        MessageBoxW(NULL, L"未找到微信进程", L"错误", MB_OK);
        return;
    }
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
    if (hProcess == NULL) {
        MessageBoxW(NULL, L"进程打开失败", L"错误", MB_OK);
        return;
    }

    // 申请内存
    LPVOID dllAdd = VirtualAllocEx(hProcess, NULL, strlen(path), MEM_COMMIT, PAGE_READWRITE);
    if (dllAdd == NULL) {
        MessageBoxW(NULL, L"内存分配失败", L"错误", MB_OK);
        return;
    }

    // 写入dll路径到上面的地址
    if (WriteProcessMemory(hProcess, dllAdd, path, strlen(path), NULL) == NULL) {
        MessageBoxW(NULL, L"地址写入失败", L"错误", MB_OK);
        return;
    }

    HMODULE hmodule = GetModuleHandle("Kernel32.dll");
    FARPROC loadlibraryAdd = GetProcAddress(hmodule, "LoadLibraryA");

    HANDLE handle = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE) loadlibraryAdd, dllAdd, 0, NULL);
    printf("%p\n", dllAdd);
    LoadLibraryA(path);
    if (handle == NULL) {
        MessageBoxW(NULL, L"远程注入失败", L"错误", MB_OK);
        return;
    }
}