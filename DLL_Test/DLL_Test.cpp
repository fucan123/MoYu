// DLL_Test.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "DLL_Test.h"
#include "Self/GameMain.h"
#include "Self/GameProc.h"
#include "Self/Move.h"
#include <stdio.h>
#include <tchar.h>

bool  g_NoUnStall = true;
HHOOK g_Hook = NULL;
bool g_Exit = false;
bool g_Loop = true;
PlayerSetMove PlayerMove = NULL;
PlayerSetMove OldPlayerMove = NULL;
DWORD* xPtr = NULL;
PVOID  HookMove = NULL;
PVOID  HookAddr = NULL;
PVOID  HookGameMap = NULL;
PVOID  HookAddr3 = NULL;
PVOID  HookAddr4 = NULL;
PVOID  HookSetPos = NULL;
pfnNtQueryInformationProcess gNtQueryInformationProcess = NULL;

GameMain gameMain;

DWORD WINAPI Run(LPVOID lParam)
{
	if (!gameMain.IsMoYu())
		return 0;

	AllocConsole();
	freopen("CON", "w", stdout);
	if (gameMain.Init()) {
		//INLOG("进入游戏！！！");
#if 1
		gameMain.Run();
		//return 0;
#endif
		Sleep(5000);
		//gameMain.m_pMove->Run(400, 400);
		while (!g_Exit) {
			if (!g_Loop)
				Sleep(5000);
			if (gameMain.m_pGameProc->m_bReStart)
				gameMain.Run();
			//gameMain.PickUpItem();
#if 0
			// 2BC8E45E-2BC6EE84=1F5DA
			// 01E6A838
			// mov eax,dword ptr ds:[eax+0xFC]
			gameMain.ReadGameMemory(0x10);
			Sleep(10000);
			//continue;
#endif
			//gameMain.AttackGuaiWu();
#if 0
			GameSelfItem* items[120];
			DWORD count = gameMain.m_pItem->ReadSelfItems(items, 120);
			if (count > 0) {
				printf("\n----------------------------\n");
				printf("背包物品数量:%d\n", count);
				for (DWORD i = 0; i < count; i++) {
					if (!items[i])
						continue;

					//printf("GameBagItem:%08X\n", items[i]);
					printf("[%02d]物品ID:%08X 类型:%08X 固定:%08X 固定2:%08X\n", i + 1, items[i]->Id, items[i]->Type, items[i]->Fix, items[i]->Fix2);
				}
				printf("\n----------------------------\n");
			}
#endif
			GameGroundItem* iii[120];
			gameMain.m_pItem->ReadGroundItems(iii, 120);
			Sleep(5000);
		}
		//gameMain.Call_Run(850, 510);
		//gameMain.Call_NPC(0x193E2);
		//Sleep(10000);
		//gameMain.Call_NPCTalk(0x00);
	}

	return 0;

#define FIND 0
	HWND hWnd = NULL;
#if FIND == 1
	while (!hWnd) {
		hWnd = FindWindow(NULL, L"【魔域】");
		Sleep(5000);
	}
#endif

	DWORD pid, pid2;
	pid = GetCurrentProcessId();
	GetWindowThreadProcessId(hWnd, &pid2);
	printf("窗口句柄:%08X %d=%d.\n", hWnd, pid, pid2);

#if FIND == 1
	if (pid != pid2) {
		printf("不是此窗口！！！\n");
		return 0;
	}
#endif

	gNtQueryInformationProcess = (pfnNtQueryInformationProcess)GetNtdllProcAddress("NtQueryInformationProcess");
	if (gNtQueryInformationProcess) {
		printf("gNtQueryInformationProcess:%08X\n", gNtQueryInformationProcess);
		//Hook::GetInstance()->Attach((PBYTE)gNtQueryInformationProcess + NTQIP_OFFSET, MyNtQueryInformationProcess);
	}

	while (true) {
		BOOL bIsDebugger = IsDebuggerPresent();
		BOOL bMyIsDebugger = FALSE, bRemoteDebugger = FALSE;
		DWORD dwPeb;
		ULONG32 dwPeb2;
		ULONG64 dwPeb3;
		ULONG ulNtGlobalFlag, ulNtGlobalFlag2;

		__asm {
			mov eax, fs:[0x30]
			mov dwPeb, eax
			xor ebx, ebx
			mov bl, byte ptr[eax + 2]
			mov byte ptr bMyIsDebugger, bl
		}

		dwPeb2 = GetPebBaseAddress32();
		dwPeb3 = GetPebBaseAddress64();
		ulNtGlobalFlag = GetNtGlobalFlag32(dwPeb2);
		ulNtGlobalFlag2 = GetNtGlobalFlag64(dwPeb3);
		CheckRemoteDebuggerPresent(GetCurrentProcess(), &bRemoteDebugger);

		printf("--- ");
		printf("PEB:%08X DEBUG:%02X | ", dwPeb, bMyIsDebugger & 0xff);
		printf("PEB:%08X DEBUG:%02X NtGF:%08X | ", dwPeb2, IsDebugger32(dwPeb2) & 0xff, ulNtGlobalFlag);
		printf("PEB:%08X DEBUG:%02X NtGF:%08X | ", (ULONG32)dwPeb3, IsDebugger64(dwPeb3) & 0xff, ulNtGlobalFlag2);
		printf("bRemoteDebugger:%02X ", bRemoteDebugger & 0xff);
		//printf("MyIsDebugger:%02X ", bMyIsDebugger & 0xff);
		//printf("IsDebuggerPresent:%02X ", bIsDebugger & 0xff);

		printf("---\n");

		if (IsDebugger64(dwPeb3)) {
			printf("--- 清除调试标志 ---\n");
			//ZeroDebugger64(dwPeb3);
		}

		Sleep(3000);
	}

	while (true) {
		/*
		bool send = SendMessage(hWnd, WM_KEYDOWN, 'I' , 0);
		if (send) {
			printf("按键成功.\n");
		}
		else {
			printf("按键失败.\n");
		}
		Sleep(10);
		SendMessage(hWnd, WM_KEYUP, 'I', 0);
		*/

		HMODULE hMod;
		if (CheckDllInProcess(0, MOD_3drole, &hMod)) {
			xPtr = (DWORD*)((PBYTE)hMod + ADDR_COOR_X_OFFSET);
			DWORD x = xPtr[0];
			DWORD y = xPtr[1];
			printf("x:%d, y:%d.\n", x, y);

			HookMove = (PBYTE)hMod + ADDR_MOVE_FUNC;
			HookAddr3 = (PBYTE)hMod + ADDR_MOVE_P;
			HookAddr4 = (PBYTE)hMod + ADDR_MOVE_PP;
			HookSetPos = (PBYTE)hMod + ADDR_SET_POS;

			// EBX=05CC0020
			// ECX=036CD538

			HMODULE hMod2;
			if (x > 0 && y > 0 && !PlayerMove 
				&& CheckDllInProcess(0, MODE_3dgamemap, &hMod2)) {
				Sleep(2000);
				PlayerMove = (PlayerSetMove)((PBYTE)hMod2 + ADDR_MOVE_FUNC2);
				HookAddr = (PBYTE)PlayerMove + 0x10;
				HookGameMap = (PBYTE)PlayerMove + 0x52;
				PBYTE addr = (PBYTE)PlayerMove;

				for (int i = 0; i < 0x10; i++) {
					printf("%02X ", addr[i]&0xff);
				}
				printf("\n");
				DWORD v = 0;
				//v = PlayerMove(x, y);
				printf("函数地址:%08X 返回值:%d.\n", PlayerMove, v);

				AddVectoredExceptionHandler(1, VectorHandler);
				//SetVectorBreak(HookMove);
				//SetVectorBreak((PVOID)IAIMANAGER_ADDR);
				//SetVectorBreak(HookAddr);
				//SetVectorBreak(HookGameMap);
				//SetVectorBreak(HookAddr3);
				//SetVectorBreak(HookAddr4);
				//SetVectorBreak(HookSetPos);
				//Hook::GetInstance()->Attach(PlayerMove, MyMove);
				break;
			}
		}

		Sleep(5000);
	}
	
	return 0;
}

