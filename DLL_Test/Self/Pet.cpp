#include "Pet.h"
#include "Game.h"
#include "Item.h"
#include "Magic.h"
#include <stdio.h>
#include <time.h>

// ...
Pet::Pet(Game* p)
{
	m_pGame = p;
}

// ��ʼ������
void Pet::InitData()
{
	ZeroMemory(m_PetRecord, sizeof(m_PetRecord));
}

// �������
bool Pet::PetOut(DWORD* nums, DWORD length, bool fuck)
{
	DWORD dwCount = ReadPetList();
	for (DWORD i = 0; i < dwCount; i++) {
		for (DWORD j = 0; j < length; j++) {
			if (i == nums[j]) {
				if (j > 0) {
					Sleep(500);
				}
				m_pGame->Call_PetOut(m_GamePet[i].Id);      // �������
				if (fuck) {
					Sleep(500);
					m_pGame->Call_PetFuck(m_GamePet[i].Id); // �������
				}
				break;
			}
		}
	}
	return dwCount > 0;
}

// �������
bool Pet::PetFuck(DWORD* nums, DWORD length)
{
	DWORD dwCount = ReadPetList();
	for (DWORD i = 0; i < dwCount; i++) {
		for (DWORD j = 0; j < length; j++) {
			if (i == nums[j]) {
				if (j > 0) {
					Sleep(300);
				}
				m_pGame->Call_PetFuck(m_GamePet[i].Id); // �������
				break;
			}
		}
	}
	return dwCount > 0;
}

// �������
bool Pet::PetUnFuck(DWORD* nums, DWORD length)
{
	DWORD dwCount = ReadPetList();
	for (DWORD i = 0; i < dwCount; i++) {
		for (DWORD j = 0; j < length; j++) {
			if (i == nums[j]) {
				if (j > 0) {
					Sleep(100);
				}
				m_pGame->Call_PetUnFuck(m_GamePet[i].Id);      // �������
			}
		}
	}
	return dwCount > 0;
}

// ���＼��
void Pet::Magic(char key)
{
	// ��ֵ��ַEAX=00D475ED EDX=00D4766A ESI=00D47698
	// ���ú�����ַ00D476F5
	// mov eax,dword ptr ds:[0x10CFDE8]
	// lea ebx,dword ptr ds:[eax+0x2D8] mov edx,ebx mov eax,dword ptr ds:[edx+0x8] 
	// lea edx,dword ptr ds:[eax+edi*4]
	// mov edx,dword ptr ds:[0x1099594] ����ǻ���[�ڸ�ֵEDX��] ���ְ�����Ҫ+��+1 lea ecx,dword ptr ds:[edx+eax+0x1]
	DWORD v = 9;
	DWORD no =  v + (key - '1') + 1;
	m_pGame->Call_PetMagic(no);
}

// ��������û��Ѫ������
DWORD Pet::Revive()
{
	DWORD dwCount = ReadPetList(0);
#if 0
	if (dwCount == 1) {
		printf("ʹ�á���������%08X\n", m_GamePet[0].Id);
		m_pGame->m_pMagic->UseMagic("������", 0, 0, m_GamePet[0].Id); // ��������
	}
#endif
	if (dwCount == 0)
		return dwCount;

	if (dwCount >= 1) { // ��ҩʹ��ҩ
		printf("��Ҫ�����������:%d\n", dwCount);
		if (m_pGame->m_pItem->GetSelfItemCountByType(��Чʥ����ҩ)) { // ӵ�д�ҩ
			m_pGame->m_pItem->UseSelfItemByType(��Чʥ����ҩ);
		}
		else { // ���ܸ���ȫ��
			printf("ʹ�á���ʥ���ա�%08X\n", m_GamePet[0].Id);
			m_pGame->m_pMagic->UseMagic("��ʥ����", 0, 0, m_GamePet[0].Id); // ȫ������
		}
	}

	// ����
	DWORD no[] = { 0, 1, 2 };
	dwCount = PetFuck(no, sizeof(no)/sizeof(DWORD));

	return dwCount;
}

// �Ƿ���Ҫ����
bool Pet::IsNeedRevive(int index, DWORD life, DWORD life_max)
{
	if (index >= 3)
		return false;

	int now_time = time(nullptr);
	PetRecord* p = &m_PetRecord[index];

	bool result = false;
	if (life == 0) {
		result = true;
		goto end;
	}

	if (p->Time == 0) // ��һ��
		goto end;
	if (!life_max && p->Life > 2000) // �Ҳ����������ֵ
		goto end;
	if (life_max == p->Life) // ��Ѫ
		goto end;
	if (life == p->Life) { // ���ϴ�����һ��
		if ((now_time - p->Time) >= 12) { // ����12�뻹һ��
			result = true;
			goto end;
		}
		else { // ��������Ϣ
			goto ret;
		}
	}
end:
	p->Life = life;
	p->Time = now_time;
ret:
	return result;
}

// ��ȡӵ�г���
DWORD Pet::ReadPetList(DWORD life)
{
	if (!m_pGame->m_GameAddr.PetPtr) {
		printf("�Ҳ��������ַ\n");
		return 0;
	}

	m_dwPetCount = 0;
	DWORD addr = m_pGame->m_GameAddr.PetPtr;
	for (DWORD i = 0; i < 5; i++) {
		// mov edx,dword ptr ds:[edi+ebx*4+0x398]
		// mov esi, dword ptr ds : [edx+0x16C]
		// edi=addr ebx=���� esi=����ID
		// mov ecx, [edi+ebx*4+0x2C0]
		// ecx=Ѫ��
		DWORD edx = 0;
		if (!m_pGame->ReadDwordMemory(addr + i * 4 + 0x3C4, edx))
			continue;
		DWORD esi = 0;
		if (!m_pGame->ReadDwordMemory(edx + 0x170, esi))
			continue;
		
		DWORD ecx = 0;
		if (!m_pGame->ReadDwordMemory(addr + i * 4 + 0x2E4, ecx))
			continue;
		
		// �ҵ����Ѫ��->�õ���ַ->��ַ�¶ϵ�ɵõ�ƫ��
		DWORD life_max = 0;
		if (m_pGame->ReadDwordMemory(addr + i * 4 + 0x404, edx)) { // �����Ѫ����Ϣ�ǱߵĻ�ַ
			m_pGame->ReadDwordMemory(edx + 0xC8, life_max);
		}

		if (!esi) { // û��ID
			break;
		}
		if (life == 0 && !IsNeedRevive(i, ecx, life_max)) // ��ȡû��Ѫ������
			continue;

		m_GamePet[m_dwPetCount].Id = esi;
		m_GamePet[m_dwPetCount].Life = ecx;
		m_dwPetCount++;

		printf("%d ����:%08X %d\n", m_dwPetCount, esi, ecx);
	}
	return m_dwPetCount;
}
