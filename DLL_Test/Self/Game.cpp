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

// ��Ϸg_pObjHeroȫ�ֱ���(CHero��thisָ��)
DWORD g_pObjHero = NULL;
// ��Ϸg_objPlayerSetȫ�ֱ���
DWORD g_objPlayerSet = NULL;

Game* Game::self = NULL;   // Game����
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

	m_pItem     = new Item(this);      // ��Ʒ��
	m_pTalk     = new Talk(this);      // �Ի���
	m_pMove     = new Move(this);      // �ƶ���
	m_pGuaiWu   = new GuaiWu(this);    // ������
	m_pMagic    = new Magic(this);     // ������
	m_pPet      = new Pet(this);       // ������

	m_pGameConf = new GameConf(this);  // ��Ϸ������
	m_pGameProc = new GameProc(this);  // ִ�й�����
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

// �ر���Ϸ
void Game::Close(bool leave_team)
{
	if (leave_team) {
		if (Call_IsTeamLeader())
			Call_TeamDismiss();
		else
			Call_TeamLeave();
	}

	m_pGameProc->Wait(3 * 1000);
	printf("�ر���Ϸ\n");
	ExitProcess(0);
	TerminateProcess(GetCurrentProcess(), 0);
	//TerminateProcess(m_hGameProcess, 0);
}

// ���ӹ������
void Game::Connect(const char* host, USHORT port)
{
	printf("׼�����ӷ�����[%s:%d]\n", host, port);
	m_pClient->Connect(host, port);
}

// ��¼��Ϸ�ʺ�
void Game::Login()
{
	printf("׼����¼...\n");
	m_Account.IsLogin = 0;

	Sleep(1500);
	LeftClick(510, 550); // ������ʽ��
	Sleep(1500);

	int x, y;
	GetSerBigClickPos(x, y);
	LeftClick(x, y); // ѡ�����
	Sleep(1500);
	GetSerSmallClickPos(x, y);
	LeftClick(x, y); // ѡ��С��

	InputUserPwd();
}

// �����ʺ�����
void Game::InputUserPwd(bool input_user)
{
	HWND edit = FindWindowEx(m_hWndPic, NULL, NULL, NULL);

	int i;
	if (input_user) {
		Sleep(2000);
		LeftClick(300, 265); // ����ʺſ�
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

	LeftClick(300, 305); // ��������
	Sleep(1000);
	for (i = 0; i < strlen(m_Account.Pwd); i++) {
		Keyborad(m_Account.Pwd[i]);
		Sleep(200);
	}
	LeftClick(265, 430); // ����
}

// ��ʼ��
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

	printf("��ʼ��ȡ��Ϸ����...\n");
	m_pClient->SendMsg("��ȡ��Ϸ����...");

	FindAllCall();
	FindBagAddr();
	
#if 1
	if (!ReadGameMemory()) {
		INLOG("�޷��������Ѫ����ַ������");
		return false;
	}
	while (!m_GameAddr.ItemPtr) {
		printf("��ȡ������Ʒ��ַ...\n");
		ReadGameMemory();
		Sleep(5000);
	}
	while (!m_GameAddr.MapName) {
		printf("��ȡ��ͼ���Ƶ�ַ..\n");
		ReadGameMemory();
		Sleep(5000);
	}
#endif
	m_pMagic->ReadMagic(nullptr, nullptr, false);

	m_pClient->SendMsg("��ȡ��Ϸ�������");

	//printf("���Ӽ��̺��������%s\n", Drv_ConnectDriver()?"�ɹ�":"ʧ��");
#if 0
	while (!m_GameAddr.Life) {
		printf("���¶�ȡѪ����ַ\n");
		ReadGameMemory();
		Sleep(1000);
	}
#endif
	return true;
}

// �ȴ���Ϸ��ʼ�����
void Game::WaitGameInit(int wait_s)
{
	m_pClient->SendMsg("�ȴ���Ϸ��ʼ�����");
	for (int i = 0; i < wait_s; i++) {
		HWND child, parent;
		if (FindButtonWnd(BUTTON_ID_CLOSEMENU, child, parent, "x")) {
			SendMessageW(parent, WM_COMMAND, MAKEWPARAM(BUTTON_ID_CLOSEMENU, BN_CLICKED), (LPARAM)child); // �ر���
			printf("---------��Ϸ�����ѳ�ʼ�����---------\n");
			m_pClient->SendMsg("��Ϸ�����ѳ�ʼ�����");
			break;
		}

		printf("�ȴ���Ϸ���ݳ�ʼ���, ��ʣ%02d��[%d��].\n", wait_s - i, wait_s);
		Sleep(1000);
	}

	m_pGameProc->Button(BUTTON_ID_CANCEL,    1500);
	m_pGameProc->Button(BUTTON_ID_SURE,      1500);
	m_pGameProc->Button(BUTTON_ID_CLOSEMENU, 1500);
}

// �����Ƿ��Ǵ��
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

	printf("��Ϸ������:%s.%s\n", ser_big, ser_small);
	printf("�ʺ�:%s|%s ��%d����ɫ %s\n", name, pwd, role_no, big?"���":"С��");
	if (getxl_logout)
		printf("��ȡ��������˳�\n");

	Login();
}

// �����Ƿ����
void Game::SetOffLine(int v)
{
	m_Account.IsOffLine = v;
}

// �����Ƿ���ħ��
bool Game::IsMoYu()
{
	wchar_t name[32] = { 0 };
	GetProcessName(name, GetCurrentProcessId());
	wprintf(L"%ws = %ws\n", name, L"soul.exe");
	return wcsstr(name, L"soul.exe") != nullptr;
}

// �Ƿ��¼��
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

// �Ƿ������ 
bool Game::IsOffLine()
{
	return m_Account.IsOffLine;
}

// �Ƚϵ�¼�ʺ�
bool Game::CmpLoginAccount(const char* name)
{
	return strcmp(name, m_Account.Name) == 0;
}

// �Ƿ���
bool Game::IsBig()
{
	//return true;
	return m_Account.IsBig;
}

// �Ƿ��ڸ���
bool Game::IsInFB()
{
	return IsInMap("������Ŀ���");
}