DWORD WINAPI KeyBoardHook(LPVOID p)
{
	HOOKPROC;
	g_Hook = SetWindowsHookExW(WH_KEYBOARD_LL, KeyBoardHookProc, NULL, 0);
	if (g_Hook)
		printf(("HOOK成功\n"));

#if 1
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (g_NoUnStall && GetMessage(&msg, 0, 0, 0)) {
		printf("MSG\n");
		DispatchMessage(&msg);
	} 
#endif
	UnhookWindowsHookEx(g_Hook);
	printf("HOOK结束\n");
	//FreeLibrary(GetModuleHandle(L"DLL_Test.dll"));

	return 0;
}

LRESULT CALLBACK KeyBoardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	KBDLLHOOKSTRUCT *Key_Info = (KBDLLHOOKSTRUCT*)lParam;
	if (HC_ACTION == nCode) {
		if (WM_KEYDOWN == wParam) { //如果按键为按下状态
			if (Key_Info->vkCode == 'U') {
				printf("UnHook\n");
				PostQuitMessage(0);
				
			}
			if (Key_Info->vkCode == 'Y') {
				gameMain.m_pGameProc->m_bStop = true;
				printf("流程退出\n");
			}
			if (Key_Info->vkCode == 'E') {
				gameMain.m_pGameProc->m_bStop = true;
				g_Exit = true;
				printf("游戏退出\n");
			}
			if (Key_Info->vkCode == 'P') {
				gameMain.m_pGameProc->m_bPause = true;
				g_Loop = false;
				printf("游戏暂停\n");
			}
			if (Key_Info->vkCode == 'C') {
				gameMain.m_pGameProc->m_bPause = false;
				g_Loop = true;
				printf("游戏继续\n");
			}
			if (Key_Info->vkCode == 'R') {
				gameMain.m_pGameProc->m_bReStart = true;
				printf("游戏重新开始\n");
			}
		}

	}
	return CallNextHookEx(g_Hook, nCode, wParam, lParam);
}

