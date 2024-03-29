
// TestDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "Test.h"
#include "TestDlg.h"
#include "afxdialogex.h"
#include <My/Common/func.h>
#include <My/Win32/PE.h>
#include <Dbghelp.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

pfnNtQueryInformationProcess gNtQueryInformationProcess=NULL;

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CTestDlg 对话框



CTestDlg::CTestDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TEST_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CTestDlg::OnBnClickedOk)
END_MESSAGE_MAP()

int WINAPI test(int x, int y)
{
	int z = x;
	int g = y;
	return z;
}
// CTestDlg 消息处理程序

typedef void(__thiscall* th)();

BOOL CTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

#if  1
	AllocConsole();
	freopen("CON", "w", stdout);
	
	SYSTEMTIME stLocal;
	::GetLocalTime(&stLocal);

	PVOID MBW =  GetProcAddress(GetModuleHandle(L"user32.dll"), "MessageBoxW");
	printf("%08X=%08X %d:%d:%d\n", MBW, ::MessageBoxW, stLocal.wHour, stLocal.wMinute, stLocal.wSecond);
	//::MessageBoxA(NULL, "XXX", "X", MB_OK);
	
	//LoadLibraryW(L"E:\\CPP\\DLL_Test\\Release\\DLL_Test.dll");
#endif //  _DEBUG
	HANDLE hProcess = ::GetCurrentProcess();
	BOOL bSource;
	IsWow64Process(GetCurrentProcess(), &bSource);
	printf("bSource:%02X %08X %08X\n", bSource&0xff, hProcess, GetCurrentProcess());
	BOOL r = CloseHandle(hProcess);

	HWND hwnd = ::FindWindow(NULL, L"【魔域】");
	HWND hwnd2 = ::FindWindowEx(hwnd, NULL, NULL, NULL);
	HWND hwnd3 = ::FindWindowEx(hwnd2, NULL, NULL, NULL);
	printf("%08X->%08X->%08X RRR:%d\n", hwnd, hwnd2, hwnd3, r);
#if 0
	HMODULE hMod = GetModuleHandle(L"ntdll.dll");
	DWORD func = (DWORD)GetProcAddress(hMod, "NtQueryInformationProcess");
	printf("ZwTestAlert偏移:%08X,模块地址:%08X,函数地址:%08X.\n", func - (DWORD)hMod, hMod, func);
	func = (DWORD)GetProcAddress(hMod, "NtAllocateVirtualMemory");
	printf("ZwAllocVirtualMemory偏移:%08X,模块地址:%08X,函数地址:%08X.\n", func - (DWORD)hMod, hMod, func);
	// 02A30208
	gNtQueryInformationProcess = (pfnNtQueryInformationProcess)GetNtdllProcAddress("NtQueryInformationProcess");
	printf("NtQueryInformationProcess:%08X\n", gNtQueryInformationProcess);
	//gNtQueryInformationProcess(0, 0, 0, 0, 0);
	test(1, 2);
	//AddVectoredExceptionHandler(1, ::VectorHandler);
	//SetVectorBreak2(gNtQueryInformationProcess, true);
	//setHwBreakpoint(gNtQueryInformationProcess);
	printf("XXXXXXXXX222\n");
	//Hook::GetInstance()->Attach((PBYTE)gNtQueryInformationProcess+NTQIP_OFFSET, MyNtQueryInformationProcess);
	//Veh::Intall();
	//Veh::AddHookAddress(MessageBoxA, VehNtQIP);
	//MessageBoxA(NULL, "xxx", "", MB_OK);
	//gNtQueryInformationProcess(0, 0, 0, 0, 0);


	pfnZwSetInformationThread zstift = (pfnZwSetInformationThread)GetNtdllProcAddress("ZwSetInformationThread");
	printf("ZwSetInformationThread:%08X\n", zstift);
	DWORD size;
	//zstift(GetCurrentThread(), (THREADINFOCLASS)17, 0, 0);
	while (false) {
		BOOL bIsDebugger = IsDebuggerPresent();
		BOOL bMyIsDebugger = 0;
		BOOL bRemoteDebugger = 0;
		DWORD dwPeb;
		ULONG32 dwPeb2;
		ULONG64 dwPeb3;
		ULONG ulNtGlobalFlag, ulNtGlobalFlag2;

		BYTE op = *(PBYTE)gNtQueryInformationProcess;
		DWORD data = *(DWORD*)((PBYTE)gNtQueryInformationProcess + 1);
		//printf("%02X %08X\n", op&0xff, data);

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
		printf("PEB:%08X DEBUG:%02X | ", dwPeb, bMyIsDebugger&0xff);
		printf("PEB:%08X DEBUG:%02X NtGF:%08X | ", dwPeb2, IsDebugger32(dwPeb2)&0xff, ulNtGlobalFlag);
		printf("PEB:%08X DEBUG:%02X NtGF:%08X | ", (ULONG32)dwPeb3, IsDebugger64(dwPeb3) &0xff, ulNtGlobalFlag2);
		printf("bRemoteDebugger:%02X ", bRemoteDebugger & 0xff);
		//printf("IsDebuggerPresent:%02X ", bIsDebugger & 0xff);

		printf("---\n");

		if (IsDebugger64(dwPeb3)) {
			printf("--- 清除调试标志 ---\n");
			ZeroDebugger64(dwPeb3);
		}

		Sleep(3000);
	}