// �Ƿ���ָ����ͼ
bool Game::IsInMap(const char* name)
{
	char map_name[32] = { 0 };
	if (ReadMemory((PVOID)m_GameAddr.MapName, map_name, sizeof(map_name))) { // ��ȡ��ͼ����
		return strcmp(map_name, name) == 0;
	}
	return false;
}

// �Ƿ���ָ���������� allow=���
bool Game::IsInArea(int x, int y, int allow)
{
	ReadCoor();
	int cx = (int)m_dwX - x;
	int cy = (int)m_dwY - y;

	return abs(cx) <= allow && abs(cy) <= allow;
}

// �Ƿ���ָ���������� allow=���
bool Game::IsNotInArea(int x, int y, int allow)
{
	ReadCoor();
	int cx = (int)m_dwX - x;
	int cy = (int)m_dwY - y;

	//printf("IsNotInArea:%d,%d %d,%d\n", m_dwX, m_dwY, x, y);
	return abs(cx) > allow || abs(cy) > allow;
}

// �Ƿ��ѻ�ȡ������
bool Game::IsGetXL()
{
	if (m_Account.IsGetXL) {
		tm t;
		time_t get_time = m_Account.GetXLTime;
		gmtime_s(&t, &get_time);

		tm t2;
		time_t now_time = time(nullptr);
		gmtime_s(&t2, &now_time);

		if (t.tm_mday != t2.tm_mday) { // ������ȡ��
			m_Account.IsGetXL = 0;
		}
	}
	return m_Account.IsGetXL != 0;
}

// ����
void Game::Run()
{
	if (0 && !IsBig())
		return;

	m_pGameProc->Run();
}

// ��ȡ��Ϸ����λ������Ļ�ϵ�����
void Game::GetGameCenterPos(int & x, int & y)
{
	GetWindowRect(m_hWnd, &m_GameWnd.Rect);
	m_GameWnd.Width = m_GameWnd.Rect.right - m_GameWnd.Rect.left;
	m_GameWnd.Height = m_GameWnd.Rect.bottom - m_GameWnd.Rect.top;

	x = m_GameWnd.Rect.left + (m_GameWnd.Width / 2);
	y = m_GameWnd.Rect.top + (m_GameWnd.Height / 2);
}

// �����Ϸ����
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
	printf("���ھ��:%0x %d %d %.2\n", m_hWnd, m_GameWnd.Width, m_GameWnd.Height, m_fScale);
	printf("��Ϸ���洰��:%08X\n", m_hWndPic);

	m_pClient->SendOpen(m_fScale, m_hWnd, m_GameWnd.Rect); // ֪ͨ�����Ѵ�
	return m_hWnd;
}

// ��ȡ����ģ���ַ
void Game::FindAllModAddr()
{
	m_GameModAddr.Mod3DRole = FindModAddr(MOD_3drole);
	m_GameModAddr.Mod3DGameMap = FindModAddr(MOD_3dgamemap);
}

// ��ȡ��Ϸ����CALL
void Game::FindAllCall()
{
	SearchModFuncMsg info[] = {
		{"?g_pObjHero",            NULL,  0, &g_pObjHero,                  "g_pObjHeroȫ�ֱ���"},
		{"?g_objPlayerSet",        NULL,  0, &g_objPlayerSet,              "g_objPlayerSetȫ�ֱ���"},
		{"?ReBorn@CHero",          NULL,  0, &m_GameCall.ReBorn,           "���︴��"},
		{"?Run@CHero",             NULL,  0, &m_GameCall.Run,              "�ƶ�����"},
		{"?ActiveNpc@CHero",       NULL,  0, &m_GameCall.ActiveNpc,        "NPC�Ի�"},
		{"?GetInstance@ITaskManager",NULL,0, &m_GameCall.ITaskGetInstance, "NPC�Ի�ѡ��"},
		{"?GetPackageItemByIndex@CHero",  NULL, 0, &m_GameCall.GetPkageItemByIndex, "��ȡ��Ʒָ��"},
		{"?UseItem@CHero",         NULL,  0, &m_GameCall.UseItem,          "ʹ����Ʒ"},
		{"?DropItem@CHero",        NULL,  0, &m_GameCall.DropItem,         "������Ʒ"},
		{"?PickUpItem@CHero",      NULL,  0, &m_GameCall.PickUpItem,       "��ʰ��Ʒ"},
		{"?SellItem@CHero",        NULL,  0, &m_GameCall.SellItem,         "����Ʒ"},
		{"?SaveMoney@CHero",       NULL,  0, &m_GameCall.SaveMoney,        "��Ǯ"},
		{"?CheckInItem@CHero",     NULL,  0, &m_GameCall.CheckInItem,      "����Զ�ֿ̲�"},
		{"?CheckOutItem@CHero",    NULL,  0, &m_GameCall.CheckOutItem,     "ȡ��Զ�ֿ̲�"},
		{"?OpenBank@CHero",        NULL,  0, &m_GameCall.OpenBank,         "��Զ�ֿ̲�"},
		{"?TransmByMemoryStone@CHero",    NULL,  0, &m_GameCall.TransmByMemoryStone, "ʹ�ÿɴ�����Ʒ"},
		{"?MagicAttack@CHero",     "POS", 0, &m_GameCall.MagicAttack_GWID, "ʹ�ü���-����ID "},
		{"?MagicAttack@CHero",     "POS", 1, &m_GameCall.MagicAttack_XY,   "ʹ�ü���-XY����"},
		{"?CallEudenmon@CHero",    NULL,  0, &m_GameCall.CallEudenmon,     "�������"},
		{"?KillEudenmon@CHero",    NULL,  0, &m_GameCall.KillEudenmon,     "�����ٻ�"},
		{"?AttachEudemon@CHero",   NULL,  0, &m_GameCall.AttachEudemon,    "�������"},
		{"?UnAttachEudemon@CHero", NULL,  0, &m_GameCall.UnAttachEudemon,  "�������"},
		{"?GetData@CPlayer",       NULL,  0, &m_GameCall.SetRealLife,      "CPlayer::GetData"},
		{"?QueryInterface_RemoteTeam@CHero", NULL,  0, &m_GameCall.QueryInf_RemoteTeam,"��ȡԶ�����������ַ"},
		{"?IsHaveTeam@CHero",      NULL,  0, &m_GameCall.IsHaveTeam,       "�Ƿ��ж���"},
		{"?IsTeamLeader@CHero",    NULL,  0, &m_GameCall.IsTeamLeader,     "�Ƿ��Ƕӳ�"},
		{"?TeamCreate@CHero",      NULL,  0, &m_GameCall.TeamCreate,       "��������"},
		{"?TeamDismiss@CHero",     NULL,  0, &m_GameCall.TeamDismiss,      "�뿪����[�ӳ�]"},
		{"?TeamLeave@CHero",       NULL,  0, &m_GameCall.TeamLeave,        "�뿪����[��Ա]"},
		{"?TeamInvite@CHero",      NULL, 0, &m_GameCall.TeamInvite,        "�������"},
		{"?SetAutoJoinStatus@CHero",NULL, 0, &m_GameCall.TeamAutoJoin,     "�Զ�����"},
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

// ��ȡNPC�Ի�����
DWORD Game::FindNPCTalkCall()
{
	// ��push 0x00��ʼ����
	DWORD codes[] = {
		0x0C8D006A, 0x04E1C192, 0x948BCA2B, 0x00000011, //0x0016948E,
		0x52C88B00, 0x013897FF, 0x006A0000, 0x1234CE8B
	};

	DWORD address = 0;
	if (SearchInMod(L"soul.exe", codes, sizeof(codes) / sizeof(DWORD), &address, 1, 1)) {
		DWORD offset = P2DW(address + 0x1F);
		address += offset + 0x23;
		printf("CallNPC�Ի�ѡ��2:%08X\n", address);
	}
	return address;
}

// ��ȡ�����������ѡ�����
DWORD Game::FindTeamChkCall()
{
	// ��push 0xFFFFFF��ʼ����
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
		printf("Call�����������ѡ��:%08X %08X\n", address, tmp);
		printf("�����������ѡ��ESIƫ��:%08X\n", m_GameAddr.TeamChkOffset);
	}
	return address;
}

// ���ְ�������
DWORD Game::FindKeyNumCall()
{
	// ��push 0x00��ʼ����
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
		printf("Call���ְ���:%08X %08X\n", address, tmp);
		printf("���ְ���ECXָ���ַ:%08X\n", m_GameAddr.KeyNumEcxPtr);
	}
	return address;
}