NTSTATUS __stdcall MyNtQueryInformationProcess(HANDLE ProcessHandle, ULONG ProcessInformationClass, PVOID ProcessInformation, UINT32 ProcessInformationLength, UINT32 * ReturnLength)
{
	Hook::GetInstance()->Detach((PBYTE)gNtQueryInformationProcess + NTQIP_OFFSET);
	NTSTATUS status = gNtQueryInformationProcess(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);
	Hook::GetInstance()->Attach((PBYTE)gNtQueryInformationProcess + NTQIP_OFFSET, MyNtQueryInformationProcess);
	printf("~~~ ProcessInformationClass:%d 状态:%08X ~~~\n", ProcessInformationClass, status);
	if (ProcessInformationClass == 0x07) {
		DWORD v = *(DWORD*)ProcessInformation;
		if (ProcessInformationLength > 0 && v) {
			printf("~~~ 清除远程调试标志:%08X 长度:%d ~~~\n", v, ProcessInformationLength);
			ZeroMemory(ProcessInformation, ProcessInformationLength);
		}
	}

	return status;
}

LONG __stdcall VectorHandler(struct _EXCEPTION_POINTERS * ExceptionInfo)
{
	LONG lResult = EXCEPTION_CONTINUE_SEARCH;
	PEXCEPTION_RECORD pExecptionRecord = ExceptionInfo->ExceptionRecord;
	PCONTEXT pContext = ExceptionInfo->ContextRecord;
	ULONG_PTR* uEsp = (ULONG_PTR*)pContext->Esp;

	if (pExecptionRecord->ExceptionCode != EXCEPTION_BREAKPOINT) {
		return lResult;
	}
	if (pExecptionRecord->ExceptionAddress == HookAddr) {
#if 0
		printf("栈区:0(%08X) 1(%08X[%d]) 2(%08X[%d]).\n", uEsp[0], uEsp[1], uEsp[1], uEsp[2], uEsp[2]);
		// push ebp
		pContext->Esp -= 4;
		*((ULONG_PTR*)pContext->Esp) = pContext->Ebp;
		// 跳到下一条指令
		pContext->Eip += 1;
#else
		//printf("1->EAX:%08X[%.2f] EDX:%08X[%.2f].\n", pContext->Eax, (float)pContext->Eax, pContext->Edx, (float)pContext->Edx);
		// sub eax,edx
		pContext->Eax -= pContext->Edx;
		// 跳到下一条指令
		pContext->Eip += 2;
#endif

#if 0
		uEsp[1] += 10;
		uEsp[2] += 10;
		printf("栈区:0(%08X) 1(%08X[%d]) 2(%08X[%d]) - 修改.\n", uEsp[0], uEsp[1], uEsp[1], uEsp[2], uEsp[2]);
#endif
		lResult = EXCEPTION_CONTINUE_EXECUTION;
	}
	else if (pExecptionRecord->ExceptionAddress == HookGameMap) {
		if (pContext->Eax == 280) {
			printf("280->EAX:%08X[%d] ECX:%08X.\n", pContext->Eax, pContext->Eax, pContext->Ecx);
		}
		if ((pContext->Ecx&0x65C) == 0x65C) {
			//printf("2->EAX:%08X[%d] ECX:%08X.\n", pContext->Eax, pContext->Eax, pContext->Ecx);
			pContext->Eax = NOW_X;
		}
		//pContext->Eax = NOW_X;
		xPtr[0] = NOW_X;
		// mov [ecx],eax
		*((ULONG_PTR*)pContext->Ecx) = pContext->Eax;
		// 跳到下一条指令
		pContext->Eip += 2;

		lResult = EXCEPTION_CONTINUE_EXECUTION;
	}
	else if (pExecptionRecord->ExceptionAddress == HookAddr3 || pExecptionRecord->ExceptionAddress == HookAddr4) {
		if ((pContext->Ecx & 0x65C) == 0x65C) {
			printf("3|4->EAX:%08X ECX:%08X[%d].\n", pContext->Eax, pContext->Ecx, pContext->Ecx);
			pContext->Ecx = NOW_X;
		}
		xPtr[0] = NOW_X;
		// mov [eax+000000B4],ecx
		*((ULONG_PTR*)((PBYTE)pContext->Eax + 0xb4)) = pContext->Ecx;
		// 跳到下一条指令
		pContext->Eip += 6;

		lResult = EXCEPTION_CONTINUE_EXECUTION;
	}
	else if (pExecptionRecord->ExceptionAddress == HookSetPos) { // SetPos函数, 不允许重置
		printf("SetPos:%08X %d,%d %08X.\n", uEsp[0], uEsp[1], uEsp[2], HookSetPos);
#if 0
		pContext->Eip = uEsp[0];
		pContext->Esp += 8; // 两个参数
#else
		xPtr[0] = NOW_X;
		uEsp[1] = NOW_X;
		// push ebp
		pContext->Esp -= 4;
		*((ULONG_PTR*)pContext->Esp) = pContext->Ebp;
		pContext->Eip += 1;
#endif
		lResult = EXCEPTION_CONTINUE_EXECUTION;
	}
	else if (pExecptionRecord->ExceptionAddress == HookMove) { // SetPos函数, 不允许重置
		if (xPtr[0] != uEsp[1]) {
			printf("Move:%08X %d,%d %08X EAX:%08X EBX:%08X ECX:%08X EDX:%08X.\n", uEsp[0], uEsp[1], uEsp[2], HookMove,
				pContext->Eax, pContext->Ebx, pContext->Ecx, pContext->Edx);
		}
		
		// 00CA971D 调用地址
		// 028FD538 - 17B0000 = 114D538 ecx=3drole.dll+114D538
		// 02A2D538 - 18E0000
		//xPtr[0] = NOW_X;
		//uEsp[1] = NOW_X;
		// push ebp
		pContext->Esp -= 4;
		*((ULONG_PTR*)pContext->Esp) = pContext->Ebp;
		pContext->Eip += 1;

		lResult = EXCEPTION_CONTINUE_EXECUTION;
	}
	else if (pExecptionRecord->ExceptionAddress == (PVOID)IAIMANAGER_ADDR
		|| pExecptionRecord->ExceptionAddress == (PVOID)IAIMANAGER_ADDR2) { // SetPos函数, 不允许重置
		if (1) {
			printf("IAI:%08X %08X,%08X %08X EAX:%08X EBX:%08X ECX:%08X EDX:%08X.\n", uEsp[0], uEsp[1], uEsp[2], IAIMANAGER_ADDR,
				pContext->Eax, pContext->Ebx, pContext->Ecx, pContext->Edx);
		}

		// ebx = 05346020
		// 00CA971D 调用地址
		// 028FD538 - 17B0000 = 114D538 ecx=3drole.dll+114D538
		// 02A2D538 - 18E0000
		//xPtr[0] = NOW_X;
		//uEsp[1] = NOW_X;
		// push ebp
		pContext->Esp -= 4;
		*((ULONG_PTR*)pContext->Esp) = pContext->Ebp;
		pContext->Eip += 1;

		lResult = EXCEPTION_CONTINUE_EXECUTION;
	}
	else if (pExecptionRecord->ExceptionAddress == (PVOID)IAIMANAGER_ADDR3) { // SetPos函数, 不允许重置
		if (1) {
			printf("IAI3:%08X %08X,%08X %08X EAX:%08X EBX:%08X ECX:%08X EDX:%08X.\n", uEsp[0], uEsp[1], uEsp[2], IAIMANAGER_ADDR,
				pContext->Eax, pContext->Ebx, pContext->Ecx, pContext->Edx);
		}

		// 00604CC1
		// mov edx,[010265EC] 6
		DWORD* ptr = (DWORD*)0x010265EC;
		pContext->Edx = *ptr;
		pContext->Eip += 6;

		lResult = EXCEPTION_CONTINUE_EXECUTION;
	}


	return lResult;
}

