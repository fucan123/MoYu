#pragma once
#include <Windows.h>
#include <tlhelp32.h>
#include <psapi.h>

#define VEH_HOOK_MAX 10

typedef void(WINAPI* VehHandler)(PEXCEPTION_RECORD pRecord, PCONTEXT pContext, ULONG_PTR* Esp);

typedef struct veh_hook_data
{
	PVOID      Address; // 函数地址
	VehHandler Handler; // 处理程序
	BYTE       OldData; // 原始数据
	BOOL       IsHook;  // 是否HOOK了
} VehHookData;

class Veh
{
public:
	// 安装VEH处理程序
	static VOID Intall(ULONG First=1);
	// VEH处理程序
	static LONG WINAPI VectorHandler(struct _EXCEPTION_POINTERS* ExceptionInfo);
	// 添加HOOK地址
	static VOID AddHookAddress(PVOID address, VehHandler handler, BOOL hook=TRUE);
	// 移除HOOK地址
	static VOID RemoveHookAddress(PVOID address);
private:
	static LONG m_lCount; // HOOK的数量
	static VehHookData m_HookList[VEH_HOOK_MAX]; // HOOK列表
	// 设置断点
	static VOID SetBreak(VehHookData& data, BOOL remove = FALSE);
};