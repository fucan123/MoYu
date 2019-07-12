#include "Veh.h"
#include <stdio.h>

LONG Veh::m_lCount = 0;
VehHookData Veh::m_HookList[VEH_HOOK_MAX];

// 安装VEH处理程序
VOID Veh::Intall(ULONG First)
{
	ZeroMemory(m_HookList, sizeof(m_HookList));
	PVOID p = AddVectoredExceptionHandler(1, VectorHandler);
	printf("AddVectoredExceptionHandler:%08X %d\n", p, GetLastError());
}

// VEH处理程序
LONG WINAPI Veh::VectorHandler(_EXCEPTION_POINTERS* ExceptionInfo)
{
	printf("Veh::VectorHandlerx\n");
	LONG lResult = EXCEPTION_CONTINUE_SEARCH;
	PEXCEPTION_RECORD pExecptionRecord = ExceptionInfo->ExceptionRecord;
	PCONTEXT pContext = ExceptionInfo->ContextRecord;
	ULONG_PTR* uEsp = (ULONG_PTR*)pContext->Esp;
	
	if (pExecptionRecord->ExceptionCode != EXCEPTION_BREAKPOINT) {
		return lResult;
	}

#if 1
	for (LONG i = 0; i < VEH_HOOK_MAX; i++) {
		if (pExecptionRecord->ExceptionAddress == m_HookList[i].Address) {
			if (m_HookList[i].Handler) {
				m_HookList[i].Handler(pExecptionRecord, pContext, uEsp);
			}
			lResult = EXCEPTION_CONTINUE_EXECUTION;
			break;
		}
	}
#endif
	return lResult;
}

// 添加HOOK地址
VOID Veh::AddHookAddress(PVOID address, VehHandler handler, BOOL hook)
{
	if (!hook)
		return;

	for (LONG i = 0; i < VEH_HOOK_MAX; i++) {
		if (!m_HookList[i].Address) {
			m_HookList[i].Address = address;
			m_HookList[i].Handler = handler;
			m_HookList[i].IsHook = TRUE;
			// 设置断点
			SetBreak(m_HookList[i], FALSE);
			return;
		}
	}
}

// 移除HOOK地址
VOID Veh::RemoveHookAddress(PVOID address)
{
	for (LONG i = 0; i < VEH_HOOK_MAX; i++) {
		if (m_HookList[i].Address == address) {
			// 移除断点
			SetBreak(m_HookList[i], TRUE);
			return;
		}
	}
}

// 设置断点
VOID Veh::SetBreak(VehHookData& data, BOOL remove)
{
	PBYTE ptr = (PBYTE)data.Address;
	DWORD oldProtect;
	if (VirtualProtect(ptr, 1, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		printf("SetBreak:%08X %02X\n", ptr, remove&0xff);
		if (!remove) { // int 3中断
			data.OldData = *ptr;
			*ptr = 0xcc;
		}
		else { // 移除断点
			*ptr = data.OldData;
			data.Address = NULL;
			data.Handler = NULL;
			data.OldData = 0xcc;
			data.IsHook = FALSE;
		}
		
		VirtualProtect(ptr, 1, oldProtect, &oldProtect);
	}
}
