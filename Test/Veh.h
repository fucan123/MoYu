#pragma once
#include <Windows.h>
#include <tlhelp32.h>
#include <psapi.h>

#define VEH_HOOK_MAX 10

typedef void(WINAPI* VehHandler)(PEXCEPTION_RECORD pRecord, PCONTEXT pContext, ULONG_PTR* Esp);

typedef struct veh_hook_data
{
	PVOID      Address; // ������ַ
	VehHandler Handler; // �������
	BYTE       OldData; // ԭʼ����
	BOOL       IsHook;  // �Ƿ�HOOK��
} VehHookData;

class Veh
{
public:
	// ��װVEH�������
	static VOID Intall(ULONG First=1);
	// VEH�������
	static LONG WINAPI VectorHandler(struct _EXCEPTION_POINTERS* ExceptionInfo);
	// ���HOOK��ַ
	static VOID AddHookAddress(PVOID address, VehHandler handler, BOOL hook=TRUE);
	// �Ƴ�HOOK��ַ
	static VOID RemoveHookAddress(PVOID address);
private:
	static LONG m_lCount; // HOOK������
	static VehHookData m_HookList[VEH_HOOK_MAX]; // HOOK�б�
	// ���öϵ�
	static VOID SetBreak(VehHookData& data, BOOL remove = FALSE);
};