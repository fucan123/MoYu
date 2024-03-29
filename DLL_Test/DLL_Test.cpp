// DLL_Test.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "DLL_Test.h"
#include "Self/GameMain.h"
#include "Self/Game.h"
#include "Self/GameClient.h"
#include "Self/GameProc.h"
#include "Self/Move.h"
#include "Self/Talk.h"
#include <stdio.h>
#include <tchar.h>

bool  g_NoUnStall = true;
HHOOK g_CldHook = NULL;
HHOOK g_Hook = NULL;
bool g_Exit = false;
bool g_Loop = true;
PlayerSetMove PlayerMove = NULL;
PlayerSetMove OldPlayerMove = NULL;
DWORD* xPtr = NULL;
PVOID  HookMove = NULL;
PVOID  HookMagic = NULL;
PVOID  HookBaseAddr = NULL;
PVOID  HookGameMap = NULL;
PVOID  HookAddr3 = NULL;
PVOID  HookAddr4 = NULL;
PVOID  HookSetPos = NULL;
PVOID  HookMessageBoxA = NULL;
PVOID  HookMessageBoxW = NULL;
pfnNtQueryInformationProcess gNtQueryInformationProcess = NULL;

GameMain gameMain;

DWORD WINAPI Run(LPVOID lParam)
{
	//MessageBox(NULL, L"", L"哈哈", MB_OK);
	if (!gameMain.IsMoYu())
		return 0;

	HookMessageBoxA = (PVOID)GetProcAddress(GetModuleHandle(L"user32.dll"), "MessageBoxA");
	AddVectoredExceptionHandler(1, VectorHandler);
	SetVectorBreak(HookMessageBoxA);

if (1 == 0) {
	AllocConsole();
	freopen("CON", "w", stdout);
}
#ifdef _DEBUG
	printf("DEBUG\n");
#else
	printf("Release\n");
#endif
	//printf("主线程ID:%d 当前线程ID:%d\n", lParam, GetCurrentThreadId());
	DWORD _tid = GetProcessThreadList();
#if 1
	g_CldHook = SetWindowsHookExW(WH_KEYBOARD, CldKeyBoardHookProc, NULL, (DWORD)_tid);
	if (g_CldHook)
		printf(("线程HOOK成功\n"));
	else
		printf(("线程HOOK不成功\n"));
#endif;

	CreateThread(NULL, NULL, ConnectServer, NULL, NULL, NULL);
	if (gameMain.Init(_tid, CldKeyBoardHookProc)) {
		//INLOG("进入游戏！！！");
#if 0
		HookMove = (PVOID)gameMain.m_GameCall.Run;
		HookMagic = (PVOID)gameMain.m_GameCall.MagicAttack_XY;
		HookBaseAddr = (PVOID)gameMain.m_GameCall.GetNpcBaseAddr;
		HookMessageBoxW = (PVOID)GetProcAddress(GetModuleHandle(L"user32.dll"), "MessageBoxW");
		//AddVectoredExceptionHandler(1, VectorHandler);
		//SetVectorBreak(HookMove);
		//SetVectorBreak(HookMagic);
		SetVectorBreak(HookMessageBoxW);
#endif

		if (1 == 1) {
		}
		if (1 == 1) {
			gameMain.Run();
			//return 0;
		}
		
		Sleep(5000);
		//gameMain.m_pMove->Run(400, 400);
		while (1 && !g_Exit) {
			if (!g_Loop)
				Sleep(5000);
			if (gameMain.m_pGameProc->m_bReStart)
				gameMain.Run();
			//gameMain.PickUpItem();
#if 1
			// 2BC8E45E-2BC6EE84=1F5DA
			// 01E6A838
			// mov eax,dword ptr ds:[eax+0xFC]
			//printf("DLL::Run搜索NPC\n");
			//gameMain.FindBagAddr();
			//gameMain.ReadGameMemory(0x20);
			//gameMain.m_pTalk->ReadNPC();
			//gameMain.m_pTalk->ReadRemotePlayer();
			Sleep(10000);
			continue;
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

					SelfItem info;
					if (!gameMain.m_pItem->GetSelfItemInfo(items[i], info)) {
						printf("无法获取背包物品信息(%d) %08X\n", GetLastError(), items[i]);
						continue;
					}
					//printf("GameBagItem:%08X\n", items[i]);
					printf("[%02d]物品:%s %08X 类型:%08X ToX:%d ToY:%d\n", i + 1, info.Name, info.Id,
						info.Type, info.ToX, info.ToY);
				}
				printf("\n----------------------------\n");
			}
#endif
#if 1
			GameGroundItem* iii[120];
			gameMain.m_pItem->ReadGroundItems(iii, 120);
			Sleep(5000);
#endif
#if 0
			GameSelfItem* iii[120];
			DWORD count = gameMain.m_pItem->ReadSaveItems(iii, 120);
			for (DWORD x = 0; x < count; x++) {
				printf("仓库物品xxx.[%08X]:%s[%08X]\n", iii[x], iii[x]->Name, iii[x]->Id);
				if (x == 0) {
					
				}	
			}
			printf("幻魔晶石数量:%d\n", gameMain.m_pItem->GetSaveItemCount("幻魔晶石"));
			gameMain.m_pItem->CheckOutItem("幻魔晶石", 6);
			Sleep(5000);
#endif
		}
		//gameMain.Call_Run(850, 510);
		//gameMain.Call_NPC(0x193E2);
		//Sleep(10000);
		//gameMain.Call_NPCTalk(0x00);
	}

	UnhookWindowsHookEx(g_CldHook);
	printf("DLL::Run完成\n");
	return 0;

