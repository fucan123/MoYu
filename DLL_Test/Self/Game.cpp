#include "GameClient.h"
#include <My/Common/mystring.h>
#include <My/Common/func.h>
#include <My/Driver/KbdMou.h>
#include <stdio.h>
#include <time.h>

#include "Game.h"
#include "GameConf.h"
#include "GameProc.h"
#include "Item.h"
#include "Talk.h"
#include "Move.h"
#include "GuaiWu.h"
#include "Magic.h"
#include "Pet.h"

// 游戏g_pObjHero全局变量(CHero类this指针)
DWORD g_pObjHero = NULL;
// 游戏g_objPlayerSet全局变量
DWORD g_objPlayerSet = NULL;

Game* Game::self = NULL;   // Game自身
// ...
Game::Game()
{
	self = this;
	m_iGetPosTime = 0;

	ZeroMemory(&m_GameModAddr, sizeof(GameModAddr));
	ZeroMemory(&m_GameAddr, sizeof(GameAddr));
	ZeroMemory(&m_GameCall, sizeof(GameCall));
	ZeroMemory(&m_Account, sizeof(m_Account));
	//m_Account.IsBig = true;

	m_bIsReadEnd = true;
	m_pReadBuffer = new BYTE[1024 * 1024 * 10];

	m_pClient   = new GameClient(this);
	m_pClient->SetSelf(m_pClient);

	m_pItem     = new Item(this);      // 物品类
	m_pTalk     = new Talk(this);      // 对话类
	m_pMove     = new Move(this);      // 移动类
	m_pGuaiWu   = new GuaiWu(this);    // 怪物类
	m_pMagic    = new Magic(this);     // 技能类
	m_pPet      = new Pet(this);       // 宠物类

	m_pGameConf = new GameConf(this);  // 游戏配置类
	m_pGameProc = new GameProc(this);  // 执行过程类
}

// >>>
Game::~Game()
{
	delete m_pReadBuffer;

	delete m_pClient;
	delete m_pItem;
	delete m_pTalk;
	delete m_pMove;
	delete m_pGuaiWu;
	delete m_pMagic;
	delete m_pPet;

	delete m_pGameProc;

	Drv_DisConnectDriver();
}

// 关闭游戏
void Game::Close(bool leave_team)
{
	if (leave_team) {
		if (Call_IsTeamLeader())
			Call_TeamDismiss();
		else
			Call_TeamLeave();
	}

	m_pGameProc->Wait(3 * 1000);
	printf("关闭游戏\n");
	ExitProcess(0);
	TerminateProcess(GetCurrentProcess(), 0);
	//TerminateProcess(m_hGameProcess, 0);
}

// 连接管理服务
void Game::Connect(const char* host, USHORT port)
{
	printf("准备连接服务器[%s:%d]\n", host, port);
	m_pClient->Connect(host, port);
}

// 登录游戏帐号
void Game::Login()
{
	printf("准备登录...\n");
	m_Account.IsLogin = 0;

	Sleep(1500);
	LeftClick(510, 550); // 进入正式版
	Sleep(1500);

	int x, y;
	GetSerBigClickPos(x, y);
	LeftClick(x, y); // 选择大区
	Sleep(1500);
	GetSerSmallClickPos(x, y);
	LeftClick(x, y); // 选择小区

	InputUserPwd();
}

// 输入帐号密码
void Game::InputUserPwd(bool input_user)
{
	HWND edit = FindWindowEx(m_hWndPic, NULL, NULL, NULL);

	int i;
	if (input_user) {
		Sleep(2000);
		LeftClick(300, 265); // 点击帐号框
		SetForegroundWindow(m_hWnd);
		Sleep(1000);
		//MouseMove(300, 265);
		for (i = 0; i < 20; i++) {
			Keyborad(VK_BACK);
			Sleep(200);
		}
		for (i = 0; i < strlen(m_Account.Name); i++) {
			Keyborad(m_Account.Name[i]);
			Sleep(200);
		}
	}

	LeftClick(300, 305); // 点击密码框
	Sleep(1000);
	for (i = 0; i < strlen(m_Account.Pwd); i++) {
		Keyborad(m_Account.Pwd[i]);
		Sleep(200);
	}
	LeftClick(265, 430); // 进入
}

// 初始化
bool Game::Init(DWORD hook_tid, HOOKPROC hook_key_proc)
{
	printf("Run:%08X\n", &Game::Call_Run);
	m_dwHookTid = hook_tid;
	m_HookKeyProc = hook_key_proc;

	FindGameWnd();

	m_hGameProcess = GetCurrentProcess();
	FindAllModAddr();
	FindCoorAddr();

	m_Account.IsLogin = 1;

	WaitGameInit(60);

	DWORD pid;
	GetWindowThreadProcessId(m_hWnd, &pid);

	printf("开始读取游戏数据...\n");
	m_pClient->SendMsg("读取游戏数据...");

	FindAllCall();
	FindBagAddr();
	
#if 1
	if (!ReadGameMemory()) {
		INLOG("无法获得人物血量地址！！！");
		return false;
	}
	while (!m_GameAddr.ItemPtr) {
		printf("获取地面物品地址...\n");
		ReadGameMemory();
		Sleep(5000);
	}
	while (!m_GameAddr.MapName) {
		printf("获取地图名称地址..\n");
		ReadGameMemory();
		Sleep(5000);
	}
#endif
	m_pMagic->ReadMagic(nullptr, nullptr, false);

	m_pClient->SendMsg("读取游戏数据完成");

	//printf("连接键盘和鼠标驱动%s\n", Drv_ConnectDriver()?"成功":"失败");
#if 0
	while (!m_GameAddr.Life) {
		printf("重新读取血量地址\n");
		ReadGameMemory();
		Sleep(1000);
	}
#endif
	return true;
}

// 等待游戏初始化完毕
void Game::WaitGameInit(int wait_s)
{
	m_pClient->SendMsg("等待游戏初始化完成");
	for (int i = 0; i < wait_s; i++) {
		HWND child, parent;
		if (FindButtonWnd(BUTTON_ID_CLOSEMENU, child, parent, "x")) {
			SendMessageW(parent, WM_COMMAND, MAKEWPARAM(BUTTON_ID_CLOSEMENU, BN_CLICKED), (LPARAM)child); // 关闭它
			printf("---------游戏数据已初始化完毕---------\n");
			m_pClient->SendMsg("游戏数据已初始化完毕");
			break;
		}

		printf("等待游戏数据初始完成, 还剩%02d秒[%d秒].\n", wait_s - i, wait_s);
		Sleep(1000);
	}

	m_pGameProc->Button(BUTTON_ID_CANCEL,    1500);
	m_pGameProc->Button(BUTTON_ID_SURE,      1500);
	m_pGameProc->Button(BUTTON_ID_CLOSEMENU, 1500);
}

// 设置是否是大号
void Game::SetAccount(const char* ser_big, const char* ser_small, const char* name, const char* pwd, int role_no, int getxl, int big, int getxl_logout)
{
	strcpy(m_Account.SerBig, ser_big);
	strcpy(m_Account.SerSmall, ser_small);
	strcpy(m_Account.Name, name);
	strcpy(m_Account.Pwd, pwd);
	m_Account.RoleNo = role_no - 1;
	m_Account.IsGetXL = getxl;
	m_Account.IsBig = big;
	m_Account.GetXLLogout = getxl_logout;
	m_Account.IsReady = true;
	if (big) {
		AllocConsole();
		freopen("CON", "w", stdout);
		m_pGameProc->OpenLogFile();
	}

	printf("游戏服务器:%s.%s\n", ser_big, ser_small);
	printf("帐号:%s|%s 第%d个角色 %s\n", name, pwd, role_no, big?"大号":"小号");
	if (getxl_logout)
		printf("领取项链完后退出\n");

	Login();
}

// 设置是否掉线
void Game::SetOffLine(int v)
{
	m_Account.IsOffLine = v;
}

// 进程是否是魔域
bool Game::IsMoYu()
{
	wchar_t name[32] = { 0 };
	GetProcessName(name, GetCurrentProcessId());
	wprintf(L"%ws = %ws\n", name, L"soul.exe");
	return wcsstr(name, L"soul.exe") != nullptr;
}