#endif
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CTestDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	//LoadLibrary(L"E:\\CPP\\DLL_Test\\Debug\\DLL_Test.dll");
	ReadGameMemory();
	//CDialogEx::OnOK();
}

// 读取游戏内存
bool CTestDlg::ReadGameMemory()
{
	CString str;
	GetDlgItem(IDC_EDIT_XXX)->GetWindowText(str);
	DWORD pid = SGetProcessId(L"soul.exe");
	printf("PID:%d\n", pid);
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!hProcess) {
		printf("没有权限打开\n");
		return false;
	}
	DWORD dwSize = 0;
#if 0
	HANDLE hMod = GetModuleBaseAddr(pid, L"3drole.dll", &dwSize);
#else
	HANDLE hMod = GetModuleBaseAddr(pid, L"soul.exe", &dwSize);
#endif
	printf("3drole.dll:%08X %08X\n", hMod, dwSize);
	BYTE* buffer = new BYTE[dwSize];
	if (dwSize) {

		DWORD page = 0x1000;
		DWORD addr = 0;
		m_pReadBuffer = new BYTE[page];

		while (addr < dwSize) {
			m_dwReadBase = (DWORD)hMod + addr;
			m_dwReadSize = page;
			// 4:0x6AEC8B55 4:0xBE7468FF 4:0xA1640326 4:0x00000000 4:0x25896450 4:0x00000000 4:0x04D8EC81 4:0x56530000
#if 0
			if (ReadProcessMemory(hProcess, (LPVOID)m_dwReadBase, &buffer[addr], m_dwReadSize, NULL)) {
#else
			if (ReadProcessMemory(hProcess, (LPVOID)m_dwReadBase, m_pReadBuffer, m_dwReadSize, NULL)) {
				// 4:0x6AEC8B55 4:0xBE7468FF 4:0xA1640326 4:0x00000000 4:0x25896450 4:0x00000000 4:0x04D8EC81 4:0x56530000
#if 0
				DWORD codes[] = {
					0x0C8D006A, 0x04E1C192, 0x948BCA2B, 0x00000011, //0x0016948E,
					0x52C88B00, 0x013897FF, 0x006A0000, 0x6BE8CE8B
				};
#endif
				DWORD codes[] = {
					0x006A006A, 0x1234006A, 0x1234006A, 0x016A068B,
					0x1234EB50, 0x51006A0E
				};
				DWORD address;
				if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address, 1, 1)) {
					DWORD ecx_offset, offset;
					ReadProcessMemory(hProcess, (LPVOID)(address + 0x1A), &ecx_offset, 4, NULL);
					ReadProcessMemory(hProcess, (LPVOID)(address + 0x1F), &offset, 4, NULL);
					printf("地址:%08X %08X %08X %08X\n", address, ecx_offset, offset, address + 0x23+offset);
				}
#endif
			}
			else {
				printf("读取失败了:%d\n", GetLastError());
			}

			addr += page;
		}
		
		printf("读取完毕\n");
		//GetDllFunctionAddress32(NULL, buffer, hMod);
		printf("GetDllFunctionAddress32读取完毕\n");
	}
	return true;

	MEMORY_BASIC_INFORMATION mbi;
	memset(&mbi, 0, sizeof(MEMORY_BASIC_INFORMATION));
	DWORD_PTR MaxPtr = 0x30000000; // 最大读取内存地址
	DWORD_PTR max = 0;

	DWORD_PTR ReadAddress = 0x00000000;
	ULONG count = 0, failed = 0;
	//printf("fuck\n");
	while (ReadAddress < MaxPtr)
	{
		SIZE_T r = VirtualQueryEx(hProcess, (LPCVOID)ReadAddress, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
		SIZE_T rSize = 0;

		//printf("r:%d\n", r);
		//printf("ReadAddress:%08X - %08X-----%X\n", ReadAddress, ReadAddress + mbi.RegionSize, mbi.RegionSize);
		if (r == 0) {
			//DPrint(("r:%lld -- %p\n", r, ReadAddress));
			break;
		}
		if (mbi.Type != MEM_PRIVATE && mbi.Protect != PAGE_READWRITE) {
			//printf("%p-%p ===跳过, 大小:%d\n", ReadAddress, ReadAddress + mbi.RegionSize, mbi.RegionSize);
			goto _c;
		}
		else {
			m_dwReadBase = ReadAddress;
			m_dwReadSize = mbi.RegionSize;
			m_pReadBuffer = new BYTE[m_dwReadSize];

			if (ReadProcessMemory(hProcess, (LPVOID)ReadAddress, m_pReadBuffer, m_dwReadSize, NULL)) {
				// 4:0x6AEC8B55 4:0xBE7468FF 4:0xA1640326 4:0x00000000 4:0x25896450 4:0x00000000 4:0x04D8EC81 4:0x56530000
				DWORD codes[] = {
					0x6AEC8B55
				};
				DWORD address;
				if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address, 1, 1)) {
					printf("地址:%08X\n", address);
				}
			}
			else {
				//printf("读取失败了:%d\n", GetLastError());
			}

			delete m_pReadBuffer;
			count++;
		}
	_c:
		// 读取地址增加
		ReadAddress += mbi.RegionSize;
		//printf("%016X---内存大小2:%08X\n", ReadAddress, mbi.RegionSize);
		// 扫0x10000000字节内存 休眠100毫秒
	}
	printf("完成\n");
	m_bIsReadEnd = true;
	return true;
}

