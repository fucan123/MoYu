#include "Item.h"
#include "Game.h"
#include <stdio.h>
#include <time.h>
#include <My/Common/mystring.h>

// ...
Item::Item(Game * p)
{
	m_pGame = p;
	m_i64DropTime = 0;
}

// ��ȡ�Լ�ӵ����Ʒ
DWORD Item::ReadSelfItems(GameSelfItem ** save, DWORD save_length)
{
	DWORD* pItem = NULL, dwCount = 0;
	__asm
	{
		mov eax, dword ptr ds : [0xF03500]
		mov eax, [eax]
		mov eax, [eax + 0x21DC]     // [eax+0x10]Ϊ������Ʒ��ַ ��һ������ ����Ϊ[eax+0x30]
		mov edx, [eax + 0x10]       // [eax+0x10]��Ʒ��ַָ��
		mov dword ptr[pItem], edx
		mov edx, [eax + 0x30]       // ��Ʒ����
		mov dword ptr[dwCount], edx
	}

	if (!pItem)
		return 0;
	//printf("pItem:%08X dwCount:%d\n", pItem, dwCount);

	for (DWORD i = 0; i < dwCount; i++, pItem++) {
		if (i >= save_length)
			break;
		save[i] = (GameSelfItem*)*pItem;
	}

	return dwCount;
}

// ������Ʒ���ͻ�ȡ��ƷID
DWORD Item::GetSelfItemIdByType(ITEM_TYPE type)
{
	// 03C5F19E
	DWORD dwId = 0;
	GameSelfItem* items[120];
	DWORD dwCount = ReadSelfItems(items, 120);
	for (DWORD i = 0; i < dwCount; i++) {
		if (items[i] && items[i]->Type == type) {
			dwId = items[i]->Id;
			break;
		}

	}
	return dwId;
}

// ������Ʒ���ͻ�ȡ��Ʒ����
DWORD Item::GetSelfItemCountByType(ITEM_TYPE type)
{
	DWORD dwNum = 0;
	GameSelfItem* items[120];
	DWORD dwCount = ReadSelfItems(items, 120);
	for (DWORD i = 0; i < dwCount; i++) {
		if (items[i] && items[i]->Type == type)
			dwNum++;
	}
	return dwNum;
}

// ʹ����Ʒ
DWORD Item::UseSelfItem(DWORD item_id)
{
	if (!item_id)
		return 0;

	Game::Call_UseItem(item_id);
	return 1;
}

// ʹ����Ʒ
DWORD Item::UseSelfItemByType(ITEM_TYPE type)
{
	return UseSelfItem(GetSelfItemIdByType(type));
}

// ������Ʒ
DWORD Item::DropSelfItem(DWORD item_id)
{
	WaitCanDrop();

	m_i64DropTime = getmillisecond();
	Game::Call_DropItem(item_id);
	return 0;
}

// ����ӵ����Ʒ
DWORD Item::DropSelfItemByType(ITEM_TYPE type, DWORD live_count)
{
	DWORD dwDropNum = 0; // ����������
	DWORD dwNum = 0;     // ӵ�е�����
	GameSelfItem* items[120];
	DWORD dwCount = ReadSelfItems(items, 120);
	for (DWORD i = 0; i < dwCount; i++) {
		if (items[i] && items[i]->Type == type) {
			if (++dwNum > live_count) { // ӵ�е���������Ҫ����������
				dwDropNum++;
				DropSelfItem(items[i]->Id);
			}
		}
			
	}
	return dwDropNum;
}

// �Ƿ��������Ʒ
bool Item::IsCanDrop()
{
	if (!m_i64DropTime)
		return true;

	__int64 ms = getmillisecond();
	return ms >= (m_i64DropTime + 500);
}

// �ȴ�����������Ʒ
void Item::WaitCanDrop()
{
	while (!IsCanDrop()) {
		Sleep(100);
	}
}


// ��ȡ������Ʒ
DWORD Item::ReadGroundItems(GameGroundItem** save, DWORD save_length)
{
	if (!m_pGame->m_GameAddr.ItemPtr)
		return 0;

	DWORD* pItemsBegin = PtrVToDwordPtr(m_pGame->m_GameAddr.ItemPtr);   // ������Ʒ�б�ָ���׵�ַ
	DWORD* pItemsEnd = PtrVToDwordPtr(m_pGame->m_GameAddr.ItemPtr + 4); // ������Ʒ�б�ָ��ĩ��ַ
	DWORD  dwCount = pItemsEnd - pItemsBegin;
	if (dwCount == 0 || dwCount > 128)
		return 0;

	dwCount = 0;
	//printf("\n---------------------------\n");
	printf("[%d]������Ʒ������%d\n", (int)time(nullptr), dwCount);
	for (DWORD* p = pItemsBegin; p < pItemsEnd; p++) {
		GameGroundItem* pItem = (GameGroundItem*)(*p);
		if (!pItem || pItem->Id == 0xFFFFFFFF)
			continue;

		save[dwCount] = pItem;
		if (++dwCount == save_length)
			break;
		printf("[%d]��ƷID:%08X ��Ʒ����:%08X X:%08X(%d) Y:%08X(%d)\n", (int)time(nullptr), pItem->Id, pItem->Type, pItem->X, pItem->X, pItem->Y, pItem->Y);

		//Call_PickUpItem(pItem);
	}
	//printf("\n---------------------------\n");

	return dwCount;
}

// �����Ƿ��д���Ʒ
bool Item::GroundHasItem(DWORD item_id)
{
	GameGroundItem* pItems[32];
	DWORD dwCount = ReadGroundItems(pItems, sizeof(pItems) / sizeof(GameGroundItem*));
	for (DWORD i = 0; i < dwCount; i++) {
		if (pItems[i]->Id == item_id)
			return true;
	}
	return false;
}

// ����Ʒ
void Item::PickUpItem()
{
	try {
		GameGroundItem* pItems[32];
		DWORD dwCount = ReadGroundItems(pItems, sizeof(pItems) / sizeof(GameGroundItem*));
		for (DWORD i = 0; i < dwCount; i++) {
			Game::Call_PickUpItem(pItems[i]);
			break;
		}
	}
	catch (void*) {

	}
}
