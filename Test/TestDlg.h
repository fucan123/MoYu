
// TestDlg.h: 头文件
//

#pragma once
#include <My/Win32/Peb.h>
#include <My/Win32/Hook.h>
#include "Veh.h"

#define NTQIP_OFFSET 0x05

LONG WINAPI VectorHandler(_EXCEPTION_POINTERS* ExceptionInfo);
NTSTATUS WINAPI MyNtQueryInformationProcess 
(HANDLE ProcessHandle, ULONG ProcessInformationClass,
	PVOID ProcessInformation, UINT32 ProcessInformationLength,
	UINT32* ReturnLength);
// CTestDlg 对话框
class CTestDlg : public CDialogEx
{
// 构造
public:
	CTestDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TEST_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	static void WINAPI VehNtQIP(PEXCEPTION_RECORD pRecord, PCONTEXT pContext, ULONG_PTR* Esp);
};

VOID SetVectorBreak2(PVOID address, bool set);
void setHwBreakpoint(PVOID address);