// 搜索特征码
DWORD CTestDlg::SearchCode(DWORD* codes, DWORD length, DWORD* save, DWORD save_length, DWORD step)
{
	if (length == 0 || save_length == 0)
		return 0;

	DWORD count = 0;
	for (DWORD i = 0; i < m_dwReadSize; i += step) {
		if ((i + length) > m_dwReadSize)
			break;

		DWORD* dw = (DWORD*)(m_pReadBuffer + i);
		bool result = true;
		for (DWORD j = 0; j < length; j++) {
			if (codes[j] == 0x11) { // 不检查
				result = true;
			}
			else if (codes[j] == 0x22) { // 需要此值不为0
				if (dw[j] == 0) {
					result = false;
					break;
				}
			}
			else if ((codes[j] & 0xffff0000) == 0x12340000) { // 低2字节相等
				if ((dw[j] & 0x0000ffff) != (codes[j] & 0x0000ffff)) {
					result = false;
					break;
				}
				else {
					//printf("%08X\n", dw[j]);
				}
			}
			else if ((codes[j] & 0x0000ffff) == 0x00001234) { // 高2字节相等
				if ((dw[j] & 0xffff0000) != (codes[j] & 0xffff0000)) {
					result = false;
					break;
				}
			}
			else {
				if ((codes[j] & 0xff00) == 0x1100) { // 比较两个地址数值相等 最低8位为比较索引
					//printf("%X:%X %08X:%08X\n", j, codes[j] & 0xff, dw[j], dw[codes[j] & 0xff]);
					if (dw[j] != dw[codes[j] & 0xff]) {
						result = false;
						break;
					}
				}
				else if (dw[j] != codes[j]) { // 两个数值不相等
					result = false;
					break;
				}
			}
		}

		if (result) {
			save[count++] = m_dwReadBase + i;
			//printf("地址:%08X   %08X\n", m_dwReadBase + i, m_pGuaiWus[0]);
			if (count == save_length)
				break;
		}
	}

	return count;
}

