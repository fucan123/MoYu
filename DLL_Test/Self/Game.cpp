#include <My/Common/mystring.h>
#include <My/Common/func.h>
#include <My/Driver/KbdMou.h>
#include <stdio.h>
#include <time.h>

#include "Game.h"
#include "GameProc.h"
#include "Item.h"
#include "Talk.h"
#include "Move.h"
#include "GuaiWu.h"
#include "Magic.h"

GameModAddr Game::m_GameModAddr; // ��Ϸģ���ַ
GameAddr    Game::m_GameAddr;    // ��ϷһЩ���ݵ�ַ
GameCall    Game::m_GameCall;    // ��ϷCALL
GameWnd     Game::m_GameWnd;     // ��Ϸ������Ϣ
CallStep    Game::m_CallStep;    // ��Ϸ����ִ�е�CALL

Game*       Game::self = NULL;   // Game����

// ...
Game::Game()
{
	self = this;

	ZeroMemory(&m_GameModAddr, sizeof(GameModAddr));
	ZeroMemory(&m_GameAddr, sizeof(GameAddr));
	ZeroMemory(&m_GameCall, sizeof(GameCall));
	ZeroMemory(&m_CallStep, sizeof(CallStep));

	m_pGuaiWus = new GameGuaiWu*[GUAIWU_MAX];
	m_bIsReadEnd = true;
	m_pReadBuffer = new BYTE[1024 * 1024 * 10];

	m_pGameProc = new GameProc(this);
	m_pItem     = new Item(this);      // ��Ʒ��
	m_pTalk     = new Talk(this);      // �Ի���
	m_pMove     = new Move(this);      // �ƶ���
	m_pGuaiWu   = new GuaiWu(this);    // ������
	m_pMagic    = new Magic(this);     // ������
}

// >>>
Game::~Game()
{
	delete m_pGuaiWus;
	delete m_pReadBuffer;

	delete m_pItem;
	delete m_pTalk;
	delete m_pMove;
	delete m_pGuaiWu;
	delete m_pMagic;
}

// ��ʼ��
bool Game::Init()
{
	m_hGameProcess = GetCurrentProcess();
	FindAllModAddr();
	FindAllCall();
	FindCoorAddr();
	if (!ReadGameMemory()) {
		INLOG("�޷��������Ѫ����ַ������");
		return false;
	}

	return true;
}


// �����Ϸ����
HWND Game::FindGameWnd()
{
	float f = ::GetScale();
	m_hWnd = FindWindow(NULL, WND_TITLE);
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
	m_GameCall.Run = m_GameModAddr.Mod3DRole + RUN_3drole;
}

// ��ȡģ���ַ
DWORD Game::FindModAddr(LPCWSTR name)
{
	HANDLE hMod = NULL;
	while (hMod == NULL) {
		hMod = GetModuleHandle(name);
	}
	return (DWORD)hMod;
}

// ��ȡ�����ַ
bool Game::FindCoorAddr()
{
	m_GameAddr.CoorX = m_GameModAddr.Mod3DRole + ADDR_COOR_X_OFFSET;
	m_GameAddr.CoorY = m_GameModAddr.Mod3DRole + ADDR_COOR_Y_OFFSET;
	return true;
}

// ��ȡ�ƶ�״̬��ַ
bool Game::FindMoveStaAddr()
{
	DWORD codes[] = {
		0x000064, 0x000064, 0x000000, 0x000000, 0x0FD308, 0x000000, 0x000000, 0x000001,
		0x0F0084, 0x000000, 0x000000, 0x000000, 0x0000FF, 0x021084, 0x06B6C1, 0x000000,
	};
	DWORD address;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.MovSta = address + 0xB0;
		INLOGVARN(32, "�ƶ�״̬��ַ:%08X", m_GameAddr.MovSta);
		printf("�ƶ�״̬��ַ��%08X\n", m_GameAddr.MovSta);
	}
}

// ��ȡ������ַ
bool Game::FindLifeAddr()
{
	DWORD codes[] = {
		0x11,  0x11, 0x101, 0x11, 0x11, 0x11, 0x11, 0x11,
		0x03,  0x0A, 0x18,  0x29, 0x11, 0x11, 0x11, 0x11,
		0x11,  0x11, 0x11,  0x11, 0x17, 0x11, 0x11, 0x111B,
		0x11,  0x11, 0x11,  0x111F
	};
	DWORD address;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.Life = address + 0x38;
		m_GameAddr.LifeMax = m_GameAddr.Life + 0x3F50;
		INLOGVARN(32, "����Ѫ����ַ:%08X", m_GameAddr.Life);
		printf("�ҵ�������ַ��%08X [%08X]\n", m_GameAddr.Life, m_GameAddr.LifeMax);
	}
}