// ��ȡ�ر���ʾ����
DWORD Game::FindCloseTipBoxCall()
{
	// mov dword ptr ds:[ebx+0xA0],esi
	// ��������->CE��ʾ��״̬��ַ���޸Ķϵ�->�ҵ��޸ĵ�ַ->OD�¶ϵ�, �鿴���ö�ջ�����ҵ�
	DWORD codes[] = {
		0x51EC8B55, 0x8B575653, 0xFF83087D, 0x0FD98B0A,
		0x00000011, 0x0C751234
	};

	DWORD address = 0;
	if (SearchInMod(L"soul.exe", codes, sizeof(codes) / sizeof(DWORD), &address, 1, 1)) {
		printf("Call�ر���ʾ������ַ:%08X\n", address);
	}
	return address;
}

// ��ȡ��ȡNPC����ַ����
DWORD Game::FindGetNpcBaseAddr()
{
	// �������� �ҵ�NPC��ַ->�·��ʶϵ㣬���NPC���ϵĶϵ�, �ҵ����ʶϵ��¶�
	// [��CGamePlayerSet::Process����]mov dword ptr ds:[ecx+0x44],edx
	// ����Ѱ�Ҽ����ҵ� add ecx,0x14
	DWORD codes[] = {
		0x83EC8B55, 0x4D8924EC, 0x08458BDC, 0xE04D8D50,
		0xF0558D51, 0xDC4D8B52
	};

	DWORD address = 0;
	if (SearchInMod(L"3drole.dll", codes, sizeof(codes) / sizeof(DWORD), &address, 1, 1)) {
		printf("Call��ȡ��ȡNPC����ַ:%08X\n", address);
	}
	return address;
}

// ��ȡģ���ַ
DWORD Game::FindModAddr(LPCWSTR name)
{
	HANDLE hMod = NULL;
	while (hMod == NULL) {
		hMod = GetModuleHandle(name);
		wprintf(L"%ws:%08X\n", name, hMod);
	}
	return (DWORD)hMod;
}

// ��ȡ�����ַ
bool Game::FindCoorAddr()
{
	while (!IsLogin()) {
		printf("�ȴ���¼��Ϸ������\n");
		Sleep(5000);
	}

	try {
		DWORD p;
		//ReadDwordMemory(0x123, p);
	}
	catch (...) {
		printf("����PPPPPPPPPPPPPPPPPPPPPPPPP\n");
	}

	m_GameAddr.CoorX = m_GameModAddr.Mod3DRole + ADDR_COOR_X_OFFSET;
	m_GameAddr.CoorY = m_GameModAddr.Mod3DRole + ADDR_COOR_Y_OFFSET;
	while (PtrToDword(m_GameAddr.CoorX) == 0 || PtrToDword(m_GameAddr.CoorY) == 0) {
		printf("���ڽ�����Ϸ������\n");
		HWND child, parent;
		if (FindButtonWnd(BUTTON_ID_LOGIN, child, parent)) { // ��ѡ��������밴ť
			//printf("����ѡ���ɫ��ť\n");
			if (1 && m_Account.RoleNo > 0) {
				printf("ѡ���ɫ:%d\n", m_Account.RoleNo + 1);
				HWND child2, parent2;
				int btn_id = BUTTON_ID_ROLENO + m_Account.RoleNo;
				FindButtonWnd(btn_id, child2, parent2, "��ɫ");
				SendMessageW(parent2, WM_COMMAND, MAKEWPARAM(btn_id, BN_CLICKED), (LPARAM)child2); // ѡ���ɫ
				
				RECT rect;
				GetWindowRect(child2, &rect);
				//Drv_LeftClick(float(rect.left + 20) / m_fScale, float(rect.top + 20) / m_fScale);
				printf("��ťID:%X %08X %08X %d %d %d %d\n", btn_id, child2, parent2, rect.left, rect.top, rect.right, rect.bottom);
				Sleep(500);
				GetWindowRect(child, &rect);
				//Drv_LeftClick(float(rect.left + 10) / m_fScale, float(rect.top + 10) / m_fScale);
				printf("��ťID2:%X %08X %08X %d %d %d %d\n", btn_id, child, parent, rect.left, rect.top, rect.right, rect.bottom);
				SendMessageW(parent, WM_COMMAND, MAKEWPARAM(BUTTON_ID_LOGIN, BN_CLICKED), (LPARAM)child); // ����
			}
		}
		Sleep(1500);
	}
	printf("��������:%d,%d\n", PtrToDword(m_GameAddr.CoorX), PtrToDword(m_GameAddr.CoorY));

	strcpy(m_Account.Name, (const char*)(m_GameModAddr.Mod3DRole + ADDR_ACCOUNT_NAME));
	strcpy(m_Account.Role, (const char*)(m_GameModAddr.Mod3DRole + ADDR_ROLE_NAME));
	printf("�ʺ�:%s ��ɫ:%s\n", m_Account.Name, m_Account.Role);
	m_pClient->SendInGame(m_Account.Name, m_Account.Role); // ֪ͨ����������
	return true;
}