// 是否登录了
bool Game::IsLogin()
{
	int length = strlen((const char*)(m_GameModAddr.Mod3DRole + ADDR_ACCOUNT_NAME));
	if (length > 0) {
		strcpy(m_Account.Name, (const char*)(m_GameModAddr.Mod3DRole + ADDR_ACCOUNT_NAME));
		strcpy(m_Account.Role, (const char*)(m_GameModAddr.Mod3DRole + ADDR_ROLE_NAME));
		m_Account.LoginTime = time(nullptr);
	}
	return length > 0;
}

// 是否掉线了 
bool Game::IsOffLine()
{
	return m_Account.IsOffLine;
}

// 比较登录帐号
bool Game::CmpLoginAccount(const char* name)
{
	return strcmp(name, m_Account.Name) == 0;
}

// 是否大号
bool Game::IsBig()
{
	//return true;
	return m_Account.IsBig;
}

// 是否在副本
bool Game::IsInFB()
{
	return IsInMap("阿拉玛的哭泣");
}

// 是否在指定地图
bool Game::IsInMap(const char* name)
{
	char map_name[32] = { 0 };
	if (ReadMemory((PVOID)m_GameAddr.MapName, map_name, sizeof(map_name))) { // 读取地图名称
		return strcmp(map_name, name) == 0;
	}
	return false;
}

// 是否在指定区域坐标 allow=误差
bool Game::IsInArea(int x, int y, int allow)
{
	ReadCoor();
	int cx = (int)m_dwX - x;
	int cy = (int)m_dwY - y;

	return abs(cx) <= allow && abs(cy) <= allow;
}

// 是否不在指定区域坐标 allow=误差
bool Game::IsNotInArea(int x, int y, int allow)
{
	ReadCoor();
	int cx = (int)m_dwX - x;
	int cy = (int)m_dwY - y;

	//printf("IsNotInArea:%d,%d %d,%d\n", m_dwX, m_dwY, x, y);
	return abs(cx) > allow || abs(cy) > allow;
}

// 是否已获取了项链
bool Game::IsGetXL()
{
	if (m_Account.IsGetXL) {
		tm t;
		time_t get_time = m_Account.GetXLTime;
		gmtime_s(&t, &get_time);

		tm t2;
		time_t now_time = time(nullptr);
		gmtime_s(&t2, &now_time);

		if (t.tm_mday != t2.tm_mday) { // 昨天领取的
			m_Account.IsGetXL = 0;
		}
	}
	return m_Account.IsGetXL != 0;
}

// 运行
void Game::Run()
{
	if (0 && !IsBig())
		return;

	m_pGameProc->Run();
}

// 获取游戏中心位置在屏幕上的坐标
void Game::GetGameCenterPos(int & x, int & y)
{
	GetWindowRect(m_hWnd, &m_GameWnd.Rect);
	m_GameWnd.Width = m_GameWnd.Rect.right - m_GameWnd.Rect.left;
	m_GameWnd.Height = m_GameWnd.Rect.bottom - m_GameWnd.Rect.top;

	x = m_GameWnd.Rect.left + (m_GameWnd.Width / 2);
	y = m_GameWnd.Rect.top + (m_GameWnd.Height / 2);
}

// 获得游戏窗口
HWND Game::FindGameWnd()
{
	while (true) {
		DWORD pid = GetCurrentProcessId();
		::EnumChildWindows(m_hWnd, EnumProc, (LPARAM)&pid);
		if (pid != GetCurrentProcessId()) {
			m_hWnd = (HWND)pid;
			while (true) {
				m_hWnd2 = FindWindowEx(m_hWnd, NULL, NULL, NULL);
				if (m_hWnd2) {
					while (true) {
						m_hWndPic = FindWindowEx(m_hWnd2, NULL, NULL, NULL);
						if (m_hWndPic)
							break;
						Sleep(1000);
					}
					break;
				}
				Sleep(1000);
			}
			break;
		}
		Sleep(5000);
	}
	
	m_fScale = ::GetScale();
	GetWindowRect(m_hWnd, &m_GameWnd.Rect);
	m_GameWnd.Width = m_GameWnd.Rect.right - m_GameWnd.Rect.left;
	m_GameWnd.Height = m_GameWnd.Rect.bottom - m_GameWnd.Rect.top;
	printf("窗口句柄:%0x %d %d %.2\n", m_hWnd, m_GameWnd.Width, m_GameWnd.Height, m_fScale);
	printf("游戏画面窗口:%08X\n", m_hWndPic);

	m_pClient->SendOpen(m_fScale, m_hWnd, m_GameWnd.Rect); // 通知窗口已打开
	return m_hWnd;
}

// 获取所有模块地址
void Game::FindAllModAddr()
{
	m_GameModAddr.Mod3DRole = FindModAddr(MOD_3drole);
	m_GameModAddr.Mod3DGameMap = FindModAddr(MOD_3dgamemap);
}

// 获取游戏所有CALL
void Game::FindAllCall()
{
	SearchModFuncMsg info[] = {
		{"?g_pObjHero",            NULL,  0, &g_pObjHero,                  "g_pObjHero全局变量"},
		{"?g_objPlayerSet",        NULL,  0, &g_objPlayerSet,              "g_objPlayerSet全局变量"},
		{"?ReBorn@CHero",          NULL,  0, &m_GameCall.ReBorn,           "人物复活"},
		{"?Run@CHero",             NULL,  0, &m_GameCall.Run,              "移动函数"},
		{"?ActiveNpc@CHero",       NULL,  0, &m_GameCall.ActiveNpc,        "NPC对话"},
		{"?GetInstance@ITaskManager",NULL,0, &m_GameCall.ITaskGetInstance, "NPC对话选择"},
		{"?GetPackageItemByIndex@CHero",  NULL, 0, &m_GameCall.GetPkageItemByIndex, "获取物品指针"},
		{"?UseItem@CHero",         NULL,  0, &m_GameCall.UseItem,          "使用物品"},
		{"?DropItem@CHero",        NULL,  0, &m_GameCall.DropItem,         "丢弃物品"},
		{"?PickUpItem@CHero",      NULL,  0, &m_GameCall.PickUpItem,       "捡拾物品"},
		{"?SellItem@CHero",        NULL,  0, &m_GameCall.SellItem,         "卖物品"},
		{"?SaveMoney@CHero",       NULL,  0, &m_GameCall.SaveMoney,        "存钱"},
		{"?CheckInItem@CHero",     NULL,  0, &m_GameCall.CheckInItem,      "存入远程仓库"},
		{"?CheckOutItem@CHero",    NULL,  0, &m_GameCall.CheckOutItem,     "取出远程仓库"},
		{"?OpenBank@CHero",        NULL,  0, &m_GameCall.OpenBank,         "打开远程仓库"},
		{"?TransmByMemoryStone@CHero",    NULL,  0, &m_GameCall.TransmByMemoryStone, "使用可传送物品"},
		{"?MagicAttack@CHero",     "POS", 0, &m_GameCall.MagicAttack_GWID, "使用技能-怪物ID "},
		{"?MagicAttack@CHero",     "POS", 1, &m_GameCall.MagicAttack_XY,   "使用技能-XY坐标"},
		{"?CallEudenmon@CHero",    NULL,  0, &m_GameCall.CallEudenmon,     "宠物出征"},
		{"?KillEudenmon@CHero",    NULL,  0, &m_GameCall.KillEudenmon,     "宠物召回"},
		{"?AttachEudemon@CHero",   NULL,  0, &m_GameCall.AttachEudemon,    "宠物合体"},
		{"?UnAttachEudemon@CHero", NULL,  0, &m_GameCall.UnAttachEudemon,  "宠物解体"},
		{"?GetData@CPlayer",       NULL,  0, &m_GameCall.SetRealLife,      "CPlayer::GetData"},
		{"?QueryInterface_RemoteTeam@CHero", NULL,  0, &m_GameCall.QueryInf_RemoteTeam,"获取远程邀请人物基址"},
		{"?IsHaveTeam@CHero",      NULL,  0, &m_GameCall.IsHaveTeam,       "是否有队伍"},
		{"?IsTeamLeader@CHero",    NULL,  0, &m_GameCall.IsTeamLeader,     "是否是队长"},
		{"?TeamCreate@CHero",      NULL,  0, &m_GameCall.TeamCreate,       "创建队伍"},
		{"?TeamDismiss@CHero",     NULL,  0, &m_GameCall.TeamDismiss,      "离开队伍[队长]"},
		{"?TeamLeave@CHero",       NULL,  0, &m_GameCall.TeamLeave,        "离开队伍[队员]"},
		{"?TeamInvite@CHero",      NULL, 0, &m_GameCall.TeamInvite,        "邀请加入"},
		{"?SetAutoJoinStatus@CHero",NULL, 0, &m_GameCall.TeamAutoJoin,     "自动加入"},
	};
	DWORD length = sizeof(info) / sizeof(SearchModFuncMsg);
	for (DWORD i = 0; i < length; i++) {
		SearchFuncInMode(&info[i], (HANDLE)m_GameModAddr.Mod3DRole);
	}

	printf("\n--------------------------------\n");
	m_GameCall.NPCTalk = FindNPCTalkCall();
	m_GameCall.TeamChk = FindTeamChkCall();
	m_GameCall.KeyNum = FindKeyNumCall();
	m_GameCall.CloseTipBox = FindCloseTipBoxCall();
	m_GameCall.GetNpcBaseAddr = FindGetNpcBaseAddr();
	printf("\n--------------------------------\n");

	m_Ecx = P2DW(g_pObjHero);
}

