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

// 游戏g_pObjHero全局变量
DWORD g_pObjHero = NULL;
// 游戏g_objPlayerSet全局变量
DWORD g_objPlayerSet = NULL;

GameModAddr Game::m_GameModAddr; // 游戏模块地址
GameAddr    Game::m_GameAddr;    // 游戏一些数据地址
GameCall    Game::m_GameCall;    // 游戏CALL
GameWnd     Game::m_GameWnd;     // 游戏窗口信息

Game*       Game::self = NULL;   // Game自身

// ...
Game::Game()
{
	self = this;

	ZeroMemory(&m_GameModAddr, sizeof(GameModAddr));
	ZeroMemory(&m_GameAddr, sizeof(GameAddr));
	ZeroMemory(&m_GameCall, sizeof(GameCall));

	m_bIsReadEnd = true;
	m_pReadBuffer = new BYTE[1024 * 1024 * 10];

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

	delete m_pItem;
	delete m_pTalk;
	delete m_pMove;
	delete m_pGuaiWu;
	delete m_pMagic;
	delete m_pPet;

	delete m_pGameProc;
}

// 初始化
bool Game::Init()
{
	FindGameWnd();

	m_hGameProcess = GetCurrentProcess();
	FindAllModAddr();
	FindCoorAddr();

	printf("等待游戏数据初始化完毕...\n");
	while (!PtrToDword(BASE_DS_OFFSET)) {
		Sleep(1000);
	}
	while (!FindBagAddr()) {
		Sleep(1000);
	}

	DWORD pid;
	GetWindowThreadProcessId(m_hWnd, &pid);

	printf("开始读取游戏数据...\n");
	FindAllCall();
	
	if (!ReadGameMemory()) {
		INLOG("无法获得人物血量地址！！！");
		return false;
	}
	m_pMagic->ReadMagic();
#if 0
	while (!m_GameAddr.Life) {
		printf("重新读取血量地址\n");
		ReadGameMemory();
		Sleep(1000);
	}
#endif
	return true;
}

// 进程是否是魔域
bool Game::IsMoYu()
{
	wchar_t name[32] = { 0 };
	GetProcessName(name, GetCurrentProcessId());
	wprintf(L"%ws = %ws\n", name, L"soul.exe");
	return wcsstr(name, L"soul.exe") != nullptr;
}

// 运行
void Game::Run()
{
	m_pGameProc->Run();
}