// ��ȡ�ƶ�״̬��ַ
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
		printf("�ƶ�״̬��ַ��%08X\n", m_GameAddr.MovSta);
	}
	return address > 0;
}

// ��ȡ�Ի���״̬��ַ
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
		printf("NPC�����Ի�ESI:%08X\n", m_GameAddr.CallNpcTalkEsi);
		printf("�Ի���״̬��ַ��%08X\n", m_GameAddr.TalKBoxSta);
	}
	return address > 0;
}

// ��ȡ�Ƿ�ѡ���������״̬��ַ
bool Game::FindTeamChkStaAddr()
{
	// 29D0
	// 4:0x00F63018 4:0x00000001 4:0x00000000 4:0x00000000 4:0x00000000 4:0x00000001 4:0x00000000
	DWORD codes[] = {
		0x00F63018, 0x00000001, 0x00000000, 0x00000000,
		0x00000000, 0x00000001, 0x00000000, 0x00000022
	};
	DWORD address = 0; // �����ַ��Callѡ���Ƿ�������麯����esi��ֵ mov ecx,dword ptr ds:[esi+0x1DC]
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.TeamChkSta = address; // +29D0��TeamChkStaƫ��ͷ ��+100��ѡ���״̬
		printf("�������״̬��ַ��%08X\n", m_GameAddr.TeamChkSta);
	}
	return address > 0;
}

// ��ȡ��ʾ��״̬��ַ
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
		printf("��ʾ��״̬��ַ��%08X\n", m_GameAddr.TipBoxSta);
	}
	return address > 0;
}

// ��ȡ������ַ
bool Game::FindLifeAddr()
{
#if 0
	m_GameAddr.Life = m_GameModAddr.Mod3DRole + ADDR_LIFE_OFFSET;
	m_GameAddr.LifeMax = m_GameModAddr.Mod3DRole + ADDR_LIFEMAX_OFFSET;
	printf("�ҵ�������ַ��%08X\n", m_GameAddr.Life);
	printf("�ҵ��������޵�ַ��%08X\n", m_GameAddr.LifeMax);
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
		printf("�ҵ�������ַ��%08X\n", m_GameAddr.Life);
		printf("�ҵ��������޵�ַ��%08X\n", m_GameAddr.LifeMax);
	}
	return address > 0;
#endif
}

// ��ȡ��������
bool Game::FindBagAddr()
{
	// Soul.exe+C8C12C
	// Soul.exe + CA97C8
	// 4:0x281 4:0x1E6 4:0xA04 4:0x05 4:0x05 4:0x1E1E
	// 023C78D5 
	// ��������->CE������Ʒ����->�·��ʶϵ�, ��û�ַ->��ַ�·��ʶϵ�õ�ƫ��
	// CHero::GetItem�������� �����ĸ�λ����Ҫ�������Ĳ鿴
	try {
		DWORD p, count;
		__asm
		{
			mov eax, g_pObjHero
			mov eax, [eax]
			mov eax, [eax + 0x224C]     // 2234
			mov edx, [eax + 0x10]       // ��Ʒ��ַָ��
			mov dword ptr[p], edx
			mov edx, [eax + 0x30]       // ��Ʒ����
			mov dword ptr[count], edx
		}
		printf("��Ʒָ��:%08X ����:%d\n", p, count);
		//m_GameAddr.Bag = p;

		return p != 0;
	}
	catch (...) {
		printf("Game::FindBagAddrʧ��!\n");
	}
	return false;
}

// ��õ�����Ʒ��ַ�ı����ַ
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
		INLOGVARN(32, "������Ʒ��ַ�����ַ:%08X", m_GameAddr.ItemPtr);
		printf("������Ʒ��ַ�����ַ:%08X\n", m_GameAddr.ItemPtr);
	}

	return address != 0;
}

// ��ȡNPC�����Ի�ESI�Ĵ�����ֵ
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
		printf("NPC�����Ի�ESI:%08X\n", m_GameAddr.CallNpcTalkEsi);
	}

	return address != 0;
}

// ��ȡ�����б����ַ
bool Game::FindPetPtrAddr()
{
	// �������� ���ҳ�����Ѫ��->CE��Ѫ�����ʶϵ�, �ҵ���ַ, �鿴������
	// ����ID[773610E3��������:800750]->��ó���ID��ַ->�·��ʶϵ�->�ҵ�ƫ��
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
	printf("�����б��ַ:%08X\n", address);
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
		printf("�����б��ַ:%08X [%08X] %08X\n", address, PtrToDword(address), codes);
	}
#endif
	return address != 0;
}

// ��ȡ��ͼ���Ƶ�ַ
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
		printf("��ͼ���Ƶ�ַ:%08X[%s]\n", m_GameAddr.MapName, m_GameAddr.MapName);
	}
	return address != 0;
}