// 获取NPC对话函数
DWORD Game::FindNPCTalkCall()
{
	// 从push 0x00开始搜索
	DWORD codes[] = {
		0x0C8D006A, 0x04E1C192, 0x948BCA2B, 0x00000011, //0x0016948E,
		0x52C88B00, 0x013897FF, 0x006A0000, 0x1234CE8B
	};

	DWORD address = 0;
	if (SearchInMod(L"soul.exe", codes, sizeof(codes) / sizeof(DWORD), &address, 1, 1)) {
		DWORD offset = P2DW(address + 0x1F);
		address += offset + 0x23;
		printf("CallNPC对话选择2:%08X\n", address);
	}
	return address;
}

// 获取副本邀请队伍选择框函数
DWORD Game::FindTeamChkCall()
{
	// 从push 0xFFFFFF开始搜索
	DWORD codes[] = {
		0xFFFFFF68, 0x12346800, 0xC88B1234, 0x123492FF,
		0x8E8B0000, 0x00001234, 0x1234E850, 0x016A1234
	};

	DWORD address = 0;
	if (SearchInMod(L"soul.exe", codes, sizeof(codes) / sizeof(DWORD), &address, 1, 1)) {
		DWORD tmp = address;
		m_GameAddr.TeamChkOffset = P2DW(address + 0x22);
		DWORD offset = P2DW(address + 0x27);
		address += offset + 0x2B;
		printf("Call副本邀请队伍选择:%08X %08X\n", address, tmp);
		printf("副本邀请队伍选择ESI偏移:%08X\n", m_GameAddr.TeamChkOffset);
	}
	return address;
}

// 数字按键函数
DWORD Game::FindKeyNumCall()
{
	// 从push 0x00开始搜索
	DWORD codes[] = {
		0x006A006A, 0x1234006A, 0x1234006A, 0x016A068B,
		0x1234EB50, 0x51006A0E
	};

	DWORD address = 0;
	if (SearchInMod(L"soul.exe", codes, sizeof(codes) / sizeof(DWORD), &address, 1, 1)) {
		DWORD tmp = address;
		m_GameAddr.KeyNumEcxPtr = P2DW(address + 0x1A);
		DWORD offset = P2DW(address + 0x1F);
		address += offset + 0x23;
		printf("Call数字按键:%08X %08X\n", address, tmp);
		printf("数字按键ECX指针地址:%08X\n", m_GameAddr.KeyNumEcxPtr);
	}
	return address;
}

// 获取关闭提示框函数
DWORD Game::FindCloseTipBoxCall()
{
	// mov dword ptr ds:[ebx+0xA0],esi
	// 搜索方法->CE提示框状态地址下修改断点->找到修改地址->OD下断点, 查看调用堆栈即可找到
	DWORD codes[] = {
		0x51EC8B55, 0x8B575653, 0xFF83087D, 0x0FD98B0A,
		0x00000011, 0x0C751234
	};

	DWORD address = 0;
	if (SearchInMod(L"soul.exe", codes, sizeof(codes) / sizeof(DWORD), &address, 1, 1)) {
		printf("Call关闭提示框函数地址:%08X\n", address);
	}
	return address;
}

// 获取获取NPC基地址函数
DWORD Game::FindGetNpcBaseAddr()
{
	// 搜索方法 找到NPC基址->下访问断点，点击NPC身上的断点, 找到合适断点下断
	// [在CGamePlayerSet::Process里面]mov dword ptr ds:[ecx+0x44],edx
	// 里面寻找即可找到 add ecx,0x14
	DWORD codes[] = {
		0x83EC8B55, 0x4D8924EC, 0x08458BDC, 0xE04D8D50,
		0xF0558D51, 0xDC4D8B52
	};

	DWORD address = 0;
	if (SearchInMod(L"3drole.dll", codes, sizeof(codes) / sizeof(DWORD), &address, 1, 1)) {
		printf("Call获取获取NPC基地址:%08X\n", address);
	}
	return address;
}

// 获取模块地址
DWORD Game::FindModAddr(LPCWSTR name)
{
	HANDLE hMod = NULL;
	while (hMod == NULL) {
		hMod = GetModuleHandle(name);
		wprintf(L"%ws:%08X\n", name, hMod);
	}
	return (DWORD)hMod;
}

// 获取坐标地址
bool Game::FindCoorAddr()
{
	while (!IsLogin()) {
		printf("等待登录游戏！！！\n");
		Sleep(5000);
	}

	try {
		DWORD p;
		//ReadDwordMemory(0x123, p);
	}
	catch (...) {
		printf("错误PPPPPPPPPPPPPPPPPPPPPPPPP\n");
	}

	m_GameAddr.CoorX = m_GameModAddr.Mod3DRole + ADDR_COOR_X_OFFSET;
	m_GameAddr.CoorY = m_GameModAddr.Mod3DRole + ADDR_COOR_Y_OFFSET;
	while (PtrToDword(m_GameAddr.CoorX) == 0 || PtrToDword(m_GameAddr.CoorY) == 0) {
		printf("正在进入游戏！！！\n");
		HWND child, parent;
		if (FindButtonWnd(BUTTON_ID_LOGIN, child, parent)) { // 有选择人物登入按钮
			//printf("发现选择角色按钮\n");
			if (1 && m_Account.RoleNo > 0) {
				printf("选择角色:%d\n", m_Account.RoleNo + 1);
				HWND child2, parent2;
				int btn_id = BUTTON_ID_ROLENO + m_Account.RoleNo;
				FindButtonWnd(btn_id, child2, parent2, "角色");
				SendMessageW(parent2, WM_COMMAND, MAKEWPARAM(btn_id, BN_CLICKED), (LPARAM)child2); // 选择角色
				
				RECT rect;
				GetWindowRect(child2, &rect);
				//Drv_LeftClick(float(rect.left + 20) / m_fScale, float(rect.top + 20) / m_fScale);
				printf("按钮ID:%X %08X %08X %d %d %d %d\n", btn_id, child2, parent2, rect.left, rect.top, rect.right, rect.bottom);
				Sleep(500);
				GetWindowRect(child, &rect);
				//Drv_LeftClick(float(rect.left + 10) / m_fScale, float(rect.top + 10) / m_fScale);
				printf("按钮ID2:%X %08X %08X %d %d %d %d\n", btn_id, child, parent, rect.left, rect.top, rect.right, rect.bottom);
				SendMessageW(parent, WM_COMMAND, MAKEWPARAM(BUTTON_ID_LOGIN, BN_CLICKED), (LPARAM)child); // 登入
			}
		}
		Sleep(1500);
	}
	printf("人物坐标:%d,%d\n", PtrToDword(m_GameAddr.CoorX), PtrToDword(m_GameAddr.CoorY));

	strcpy(m_Account.Name, (const char*)(m_GameModAddr.Mod3DRole + ADDR_ACCOUNT_NAME));
	strcpy(m_Account.Role, (const char*)(m_GameModAddr.Mod3DRole + ADDR_ROLE_NAME));
	printf("帐号:%s 角色:%s\n", m_Account.Name, m_Account.Role);
	m_pClient->SendInGame(m_Account.Name, m_Account.Role); // 通知人物已上线
	return true;
}