void __stdcall CTestDlg::VehNtQIP(PEXCEPTION_RECORD pRecord, PCONTEXT pContext, ULONG_PTR * Esp)
{
	//77D7AA40 B8 19 00 00 00       mov         eax,19h 
	printf("VehNtQIP\n");
	pContext->Eax = 0x19;
	pContext->Eip += 5;
}

LONG WINAPI VectorHandler(_EXCEPTION_POINTERS* ExceptionInfo)
{
	LONG lResult = EXCEPTION_CONTINUE_SEARCH;
	PEXCEPTION_RECORD pExecptionRecord = ExceptionInfo->ExceptionRecord;
	PCONTEXT pContext = ExceptionInfo->ContextRecord;
	ULONG_PTR* uEsp = (ULONG_PTR*)pContext->Esp;

	printf("VectorHandler:%08X\n", pExecptionRecord->ExceptionCode);
	if (pExecptionRecord->ExceptionCode != EXCEPTION_BREAKPOINT 
		&& pExecptionRecord->ExceptionCode != EXCEPTION_SINGLE_STEP) {
		return lResult;
	}

	if (pExecptionRecord->ExceptionAddress == gNtQueryInformationProcess) {
		printf("address:%08X\n", pExecptionRecord->ExceptionAddress);
		pContext->Eax = 0x19;
		pContext->Eip = pContext->Eip + 5;
		lResult = EXCEPTION_CONTINUE_EXECUTION;
	}

#if 0
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


VOID SetVectorBreak2(PVOID address, bool set)
{
	DWORD oldProtect;
	if (VirtualProtect(address, 1, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		printf("设置int 3软中断[%08X].\n", address);
		// int 3中断
		int x = 1;
		printf("x2:%d\n", x);
		BYTE ch = 0xcc;
		if (WriteProcessMemory(GetCurrentProcess(), address, &ch, 1, NULL)) {
			printf("写入成功啦\n");
		}
		//__asm {
		//	mov byte ptr address, 0xcc
		//}
		//*((PBYTE)address) = 0xCC;
		VirtualProtect(address, 1, oldProtect, &oldProtect);
	}
}

void setHwBreakpoint(PVOID address) {//设置硬断
	HANDLE hthrad;
	CONTEXT dst_context;

	memset(&dst_context, 0, sizeof(CONTEXT));//清零结构内存
	dst_context.ContextFlags = CONTEXT_ALL;//设置flag
	hthrad = OpenThread(THREAD_ALL_ACCESS, FALSE, GetCurrentThreadId());//这里我直接用的硬编码 实际作用需要全局线程HOOK 我记得DRX可以不用XX每个线程达到目的的=//打开线程
	//SuspendThread(hthrad);//暂停线程
    BOOL getttt = GetThreadContext(hthrad, &dst_context);//获取线程环境(上下文)
	dst_context.Dr0 = (DWORD)address;//这里是HOOK地址
	dst_context.Dr7 = 0x30001;
	//dst_context.Dr7 = 0x405;

	BOOL setttt = SetThreadContext(hthrad, &dst_context);//设置上下文
	printf("getttt:%d setttt:%d\n", getttt, setttt);
	ResumeThread(hthrad);//线程跑起来吧~~~
	CloseHandle(hthrad);//防止句柄泄露 关句柄好习惯~~~
}