VOID SetVectorBreak(PVOID address, bool set)
{
	DWORD oldProtect;
	if (VirtualProtect(address, 1, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		printf("设置int 3软中断[%08X].\n", address);
		// int 3中断
		*((PBYTE)address) = set ? 0xCC : 0x55;
		VirtualProtect(address, 1, oldProtect, &oldProtect);
	}
}

DWORD __stdcall MyMove(DWORD x, DWORD y)
{
    printf("MyMove:%d,%d\n", x, y);
	return OldMove(x, y);
}

DWORD __stdcall OldMove(DWORD x, DWORD y)
{
	__asm {
		push ebp;
		mov ebp, esp;
		push byte ptr[-1];
		mov eax, PlayerMove;
		add eax, 5;
		jmp eax;
	}
}


/*
  判断DLL是否存在某一进程
  @dwPID 进程ID
  @szDllPath DLL名称
  @return bool
*/
BOOL CheckDllInProcess(DWORD dwPID, LPCTSTR szDllPath, HMODULE* hMod)
{
	BOOL                    bMore = FALSE;
	HANDLE                  hSnapshot = INVALID_HANDLE_VALUE;
	MODULEENTRY32           me = { sizeof(me), };

	if (INVALID_HANDLE_VALUE ==
		(hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID))) { // 获得进程的快照
		return FALSE;
	}
	bMore = Module32First(hSnapshot, &me); // 遍历进程内得的所有模块
	for (; bMore; bMore = Module32Next(hSnapshot, &me)) {
		//_tprintf(L"%ws %ws\n", me.szModule, me.szExePath);
		if (!_tcsicmp(me.szModule, szDllPath) || !_tcsicmp(me.szExePath, szDllPath)) { //模块名或含路径的名相符
			_tprintf(L"%ws %ws %08X\n", me.szModule, me.szExePath, me.hModule);
			if (hMod) {
				*hMod = me.hModule;
			}
			CloseHandle(hSnapshot);
			return TRUE;
		}
		me.modBaseAddr;
	}
	CloseHandle(hSnapshot);
	return FALSE;
}

