#include "Talk.h"
#include "Game.h"
#include <stdio.h>
#include <My/Common/mystring.h>

// ...
Talk::Talk(Game * p)
{
	m_pGame = p;
	InitData();
}

// ��ʼ������
void Talk::InitData()
{
	m_bIsSearchNPC = false;
	ClearSearched();
}

// ��������˵�
void Talk::ClearSearched()
{
	m_dwIsSearchCount = 0;
}

// NPC
void Talk::NPC(DWORD npc_id)
{
	Game::Call_NPC(npc_id);
}

// NPC
DWORD Talk::NPC(const char* name)
{
	printf("Talk::NPC:%s\n", name);
	DWORD npc_id = 0;
	for (int i = 1; i <= 3; i++) {
		printf("��%d��Ѱ��NPC\n", i);
		npc_id = GetNPCId(name);
		if (npc_id) {
			NPC(npc_id);
			break;
#if 0
			if (WaitTalkBoxOpen()) {
				break;
			}
			else {
				printf("NPC������Ч.\n");
			}
#endif
		}
		Sleep(500);
	}
	return npc_id;
}

// NPC�Ի�ѡ����
void Talk::NPCTalk(DWORD no)
{
	Game::Call_NPCTalk(no);
}

// NPC�Ի�״̬[�Ի����Ƿ��]
bool Talk::NPCTalkStatus()
{
	if (!m_pGame->m_GameAddr.TalKBoxSta)
		return false;

	return PtrToDword(m_pGame->m_GameAddr.TalKBoxSta) != 0;
}

// �Ƿ�ѡ���������
bool Talk::CheckTeamSta()
{
	// +29C0��TeamChkStaƫ��ͷ +100��ѡ���״̬
	return PtrToDword(m_pGame->m_GameAddr.TeamChkSta + 0x29C0 + 0x100) != 0;
}

// ��ʾ���Ƿ��
bool Talk::TipBoxStatus()
{
	if (!m_pGame->m_GameAddr.TipBoxSta)
		return false;

	return PtrToDword(m_pGame->m_GameAddr.TipBoxSta + 0xA0) != 0;
}

// �ر���ʾ��
void Talk::CloseTipBox(int close)
{
	Game::Call_CloseTipBox(close);
}

// �ȴ��Ի����
bool Talk::WaitTalkBoxOpen()
{
	__int64 ms = getmillisecond();
	while (!NPCTalkStatus()) {
		if (getmillisecond() > (ms + 2000)) { // ����3��
			return false;
		}
		Sleep(100);
	}
	return true;
}

// ��ȡNPCID
DWORD Talk::GetNPCId(const char* name)
{
	GamePlayer* p = NULL;
	ReadNPC(name, &p, 1, true);
	return p ? p->Id : 0;
}

// ��ȡNPC����
DWORD Talk::ReadNPCCount()
{
	if (!g_objPlayerSet)
		return 0;

	return PtrToDword(g_objPlayerSet + 0x14 + 0x2C);
}

// ��ȡNPC
DWORD Talk::ReadNPC(const char* name, GamePlayer** save, DWORD save_count, bool no_repeat)
{
	if (!m_pGame->m_GameCall.GetNpcBaseAddr)
		return 0;

	DWORD dwSaveCount = 0;
	DWORD dwCount = ReadNPCCount();
	DWORD _ecx = g_objPlayerSet + 0x14;
	DWORD func = m_pGame->m_GameCall.GetNpcBaseAddr;
	printf("��Χ��ɫ����:%d\n", dwCount);
	for (DWORD i = 0; i < dwCount; i++) {
		GamePlayer* pGamePlayer;
		__asm
		{
			push i
			mov ecx, _ecx
			call func
			mov eax, dword ptr ds : [eax]
			mov dword ptr[pGamePlayer], eax
		}
		if (!pGamePlayer->Id)
			continue;

		DWORD life = pGamePlayer->Life;
		printf("��ɫ[%08X]:%s %08X ����:%d,%d Ѫ��:%d\n", pGamePlayer, pGamePlayer->Name, pGamePlayer->Id, pGamePlayer->X, pGamePlayer->Y, life);
		if (!save)
			continue;

		bool is_repeat = false;
		bool is_save = true;
		if (name) {
			is_save = false;
			if (strcmp(pGamePlayer->Name, name) == 0) {
				//printf("NPC[%08X]:%s %08X\n", addr, npc_name, npc_id);
				if (no_repeat) { // �������ظ�
					for (DWORD j = 0; j < m_dwIsSearchCount; j++) { // �Ѿ������˵� ����ʹ��
						if (m_ListIsSearch[j] == pGamePlayer->Id) {
							printf("��ɫ�ظ�:%s %08X\n", pGamePlayer->Name, pGamePlayer->Id);
							is_repeat = true;
							break;
						}
					}
				}
				is_save = true;
			}
		}
		if (is_save) {
			save[dwSaveCount++] = pGamePlayer;
			if (is_repeat) { // �ظ�������
				dwSaveCount--;
			}
			else {
				if (no_repeat) { // �������ظ�
					m_ListIsSearch[m_dwIsSearchCount++] = pGamePlayer->Id;
				}
			}
			if (dwSaveCount == save_count)
				break;
		}
	}
	return dwSaveCount;
}