// ��ĳ��ģ������������
DWORD Game::SearchFuncInMode(SearchModFuncMsg* info, HANDLE hMod)
{
	//�õ�DOSͷ
	IMAGE_DOS_HEADER* dosheader = (PIMAGE_DOS_HEADER)hMod;
	//Ч���Ƿ�DOSͷ
	if (dosheader->e_magic != IMAGE_DOS_SIGNATURE)return NULL;

	//NTͷ
	PIMAGE_NT_HEADERS32 NtHdr = (PIMAGE_NT_HEADERS32)((CHAR*)hMod + dosheader->e_lfanew);
	//Ч���Ƿ�PEͷ
	if (NtHdr->Signature != IMAGE_NT_SIGNATURE)return NULL;

	//�õ�PEѡ��ͷ
	IMAGE_OPTIONAL_HEADER32* opthdr = (PIMAGE_OPTIONAL_HEADER32)((PBYTE)hMod + dosheader->e_lfanew + 24);
	//�õ�������
	IMAGE_EXPORT_DIRECTORY* pExportTable = (PIMAGE_EXPORT_DIRECTORY)((PBYTE)hMod + opthdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
	//�õ�������ַ�б�
	PDWORD arrayOfFunctionAddresses = (PDWORD)((PBYTE)hMod + pExportTable->AddressOfFunctions);
	//�õ����������б�
	PDWORD arrayOfFunctionNames = (PDWORD)((PBYTE)hMod + pExportTable->AddressOfNames);
	//�õ��������
	PWORD arrayOfFunctionOrdinals = (PWORD)((PBYTE)hMod + pExportTable->AddressOfNameOrdinals);
	//���������ַ
	DWORD Base = pExportTable->Base;
	//ѭ��������
	for (DWORD x = 0; x < pExportTable->NumberOfNames; x++)			//�������������� ���֮�֣�������������=���Ƶ���+��ŵ�����������ѭ���������Ƶĺ���
	{
		//�õ������� 
		PSTR functionName = (PSTR)((PBYTE)hMod + arrayOfFunctionNames[x]);
		if (strstr(functionName, info->Name)) { // ���ڴ�����
			bool find = true;
			if (info->Substr) { // �����Ӵ�
				PSTR sub = strstr(functionName, info->Substr);
				if ((sub && !info->Flag) || (!sub && info->Flag)) { // ������Ҫ��
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

// ��ĳ��ģ����������
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

// ����������
DWORD Game::SearchCode(DWORD* codes, DWORD length, DWORD* save, DWORD save_length, DWORD step)
{
	if (length == 0 || save_length == 0)
		return 0;

	DWORD count = 0;
	for (DWORD i = 0; i < m_dwReadSize; i += step) {
		if ((i + length) > m_dwReadSize)
			break;

		DWORD addr = m_dwReadBase + i;
		if (addr == (DWORD)codes) { // �����Լ�
			//printf("���������Լ�:%08X\n", codes);
			return 0;
		}

		DWORD* dw = (DWORD*)(m_pReadBuffer + i);
		bool result = true;
		for (DWORD j = 0; j < length; j++) {
			if (codes[j] == 0x11) { // �����
				result = true;
			}
			else if (codes[j] == 0x22) { // ��Ҫ��ֵ��Ϊ0
				if (dw[j] == 0) {
					result = false;
					break;
				}
			}
			else if (((codes[j] & 0xffff0000) == 0x12340000)) { // ��2�ֽ����
				if ((dw[j]&0x0000ffff) != (codes[j]&0x0000ffff)) {
					result = false;
					break;
				}
				else {
					//printf("%08X\n", dw[j]);
				}
			}
			else if (((codes[j] & 0x0000ffff) == 0x00001234)) { // ��2�ֽ����
				if ((dw[j] & 0xffff0000) != (codes[j] & 0xffff0000)) {
					result = false;
					break;
				}
			}
			else {
				if ((codes[j] & 0xff00) == 0x1100) { // �Ƚ�������ַ��ֵ��� ���8λΪ�Ƚ�����
					//printf("%X:%X %08X:%08X\n", j, codes[j] & 0xff, dw[j], dw[codes[j] & 0xff]);
					if (dw[j] != dw[codes[j] & 0xff]) {
						result = false;
						break;
					}
				}
				else if (dw[j] != codes[j]) { // ������ֵ�����
					result = false;
					break;
				}
			}
		}

		if (result) {
			save[count++] = addr;
			//printf("��ַ:%08X   %08X\n", addr, codes);
			if (count == save_length)
				break;
		}
	}

	return count;
}

// ��ȡ���ֽ�����
bool Game::ReadDwordMemory(DWORD addr, DWORD& v)
{
	return ReadMemory((PVOID)addr, &v, 4);
}

// ��ȡ�ڴ�
bool Game::ReadMemory(PVOID addr, PVOID save, DWORD length)
{
	if (IsBadReadPtr(addr, length)) {
		printf("�����ַ:%08X %d\n", addr, length);
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
	//printf("ReadProcessMemory:%d %08X %d Read:%d ��ֵ:%d(%08X)\n", GetLastError(), addr, result, dwRead, *(DWORD*)save, *(DWORD*)save);

	if (!result || dwRead != length) {
		printf("ReadProcessMemory����:%d %08X %d\n", GetLastError(), addr, result);
		if (GetLastError() == 6) {
			m_pGameProc->WriteLog("GetLastError() == 6", true);
			Sleep(100);
			//CloseHandle(m_hGameProcess);
			m_pGameProc->WriteLog("�ر�m_hGameProcess", true);
			Sleep(100);
			m_hGameProcess = GetCurrentProcess();
			return ReadProcessMemory(m_hGameProcess, addr, save, length, NULL);
		}
	}

	if (mod)
		VirtualProtect(addr, length, oldProtect, &oldProtect);
	return result;
}

// ��ȡ����
bool Game::ReadCoor(DWORD* x, DWORD* y)
{
start:
	DWORD vx = 0, vy = 0;
	if (!ReadDwordMemory(m_GameAddr.CoorX, vx)) {
		printf("�޷���ȡ����X(%d) %08X\n", GetLastError(), m_GameAddr.CoorX);
		return false;
	}
	if (!ReadDwordMemory(m_GameAddr.CoorY, vy)) {
		printf("�޷���ȡ����Y(%d) %08X\n", GetLastError(), m_GameAddr.CoorY);
		return false;
	}

	int now_time = time(nullptr);
	if (m_iFlagPosTime == 0) { // ��һ�β�����
		m_iFlagPosTime = now_time;
	}
	else {
		if (m_dwX == vx && m_dwY == vy) { // �������ϴ�һ��
			int allow_second = 120; // 1�������껹һ����������
			if ((now_time - m_iGetPosTime) < allow_second) { // ��ȡ����ʱ�������
				if ((now_time - m_iFlagPosTime) > allow_second) { // �ܾ����껹һ��, ���ܵ�����
					printf("���ܵ�����\n");
					//m_pClient->SendMsg("���ܵ�����, ׼��������Ϸ");
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

// ��ȡ����ֵ
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

// ��ȡҩ������
bool Game::ReadQuickKey2Num(int* nums, int length)
{
	if (!m_GameAddr.QuickKey2Num) {
		memset(nums, 0, length);
		return false;
	}

	return ReadProcessMemory(m_hGameProcess, (PVOID)m_GameAddr.QuickKey2Num, nums, length, NULL);
}

// ��ȡ������Ʒ
bool Game::ReadBag(DWORD* bag, int length)
{
	return ReadProcessMemory(m_hGameProcess, (PVOID)m_GameAddr.Bag, bag, length, NULL);
}

// ��ô��ھ��
bool Game::FindButtonWnd(int button_id, HWND& hwnd, HWND& parent, const char* text)
{
	HWND wnds[] = { (HWND)button_id, NULL, (HWND)text };
	::EnumChildWindows(m_hWnd, EnumChildProc, (LPARAM)wnds);
	hwnd = wnds[0];
	parent = wnds[1];
	return parent != NULL;
}

// ��ȡ��Ϸ�ڴ�
bool Game::ReadGameMemory(DWORD flag)
{
	m_dwGuaiWuCount = 0; // ���ù�������
	m_bIsReadEnd = false;

	MEMORY_BASIC_INFORMATION mbi;
	memset(&mbi, 0, sizeof(MEMORY_BASIC_INFORMATION));
	DWORD_PTR MaxPtr = 0x70000000; // ����ȡ�ڴ��ַ
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
			//printf("%p-%p ===����, ��С:%d\n", ReadAddress, ReadAddress + mbi.RegionSize, mbi.RegionSize);
			goto _c;
		}
		else {
			DWORD pTmpReadAddress = ReadAddress;
			DWORD dwOneReadSize = 0x1000; // ÿ�ζ�ȡ����
			DWORD dwReadSize = 0x00;      // �Ѷ�ȡ����
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
					//printf("%p-%p ===��ȡʧ��, ��С:%d, ������:%d\n", pTmpReadAddress, pTmpReadAddress + m_dwReadSize, (int)m_dwReadSize, GetLastError());
				}

				dwReadSize += m_dwReadSize;
				pTmpReadAddress += m_dwReadSize;
			}

			count++;
		}
	_c:
		// ��ȡ��ַ����
		ReadAddress += mbi.RegionSize;
		//Sleep(8);
		//printf("%016X---�ڴ��С2:%08X\n", ReadAddress, mbi.RegionSize);
		// ɨ0x10000000�ֽ��ڴ� ����100����
	}
end:
	__int64 ms2 = getmillisecond();
	printf("��ȡ���ڴ���ʱ:%d����\n", ms2 - ms);
	m_bIsReadEnd = true;
	return true;
}

// ��ӡ�ռ�
void Game::InsertLog(char * text)
{
	//g_dlg->InsertLog(text);
}

// ��ӡ�ռ�
void Game::InsertLog(wchar_t * text)
{
	//g_dlg->InsertLog(text);
}

// ��ȡ�����������
void Game::GetSerBigClickPos(int& x, int& y)
{
	const char* str = m_Account.SerBig;
	if (strstr(str, "��һ����")) {
		SET_VAR2(x, 125, y, 150);
		return;
	}
	if (strstr(str, "�ڶ�����")) {
		SET_VAR2(x, 125, y, 185);
		return;
	}
	if (strstr(str, "���ߴ���")) {
		SET_VAR2(x, 125, y, 350);
		return;
	}
	if (strstr(str, "��ʮ������")) {
		SET_VAR2(x, 125, y, 510);
		return;
	}
	if (strstr(str, "��ʮ������")) {
		SET_VAR2(x, 125, y, 545);
		return;
	}
	if (strstr(str, "����ʮ������")) {
		SET_VAR2(x, 306, y, 376);
		return;
	}
	if (strstr(str, "����ʮ�˴���")) {
		SET_VAR2(x, 500, y, 507);
		return;
	}
}

// ��ȡ�����������
void Game::GetSerSmallClickPos(int & x, int & y)
{
	const char* str = m_Account.SerSmall;
	if (strstr(str, "������/��/����")) {
		SET_VAR2(x, 520, y, 435);
		return;
	}
	if (strstr(str, "�㺼��/����")) {
		SET_VAR2(x, 520, y, 510);
		return;
	}
	if (strstr(str, "����һ��")) {
		SET_VAR2(x, 520, y, 475);
		return;
	}
	if (strstr(str, "������/����")) {
		SET_VAR2(x, 520, y, 435);
		return;
	}
	if (strstr(str, "ǭ����/����")) {
		SET_VAR2(x, 520, y, 473);
		return;
	}
	if (strstr(str, "������ʮ������")) {
		SET_VAR2(x, 520, y, 355);
		return;
	}
}

LRESULT Game::CldKeyBoardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

// ö�ٴ���
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

// ö���Ӵ���
BOOL Game::EnumChildProc(HWND hWnd, LPARAM lParam)
{
	HWND* param = (HWND*)lParam;
	HWND hWnd_Child = ::GetDlgItem(hWnd, (int)param[0]);
	if (hWnd_Child) { // �ҵ����Ӵ���
		if (param[2]) { // ���������ť����
			char text[32];
			GetWindowTextA(hWnd_Child, text, sizeof(text)); // ��ȡ��ť����
			if (strstr(text, (const char*)param[2]) == nullptr)
				return TRUE;
		}

		param[0] = hWnd_Child; // �Ӵ��ھ�� 
		param[1] = hWnd;       // �����ھ��
		
		return FALSE;
	}
	return TRUE;
}

// ����
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
	//printf("����:%d %08X %08X %d\n", key, lParam, hwnd, GetLastError());
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
	//printf("�������:%d,%d %08X\n", x, y, hwnd);
}

// ����ƶ�
void Game::MouseMove(int x, int y)
{
	PostMessageA(m_hWndPic, WM_MOUSEMOVE, 0x00, MAKELPARAM(x, y));
	Sleep(50);
}

// ���︴��
void Game::Call_ReBoren()
{
	printf("���︴��\n");
	_FUNC1 func = (_FUNC1)m_GameCall.ReBorn;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(0x00);
	ASM_RESTORE_ECX();
}

// �����ƶ�����
void Game::Call_Run(int x, int y)
{
	printf("�ƶ�:%d,%d\n", x, y);
	// 4:0x6AEC8B55 4:0xBE7468FF 4:0xA1640326 4:0x00000000 4:0x25896450 4:0x00000000 4:0x04D8EC81 4:0x56530000
	_FUNC3 func = (_FUNC3)m_GameCall.Run;
	if (IsBadCodePtr((FARPROC)func)) {
		printf("Run������Ч\n");
		return;
	}
	// 20674C3
	// 20675CA
	DWORD t = 0;// P2DW(0x000);
	if (t) {
		printf("ttttttttttttttttt\n");
	}
	//printf("׼�������ƶ�����\n");
	char tmp[0x1000] = { 0 };
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(x, y, 0x00);
	ASM_RESTORE_ECX();
	//printf("�ƶ������������\n");
	if (tmp[0] == 0) {
		DWORD ttt = 2;
		if (ttt && tmp[0]) {
			printf("���������!\n");
		}
	}
}

// ����CALL[type 0=����Ƶ�� 1=˽�� 2=����]
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
		push 0x00       // 0x015A63F8 // �����Ǳ�����ַ
		push 0x15A62D0  // ��������[�̶���ַ]
		push 0x00       // 0x015A63D8 // �����Ǳ�����ַ
		mov eax, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [eax]
		call func
	}
}

// NPC�Ի�
void Game::Call_NPC(int npc_id)
{
	//INLOGVARN(32, "��NPC:%08X\n", npc_id);
	printf("��NPC:%08X\n", npc_id);
	_FUNC2 func = (_FUNC2)m_GameCall.ActiveNpc;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(npc_id, 0x00);
	ASM_RESTORE_ECX();
}

// NPC�����Ի�
void Game::Call_NPCTalk(int no, bool close)
{
	//INLOGVARN(32, "�Ի�NPC��%d\n", no);
	// eax=038735B8-2550000=13235B8
	// edi=0343DDD8-2550000=EEDDD8
	// esi=05DD5314

	_FUNC_R func = (_FUNC_R)m_GameCall.ITaskGetInstance;
	DWORD _eax = func();
	DWORD _esi = m_GameAddr.CallNpcTalkEsi;
	printf("NPC�Ի�ѡ�� no:%d _eax:%08X _edi:%08X _esi:%08X\n", no, _eax, PtrToDword(_eax), _esi);

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

// �ر���ʾ��
void Game::Call_CloseTipBox(int close)
{
	printf("�ر���ʾ��:%d\n", close);
	DWORD _ecx = m_GameAddr.TipBoxSta;
	_FUNC3 func = (_FUNC3)m_GameCall.CloseTipBox;
	__asm { mov ecx, _ecx };
	func(0x0A, close, 0x00);
}

// ��ȡ��Ʒָ��
DWORD Game::Call_GetPackageItemByIndex(int index)
{
	_FUNC2_R func = (_FUNC2_R)m_GameCall.GetPkageItemByIndex;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	return func(index, 0x0A);
	ASM_RESTORE_ECX();
}
	

// ʹ����Ʒ
void Game::Call_UseItem(int item_id)
{
	printf("ʹ����Ʒ:%08X\n", item_id);
	_FUNC5 func = (_FUNC5)m_GameCall.UseItem;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(item_id, 0x00, 0xFFFFFFFF, 0xFFFFFFFF, 0x00);
	ASM_RESTORE_ECX();
}

// ����Ʒ
void Game::Call_DropItem(int item_id)
{
	printf("������Ʒ:%08X\n", item_id);
	_FUNC3 func = (_FUNC3)m_GameCall.DropItem;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(item_id, 0x00, 0x00);
	ASM_RESTORE_ECX();
	// 4:0x00 4:0x04 4:0x80000000 4:0x80000000 4:0x7FFFFFFF 4:0x7FFFFFFF 4:* 4:0x01 4:0x19 4:0x00400000
}

// ����Ʒ
void Game::Call_PickUpItem(DWORD id, DWORD x, DWORD y)
{
	printf("ʰȡ��Ʒ:%08X %d,%d\n", id, x, y);
	_FUNC3 func = (_FUNC3)m_GameCall.PickUpItem;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(id, x, y);
	ASM_RESTORE_ECX();
}

// ������
void Game::Call_SellItem(int item_id)
{
	printf("������:%08X\n", item_id);
	_FUNC1 func = (_FUNC1)m_GameCall.SellItem;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(item_id);
	ASM_RESTORE_ECX();
}

// ��Ǯ
void Game::Call_SaveMoney(int money)
{
	printf("��Ǯ:%d\n", money);
	_FUNC1 func = (_FUNC1)m_GameCall.SaveMoney;
	if (!func)
		return;

	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(money);
	ASM_RESTORE_ECX();
}

// ����Զ�ֿ̲�
void Game::Call_CheckInItem(int item_id)
{
	//printf("������Ʒ:%08X\n", item_id);
	_FUNC2 func = (_FUNC2)m_GameCall.CheckInItem;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(item_id, 0x0A);
	ASM_RESTORE_ECX();
}

// ȡ���ֿ���Ʒ
void Game::Call_CheckOutItem(int item_id)
{
	printf("ȡ���ֿ���Ʒ:%08X\n", item_id);
	_FUNC3 func = (_FUNC3)m_GameCall.CheckOutItem;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(item_id, 0x0A, 0x00);
	ASM_RESTORE_ECX();
}

// ʹ�ÿɴ�����Ʒ
void Game::Call_TransmByMemoryStone(int item_id)
{
	printf("ʹ�ÿɴ�����Ʒ:%08X\n", item_id);
	_FUNC1 func = (_FUNC1)m_GameCall.TransmByMemoryStone;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(item_id);
	ASM_RESTORE_ECX();
}

// �ż���
void Game::Call_Magic(int magic_id, int guaiwu_id)
{
	printf("���ܹ���ID:%08X %08X\n", magic_id, guaiwu_id);
	_FUNC4 func = (_FUNC4)m_GameCall.MagicAttack_GWID;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(magic_id, guaiwu_id, 0x00, 0x00);
	ASM_RESTORE_ECX();
}

// �ż���
void Game::Call_Magic(int magic_id, int x, int y)
{
	// 05D05020 06832EA4 mov ecx,dword ptr ds:[0xF1A518]
	printf("����XY:%08X %d,%d\n", magic_id, x, y);
	_FUNC5 func = (_FUNC5)m_GameCall.MagicAttack_XY;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(magic_id, x, y, 0x00, 0x00);
	ASM_RESTORE_ECX();
	printf("����ʹ�����\n");
}

// �������
void Game::Call_PetOut(int pet_id)
{
	// 00870741 - 89 8C B3 C0020000  - mov [ebx+esi*4+000002C0],ecx
	// 724-008
	printf("�������:%08X\n", pet_id);
	_FUNC2 func = (_FUNC2)m_GameCall.CallEudenmon;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(pet_id, 0x00);
	ASM_RESTORE_ECX();
}

// �����ٻ�
void Game::Call_PetIn(int pet_id)
{
	_FUNC1 func = (_FUNC1)m_GameCall.KillEudenmon;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(pet_id);
	ASM_RESTORE_ECX();
}

// �������
void Game::Call_PetFuck(int pet_id)
{
	printf("�������:%08X\n", pet_id);
	_FUNC1 func = (_FUNC1)m_GameCall.AttachEudemon;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(pet_id);
	ASM_RESTORE_ECX();
}

// �������
void Game::Call_PetUnFuck(int pet_id)
{
	printf("�������:%08X\n", pet_id);
	_FUNC1 func = (_FUNC1)m_GameCall.UnAttachEudemon;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(pet_id);
	ASM_RESTORE_ECX();
}

// ���＼�� key_no=�������� 1=0 2=1 ...
void Game::Call_PetMagic(int key_no)
{
	printf("���＼��:%d\n", key_no);
	_FUNC6 func = (_FUNC6)m_GameCall.KeyNum;
	DWORD _this = P2DW(m_GameAddr.KeyNumEcxPtr);
	ASM_STORE_ECX();
	__asm { mov ecx, _this }
	func(key_no, 0x00, 0x00, 0x00, 0x00, 0x00);
	ASM_RESTORE_ECX();
}

// ��ȡԶ������������Ϣ
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

// �Ƿ��ж���
bool Game::Call_IsHaveTeam()
{

	_FUNC_R func = (_FUNC_R)m_GameCall.IsHaveTeam;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	DWORD r = func();
	ASM_RESTORE_ECX();
	return r != 0;
}

// �Ƿ��Ƕӳ�
bool Game::Call_IsTeamLeader()
{
	_FUNC_R func = (_FUNC_R)m_GameCall.IsTeamLeader;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	DWORD r = func();
	ASM_RESTORE_ECX();
	return r != 0;
}

// ��������
void Game::Call_TeamCreate()
{
	printf("��������\n");
	_FUNC func = (_FUNC)m_GameCall.TeamCreate;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func();
	ASM_RESTORE_ECX();
}

// �뿪����[�ӳ�]
void Game::Call_TeamDismiss()
{
	printf("�뿪����\n");
	_FUNC func = (_FUNC)m_GameCall.TeamDismiss;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func();
	ASM_RESTORE_ECX();
}

// �뿪����[��Ա]
void Game::Call_TeamLeave()
{
	printf("�뿪����\n");
	_FUNC func = (_FUNC)m_GameCall.TeamLeave;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func();
	ASM_RESTORE_ECX();
}

// �������
void Game::Call_TeamInvite(int player_id)
{
	printf("�������:%08X\n", player_id);
	_FUNC1 func = (_FUNC1)m_GameCall.TeamInvite;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(player_id);
	ASM_RESTORE_ECX();
}

// �Զ����
void Game::Call_TeamAutoJoin(int open)
{
	printf("�Զ����:%d\n", open);
	_FUNC1 func = (_FUNC1)m_GameCall.TeamAutoJoin;
	ASM_STORE_ECX();
	ASM_SET_ECX();
	func(open);
	ASM_RESTORE_ECX();
}

// �Ƿ�ѡ���������
void Game::Call_CheckTeam(int v)
{
	if (!m_GameAddr.TeamChkSta)
		return;

	printf("�����������ѡ��:%d\n", v);
	_FUNC1 func = (_FUNC1)m_GameCall.TeamChk;
	DWORD _this = P2DW(m_GameAddr.TeamChkSta + m_GameAddr.TeamChkOffset);
	ASM_STORE_ECX();
	__asm { mov ecx, _this }
	func(v);
	ASM_RESTORE_ECX();
}

// ��ȡ��ַ[��֪��ʲô���Ի�ȡ]
DWORD Game::Call_GetBaseAddr(int index, DWORD _ecx)
{
	_FUNC1_R func = (_FUNC1_R)m_GameCall.GetNpcBaseAddr;
	ASM_STORE_ECX();
	__asm { mov ecx, _ecx };
	DWORD p = func(index);
	ASM_RESTORE_ECX();
	return PtrToDword(p);
}