// 获得游戏窗口
HWND Game::FindGameWnd()
{
	while (true) {
		DWORD pid = GetCurrentProcessId();
		::EnumChildWindows(m_hWnd, EnumProc, (LPARAM)&pid);
		if (pid != GetCurrentProcessId()) {
			m_hWnd = (HWND)pid;
			break;
		}
		Sleep(5000);
	}
	
	float f = ::GetScale();
	GetWindowRect(m_hWnd, &m_GameWnd.Rect);
	m_GameWnd.Width = m_GameWnd.Rect.right - m_GameWnd.Rect.left;
	m_GameWnd.Height = m_GameWnd.Rect.bottom - m_GameWnd.Rect.top;
	printf("窗口句柄:%0x %d %d %.2\n", m_hWnd, m_GameWnd.Width, m_GameWnd.Height, f);
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
		{"?Run@CHero",             NULL,  0, &m_GameCall.Run,              "移动函数"},
		{"?ActiveNpc@CHero",       NULL,  0, &m_GameCall.ActiveNpc,        "NPC对话"},
		{"?UseItem@CHero",         NULL,  0, &m_GameCall.UseItem,          "使用物品"},
		{"?DropItem@CHero",        NULL,  0, &m_GameCall.DropItem,         "丢弃物品"},
		{"?PickUpItem@CHero",      NULL,  0, &m_GameCall.PickUpItem,       "捡拾物品"},
		{"?SellItem@CHero",        NULL,  0, &m_GameCall.SellItem,         "卖物品"},
		{"?CheckInItem@CHero",     NULL,  0, &m_GameCall.CheckInItem,      "存入远程仓库"},
		{"?OpenBank@CHero",        NULL,  0, &m_GameCall.OpenBank,         "打开远程仓库"},
		{"?TransmByMemoryStone@CHero",    NULL,  0, &m_GameCall.TransmByMemoryStone, "使用可传送物品"},
		{"?MagicAttack@CHero",     "POS", 0, &m_GameCall.MagicAttack_GWID, "使用技能-怪物ID "},
		{"?MagicAttack@CHero",     "POS", 1, &m_GameCall.MagicAttack_XY,   "使用技能-XY坐标"},
		{"?CallEudenmon@CHero",    NULL,  0, &m_GameCall.CallEudenmon,     "宠物出征"},
		{"?KillEudenmon@CHero",    NULL,  0, &m_GameCall.KillEudenmon,     "宠物召回"},
		{"?AttachEudemon@CHero",   NULL,  0, &m_GameCall.AttachEudemon,    "宠物合体"},
		{"?UnAttachEudemon@CHero", NULL,  0, &m_GameCall.UnAttachEudemon,  "宠物解体"},
		{"?GetData@CPlayer",       NULL,  0, &m_GameCall.SetRealLife,      "CPlayer::GetData"},
	};
	DWORD length = sizeof(info) / sizeof(SearchModFuncMsg);
	for (DWORD i = 0; i < length; i++) {
		SearchFuncInMode(&info[i], (HANDLE)m_GameModAddr.Mod3DRole);
	}

	m_GameCall.CloseTipBox = FindCloseTipBoxCall();
	m_GameCall.GetNpcBaseAddr = FindGetNpcBaseAddr();
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
	// 里面寻找即可找到
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

// 比较登录帐号
bool Game::CmpLoginAccount(const char* name)
{
	return strcmp(name, (const char*)(m_GameModAddr.Mod3DRole+ADDR_ACCOUNT_NAME)) == 0;
}

// 获取坐标地址
bool Game::FindCoorAddr()
{
	m_GameAddr.CoorX = m_GameModAddr.Mod3DRole + ADDR_COOR_X_OFFSET;
	m_GameAddr.CoorY = m_GameModAddr.Mod3DRole + ADDR_COOR_Y_OFFSET;
	while (PtrToDword(m_GameAddr.CoorX) == 0 || PtrToDword(m_GameAddr.CoorY) == 0) {
		printf("等待进入游戏！！！\n");
		Sleep(5000);
	}
	printf("人物坐标:%d,%d\n", PtrToDword(m_GameAddr.CoorX), PtrToDword(m_GameAddr.CoorY));
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
	// 4:0x032AD6F0 4:0x00 4:0xFFFFFFFF 4:0x01 4:0x00 4:0x00 4:0x03CFF320 4:0x00
	// +19C
	DWORD codes[] = {
		0x1234D6F0, 0x00000000, 0xFFFFFFFF, 0x00000001,
		0x00000000, 0x00000000, 0x1234F320, 0x00000000
	};
#endif;
	DWORD address = 0;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.MovSta = address + 0x1A0;
		INLOGVARN(32, "移动状态地址:%08X", m_GameAddr.MovSta);
		printf("移动状态地址：%08X\n", m_GameAddr.MovSta);
	}
	return address > 0;
}

// 获取对话框状态地址
bool Game::FindTalkBoxStaAddr()
{
	// 4:0x00F87258 4:0x00000001 4:0x00000000 4:0x00000000 4:0x00000000 4:0x00000001 4:0x00000000
	DWORD codes[] = {
		0x00F93E10, 0x00000001, 0x00000000, 0x00000000,
		0x00000000, 0x00000001, 0x00000000, 0x00000022
	};
	DWORD address = 0;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.TalKBoxSta = address + 0xA0;
		printf("对话框状态地址：%08X\n", m_GameAddr.TalKBoxSta);
	}
	return address > 0;
}