// ��ȡ��ɫ��ǰѪ��
DWORD Talk::GetLife(GamePlayer* p)
{
	// ������ʽ:CE�ҳ�����Ѫ��, �·��ʶϵ�->OD���ʵ�ַ�¶ϵ�, ���ö�ջ���ص�CPlayer::GetData->���Լ���
	// ֱ�ӵõ�CPlayer::GetData����鿴
	__asm {
		push ebp
		mov  ebp, dword ptr p
		mov  ebp, [ebp + 0x15FC]    // mov ecx,dword ptr ds:[edx+0x15FC]
		mov  esi, [ebp + 0x8]       // mov esi,dword ptr ss:[ebp+0x8]
		sub  esi, 1                 // sub esi,ecx[ecxӦ�ù̶���1]
		dec  esi                    // dec esi
		mov  eax, [ebp + 0xC]       // mov eax,dword ptr ss:[ebp+0xC]
		mov  eax, [eax + esi * 8]   // Ѫ��
		pop  ebp
	}
}

// ��ȡ����
bool Talk::ReadNPC_Old()
{
	//printf("����NPC\n");
	// ��������:�ҵ�NPCID->CE�������Һ��ʵ�ַ
	// 4:0x03E5D678 4:* 4:* 4:* 4:* 4:0x00 4:0x00 4:0xFFFFFFDB
	// eax=25B74A38 ecx=0361BE48
	// CRole::IsPlayer esi==0656B710 edx=260679DC ebx=25F96DE8 25F96D68
	DWORD codes[] = {
		0x1234D678, 0x00000011, 0x00000011, 0x00000011,
		0x00000011, 0x00000000, 0x00000000, 0xFFFFFFDB,
	};

	DWORD address[16];
	DWORD count = m_pGame->SearchCode(codes, sizeof(codes) / sizeof(DWORD), address, 16);
	if (count) {
		//printf("NPC�б�:%d\n", count);
		DWORD num = 0, near_index = 0xff, near_dist = 0;
		for (DWORD i = 0; i < count; i++) {
			try {
				DWORD dwRepeatId = 0;
				char* name = (char*)((DWORD)address[i] + 0x50);
				DWORD id = PtrToDword(address[i] + 0xD0);
				GameNPC* pNPC = (GameNPC*)address[i];
				if (1 || id) {
					printf("%02d[%08X].%s[%08X]\n", i + 1, pNPC, name, id);
					num++;
				}

				if (m_bIsSearchNPC) {
					if (id && strcmp(m_chSearchNPCName, name) == 0) {
						bool no_search = true;
						for (DWORD j = 0; j < m_dwIsSearchCount; j++) { // �Ѿ������˵� ����ʹ��
							if (m_ListIsSearch[j] == id) {
								printf("NPCID:%08X�ظ�\n", id);
								no_search = false;
								dwRepeatId = id;
								break;
							}
						}
						if (no_search) {
							printf("NPCID:%08X\n", id);
							m_dwSearchNPCId = id;
							m_bIsSearchNPC = false;
							m_ListIsSearch[m_dwIsSearchCount++] = id;
							return false;
						}
					}
				}
				if (dwRepeatId)
					m_dwSearchNPCId = dwRepeatId;
			}
			catch (void*) {
			}
		}

	}

	return true;
}