// 获取移动状态地址
bool Game::FindMoveStaAddr()
{
#if 0
	DWORD codes[] = {
		0x000064, 0x000064, 0x000000, 0x000000, 0x0FD308, 0x000000, 0x000000, 0x000001,
		0x0F0084, 0x000000, 0x000000, 0x000000, 0x0000FF, 0x021084, 0x06B6C1, 0x000000,
	};
	//+0xB0
#else
	// 4:0x03FB87E8 4:0x00 4:0xFFFFFFFF 4:0x01 4:0x00 4:0x00 4:0x02FEF320 4:0x00
	// +19C 77D8
	DWORD codes[] = {
		0x123487E8, 0x00000000, 0xFFFFFFFF, 0x00000001,
		0x00000000, 0x00000000, 0x1234F320, 0x00000000
	};
#endif;
	DWORD address = 0;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.MovSta = address + 0x1A0;
		printf("移动状态地址：%08X\n", m_GameAddr.MovSta);
	}
	return address > 0;
}

// 获取对话框状态地址
bool Game::FindTalkBoxStaAddr()
{
	// 4:0x00FB4CA0 4:0x00000001 4:0x00000000 4:0x00000000 4:0x00000000 4:0x00000001 4:0x00000000
	DWORD codes[] = {
		0x00FB4CA0, 0x00000001, 0x00000000, 0x00000000,
		0x00000000, 0x00000001, 0x00000000, 0x00000022
	};
	DWORD address = 0;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.CallNpcTalkEsi = address;
		m_GameAddr.TalKBoxSta = address + 0xA0;
		printf("NPC二级对话ESI:%08X\n", m_GameAddr.CallNpcTalkEsi);
		printf("对话框状态地址：%08X\n", m_GameAddr.TalKBoxSta);
	}
	return address > 0;
}

// 获取是否选择邀请队伍状态地址
bool Game::FindTeamChkStaAddr()
{
	// 29D0
	// 4:0x00F63018 4:0x00000001 4:0x00000000 4:0x00000000 4:0x00000000 4:0x00000001 4:0x00000000
	DWORD codes[] = {
		0x00F63018, 0x00000001, 0x00000000, 0x00000000,
		0x00000000, 0x00000001, 0x00000000, 0x00000022
	};
	DWORD address = 0; // 这个地址是Call选择是否邀请队伍函数中esi的值 mov ecx,dword ptr ds:[esi+0x1DC]
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.TeamChkSta = address; // +29D0是TeamChkSta偏移头 再+100是选择框状态
		printf("邀请队伍状态地址：%08X\n", m_GameAddr.TeamChkSta);
	}
	return address > 0;
}

// 获取提示框状态地址
bool Game::FindTipBoxStaAddr()
{
	// 4:0x00F39588 4:0x00000001 4:0x00000000 4:0x00000000 4:0x00000000 4:0x00000001 4:0x00000000
	DWORD codes[] = {
		0x00F39588, 0x00000001, 0x00000000, 0x00000000,
		0x00000000, 0x00000001, 0x00000000, 0x00000022
	};
	DWORD address = 0;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.TipBoxSta = address;
		printf("提示框状态地址：%08X\n", m_GameAddr.TipBoxSta);
	}
	return address > 0;
}

// 获取生命地址
bool Game::FindLifeAddr()
{
#if 0
	m_GameAddr.Life = m_GameModAddr.Mod3DRole + ADDR_LIFE_OFFSET;
	m_GameAddr.LifeMax = m_GameModAddr.Mod3DRole + ADDR_LIFEMAX_OFFSET;
	printf("找到生命地址：%08X\n", m_GameAddr.Life);
	printf("找到生命上限地址：%08X\n", m_GameAddr.LifeMax);
	return true;
#else
	// 4:0x00 4:0x00 4:* 4:0x03 4:0x0A 4:0x18 4:0x29 4:0x00
	// 4:0x18 4:0x87 4:0x00 4:0x0A 4:* 4:0x00 4:0x0A 4:0x00
	DWORD codes[] = {
		0x00000000, 0x00000000, 0x00000011, 0x00000003,
		0x0000000A, 0x00000018, 0x00000029, 0x00000000,
	};
	DWORD address = 0;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address, 1, 1)) {
		m_GameAddr.Life = address + 0x20;
		m_GameAddr.LifeMax = m_GameModAddr.Mod3DRole + ADDR_LIFEMAX_OFFSET;
		printf("找到生命地址：%08X\n", m_GameAddr.Life);
		printf("找到生命上限地址：%08X\n", m_GameAddr.LifeMax);
	}
	return address > 0;
#endif
}

// 获取背包代码
bool Game::FindBagAddr()
{
	// Soul.exe+C8C12C
	// Soul.exe + CA97C8
	// 4:0x281 4:0x1E6 4:0xA04 4:0x05 4:0x05 4:0x1E1E
	// 023C78D5 
	// 搜索方法->CE查找物品数量->下访问断点, 获得基址->基址下访问断点得到偏移
	// CHero::GetItem函数里面 具体哪个位置需要配合上面的查看
	try {
		DWORD p, count;
		__asm
		{
			mov eax, g_pObjHero
			mov eax, [eax]
			mov eax, [eax + 0x224C]     // 2234
			mov edx, [eax + 0x10]       // 物品地址指针
			mov dword ptr[p], edx
			mov edx, [eax + 0x30]       // 物品数量
			mov dword ptr[count], edx
		}
		printf("物品指针:%08X 数量:%d\n", p, count);
		//m_GameAddr.Bag = p;

		return p != 0;
	}
	catch (...) {
		printf("Game::FindBagAddr失败!\n");
	}
	return false;
}

// 获得地面物品地址的保存地址
bool Game::FindItemPtr()
{
	// 4:0x00 4:0x04 4:0x80000000 4:0x80000000 4:0x7FFFFFFF 4:0x7FFFFFFF 4:* 4:0x01 4:0x19 4:0x00400000
	DWORD codes[] = {
		0x00000000, 0x00000004, 0x80000000, 0x80000000,
		0x7FFFFFFF, 0x7FFFFFFF, 0x00000011, 0x00000011,
		0x00000022, 0x00400000, 0x00000022, 0x00000022,
	};
	DWORD address;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.ItemPtr = address + 0x30;
		INLOGVARN(32, "地面物品地址保存地址:%08X", m_GameAddr.ItemPtr);
		printf("地面物品地址保存地址:%08X\n", m_GameAddr.ItemPtr);
	}

	return address != 0;
}

// 获取NPC二级对话ESI寄存器数值
bool Game::FindCallNPCTalkEsi()
{
	return true;
	// 0x00F6FE30
	DWORD codes[] = {
		0x00FB1B10, 0x00000001, 0x00000000, 0x00000000,
		0x00000000, 0x00000001, 0x00000000, 0x00000022
	};
	DWORD address = 0;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.CallNpcTalkEsi = address;
		printf("NPC二级对话ESI:%08X\n", m_GameAddr.CallNpcTalkEsi);
	}

	return address != 0;
}

// 获取宠物列表基地址
bool Game::FindPetPtrAddr()
{
	// 搜索方法 先找出宠物血量->CE下血量访问断点, 找到基址, 查看特征码
	// 宠物ID[773610E3宠物名字:800750]->获得宠物ID地址->下访问断点->找到偏移
	// 4:0x00F855F8 4:0x00000001 4:0x00000000 4:0x00000000 4:0x00000000 4:0x00000001 4:0x00000000
#if 0
	DWORD address = NULL;
	__asm
	{
		mov ecx, dword ptr ds : [BASE_PET_OFFSET]
		mov ecx, dword ptr ds : [ecx + 0x50C]
		mov ecx, dword ptr ds : [ecx + 0x458]
		mov dword ptr[address], ecx
	}
	m_GameAddr.PetPtr = address;
	printf("宠物列表基址:%08X\n", address);
#else
	DWORD codes[] = {
		0x00F855F8, 0x00000001, 0x00000000, 0x00000000,
		0x00000000, 0x00000001, 0x00000000, 0x00000022,
		//0x00000022, 0x00000000, 0x00000010, 0x00000022,
		//0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000,
		//0x0000016B, 0x0000016B, 0x00000000, 0x00000000,
	};
	DWORD address = 0;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.PetPtr = address;
		printf("宠物列表基址:%08X [%08X] %08X\n", address, PtrToDword(address), codes);
	}