// 获取是否选择邀请队伍状态地址
bool Game::FindTeamChkStaAddr()
{
	// 29C0
	// 4:0x00F37668 4:0x00000001 4:0x00000000 4:0x00000000 4:0x00000000 4:0x00000001 4:0x00000000
	DWORD codes[] = {
		0x00F37668, 0x00000001, 0x00000000, 0x00000000,
		0x00000000, 0x00000001, 0x00000000, 0x00000022
	};
	DWORD address = 0; // 这个地址是Call选择是否邀请队伍函数中esi的值 mov ecx,dword ptr ds:[esi+0x1DC]
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.TeamChkSta = address; // +29C0是TeamChkSta偏移头 +100是选择框状态
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
	// 4:0x00 4:* 4:0x03 4:0x0A 4:0x18 4:0x29 4:0x00 4:0x00
	// 4:0x18 4:0x87 4:0x00 4:0x0A 4:* 4:0x00 4:0x0A 4:0x00
	DWORD codes[] = {
		0x00000000, 0x00000011, 0x00000003, 0x0000000A,
		0x00000018, 0x00000029, 0x00000000, 0x00000000,
	};
	DWORD address = 0;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address, 1, 1)) {
		m_GameAddr.Life = address + 0x20;
		m_GameAddr.LifeMax = m_GameAddr.Life + 0x40F0;
		printf("找到生命地址：%08X\n", m_GameAddr.Life);

		DWORD codes2[] = {
			0x00000018, 0x00000087, 0x00000000, 0x0000000A,
			0x00000011, 0x00000000, 0x0000000A, 0x00000000,
		};
		DWORD address2 = 0;
		if (SearchCode(codes2, sizeof(codes2) / sizeof(DWORD), &address2)) {
			m_GameAddr.LifeMax = address2 + 0x20;
		}
		printf("找到生命上限地址：%08X\n", m_GameAddr.LifeMax);
	}
	return address > 0;
}