#define FIND 0
	HWND hWnd = NULL;
#if FIND == 1
	while (!hWnd) {
		hWnd = FindWindow(NULL, L"【魔域】");
		Sleep(5000);
	}
#endif
	WM_COMMAND;
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

	while (false) {
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

	while (false) {
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

DWORD WINAPI ConnectServer(LPVOID p)
{
	//printf("准备连接服务器...\n");
	gameMain.Connect("127.0.0.1", 12379);
	printf("无法连接服务器\n");
	return 0;
}

DWORD WINAPI ReLogin(LPVOID p)
{
	// 重新登录
	gameMain.m_pClient->SendMsg("掉线了,10秒后自动登录");
	gameMain.m_pGameProc->Wait(10 * 1000);
	gameMain.InputUserPwd(false);
	return 0;
}

int GetProcessThreadList() //进程的ID 
{
#if 0
	pfnNtQuerySetInformationThread NtQueryInformationThread  = (pfnNtQuerySetInformationThread)GetNtdllProcAddress("NtQueryInformationThread");
	printf("NtQueryInformationThreadcxx:%08X\n", NtQueryInformationThread);
	if (!NtQueryInformationThread) {
		return 0;
	}
#endif
	DWORD th32ProcessID = GetCurrentProcessId();
	printf("进程ID:%d\n", th32ProcessID);
	HANDLE hThreadSnap;
	THREADENTRY32 th32;
	hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, th32ProcessID);
	if (hThreadSnap == INVALID_HANDLE_VALUE)
	{
		return -1;
	}
	th32.dwSize = sizeof(THREADENTRY32);
	if (!Thread32First(hThreadSnap, &th32)) {
		CloseHandle(hThreadSnap);
		return -1;
	}
	do {
		//printf("%d[%08X] %d[%08X]\n", th32.th32ThreadID, th32.th32ThreadID, th32.th32OwnerProcessID, th32.th32OwnerProcessID);
		if (th32.th32OwnerProcessID == th32ProcessID) {
			DWORD tid = th32.th32ThreadID;
			return tid;
			HANDLE ThreadHandle = OpenThread(THREAD_QUERY_INFORMATION, FALSE, tid);
			if (!ThreadHandle) {
				printf("打开线程失败!\n");
				//continue;
			}

			DWORD dwStaAddr = NULL;
			//NtQueryInformationThread(ThreadHandle, ThreadQuerySetWin32StartAddress, &dwStaAddr, sizeof(dwStaAddr), NULL);
			CloseHandle(ThreadHandle);

			printf("%d[%08X] %08X\n", tid, tid, dwStaAddr);
			if (1 || dwStaAddr < 0x1000)
				return tid;
		}
	} while (Thread32Next(hThreadSnap, &th32));

	CloseHandle(hThreadSnap);
	return 0;
}

LRESULT CALLBACK CldKeyBoardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	//return 0;
	//printf("LRESULT:nCode:%08X WPARAM:%08X LPARAM:%08X %d\n", nCode, wParam, lParam, GetLastError());
	if (lParam & 0x80000000) {
		if (wParam == 'A') {
			gameMain.m_pGameProc->m_bPause = false;
			gameMain.m_pGameProc->m_iChkFile = 1;
			g_Loop = true;
			printf("选择 魔域副本流程.txt\n");
		}
		if (wParam == 'B') {
			gameMain.m_pGameProc->m_bPause = false;
			gameMain.m_pGameProc->m_iChkFile = 2;
			g_Loop = true;
			printf("选择 魔域副本流程2.txt\n");
		}
		if (wParam == 'C') {
			gameMain.m_pGameProc->m_bPause = false;
			g_Loop = true;
			printf("游戏继续\n");
		}
		if (wParam == 'P') {
			gameMain.m_pGameProc->m_bPause = true;
			g_Loop = false;
			printf("游戏暂停\n");
		}
		if (wParam == 'R') {
			gameMain.m_pGameProc->m_bReStart = true;
			printf("流程程序加载\n");
		}
	}
	LRESULT result = CallNextHookEx(g_CldHook, nCode, wParam, lParam);
	return result;
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
				//printf("UnHook\n");
				//PostQuitMessage(0);
				
			}
			if (Key_Info->vkCode == 'Y') {
				//gameMain.m_pGameProc->m_bStop = true;
				//printf("流程退出\n");
			}
			if (Key_Info->vkCode == 'E') {
				//gameMain.m_pGameProc->m_bStop = true;
				//g_Exit = true;
				//printf("游戏退出\n");
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
				//gameMain.m_pGameProc->m_bReStart = true;
				//printf("游戏重新开始\n");
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

	PVOID ea = pExecptionRecord->ExceptionAddress;

	if (pExecptionRecord->ExceptionCode != EXCEPTION_BREAKPOINT) {
		return lResult;
	}
	if (ea == HookMove || ea == HookMagic) {
		if (pContext->Ecx == gameMain.m_Ecx) {
			// push ebp
			pContext->Esp -= 4;
			*((ULONG_PTR*)pContext->Esp) = pContext->Ebp;
			// 跳到下一条指令
			pContext->Eip += 1;
		}
		else {
			// 不执行
			pContext->Eip = uEsp[0];
			pContext->Esp += 0x10; // 三个参数加函数地址

			if (ea == HookMove)
				printf("移动函数ECX参数无效, 不执行 ");
			if (ea == HookMagic) {
				printf("技能函数ECX参数无效, 不执行 ");
				pContext->Esp += 0x08;
			}
				
			printf("ECX:%08X Game ECX:%08X ESP:%08X\n", pContext->Ecx, gameMain.m_Ecx, uEsp);
		}
		
		lResult = EXCEPTION_CONTINUE_EXECUTION;
	}
	else if (ea == HookBaseAddr) {
		printf("ECX:%08X Game ECX:%08X ESP:%08X\n", pContext->Ecx, gameMain.m_Ecx, uEsp);
		// push ebp
		pContext->Esp -= 4;
		*((ULONG_PTR*)pContext->Esp) = pContext->Ebp;
		// 跳到下一条指令
		pContext->Eip += 1;

		lResult = EXCEPTION_CONTINUE_EXECUTION;
	}
	else if (ea == HookMessageBoxA) {
		CHAR* text = (CHAR*)*(uEsp + 0x02);
		printf("HookMessageBoxA:%s %s\n", text, *(uEsp + 0x03));

		if (1 || strstr(text, "连接中断")) {
			gameMain.SetOffLine(1);
			gameMain.m_pClient->SendMsg(text);
			gameMain.m_pClient->Send(SCK_OFFLINE, true);
			// 不执行
			pContext->Eip = uEsp[0];
			pContext->Esp += 0x14; // 四个参数加函数地址

			CreateThread(NULL, NULL, ReLogin, NULL, NULL, NULL);
		}
		else {
			// 跳到下一条指令
			pContext->Eip += 2; // mov edi,edi
		}
		lResult = EXCEPTION_CONTINUE_EXECUTION;
	}
	else if (ea == HookMessageBoxW) {
		printf("HookMessageBoxW:%ws\n", (CHAR*)*(uEsp + 0x02));
		//::MessageBoxW(NULL, L"可能掉线了", L"提示", MB_OK);
		// 跳到下一条指令
		pContext->Eip += 2; // mov edi,edi

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

