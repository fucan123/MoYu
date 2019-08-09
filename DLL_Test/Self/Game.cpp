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

// ��Ϸg_pObjHeroȫ�ֱ���
DWORD g_pObjHero = NULL;
// ��Ϸg_objPlayerSetȫ�ֱ���
DWORD g_objPlayerSet = NULL;

GameModAddr Game::m_GameModAddr; // ��Ϸģ���ַ
GameAddr    Game::m_GameAddr;    // ��ϷһЩ���ݵ�ַ
GameCall    Game::m_GameCall;    // ��ϷCALL
GameWnd     Game::m_GameWnd;     // ��Ϸ������Ϣ

Game*       Game::self = NULL;   // Game����

// ...
Game::Game()
{
	self = this;

	ZeroMemory(&m_GameModAddr, sizeof(GameModAddr));
	ZeroMemory(&m_GameAddr, sizeof(GameAddr));
	ZeroMemory(&m_GameCall, sizeof(GameCall));

	m_bIsReadEnd = true;
	m_pReadBuffer = new BYTE[1024 * 1024 * 10];

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

	delete m_pItem;
	delete m_pTalk;
	delete m_pMove;
	delete m_pGuaiWu;
	delete m_pMagic;
	delete m_pPet;

	delete m_pGameProc;
}

// ��ʼ��
bool Game::Init()
{
	FindGameWnd();

	m_hGameProcess = GetCurrentProcess();
	FindAllModAddr();
	FindCoorAddr();

	printf("�ȴ���Ϸ���ݳ�ʼ�����...\n");
	while (!PtrToDword(BASE_DS_OFFSET)) {
		Sleep(1000);
	}
	while (!FindBagAddr()) {
		Sleep(1000);
	}

	DWORD pid;
	GetWindowThreadProcessId(m_hWnd, &pid);

	printf("��ʼ��ȡ��Ϸ����...\n");
	FindAllCall();
	
	if (!ReadGameMemory()) {
		INLOG("�޷��������Ѫ����ַ������");
		return false;
	}
	m_pMagic->ReadMagic();
#if 0
	while (!m_GameAddr.Life) {
		printf("���¶�ȡѪ����ַ\n");
		ReadGameMemory();
		Sleep(1000);
	}
#endif
	return true;
}

// �����Ƿ���ħ��
bool Game::IsMoYu()
{
	wchar_t name[32] = { 0 };
	GetProcessName(name, GetCurrentProcessId());
	wprintf(L"%ws = %ws\n", name, L"soul.exe");
	return wcsstr(name, L"soul.exe") != nullptr;
}

// ����
void Game::Run()
{
	m_pGameProc->Run();
}


// �����Ϸ����
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
	printf("���ھ��:%0x %d %d %.2\n", m_hWnd, m_GameWnd.Width, m_GameWnd.Height, f);
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
		{"?Run@CHero",             NULL,  0, &m_GameCall.Run,              "�ƶ�����"},
		{"?ActiveNpc@CHero",       NULL,  0, &m_GameCall.ActiveNpc,        "NPC�Ի�"},
		{"?UseItem@CHero",         NULL,  0, &m_GameCall.UseItem,          "ʹ����Ʒ"},
		{"?DropItem@CHero",        NULL,  0, &m_GameCall.DropItem,         "������Ʒ"},
		{"?PickUpItem@CHero",      NULL,  0, &m_GameCall.PickUpItem,       "��ʰ��Ʒ"},
		{"?SellItem@CHero",        NULL,  0, &m_GameCall.SellItem,         "����Ʒ"},
		{"?CheckInItem@CHero",     NULL,  0, &m_GameCall.CheckInItem,      "����Զ�ֿ̲�"},
		{"?OpenBank@CHero",        NULL,  0, &m_GameCall.OpenBank,         "��Զ�ֿ̲�"},
		{"?TransmByMemoryStone@CHero",    NULL,  0, &m_GameCall.TransmByMemoryStone, "ʹ�ÿɴ�����Ʒ"},
		{"?MagicAttack@CHero",     "POS", 0, &m_GameCall.MagicAttack_GWID, "ʹ�ü���-����ID "},
		{"?MagicAttack@CHero",     "POS", 1, &m_GameCall.MagicAttack_XY,   "ʹ�ü���-XY����"},
		{"?CallEudenmon@CHero",    NULL,  0, &m_GameCall.CallEudenmon,     "�������"},
		{"?KillEudenmon@CHero",    NULL,  0, &m_GameCall.KillEudenmon,     "�����ٻ�"},
		{"?AttachEudemon@CHero",   NULL,  0, &m_GameCall.AttachEudemon,    "�������"},
		{"?UnAttachEudemon@CHero", NULL,  0, &m_GameCall.UnAttachEudemon,  "�������"},
		{"?GetData@CPlayer",       NULL,  0, &m_GameCall.SetRealLife,      "CPlayer::GetData"},
	};
	DWORD length = sizeof(info) / sizeof(SearchModFuncMsg);
	for (DWORD i = 0; i < length; i++) {
		SearchFuncInMode(&info[i], (HANDLE)m_GameModAddr.Mod3DRole);
	}

	m_GameCall.CloseTipBox = FindCloseTipBoxCall();
	m_GameCall.GetNpcBaseAddr = FindGetNpcBaseAddr();
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
	// ����Ѱ�Ҽ����ҵ�
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