// ��ȡ��ݼ�������Ʒ������ַ
bool Game::FindQuickKeyAddr()
{
	for (DWORD i = 0; i < m_dwReadSize; i += 4) {
		if ((i + 0x20) > m_dwReadSize)
			break;

		DWORD addr = m_dwReadBase + i;
		DWORD* dw = (DWORD*)(m_pReadBuffer + i);

		bool r = dw[0] == 0xCA000000 && dw[1] == 0xCA000000 && dw[2] == 0xCA000000;
		r = r && dw[3] == 0x01 && dw[4] == 0x00 && dw[5] == 0x00;
		r = r && dw[6] == 0x136 && dw[7] == 0x20;

		if (r) {
			DWORD num_offset = 0x20;
			DWORD type_offset = 0x200;
			DWORD type = *(DWORD*)(m_pReadBuffer + i + num_offset + type_offset);
			addr += num_offset;
			if (type == 0x02020202) {
				m_GameAddr.QuickKeyNum = addr;
				m_GameAddr.QuickKeyType = addr + type_offset;
				INLOGVARN(64, "��һ�ſ�ݼ���ַ ����:%08X ����:%08X", m_GameAddr.QuickKeyNum, m_GameAddr.QuickKeyType);
			}
			else {
				m_GameAddr.QuickKey2Num = addr;
				m_GameAddr.QuickKey2Type = addr + type_offset;
				INLOGVARN(64, "�ڶ��ſ�ݼ���ַ ����:%08X ����:%08X", m_GameAddr.QuickKey2Num, m_GameAddr.QuickKey2Type);
			}
		}
	}

	return true;
}

// ��ȡ��������
bool Game::FindBagAddr()
{
	// Soul.exe+C8C12C
	// Soul.exe + CA97C8
	// 4:0x281 4:0x1E6 4:0xA04 4:0x05 4:0x05 4:0x1E1E
	DWORD p, count;
	__asm
	{
		mov eax, dword ptr ds : [0xF03500]
		mov eax, [eax]
		mov eax, [eax+0x21DC]
		mov edx, [eax+0x10]       // ��Ʒ��ַָ��
		mov dword ptr [p], edx
		mov edx, [eax+0x30]       // ��Ʒ����
		mov dword ptr [count], edx
	}
	printf("��Ʒָ��:%08X ����:%d\n", p, count);

	m_GameAddr.Bag = p;
	return true;
}

// ��õ�����Ʒ��ַ�ı����ַ
bool Game::FindItemPtr()
{
	// 4:0x00 4:0x04 4:0x80000000 4:0x80000000 4:0x7FFFFFFF 4:0x7FFFFFFF 4:* 4:0x01 4:0x19 4:0x00400000
	DWORD codes[] = {
		0x00000000, 0x00000004, 0x80000000, 0x80000000,
		0x7FFFFFFF, 0x7FFFFFFF, 0x00000011, 0x00000001,
		0x00000019, 0x00400000, 0x00000011, 0x00000011,
	};
	DWORD address;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.ItemPtr = address + 0x30;
		INLOGVARN(32, "������Ʒ��ַ�����ַ:%08X", m_GameAddr.ItemPtr);
	}

	return address != 0;
}

// ��ȡNPC�����Ի�ESI�Ĵ�����ֵ
bool Game::FindCallNPCTalkEsi()
{
	DWORD codes[] = {
		0x00F6FE30, 0x00000001, 0x00000000, 0x00000000,
		0x00000000, 0x00000001, 0x00000000, 0x00000022
	};
	DWORD address;
	if (SearchCode(codes, sizeof(codes) / sizeof(DWORD), &address)) {
		m_GameAddr.CallNpcTalkEsi = address;
		INLOGVARN(32, "NPC�����Ի�ESI:%08X", m_GameAddr.CallNpcTalkEsi);
	}

	return address != 0;
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
			else if ((codes[j] & 0xffff0000) == 0x12340000) { // ��2�ֽ����
				if ((dw[j]&0x0000ffff) != (codes[j]&0x0000ffff)) {
					result = false;
					break;
				}
			}
			else if ((codes[j] & 0x0000ffff) == 0x00001234) { // ��2�ֽ����
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
			save[count++] = m_dwReadBase + i;
			//printf("��ַ:%08X   %08X\n", m_dwReadBase + i, m_pGuaiWus[0]);
			if (count == save_length)
				break;
		}
	}

	return count;
}