#endif
	return address != 0;
}

// 获取地图名称地址
bool Game::FindMapName()
{
	// 4:* 4:0x3E8 4:0x3E8 4:0x3E8 4:0x00 4:0x00200000 4:0x00120080 4:0x00 4:0x00 4:0x01 4:0xFFFFFFFF 4:0xFFFFFFFF
	DWORD codes[] = {
		0x00000011, 0x00000022, 0x00000022, 0x00001102,
		0x00000011, 0x00000022, 0x00000022, 0x00000000,
		0x00000000, 0x00000001, 0xFFFFFFFF, 0xFFFFFFFF,
	};
	DWORD address = 0;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.MapName = address + 0x30;
		printf("地图名称地址:%08X[%s]\n", m_GameAddr.MapName, m_GameAddr.MapName);
	}
	return address != 0;
}

// 在某个模块中搜索函数
DWORD Game::SearchFuncInMode(SearchModFuncMsg* info, HANDLE hMod)
{
	//得到DOS头
	IMAGE_DOS_HEADER* dosheader = (PIMAGE_DOS_HEADER)hMod;
	//效验是否DOS头
	if (dosheader->e_magic != IMAGE_DOS_SIGNATURE)return NULL;

	//NT头
	PIMAGE_NT_HEADERS32 NtHdr = (PIMAGE_NT_HEADERS32)((CHAR*)hMod + dosheader->e_lfanew);
	//效验是否PE头
	if (NtHdr->Signature != IMAGE_NT_SIGNATURE)return NULL;

	//得到PE选项头
	IMAGE_OPTIONAL_HEADER32* opthdr = (PIMAGE_OPTIONAL_HEADER32)((PBYTE)hMod + dosheader->e_lfanew + 24);
	//得到导出表
	IMAGE_EXPORT_DIRECTORY* pExportTable = (PIMAGE_EXPORT_DIRECTORY)((PBYTE)hMod + opthdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
	//得到函数地址列表
	PDWORD arrayOfFunctionAddresses = (PDWORD)((PBYTE)hMod + pExportTable->AddressOfFunctions);
	//得到函数名称列表
	PDWORD arrayOfFunctionNames = (PDWORD)((PBYTE)hMod + pExportTable->AddressOfNames);
	//得到函数序号
	PWORD arrayOfFunctionOrdinals = (PWORD)((PBYTE)hMod + pExportTable->AddressOfNameOrdinals);
	//导出表基地址
	DWORD Base = pExportTable->Base;
	//循环导出表
	for (DWORD x = 0; x < pExportTable->NumberOfNames; x++)			//导出函数有名称 编号之分，导出函数总数=名称导出+编号导出，这里是循环导出名称的函数
	{
		//得到函数名 
		PSTR functionName = (PSTR)((PBYTE)hMod + arrayOfFunctionNames[x]);
		if (strstr(functionName, info->Name)) { // 存在此名称
			bool find = true;
			if (info->Substr) { // 搜索子串
				PSTR sub = strstr(functionName, info->Substr);
				if ((sub && !info->Flag) || (!sub && info->Flag)) { // 不符合要求
					find = false;
				}
			}
			if (find == true) {
				DWORD functionOrdinal = arrayOfFunctionOrdinals[x];
				*info->Save = (DWORD)hMod + arrayOfFunctionAddresses[functionOrdinal];
				printf("Call%s:%08X:%s\n", info->Remark, *info->Save, functionName);
				goto end;
			}
		}
	}
end:
	return *(info->Save);
}

// 在某个模块里面搜索
DWORD Game::SearchInMod(LPCTSTR name, DWORD * codes, DWORD length, DWORD * save, DWORD save_length, DWORD step)
{
	DWORD result = 0;
	DWORD dwSize = 0;
	HANDLE hMod = GetModuleBaseAddr(GetCurrentProcessId(), name, &dwSize);
	//printf("hMod:%08X %08X\n", hMod, dwSize);

	if (dwSize) {
		DWORD page = 0x1000;
		DWORD addr = 0;
		while (addr < dwSize) {
			m_dwReadBase = (DWORD)hMod + addr;
			m_dwReadSize = page;
			if (ReadProcessMemory(m_hGameProcess, (LPVOID)m_dwReadBase, m_pReadBuffer, m_dwReadSize, NULL)) {
				if (SearchCode(codes, length, save, save_length, step)) {
					//printf("SearchInMod.....\n");
					result = save[0];
					break;
				}
			}
			addr += page;
		}
	}

	return result;
}

// 搜索特征码
DWORD Game::SearchCode(DWORD* codes, DWORD length, DWORD* save, DWORD save_length, DWORD step)
{
	if (length == 0 || save_length == 0)
		return 0;

	DWORD count = 0;
	for (DWORD i = 0; i < m_dwReadSize; i += step) {
		if ((i + length) > m_dwReadSize)
			break;

		DWORD addr = m_dwReadBase + i;
		if (addr == (DWORD)codes) { // 就是自己
			//printf("搜索到了自己:%08X\n", codes);
			return 0;
		}

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
			else if (((codes[j] & 0xffff0000) == 0x12340000)) { // 低2字节相等
				if ((dw[j]&0x0000ffff) != (codes[j]&0x0000ffff)) {
					result = false;
					break;
				}
				else {
					//printf("%08X\n", dw[j]);
				}
			}
			else if (((codes[j] & 0x0000ffff) == 0x00001234)) { // 高2字节相等
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
			save[count++] = addr;
			//printf("地址:%08X   %08X\n", addr, codes);
			if (count == save_length)
				break;
		}
	}

	return count;
}

// 读取四字节内容
bool Game::ReadDwordMemory(DWORD addr, DWORD& v)
{
	return ReadMemory((PVOID)addr, &v, 4);
}

// 读取内存
bool Game::ReadMemory(PVOID addr, PVOID save, DWORD length)
{
	if (IsBadReadPtr(addr, length)) {
		printf("错误地址:%08X %d\n", addr, length);
		return false;
	}

	bool mod = false;
	DWORD oldProtect;
#if 0
	mod = VirtualProtect(addr, length, PAGE_EXECUTE_READWRITE, &oldProtect);
	if (!mod)
		printf("!VirtualProtect\n");
#endif

	DWORD dwRead = 0;
	bool result = ReadProcessMemory(m_hGameProcess, addr, save, length, &dwRead);
	//printf("ReadProcessMemory:%d %08X %d Read:%d 数值:%d(%08X)\n", GetLastError(), addr, result, dwRead, *(DWORD*)save, *(DWORD*)save);

	if (!result || dwRead != length) {
		printf("ReadProcessMemory错误:%d %08X %d\n", GetLastError(), addr, result);
		if (GetLastError() == 6) {
			m_pGameProc->WriteLog("GetLastError() == 6", true);
			Sleep(100);
			//CloseHandle(m_hGameProcess);
			m_pGameProc->WriteLog("关闭m_hGameProcess", true);
			Sleep(100);
			m_hGameProcess = GetCurrentProcess();
			return ReadProcessMemory(m_hGameProcess, addr, save, length, NULL);
		}
	}

	if (mod)
		VirtualProtect(addr, length, oldProtect, &oldProtect);
	return result;
}

// 读取坐标
bool Game::ReadCoor(DWORD* x, DWORD* y)
{
start:
	DWORD vx = 0, vy = 0;
	if (!ReadDwordMemory(m_GameAddr.CoorX, vx)) {
		printf("无法读取坐标X(%d) %08X\n", GetLastError(), m_GameAddr.CoorX);
		return false;
	}
	if (!ReadDwordMemory(m_GameAddr.CoorY, vy)) {
		printf("无法读取坐标Y(%d) %08X\n", GetLastError(), m_GameAddr.CoorY);
		return false;
	}

	int now_time = time(nullptr);
	if (m_iFlagPosTime == 0) { // 第一次不计算
		m_iFlagPosTime = now_time;
	}
	else {
		if (m_dwX == vx && m_dwY == vy) { // 坐标与上次一样
			int allow_second = 120; // 1分钟坐标还一样当作掉线
			if ((now_time - m_iGetPosTime) < allow_second) { // 获取坐标时间在最近
				if ((now_time - m_iFlagPosTime) > allow_second) { // 很久坐标还一样, 可能掉线了
					printf("可能掉线了\n");
					//m_pClient->SendMsg("可能掉线了, 准备结束游戏");
					//Sleep(500);
					//Close(false);
				}
			}
		}
		else {
			m_iFlagPosTime = now_time;
		}
	}

	m_dwX = vx;
	m_dwY = vy;
	m_iGetPosTime = now_time;

	if (x) {
		*x = m_dwX;
	}
	if (y) {
		*y = m_dwY;
	}

	return true;
}

// 读取生命值
bool Game::ReadLife(int& life, int& life_max)
{
	ReadDwordMemory(m_GameAddr.Life, (DWORD&)life);
	life_max = m_dwLifeMax;
	if (!m_dwLifeMax) {
		ReadDwordMemory(m_GameAddr.LifeMax, (DWORD&)life_max);
		m_dwLifeMax = life_max;
	}
	
	return true;	
}

// 读取药包数量
bool Game::ReadQuickKey2Num(int* nums, int length)
{
	if (!m_GameAddr.QuickKey2Num) {
		memset(nums, 0, length);
		return false;
	}

	return ReadProcessMemory(m_hGameProcess, (PVOID)m_GameAddr.QuickKey2Num, nums, length, NULL);
}

// 读取包包物品
bool Game::ReadBag(DWORD* bag, int length)
{
	return ReadProcessMemory(m_hGameProcess, (PVOID)m_GameAddr.Bag, bag, length, NULL);
}

// 获得窗口句柄
bool Game::FindButtonWnd(int button_id, HWND& hwnd, HWND& parent, const char* text)
{
	HWND wnds[] = { (HWND)button_id, NULL, (HWND)text };
	::EnumChildWindows(m_hWnd, EnumChildProc, (LPARAM)wnds);
	hwnd = wnds[0];
	parent = wnds[1];
	return parent != NULL;
}

// 读取游戏内存
bool Game::ReadGameMemory(DWORD flag)
{
	m_dwGuaiWuCount = 0; // 重置怪物数量
	m_bIsReadEnd = false;

	MEMORY_BASIC_INFORMATION mbi;
	memset(&mbi, 0, sizeof(MEMORY_BASIC_INFORMATION));
	DWORD_PTR MaxPtr = 0x70000000; // 最大读取内存地址
	DWORD_PTR max = 0;

	__int64 ms = getmillisecond();
	DWORD_PTR ReadAddress = 0x00000000;
	ULONG count = 0, failed = 0;
	//printf("fuck\n");
	while (ReadAddress < MaxPtr)
	{
		SIZE_T r = VirtualQueryEx(m_hGameProcess, (LPCVOID)ReadAddress, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
		SIZE_T rSize = 0;

		//printf("r:%d\n", r);
		//printf("ReadAddress:%08X - %08X-----%X\n", ReadAddress, ReadAddress + mbi.RegionSize, mbi.RegionSize);
		if (r == 0) {
			DPrint(("r:%d -- %p\n", (int)r, ReadAddress));
			break;
		}
		if (mbi.Type != MEM_PRIVATE && mbi.Protect != PAGE_READWRITE) {
			//printf("%p-%p ===跳过, 大小:%d\n", ReadAddress, ReadAddress + mbi.RegionSize, mbi.RegionSize);
			goto _c;
		}
		else {
			DWORD pTmpReadAddress = ReadAddress;
			DWORD dwOneReadSize = 0x1000; // 每次读取数量
			DWORD dwReadSize = 0x00;      // 已读取数量
			while (dwReadSize < mbi.RegionSize) {
				m_dwReadBase = pTmpReadAddress;
				m_dwReadSize = (dwReadSize + dwOneReadSize) <= mbi.RegionSize 
					? dwOneReadSize : mbi.RegionSize - dwReadSize;

				if (ReadProcessMemory(m_hGameProcess, (LPVOID)pTmpReadAddress, m_pReadBuffer, m_dwReadSize, NULL)) {
					if (flag & 0x10) {
						//printf("ReadGuaiWu\n");
						if (!m_pGuaiWu->ReadGuaiWu())
							flag &= ~0x10;
					}
					if (flag & 0x20) {
						if (!m_pTalk->ReadNPC())
							flag &= ~0x20;
					}
					//printf("flag:%08X %p-%p\n", flag, ReadAddress, ReadAddress + mbi.RegionSize);

					if (flag & 0x01) {
						if (!m_GameAddr.MovSta) {
							FindMoveStaAddr();
						}
						if (!m_GameAddr.TalKBoxSta) {
							FindTalkBoxStaAddr();
						}
						if (!m_GameAddr.TipBoxSta) {
							FindTipBoxStaAddr();
						}
						if (!m_GameAddr.TeamChkSta) {
							FindTeamChkStaAddr();
						}
						if (!m_GameAddr.Life) {
							FindLifeAddr();
						}
						if (!m_GameAddr.CallNpcTalkEsi) {
							FindCallNPCTalkEsi();
						}
						if (!m_GameAddr.ItemPtr) {
							FindItemPtr();
						}
						if (!m_GameAddr.PetPtr) {
							FindPetPtrAddr();
						}
						if (!m_GameAddr.MapName) {
							FindMapName();
						}
					}
					if (!flag)
						goto end;
				}
				else {
					//printf("%p-%p ===读取失败, 大小:%d, 错误码:%d\n", pTmpReadAddress, pTmpReadAddress + m_dwReadSize, (int)m_dwReadSize, GetLastError());
				}

				dwReadSize += m_dwReadSize;
				pTmpReadAddress += m_dwReadSize;
			}

			count++;
		}
	_c:
		// 读取地址增加
		ReadAddress += mbi.RegionSize;
		//Sleep(8);
		//printf("%016X---内存大小2:%08X\n", ReadAddress, mbi.RegionSize);
		// 扫0x10000000字节内存 休眠100毫秒
	}
end:
	__int64 ms2 = getmillisecond();
	printf("读取完内存用时:%d毫秒\n", ms2 - ms);
	m_bIsReadEnd = true;
	return true;
}

// 打印日记
void Game::InsertLog(char * text)
{
	//g_dlg->InsertLog(text);
}

// 打印日记
void Game::InsertLog(wchar_t * text)
{
	//g_dlg->InsertLog(text);
}

// 获取大区点击坐标
void Game::GetSerBigClickPos(int& x, int& y)
{
	const char* str = m_Account.SerBig;
	if (strstr(str, "第一大区")) {
		SET_VAR2(x, 125, y, 150);
		return;
	}
	if (strstr(str, "第二大区")) {
		SET_VAR2(x, 125, y, 185);
		return;
	}
	if (strstr(str, "第七大区")) {
		SET_VAR2(x, 125, y, 350);
		return;
	}
	if (strstr(str, "第十二大区")) {
		SET_VAR2(x, 125, y, 510);
		return;
	}
	if (strstr(str, "第十三大区")) {
		SET_VAR2(x, 125, y, 545);
		return;
	}
	if (strstr(str, "第四十二大区")) {
		SET_VAR2(x, 306, y, 376);
		return;
	}
	if (strstr(str, "第七十八大区")) {
		SET_VAR2(x, 500, y, 507);
		return;
	}
}

// 获取大区点击坐标
void Game::GetSerSmallClickPos(int & x, int & y)
{
	const char* str = m_Account.SerSmall;
	if (strstr(str, "济南五/八/九区")) {
		SET_VAR2(x, 520, y, 435);
		return;
	}
	if (strstr(str, "浙杭七/八区")) {
		SET_VAR2(x, 520, y, 510);
		return;
	}
	if (strstr(str, "桂林一区")) {
		SET_VAR2(x, 520, y, 475);
		return;
	}
	if (strstr(str, "徽州五/七区")) {
		SET_VAR2(x, 520, y, 435);
		return;
	}
	if (strstr(str, "黔州五/六区")) {
		SET_VAR2(x, 520, y, 473);
		return;
	}
	if (strstr(str, "神祗三十三电信")) {
		SET_VAR2(x, 520, y, 355);
		return;
	}
}

LRESULT Game::CldKeyBoardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

// 枚举窗口
BOOL Game::EnumProc(HWND hWnd, LPARAM lParam)
{
	DWORD thepid = *(DWORD*)lParam;
	DWORD pid;
	GetWindowThreadProcessId(hWnd, &pid);
	//printf("HWND:%08X pid:%d thepid:%d\n", hWnd, pid, thepid);
	if (pid == thepid) {
		char name[64];
		GetWindowTextA(hWnd, name, sizeof(name));
		if (strcmp(name, WND_TITLE_A) == 0) {
			*(HWND*)lParam = hWnd;
			return FALSE;
		}
		
	}
	return TRUE;
}

// 枚举子窗口
BOOL Game::EnumChildProc(HWND hWnd, LPARAM lParam)
{
	HWND* param = (HWND*)lParam;
	HWND hWnd_Child = ::GetDlgItem(hWnd, (int)param[0]);
	if (hWnd_Child) { // 找到了子窗口
		if (param[2]) { // 如果搜索按钮文字
			char text[32];
			GetWindowTextA(hWnd_Child, text, sizeof(text)); // 获取按钮文字
			if (strstr(text, (const char*)param[2]) == nullptr)
				return TRUE;
		}

		param[0] = hWnd_Child; // 子窗口句柄 
		param[1] = hWnd;       // 父窗口句柄
		
		return FALSE;
	}
	return TRUE;
}

// 按键
void Game::Keyborad(int key, HWND hwnd)
{
	SetForegroundWindow(m_hWnd);

	if (hwnd == NULL)
		hwnd = m_hWndPic;
	if (key >= 'a' && key <= 'z')
		key -= 32;

	LPARAM lParam = (MapVirtualKey(key, 0) << 16) + 1;
#if 1
	keybd_event(key, MapVirtualKey(key, 0), 0, 0);
	Sleep(150);
	keybd_event(key, MapVirtualKey(key, 0), KEYEVENTF_KEYUP, 0);
#else
	SendMessageA(m_hWnd, 0x36E, 0x02, 0x015FFA18);
	Sleep(10);
	SendMessageA(m_hWnd, 0x36E, 0x00, 0x015FF9F0);

	LPARAM lParam = (MapVirtualKey(key, 0) << 16) + 1;
	PostMessageA(hwnd, WM_KEYDOWN, key, lParam);
#if 1
	Sleep(50);
	lParam |= 0xC0000001;
	PostMessageA(hwnd, WM_KEYUP, key, lParam);
#endif
#endif
	//printf("按键:%d %08X %08X %d\n", key, lParam, hwnd, GetLastError());
}

void Game::LeftClick(int x, int y, HWND hwnd)
{
	if (hwnd == NULL)
		hwnd = m_hWndPic; 

	PostMessageA(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
	//Sleep(10);
	PostMessageA(hwnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(x, y));
	PostMessageA(hwnd, WM_LBUTTONUP, 0x00, MAKELPARAM(x, y));
	//PostMessageA(hwnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(x, y));
	//printf("单击完成:%d,%d %08X\n", x, y, hwnd);
}

// 鼠标移动
void Game::MouseMove(int x, int y)
{
	PostMessageA(m_hWndPic, WM_MOUSEMOVE, 0x00, MAKELPARAM(x, y));
	Sleep(50);
}

// 人物复活
void Game::Call_ReBoren()
{
	printf("人物复活\n");
	_FUNC1 func = (_FUNC1)m_GameCall.ReBorn;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(0x00);
	ASM_RESTORE_ECX();
}

// 人物移动函数
void Game::Call_Run(int x, int y)
{
	printf("移动:%d,%d\n", x, y);
	// 4:0x6AEC8B55 4:0xBE7468FF 4:0xA1640326 4:0x00000000 4:0x25896450 4:0x00000000 4:0x04D8EC81 4:0x56530000
	_FUNC3 func = (_FUNC3)m_GameCall.Run;
	if (IsBadCodePtr((FARPROC)func)) {
		printf("Run函数无效\n");
		return;
	}
	// 20674C3
	// 20675CA
	DWORD t = 0;// P2DW(0x000);
	if (t) {
		printf("ttttttttttttttttt\n");
	}
	//printf("准备调用移动函数\n");
	char tmp[0x1000] = { 0 };
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(x, y, 0x00);
	ASM_RESTORE_ECX();
	//printf("移动函数调用完成\n");
	if (tmp[0] == 0) {
		DWORD ttt = 2;
		if (ttt && tmp[0]) {
			printf("不可能输出!\n");
		}
	}
}

// 喊话CALL[type 0=公共频道 1=私人 2=队伍]
void Game::Call_Talk(const char* msg, int type)
{
	return;
	DWORD arg = 0, func = NULL;
	if (type == 1) {
		arg = 0x07D1;
		func = PtrToDword(CALLTALK_DS_COMMON);
	}
	else if (type == 2) {
		arg = 0x07D3;
		func = PtrToDword(CALLTALK_DS_TEAM);
	}
	// 15A62D0
	ZeroMemory((PVOID)0x15A62D0, 0x100);
	memcpy((PVOID)0x15A62D0, msg, strlen(msg));
	// B7859a
	__asm {
		push 0x00
		push arg
		push 0xFFFFFF
		push 0x00       // 0x015A63F8 // 可能是变量地址
		push 0x15A62D0  // 喊话内容[固定地址]
		push 0x00       // 0x015A63D8 // 可能是变量地址
		mov eax, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [eax]
		call func
	}
}

// NPC对话
void Game::Call_NPC(int npc_id)
{
	//INLOGVARN(32, "打开NPC:%08X\n", npc_id);
	printf("打开NPC:%08X\n", npc_id);
	_FUNC2 func = (_FUNC2)m_GameCall.ActiveNpc;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(npc_id, 0x00);
	ASM_RESTORE_ECX();
}

// NPC二级对话
void Game::Call_NPCTalk(int no, bool close)
{
	//INLOGVARN(32, "对话NPC：%d\n", no);
	// eax=038735B8-2550000=13235B8
	// edi=0343DDD8-2550000=EEDDD8
	// esi=05DD5314

	_FUNC_R func = (_FUNC_R)m_GameCall.ITaskGetInstance;
	DWORD _eax = func();
	DWORD _esi = m_GameAddr.CallNpcTalkEsi;
	printf("NPC对话选择 no:%d _eax:%08X _edi:%08X _esi:%08X\n", no, _eax, PtrToDword(_eax), _esi);

	ASM_STORE_ECX();
	_FUNC2 func2 = (_FUNC2)PtrToDword((PtrToDword(_eax) + 0x138));
	__asm { 
		mov ecx, _eax
	}
	func2(no, 0x00);

	if (m_GameCall.NPCTalk) {
		_FUNC1 func3 = (_FUNC1)m_GameCall.NPCTalk;
		_asm {
			mov ecx, _esi
		}
		func3(0x00);
	}
	else {
		if (close) {
			m_pGameProc->Button(BUTTON_ID_CLOSEMENU, 0, "C");
		}
	}
	ASM_RESTORE_ECX();
}

// 关闭提示框
void Game::Call_CloseTipBox(int close)
{
	printf("关闭提示框:%d\n", close);
	DWORD _ecx = m_GameAddr.TipBoxSta;
	_FUNC3 func = (_FUNC3)m_GameCall.CloseTipBox;
	__asm { mov ecx, _ecx };
	func(0x0A, close, 0x00);
}

// 获取物品指针
DWORD Game::Call_GetPackageItemByIndex(int index)
{
	_FUNC2_R func = (_FUNC2_R)m_GameCall.GetPkageItemByIndex;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	return func(index, 0x0A);
	ASM_RESTORE_ECX();
}
	

// 使用物品
void Game::Call_UseItem(int item_id)
{
	printf("使用物品:%08X\n", item_id);
	_FUNC5 func = (_FUNC5)m_GameCall.UseItem;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(item_id, 0x00, 0xFFFFFFFF, 0xFFFFFFFF, 0x00);
	ASM_RESTORE_ECX();
}

// 扔物品
void Game::Call_DropItem(int item_id)
{
	printf("丢弃物品:%08X\n", item_id);
	_FUNC3 func = (_FUNC3)m_GameCall.DropItem;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(item_id, 0x00, 0x00);
	ASM_RESTORE_ECX();
	// 4:0x00 4:0x04 4:0x80000000 4:0x80000000 4:0x7FFFFFFF 4:0x7FFFFFFF 4:* 4:0x01 4:0x19 4:0x00400000
}

// 捡物品
void Game::Call_PickUpItem(DWORD id, DWORD x, DWORD y)
{
	printf("拾取物品:%08X %d,%d\n", id, x, y);
	_FUNC3 func = (_FUNC3)m_GameCall.PickUpItem;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(id, x, y);
	ASM_RESTORE_ECX();
}

// 卖东西
void Game::Call_SellItem(int item_id)
{
	printf("卖东西:%08X\n", item_id);
	_FUNC1 func = (_FUNC1)m_GameCall.SellItem;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(item_id);
	ASM_RESTORE_ECX();
}

// 存钱
void Game::Call_SaveMoney(int money)
{
	printf("存钱:%d\n", money);
	_FUNC1 func = (_FUNC1)m_GameCall.SaveMoney;
	if (!func)
		return;

	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(money);
	ASM_RESTORE_ECX();
}

// 存入远程仓库
void Game::Call_CheckInItem(int item_id)
{
	//printf("存入物品:%08X\n", item_id);
	_FUNC2 func = (_FUNC2)m_GameCall.CheckInItem;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(item_id, 0x0A);
	ASM_RESTORE_ECX();
}

// 取出仓库物品
void Game::Call_CheckOutItem(int item_id)
{
	printf("取出仓库物品:%08X\n", item_id);
	_FUNC3 func = (_FUNC3)m_GameCall.CheckOutItem;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(item_id, 0x0A, 0x00);
	ASM_RESTORE_ECX();
}

// 使用可传送物品
void Game::Call_TransmByMemoryStone(int item_id)
{
	printf("使用可传送物品:%08X\n", item_id);
	_FUNC1 func = (_FUNC1)m_GameCall.TransmByMemoryStone;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(item_id);
	ASM_RESTORE_ECX();
}

// 放技能
void Game::Call_Magic(int magic_id, int guaiwu_id)
{
	printf("技能怪物ID:%08X %08X\n", magic_id, guaiwu_id);
	_FUNC4 func = (_FUNC4)m_GameCall.MagicAttack_GWID;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(magic_id, guaiwu_id, 0x00, 0x00);
	ASM_RESTORE_ECX();
}

// 放技能
void Game::Call_Magic(int magic_id, int x, int y)
{
	// 05D05020 06832EA4 mov ecx,dword ptr ds:[0xF1A518]
	printf("技能XY:%08X %d,%d\n", magic_id, x, y);
	_FUNC5 func = (_FUNC5)m_GameCall.MagicAttack_XY;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(magic_id, x, y, 0x00, 0x00);
	ASM_RESTORE_ECX();
	printf("技能使用完成\n");
}

// 宠物出征
void Game::Call_PetOut(int pet_id)
{
	// 00870741 - 89 8C B3 C0020000  - mov [ebx+esi*4+000002C0],ecx
	// 724-008
	printf("宠物出征:%08X\n", pet_id);
	_FUNC2 func = (_FUNC2)m_GameCall.CallEudenmon;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(pet_id, 0x00);
	ASM_RESTORE_ECX();
}

// 宠物召回
void Game::Call_PetIn(int pet_id)
{
	_FUNC1 func = (_FUNC1)m_GameCall.KillEudenmon;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(pet_id);
	ASM_RESTORE_ECX();
}

// 宠物合体
void Game::Call_PetFuck(int pet_id)
{
	printf("宠物合体:%08X\n", pet_id);
	_FUNC1 func = (_FUNC1)m_GameCall.AttachEudemon;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(pet_id);
	ASM_RESTORE_ECX();
}

// 宠物解体
void Game::Call_PetUnFuck(int pet_id)
{
	printf("宠物解体:%08X\n", pet_id);
	_FUNC1 func = (_FUNC1)m_GameCall.UnAttachEudemon;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(pet_id);
	ASM_RESTORE_ECX();
}

// 宠物技能 key_no=按键索引 1=0 2=1 ...
void Game::Call_PetMagic(int key_no)
{
	printf("宠物技能:%d\n", key_no);
	_FUNC6 func = (_FUNC6)m_GameCall.KeyNum;
	DWORD _this = P2DW(m_GameAddr.KeyNumEcxPtr);
	ASM_STORE_ECX();
	__asm { mov ecx, _this }
	func(key_no, 0x00, 0x00, 0x00, 0x00, 0x00);
	ASM_RESTORE_ECX();
}

// 获取远程邀请人物信息
DWORD Game::Call_QueryRemoteTeam(int no)
{
	_FUNC_R func = (_FUNC_R)m_GameCall.QueryInf_RemoteTeam;
	ASM_SET_ECX();
	DWORD base_addr = func();
	printf("%08X %08X %08X\n", base_addr, P2DW(base_addr), P2DW(base_addr) + 0x30);
	//return base_addr;
	_FUNC1_R func2 = (_FUNC1_R)P2DW((P2DW(base_addr) + 0x30));
	__asm mov ecx, base_addr;
	DWORD r = func2(no);
	return r;
}

// 是否有队伍
bool Game::Call_IsHaveTeam()
{

	_FUNC_R func = (_FUNC_R)m_GameCall.IsHaveTeam;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	DWORD r = func();
	ASM_RESTORE_ECX();
	return r != 0;
}

// 是否是队长
bool Game::Call_IsTeamLeader()
{
	_FUNC_R func = (_FUNC_R)m_GameCall.IsTeamLeader;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	DWORD r = func();
	ASM_RESTORE_ECX();
	return r != 0;
}

// 创建队伍
void Game::Call_TeamCreate()
{
	printf("创建队伍\n");
	_FUNC func = (_FUNC)m_GameCall.TeamCreate;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func();
	ASM_RESTORE_ECX();
}

// 离开队伍[队长]
void Game::Call_TeamDismiss()
{
	printf("离开队伍\n");
	_FUNC func = (_FUNC)m_GameCall.TeamDismiss;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func();
	ASM_RESTORE_ECX();
}

// 离开队伍[队员]
void Game::Call_TeamLeave()
{
	printf("离开队伍\n");
	_FUNC func = (_FUNC)m_GameCall.TeamLeave;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func();
	ASM_RESTORE_ECX();
}

// 邀请入队
void Game::Call_TeamInvite(int player_id)
{
	printf("邀请入队:%08X\n", player_id);
	_FUNC1 func = (_FUNC1)m_GameCall.TeamInvite;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(player_id);
	ASM_RESTORE_ECX();
}

// 自动组队
void Game::Call_TeamAutoJoin(int open)
{
	printf("自动组队:%d\n", open);
	_FUNC1 func = (_FUNC1)m_GameCall.TeamAutoJoin;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(open);
	ASM_RESTORE_ECX();
}

// 是否选择邀请队伍
void Game::Call_CheckTeam(int v)
{
	if (!m_GameAddr.TeamChkSta)
		return;

	printf("副本邀请队伍选择:%d\n", v);
	_FUNC1 func = (_FUNC1)m_GameCall.TeamChk;
	DWORD _this = P2DW(m_GameAddr.TeamChkSta + m_GameAddr.TeamChkOffset);
	ASM_STORE_ECX();
	__asm { mov ecx, _this }
	func(v);
	ASM_RESTORE_ECX();
}

// 获取基址[不知道什么可以获取]
DWORD Game::Call_GetBaseAddr(int index, DWORD _ecx)
{
	_FUNC1_R func = (_FUNC1_R)m_GameCall.GetNpcBaseAddr;
	ASM_STORE_ECX();
	__asm { mov ecx, _ecx };
	DWORD p = func(index);
	ASM_RESTORE_ECX();
	return PtrToDword(p);
}