// �Ƚϵ�¼�ʺ�
bool Game::CmpLoginAccount(const char* name)
{
	return strcmp(name, (const char*)(m_GameModAddr.Mod3DRole+ADDR_ACCOUNT_NAME)) == 0;
}

// ��ȡ�����ַ
bool Game::FindCoorAddr()
{
	m_GameAddr.CoorX = m_GameModAddr.Mod3DRole + ADDR_COOR_X_OFFSET;
	m_GameAddr.CoorY = m_GameModAddr.Mod3DRole + ADDR_COOR_Y_OFFSET;
	while (PtrToDword(m_GameAddr.CoorX) == 0 || PtrToDword(m_GameAddr.CoorY) == 0) {
		printf("�ȴ�������Ϸ������\n");
		Sleep(5000);
	}
	printf("��������:%d,%d\n", PtrToDword(m_GameAddr.CoorX), PtrToDword(m_GameAddr.CoorY));
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
		INLOGVARN(32, "�ƶ�״̬��ַ:%08X", m_GameAddr.MovSta);
		printf("�ƶ�״̬��ַ��%08X\n", m_GameAddr.MovSta);
	}
	return address > 0;
}

// ��ȡ�Ի���״̬��ַ
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
		printf("�Ի���״̬��ַ��%08X\n", m_GameAddr.TalKBoxSta);
	}
	return address > 0;
}

// ��ȡ�Ƿ�ѡ���������״̬��ַ
bool Game::FindTeamChkStaAddr()
{
	// 29C0
	// 4:0x00F37668 4:0x00000001 4:0x00000000 4:0x00000000 4:0x00000000 4:0x00000001 4:0x00000000
	DWORD codes[] = {
		0x00F37668, 0x00000001, 0x00000000, 0x00000000,
		0x00000000, 0x00000001, 0x00000000, 0x00000022
	};
	DWORD address = 0; // �����ַ��Callѡ���Ƿ�������麯����esi��ֵ mov ecx,dword ptr ds:[esi+0x1DC]
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.TeamChkSta = address; // +29C0��TeamChkStaƫ��ͷ +100��ѡ���״̬
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
		printf("�ҵ�������ַ��%08X\n", m_GameAddr.Life);

		DWORD codes2[] = {
			0x00000018, 0x00000087, 0x00000000, 0x0000000A,
			0x00000011, 0x00000000, 0x0000000A, 0x00000000,
		};
		DWORD address2 = 0;
		if (SearchCode(codes2, sizeof(codes2) / sizeof(DWORD), &address2)) {
			m_GameAddr.LifeMax = address2 + 0x20;
		}
		printf("�ҵ��������޵�ַ��%08X\n", m_GameAddr.LifeMax);
	}
	return address > 0;
}