// ��ȡ����
bool Game::ReadCoor(DWORD* x, DWORD* y)
{
	bool result;
	if (!m_GameAddr.CoorX || !m_GameAddr.CoorY) {
		m_dwX = 0;
		m_dwY = 0;
		result = false;
	}
	else {
		m_dwX = PtrToDword(m_GameAddr.CoorX);
		m_dwY = PtrToDword(m_GameAddr.CoorY);
		result = true;
	}
	if (x) {
		*x = m_dwX;
	}
	if (y) {
		*y = m_dwY;
	}

	return result;
}

// ��ȡ����ֵ
bool Game::ReadLife(int& life, int& life_max)
{
	bool result;
	result = ReadProcessMemory(m_hGameProcess, (PVOID)m_GameAddr.Life, &life, 4, NULL);
	result = result && ReadProcessMemory(m_hGameProcess, (PVOID)m_GameAddr.LifeMax, &life_max, 4, NULL);

	return result;
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

// �����Ƿ����ƶ�
bool Game::IsMove()
{
	if (!m_GameAddr.MovSta)
		return true;

	int v = 4;
	if (!ReadProcessMemory(m_hGameProcess, (PVOID)m_GameAddr.MovSta, &v, 4, NULL))
		return true;

	bool r = v >= 4;
	if (!r) {
		//INLOGVARN(16, "MovSta:%d", v);
	}
	return r;
}

// ��������
void Game::AttackGuaiWu()
{
	printf("����������:%d\n", m_dwGuaiWuCount);

	DWORD m = m_dwGuaiWuCount;// > 3 ? 3 : m_dwGuaiWuCount;
	ReadCoor();
	DWORD num = 0, near_index = 0xff, near_dist = 0;
	for (DWORD i = 0; i < m; i++) {
		try {
			GameGuaiWu* pGuaiWu = m_pGuaiWus[i];
			//printf("�����ַ:%08X\n", pGuaiWu);
			if (pGuaiWu->X > 0 && pGuaiWu->Y > 0 && pGuaiWu->Type && pGuaiWu->Type != 0x6E) {
				char* name = (char*)((DWORD)m_pGuaiWus[i] + 0x520);
				printf("%02d.%s[%08X]: x:%X[%d] y:%X[%d] ����:%X\n", i + 1, name, pGuaiWu->Id, pGuaiWu->X, pGuaiWu->X, pGuaiWu->Y, pGuaiWu->Y, pGuaiWu->Type);

				if (m_dwX && m_dwY) {
					int cx = m_dwX - pGuaiWu->X, cy = m_dwY - pGuaiWu->Y;
					DWORD cxy = abs(cx) + abs(cy);
					if (near_index == 0xff || cxy < near_dist) {
						near_index = i;
						near_dist = cxy;
					}
				}

				num++;
			}
		}
		catch (void*) {

		}
	}
	return;
	if (num > 0) {
		if (near_index != 0xff) {
			try {
				GameGuaiWu* pGuaiWu = (GameGuaiWu*)m_pGuaiWus[near_index];
				int mvx = m_dwX, mvy = m_dwY;
				int cx = m_dwX - pGuaiWu->X, cy = m_dwY - pGuaiWu->Y;
				if (abs(cx) >= 8) {
					mvx = pGuaiWu->X > m_dwX ? pGuaiWu->X - 5 : pGuaiWu->X + 5;
				}
				if (abs(cy) >= 8) {
					mvy = pGuaiWu->Y > m_dwY ? pGuaiWu->Y - 5 : pGuaiWu->Y + 5;
				}

				if (mvx != m_dwX || mvy != m_dwY) {
					Call_Run(mvx, mvy);
				}
				else {
					Call_Magic(�����, pGuaiWu->Id);
					//Call_Magic(����, pGuaiWu->X, pGuaiWu->Y);
				}
			}
			catch (void*) {

			}
		}
		printf("\n---------------------------\n");
	}
}

// ��ȡ��Ϸ�ڴ�
bool Game::ReadGameMemory(bool read_guaiwu)
{
	m_dwGuaiWuCount = 0; // ���ù�������
	m_bIsReadEnd = false;

	MEMORY_BASIC_INFORMATION mbi;
	memset(&mbi, 0, sizeof(MEMORY_BASIC_INFORMATION));
	DWORD_PTR MaxPtr = 0x30000000; // ����ȡ�ڴ��ַ
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
			DPrint(("r:%lld -- %p\n", r, ReadAddress));
			break;
		}
		if (mbi.Type != MEM_PRIVATE && mbi.Protect != PAGE_READWRITE) {
			//printf("%p-%p ===����, ��С:%d\n", ReadAddress, ReadAddress + mbi.RegionSize, mbi.RegionSize);
			goto _c;
		}
		else {
			m_dwReadBase = ReadAddress;
			m_dwReadSize = mbi.RegionSize;

			if (ReadProcessMemory(m_hGameProcess, (LPVOID)ReadAddress, m_pReadBuffer, m_dwReadSize, NULL)) {
				if (read_guaiwu) {
					m_pGuaiWu->ReadGuaiWu();
					goto _c;
				}

				if (!m_GameAddr.MovSta) {
					FindMoveStaAddr();
				}
				if (!m_GameAddr.Life) {
					FindLifeAddr();
				}
				if (!m_GameAddr.QuickKeyType) {
					FindQuickKeyAddr();
				}
				if (!m_GameAddr.Bag) {
					FindBagAddr();
				}
				if (!m_GameAddr.CallNpcTalkEsi) {
					FindCallNPCTalkEsi();
				}
				if (!m_GameAddr.ItemPtr) {
					FindItemPtr();
				}
			}
			count++;
		}
	_c:
		// ��ȡ��ַ����
		ReadAddress += mbi.RegionSize;
		//printf("%016X---�ڴ��С2:%08X\n", ReadAddress, mbi.RegionSize);
		// ɨ0x10000000�ֽ��ڴ� ����100����
	}
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

