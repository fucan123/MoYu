#include "Pet.h"
#include "Game.h"
#include "Item.h"
#include "Magic.h"
#include <stdio.h>

// ...
Pet::Pet(Game* p)
{
	m_pGame = p;
}

// �������
bool Pet::PetOut(DWORD* nums, DWORD length, bool fuck)
{
	DWORD dwCount = ReadPetList();
	for (DWORD i = 0; i < dwCount; i++) {
		for (DWORD j = 0; j < length; j++) {
			if (i == nums[j]) {
				if (j > 0) {
					Sleep(100);
				}
				Game::Call_PetOut(m_GamePet[i].Id);      // �������
				if (fuck) {
					Sleep(200);
					Game::Call_PetFuck(m_GamePet[i].Id); // �������
				}
				break;
			}
		}
	}
	return dwCount > 0;
}

// ��������û��Ѫ������
DWORD Pet::Revive()
{
	DWORD dwCount = ReadPetList(0);
	if (dwCount == 1) {
		printf("ʹ�á���������%08X\n", m_GamePet[0].Id);
		m_pGame->m_pMagic->UseMagic(������, 0, 0, m_GamePet[0].Id); // ��������
	}
	if (dwCount > 1) { // ��ҩʹ��ҩ
		if (m_pGame->m_pItem->GetSelfItemCountByType(��Чʥ����ҩ)) { // ӵ�д�ҩ
			m_pGame->m_pItem->UseSelfItemByType(��Чʥ����ҩ);
		}
		else { // ���ܸ���ȫ��
			printf("ʹ�á���ʥ���ա�%08X\n", m_GamePet[0].Id);
			m_pGame->m_pMagic->UseMagic(��ʥ����, 0, 0, m_GamePet[0].Id); // ȫ������
		}
	}
	if (dwCount > 0) {
		printf("��Ҫ�����������:%d\n", dwCount);
		for (DWORD i = 0; i < dwCount; i++) {
			Sleep(200);
			Game::Call_PetFuck(m_GamePet[i].Id); // �������[����Ҫ������]
		}
	}
	return dwCount;
}

// ��ȡӵ�г���
DWORD Pet::ReadPetList(DWORD life)
{
	m_dwPetCount = 0;
	try {
		DWORD addr = m_pGame->m_GameAddr.PetPtr;
		for (DWORD i = 0; i < 8; i++) {
			// mov edx,dword ptr ds:[edi+ebx*4+0x398]
			// mov esi, dword ptr ds : [edx+0x16C]
			// edi=addr ebx=���� esi=����ID
			// mov ecx, [edi+ebx*4+0x2C0]
			// ecx=Ѫ��
			DWORD edx = PtrToDword(addr + i * 4 + 0x3B0);
			DWORD esi = PtrToDword(edx + 0x16C);
			DWORD ecx = PtrToDword(addr + i * 4 + 0x2D0);
			if (!esi) { // û��ID
				break;
			}
			if (life == 0 && ecx > 0) // ��ȡû��Ѫ������
				continue;

			m_GamePet[i].Id = esi;
			m_GamePet[i].Life = ecx;
			m_dwPetCount++;
		}
	}
	catch (...) {
		printf("Pet::ReadPetListʧ��\n");
	}
	return m_dwPetCount;
}
