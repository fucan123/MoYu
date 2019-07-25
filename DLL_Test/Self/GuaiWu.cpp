#include "GuaiWu.h"
#include "Game.h"
#include <stdio.h>
#include <My/Common/mystring.h>

// ...
GuaiWu::GuaiWu(Game * p)
{
	m_pGame = p;
	InitAttack();
}

// �������
bool GuaiWu::Clear(MagicType type, DWORD cx, DWORD cy)
{
	if (type == δ֪����)
		return false;

	m_bIsClear = true;
	m_dwCX = cx;
	m_dwCY = cy;

	while (true) { // ���������
		if (IsIgnoreAttack()) // �������ڹ�������
			m_pGame->ReadGameMemory(0x10); // ��ȡ�����б�

		if (!m_pAttack)
			break;

		try {
			printf("GuaiWu::Clear->UseMagic:%08X %d,%d\n", type, m_pAttack->X, m_pAttack->Y);
			m_pGame->m_pMagic->UseMagic(type, m_pAttack->X, m_pAttack->Y, m_pAttack->Id);
		}
		catch (...) {
			printf("GuaiWu::Clear������Ч\n");
			break;
		}
		Sleep(600);
	}

	m_bIsClear = false;
	return true;
}

// ��ʼ���������Ĺ���
void GuaiWu::InitAttack()
{
	m_pAttack = nullptr;
	m_i64AttackTime = 0;
}

// ���ñ���������
void GuaiWu::SetAttack(GameGuaiWu * p)
{
	m_pAttack = p;
	m_i64AttackTime = getmillisecond();
}

// �Ƿ�������ڹ�������
bool GuaiWu::IsIgnoreAttack()
{
	try {
		if (!m_pAttack)  // û�����ڹ�����
			throw nullptr;
		if (!m_pAttack->Id || !m_pAttack->X || !m_pAttack->Y) { // ��Ϣ��Ч
			throw nullptr;
		}
		if (!GetLife(m_pAttack)) { // ��Ѫ��
			printf("GuaiWu::IsIgnoreAttack����Ѫ��Ϊ0\n");
			throw nullptr;
		}
		return false;
	}
	catch (...) {
		InitAttack();
		return true;
	}
}

// ��ȡ����
bool GuaiWu::ReadGuaiWu()
{
	// 037F3C05
	// 037D81B0
	// 4:* 4:0x00 4:0xFFFFFFFF 4:0x01 4:0x00 4:0x00 4:* 4:0x00
	DWORD codes[] = {
		0x1234AA40, 0x00000000, 0xFFFFFFFF, 0x00000001,
		0x00000000, 0x00000000, 0x1234F320, 0x00000000
	};

	//ƫ��15CC��+0C��Ѫ����ַ

	DWORD address[16];
	DWORD count = m_pGame->SearchCode(codes, sizeof(codes) / sizeof(DWORD), address, 16);

	if (count) {
		m_pGame->ReadCoor();
		//printf("\n---------------------------\n");
		//printf("[%d]����������%d(%d)\n", (int)time(nullptr), count, m_dwGuaiWuCount + count);
		DWORD num = 0, near_index = 0xff, near_dist = 0;
		for (DWORD i = 0; i < count; i++) {
			//if (address[i] != ((DWORD)m_pGuaiWus[i + m_dwGuaiWuCount])) {
				//printf("�����%d!=%d\n", i, i + m_dwGuaiWuCount);
			//}
			try {
				GameGuaiWu* pGuaiWu = (GameGuaiWu*)address[i];
				//printf("�����ַ:%08X\n", pGuaiWu);
				//continue;
				if (pGuaiWu->X > 0 && pGuaiWu->Y > 0 && pGuaiWu->Type) {
					char* name = (char*)((DWORD)address[i] + 0x520);
					DWORD life = GetLife(pGuaiWu);

					printf("%02d[%08X].%s[%08X]: x:%X[%d] y:%X[%d] ����:%X Ѫ��:%d\n", i + 1, pGuaiWu, name, pGuaiWu->Id, pGuaiWu->X, pGuaiWu->X, pGuaiWu->Y, pGuaiWu->Y, pGuaiWu->Type, life);

					if (m_bIsClear) { // ���
						DWORD cx = abs((int)m_pGame->m_dwX - (int)pGuaiWu->X);
						DWORD cy = abs((int)m_pGame->m_dwY - (int)pGuaiWu->Y);
						printf("%d,%d %d,%d\n", cx, cy, m_dwCX, m_dwCY);
						if (cx <= m_dwCX && cy <= m_dwCY) { // �ڹ�����Χ��
							printf("ѡ�񹥻�����:%08X\n", pGuaiWu->Id);
							m_pAttack = pGuaiWu;
							return false;
						}
					}
#if 0
					if (m_pGame->m_dwX && y) {
						int cx = m_dwX - pGuaiWu->X, cy = y - pGuaiWu->Y;
						DWORD cxy = abs(cx) + abs(cy);
						if (near_index == 0xff || cxy < near_dist) {
							near_index = i;
							near_dist = cxy;
						}
					}
#endif
					num++;
				}
			}
			catch (void*) {
			}
		}

	}

#if 0
	m_dwGuaiWuCount += count;
	if (m_dwGuaiWuCount > GUAIWU_MAX) {
		printf("���������ﵽ����:%d\n", m_dwGuaiWuCount);
		m_dwGuaiWuCount = GUAIWU_MAX;
	}
#endif

	return true;
}

// ��ȡ���ﵱǰѪ��
DWORD GuaiWu::GetLife(GameGuaiWu* p)
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
