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

// 宠物出征
bool Pet::PetOut(DWORD* nums, DWORD length, bool fuck)
{
	DWORD dwCount = ReadPetList();
	for (DWORD i = 0; i < dwCount; i++) {
		for (DWORD j = 0; j < length; j++) {
			if (i == nums[j]) {
				if (j > 0) {
					Sleep(100);
				}
				Game::Call_PetOut(m_GamePet[i].Id);      // 宠物出征
				if (fuck) {
					Sleep(200);
					Game::Call_PetFuck(m_GamePet[i].Id); // 宠物合体
				}
				break;
			}
		}
	}
	return dwCount > 0;
}

// 复活所有没有血量宠物
DWORD Pet::Revive()
{
	DWORD dwCount = ReadPetList(0);
	if (dwCount == 1) {
		printf("使用【生命祈祷】%08X\n", m_GamePet[0].Id);
		m_pGame->m_pMagic->UseMagic(生命祈祷, 0, 0, m_GamePet[0].Id); // 单个复活
	}
	if (dwCount > 1) { // 有药使用药
		if (m_pGame->m_pItem->GetSelfItemCountByType(速效圣兽灵药)) { // 拥有此药
			m_pGame->m_pItem->UseSelfItemByType(速效圣兽灵药);
		}
		else { // 技能复活全部
			printf("使用【神圣复苏】%08X\n", m_GamePet[0].Id);
			m_pGame->m_pMagic->UseMagic(神圣复苏, 0, 0, m_GamePet[0].Id); // 全部复活
		}
	}
	if (dwCount > 0) {
		printf("需要复活宠物数量:%d\n", dwCount);
		for (DWORD i = 0; i < dwCount; i++) {
			Sleep(200);
			Game::Call_PetFuck(m_GamePet[i].Id); // 宠物合体[不需要出征了]
		}
	}
	return dwCount;
}

// 读取拥有宠物
DWORD Pet::ReadPetList(DWORD life)
{
	m_dwPetCount = 0;
	try {
		DWORD addr = m_pGame->m_GameAddr.PetPtr;
		for (DWORD i = 0; i < 8; i++) {
			// mov edx,dword ptr ds:[edi+ebx*4+0x398]
			// mov esi, dword ptr ds : [edx+0x16C]
			// edi=addr ebx=索引 esi=宠物ID
			// mov ecx, [edi+ebx*4+0x2C0]
			// ecx=血量
			DWORD edx = PtrToDword(addr + i * 4 + 0x3B0);
			DWORD esi = PtrToDword(edx + 0x16C);
			DWORD ecx = PtrToDword(addr + i * 4 + 0x2D0);
			if (!esi) { // 没有ID
				break;
			}
			if (life == 0 && ecx > 0) // 获取没有血量宠物
				continue;

			m_GamePet[i].Id = esi;
			m_GamePet[i].Life = ecx;
			m_dwPetCount++;
		}
	}
	catch (...) {
		printf("Pet::ReadPetList失败\n");
	}
	return m_dwPetCount;
}