// 获取背包代码
bool Game::FindBagAddr()
{
	// Soul.exe+C8C12C
	// Soul.exe + CA97C8
	// 4:0x281 4:0x1E6 4:0xA04 4:0x05 4:0x05 4:0x1E1E
	// 023C78D5 
	// 搜索方法->CE查找物品数量->下访问断点, 获得基址->基址下访问断点得到偏移
	// CHero::GetItem函数里面
	try {
		DWORD p, count;
		__asm
		{
			mov eax, dword ptr ds : [BASE_DS_OFFSET]
			//mov eax, g_pObjHero
			mov eax, [eax]
			mov eax, [eax + 0x2234]     // 2234
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
	// 0x00F6FE30
	DWORD codes[] = {
		0x00F93E10, 0x00000001, 0x00000000, 0x00000000,
		0x00000000, 0x00000001, 0x00000000, 0x00000022
	};
	DWORD address = 0;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.CallNpcTalkEsi = address;
		INLOGVARN(32, "NPC二级对话ESI:%08X", m_GameAddr.CallNpcTalkEsi);
		printf("NPC二级对话ESI:%08X\n", m_GameAddr.CallNpcTalkEsi);
	}

	return address != 0;
}

// 获取宠物列表基地址
bool Game::FindPetPtrAddr()
{
	// 搜索方法 先找出宠物血量->CE下血量访问断点, 找到基址, 查看特征码
	// 宠物ID->获得宠物ID地址->下访问断点->找到偏移
	// 4:0x00F654F0 4:0x00000001 4:0x00000000 4:0x00000000 4:0x00000000 4:0x00000001 4:0x00000000
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
#if 0
	DWORD codes[] = {
		0x00F654F0, 0x00000001, 0x00000000, 0x00000000,
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

// 读取坐标
bool Game::ReadCoor(DWORD* x, DWORD* y)
{
	m_dwX = PtrToDword(m_GameAddr.CoorX);
	m_dwY = PtrToDword(m_GameAddr.CoorY);

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
	life = PtrToDword(m_GameAddr.Life);
	life_max = PtrToDword(m_GameAddr.LifeMax);
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
bool Game::FindButtonWnd(int button_id, HWND& hwnd, HWND& parent)
{
	HWND wnds[] = { (HWND)button_id, NULL };
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
		param[0] = hWnd_Child; // 子窗口句柄 
		param[1] = hWnd;       // 父窗口句柄
		return FALSE;
	}
	return TRUE;
}

// 按键
void Game::Keyborad(int type, int key)
{
	UINT scan_code = MapVirtualKey(key, MAPVK_VK_TO_VSC);
	LPARAM lParam = 0x0001; // 0-15为按键次数
	lParam |= (scan_code&0xff) << 16; // 16-23为按键扫描码
	if (key == WM_KEYUP) { // 按键弹起
		lParam |= 0xC0000000;
	}

	Game_KbdProc func = ((Game_KbdProc)HOOK_KBD_FUNC);
#if 0
	func(3, key, lParam);
#else
	__asm
	{
		push lParam
		push key
		push 0x03
		mov  ebx, 0x002C5000
		mov  edi, lParam
		mov  ecx, HOOK_KBD_FUNC
		mov  esi, ecx
		call esi
	}
#endif
}

// 按键
void Game::KeyDown(char key)
{
	Keyborad(WM_KEYDOWN, key);
}

// 按键
void Game::KeyUp(char key)
{
	Keyborad(WM_KEYUP, key);
}

// 人物移动函数
void Game::Call_Run(int x, int y)
{
	printf("移动到:%d,%d\n", x, y);
	// 4:0x6AEC8B55 4:0xBE7468FF 4:0xA1640326 4:0x00000000 4:0x25896450 4:0x00000000 4:0x04D8EC81 4:0x56530000
	DWORD func = m_GameCall.Run;
	__asm {
		mov ecx, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [ecx] // this指针
		//pushf
		//mov eax, [esp]
		//and eax, 0xFFFFFF6E // CAS标志为0 可能是点击地图
		//mov[esp], eax
		//popf
		push 0 // 0
		push y // y
		push x // x
		call func
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
	DWORD func = m_GameCall.ActiveNpc;
	__asm {
		push 0
		push npc_id
		mov ecx, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [ecx]
		call func
	}
}

// NPC二级对话
void Game::Call_NPCTalk(int no)
{
	INLOGVARN(32, "对话NPC：%d\n", no);
	// eax=038735B8-2550000=13235B8
	// edi=0343DDD8-2550000=EEDDD8
	// esi=05DD5314

	DWORD _eax = m_GameModAddr.Mod3DRole + NPCTALK_EAX_3drole;
	DWORD _esi = m_GameAddr.CallNpcTalkEsi;
	printf("NPCTalk no:%d _eax:%08X _edi:%08X _esi:%08X\n", no, _eax, PtrToDword(_eax), _esi);
	__asm {
		push 0
		push no
		mov eax, _eax
		mov ecx, eax
		mov edi, [eax]
		call dword ptr ds : [edi + 0x138]

		push 0x00
		mov esi, _esi
		mov ecx, esi
		mov eax, 0x00CF0360
		call eax
	}
}

// 关闭提示框
void Game::Call_CloseTipBox(int close)
{
	printf("关闭提示框:%d\n", close);
	DWORD _ecx = m_GameAddr.TipBoxSta;
	DWORD func = m_GameCall.CloseTipBox;
	__asm
	{
		push 0
		push close
		push 0x0A
		mov  ecx, _ecx
		call func
	}
}
	

// 使用物品
void Game::Call_UseItem(int item_id)
{
	printf("使用物品:%08X\n", item_id);
	DWORD func = m_GameCall.UseItem;
	__asm
	{
		push 0x00
		push 0xFFFFFFFF
		push 0xFFFFFFFF
		push 0x00
		push item_id     // 物品ID
		mov eax, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [eax]
		call func
	}
}

// 扔物品
void Game::Call_DropItem(int item_id)
{
	printf("丢弃物品:%08X\n", item_id);
	DWORD func = m_GameCall.DropItem;
	// 4:0x00 4:0x04 4:0x80000000 4:0x80000000 4:0x7FFFFFFF 4:0x7FFFFFFF 4:* 4:0x01 4:0x19 4:0x00400000
	__asm
	{
		push 00      // 应该是扔掉的相对坐标
		push 00      // 应该是扔掉的相对坐标
		push item_id // 物品id
		mov eax, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [eax]
		call func
	}
}

// 捡物品
void Game::Call_PickUpItem(GameGroundItem* p)
{
	printf("拾取物品:%08X %d,%d\n", p->Id, p->X, p->Y);
	DWORD id = p->Id, x = p->X, y = p->Y;
	DWORD func = m_GameCall.PickUpItem;
	__asm
	{
		push y
		push x
		push id
		mov eax, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [eax]
		call func
	}
}

// 卖东西
void Game::Call_SellItem(int item_id)
{
	printf("卖东西:%08X\n", item_id);
	DWORD func = m_GameCall.SellItem;
	__asm
	{
		push item_id
		mov eax, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [eax]
		call func
	}
}

// 存入远程仓库
void Game::Call_CheckInItem(int item_id)
{
	//printf("存入物品:%08X\n", item_id);
	DWORD func = m_GameCall.CheckInItem;
	__asm
	{
		push 0x0A
		push item_id
		mov eax, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [eax]
		call func
	}
}

// 使用可传送物品
void Game::Call_TransmByMemoryStone(int item_id)
{
	printf("使用可传送物品:%08X\n", item_id);
	DWORD func = m_GameCall.TransmByMemoryStone;
	__asm
	{
		push item_id
		mov eax, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [eax]
		call func
	}
}

// 放技能
void Game::Call_Magic(int magic_id, int guaiwu_id)
{
	DWORD func = m_GameCall.MagicAttack_GWID;
	__asm
	{
		push 0
		push 0
		push guaiwu_id
		push magic_id
		mov eax, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [eax]
		call func
	}
}

// 放技能
void Game::Call_Magic(int magic_id, int x, int y)
{
	// 05D05020 06832EA4 mov ecx,dword ptr ds:[0xF1A518]
	DWORD func = m_GameCall.MagicAttack_XY;
	__asm
	{
		push 0
		push 0
		push y
		push x
		push magic_id
		mov edx, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [edx]
		call func
	}
	
}

// 宠物出征
void Game::Call_PetOut(int pet_id)
{
	// 00870741 - 89 8C B3 C0020000  - mov [ebx+esi*4+000002C0],ecx
	// 724-008
	printf("宠物出征:%08X\n", pet_id);
	DWORD func = m_GameCall.CallEudenmon;
	__asm
	{
		push 0x00
		push pet_id
		mov ecx, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [ecx]
		call func
	}
}

// 宠物召回
void Game::Call_PetIn(int pet_id)
{
	DWORD func = m_GameCall.KillEudenmon;
	__asm
	{
		push pet_id
		mov edx, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [edx]
		call func
	}
}

// 宠物合体
void Game::Call_PetFuck(int pet_id)
{
	printf("宠物合体:%08X\n", pet_id);
	DWORD func = m_GameCall.AttachEudemon;
	__asm
	{
		push pet_id
		mov edx, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [edx]
		call func
	}
}

// 宠物解体
void Game::Call_PetUnFuck(int pet_id)
{
	printf("宠物解体:%08X\n", pet_id);
	DWORD func = m_GameCall.UnAttachEudemon;
	__asm
	{
		push pet_id
		mov edx, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [edx]
		call func
	}
}

// 是否选择邀请队伍
void Game::Call_CheckTeam(int v)
{
	if (!m_GameAddr.TeamChkSta)
		return;

	DWORD _esi = m_GameAddr.TeamChkSta;
	__asm
	{
		push v
		mov esi, _esi
		mov ecx, dword ptr ds : [esi + 0x1DC]
		mov eax, 0x00CDB440
		call eax
	}
}
