//
// Created by Cummins on 2022/10/23.
//

#ifndef C_DLL_H
#define C_DLL_H

#define WECHAT "WeChat.exe"
#define SZClASSNAME TEXT("dll")
#define WINDOWNAME TEXT("测试")
#define BTN_ID_INJECT (HMENU)200

DWORD ProcessNameFindPID(CHAR *NAME);

VOID InjectDll(VOID);

#endif //C_DLL_H