// ��ȡ��������
bool Game::FindBagAddr()
{
	// Soul.exe+C8C12C
	// Soul.exe + CA97C8
	// 4:0x281 4:0x1E6 4:0xA04 4:0x05 4:0x05 4:0x1E1E
	// 023C78D5 
	// ��������->CE������Ʒ����->�·��ʶϵ�, ��û�ַ->��ַ�·��ʶϵ�õ�ƫ��
	// CHero::GetItem��������
	try {
		DWORD p, count;
		__asm
		{
			mov eax, dword ptr ds : [BASE_DS_OFFSET]
			//mov eax, g_pObjHero
			mov eax, [eax]
			mov eax, [eax + 0x2234]     // 2234
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
	// 0x00F6FE30
	DWORD codes[] = {
		0x00F93E10, 0x00000001, 0x00000000, 0x00000000,
		0x00000000, 0x00000001, 0x00000000, 0x00000022
	};
	DWORD address = 0;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.CallNpcTalkEsi = address;
		INLOGVARN(32, "NPC�����Ի�ESI:%08X", m_GameAddr.CallNpcTalkEsi);
		printf("NPC�����Ի�ESI:%08X\n", m_GameAddr.CallNpcTalkEsi);
	}

	return address != 0;
}

// ��ȡ�����б����ַ
bool Game::FindPetPtrAddr()
{
	// �������� ���ҳ�����Ѫ��->CE��Ѫ�����ʶϵ�, �ҵ���ַ, �鿴������
	// ����ID->��ó���ID��ַ->�·��ʶϵ�->�ҵ�ƫ��
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
	printf("�����б��ַ:%08X\n", address);
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
		printf("�����б��ַ:%08X [%08X] %08X\n", address, PtrToDword(address), codes);
	}
#endif
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

// ��ȡ����
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

// ��ȡ����ֵ
bool Game::ReadLife(int& life, int& life_max)
{
	life = PtrToDword(m_GameAddr.Life);
	life_max = PtrToDword(m_GameAddr.LifeMax);
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
bool Game::FindButtonWnd(int button_id, HWND& hwnd, HWND& parent)
{
	HWND wnds[] = { (HWND)button_id, NULL };
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
		param[0] = hWnd_Child; // �Ӵ��ھ�� 
		param[1] = hWnd;       // �����ھ��
		return FALSE;
	}
	return TRUE;
}

// ����
void Game::Keyborad(int type, int key)
{
	UINT scan_code = MapVirtualKey(key, MAPVK_VK_TO_VSC);
	LPARAM lParam = 0x0001; // 0-15Ϊ��������
	lParam |= (scan_code&0xff) << 16; // 16-23Ϊ����ɨ����
	if (key == WM_KEYUP) { // ��������
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

// ����
void Game::KeyDown(char key)
{
	Keyborad(WM_KEYDOWN, key);
}

// ����
void Game::KeyUp(char key)
{
	Keyborad(WM_KEYUP, key);
}

// �����ƶ�����
void Game::Call_Run(int x, int y)
{
	printf("�ƶ���:%d,%d\n", x, y);
	// 4:0x6AEC8B55 4:0xBE7468FF 4:0xA1640326 4:0x00000000 4:0x25896450 4:0x00000000 4:0x04D8EC81 4:0x56530000
	DWORD func = m_GameCall.Run;
	__asm {
		mov ecx, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [ecx] // thisָ��
		//pushf
		//mov eax, [esp]
		//and eax, 0xFFFFFF6E // CAS��־Ϊ0 �����ǵ����ͼ
		//mov[esp], eax
		//popf
		push 0 // 0
		push y // y
		push x // x
		call func
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
	DWORD func = m_GameCall.ActiveNpc;
	__asm {
		push 0
		push npc_id
		mov ecx, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [ecx]
		call func
	}
}

// NPC�����Ի�
void Game::Call_NPCTalk(int no)
{
	INLOGVARN(32, "�Ի�NPC��%d\n", no);
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

// �ر���ʾ��
void Game::Call_CloseTipBox(int close)
{
	printf("�ر���ʾ��:%d\n", close);
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
	

// ʹ����Ʒ
void Game::Call_UseItem(int item_id)
{
	printf("ʹ����Ʒ:%08X\n", item_id);
	DWORD func = m_GameCall.UseItem;
	__asm
	{
		push 0x00
		push 0xFFFFFFFF
		push 0xFFFFFFFF
		push 0x00
		push item_id     // ��ƷID
		mov eax, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [eax]
		call func
	}
}

// ����Ʒ
void Game::Call_DropItem(int item_id)
{
	printf("������Ʒ:%08X\n", item_id);
	DWORD func = m_GameCall.DropItem;
	// 4:0x00 4:0x04 4:0x80000000 4:0x80000000 4:0x7FFFFFFF 4:0x7FFFFFFF 4:* 4:0x01 4:0x19 4:0x00400000
	__asm
	{
		push 00      // Ӧ�����ӵ����������
		push 00      // Ӧ�����ӵ����������
		push item_id // ��Ʒid
		mov eax, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [eax]
		call func
	}
}

// ����Ʒ
void Game::Call_PickUpItem(GameGroundItem* p)
{
	printf("ʰȡ��Ʒ:%08X %d,%d\n", p->Id, p->X, p->Y);
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

// ������
void Game::Call_SellItem(int item_id)
{
	printf("������:%08X\n", item_id);
	DWORD func = m_GameCall.SellItem;
	__asm
	{
		push item_id
		mov eax, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [eax]
		call func
	}
}

// ����Զ�ֿ̲�
void Game::Call_CheckInItem(int item_id)
{
	//printf("������Ʒ:%08X\n", item_id);
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

// ʹ�ÿɴ�����Ʒ
void Game::Call_TransmByMemoryStone(int item_id)
{
	printf("ʹ�ÿɴ�����Ʒ:%08X\n", item_id);
	DWORD func = m_GameCall.TransmByMemoryStone;
	__asm
	{
		push item_id
		mov eax, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [eax]
		call func
	}
}

// �ż���
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

// �ż���
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

// �������
void Game::Call_PetOut(int pet_id)
{
	// 00870741 - 89 8C B3 C0020000  - mov [ebx+esi*4+000002C0],ecx
	// 724-008
	printf("�������:%08X\n", pet_id);
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

// �����ٻ�
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

// �������
void Game::Call_PetFuck(int pet_id)
{
	printf("�������:%08X\n", pet_id);
	DWORD func = m_GameCall.AttachEudemon;
	__asm
	{
		push pet_id
		mov edx, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [edx]
		call func
	}
}

// �������
void Game::Call_PetUnFuck(int pet_id)
{
	printf("�������:%08X\n", pet_id);
	DWORD func = m_GameCall.UnAttachEudemon;
	__asm
	{
		push pet_id
		mov edx, dword ptr ds : [BASE_DS_OFFSET]
		mov ecx, dword ptr ds : [edx]
		call func
	}
}

// �Ƿ�ѡ���������
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