// �Ƿ�����ִ��CALL
bool Game::IsCalling()
{
	if (m_CallStep.Type != CST_NOTHING) {
		//printf("IsCalling:%08X\n", m_CallStep.Type);
		if (CallIsComplete()) {
			printf("ClearCallStep\n");
			ClearCallStep();
		}
	}

	return m_CallStep.Type != CST_NOTHING;
}

// CALL�Ƿ������
bool Game::CallIsComplete()
{
	bool result = true;
	switch (m_CallStep.Type)
	{
	case CST_RUN:
		if (Game::self->m_pMove->IsMoveEnd()     // �ﵽָ��λ��
			|| !Game::self->m_pMove->IsMove()) { // û���ƶ���
			result = true;
			Game::self->m_pMove->ClearMove();
		}
		break;
	case CST_NPC:
		result = Game::self->m_pTalk->NPCTalkStatus();
		break;
	case CST_NPCTALK:
		result = Game::self->m_pTalk->NPCTalkStatus();
		break;
	case CST_PICKUP:
		result = !Game::self->m_pItem->GroundHasItem(m_CallStep.ItemId);
		break;
	default:
		result = true;
		break;
	}
	if (!result) {
		result = time(nullptr) > (m_CallStep.CallTime + 2);
	}

	return result;
}

// ����CALL STEP����
void Game::SetCallStep(CallStepType type, DWORD v1, DWORD v2)
{
	m_CallStep.v1 = v1;
	m_CallStep.v2 = v2;
	m_CallStep.Type = type;
	m_CallStep.CallTime = time(nullptr);
}

// ���CALL STEP����
void Game::ClearCallStep()
{
	ZeroMemory(&m_CallStep, sizeof(m_CallStep));
}

// �����ƶ�����
void Game::Call_Run(int x, int y)
{
	if (IsCalling())
		return;

	SetCallStep(CST_RUN, x, y);
	Game::self->m_pMove->SetMove(x, y);
	DWORD func = m_GameCall.Run;
	__asm {
		mov ecx, dword ptr ds : [0xF03500]
		mov ecx, dword ptr ds : [ecx] // thisָ��
		pushf
		mov eax, [esp]
		and eax, 0xFFFFFF6E // CAS��־Ϊ0 �����ǵ����ͼ
		mov[esp], eax
		popf
		push 0 // 0
		push y // y
		push x // x
		mov eax, func
		call eax
	}
}

// ����CALL[type 0=����Ƶ�� 1=˽�� 2=����]
void Game::Call_Talk(const char* msg, int type)
{
	DWORD arg = 0, func = NULL;
	if (type == 1) {
		arg = 0x07D1;
		func = PtrToDword(0xF034FC);
	}
	else if (type == 2) {
		arg = 0x07D3;
		func = PtrToDword(0xF034F8);
	}
	if (!func)
		return;
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
		mov edx, dword ptr ds : [0xF03500]
		mov ecx, dword ptr ds : [edx]
		call func
	}
}

// NPC�Ի�
void Game::Call_NPC(int npc_id)
{
	if (IsCalling())
		return;

	SetCallStep(CST_NPC, npc_id);
	INLOGVARN(32, "��NPC:%08X\n", npc_id);
	__asm {
		push 0
		push npc_id
		mov ecx, dword ptr ds : [0xF03500]
		mov ecx, dword ptr ds : [ecx]
		call dword ptr ds : [0xF056F4]
	}
}

// NPC�����Ի�
void Game::Call_NPCTalk(int no)
{
	if (IsCalling())
		return;

	SetCallStep(CST_NPCTALK, no);
	INLOGVARN(32, "�Ի�NPC��%d\n", no);
	// eax=038735B8-2550000=13235B8
	// edi=0343DDD8-2550000=EEDDD8
	// esi=05DD5314

	DWORD _eax = m_GameModAddr.Mod3DRole + NPCTALK_EAX_3drole;
	DWORD _edi = m_GameModAddr.Mod3DRole + NPCTALK_EDI_3drole;
	DWORD _esi = m_GameAddr.CallNpcTalkEsi;
	__asm {
		push 0
		push no
		mov eax, _eax
		mov ecx, eax
		mov edi, _edi
		call dword ptr ds : [edi + 0x138]

		push 0x00
		mov esi, _esi
		mov ecx, esi
		mov eax, 0x00CD1790
		call eax
	}
}

// ʹ����Ʒ
void Game::Call_UseItem(int item_id)
{
	__asm
	{
		push 0x00
		push 0xFFFFFFFF
		push 0xFFFFFFFF
		push 0x00
		push item_id     // ��ƷID
		mov eax, dword ptr ds : [0xF03500]
		mov ecx, dword ptr ds : [eax]
		call dword ptr ds : [0xF04230]
	}
}

// ����Ʒ
void Game::Call_DropItem(int item_id)
{
	if (IsCalling())
		return;

	SetCallStep(CST_DROP, item_id);
	// 4:0x00 4:0x04 4:0x80000000 4:0x80000000 4:0x7FFFFFFF 4:0x7FFFFFFF 4:* 4:0x01 4:0x19 4:0x00400000
	__asm
	{
		push 00      // Ӧ�����ӵ����������
		push 00      // Ӧ�����ӵ����������
		push item_id // ��Ʒid
		mov eax, dword ptr ds : [0xF03500]
		mov ecx, dword ptr ds : [eax]
		call dword ptr ds : [0xF04A80]
	}
}

// ����Ʒ
void Game::Call_PickUpItem(GameGroundItem* p)
{
	if (IsCalling())
		return;

	SetCallStep(CST_PICKUP, p->Id);
	DWORD id = p->Id, x = p->X, y = p->Y;
	__asm
	{
		push y
		push x
		push id
		mov eax, dword ptr ds : [0xF03500]
		mov ecx, dword ptr ds : [eax]
		call dword ptr ds : [0xF058C4]
	}
}

// �ż���
void Game::Call_Magic(int magic_id, int guaiwu_id)
{
	if (IsCalling())
		return;

	SetCallStep(CST_MAGIC, magic_id);
	__asm
	{
		push 0
		push 0
		push guaiwu_id
		push magic_id
		mov eax, dword ptr ds : [0xF03500]
		mov ecx, dword ptr ds : [eax]
		call dword ptr ds : [0xF05768]
	}
}

// �ż���
void Game::Call_Magic(int magic_id, int x, int y)
{
	if (IsCalling())
		return;

	SetCallStep(CST_MAGIC, magic_id);
	__asm
	{
		push 0
		push 0
		push y
		push x
		push magic_id
		mov edx, dword ptr ds : [0xF03500]
		mov ecx, dword ptr ds : [edx]
		call dword ptr ds : [0xF04C5C]
	}
}
