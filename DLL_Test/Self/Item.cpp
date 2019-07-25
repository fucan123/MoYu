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

// 读取自己拥有物品
DWORD Item::ReadSelfItems(GameSelfItem ** save, DWORD save_length)
{
	// 搜索方法->CE查找物品数量->下访问断点, 获得基址->基址下访问断点得到偏移
	DWORD* pItem = NULL, dwCount = 0;
	try {
		__asm
		{
			mov eax, dword ptr ds : [BASE_DS_OFFSET]
			mov eax, [eax]
			mov eax, [eax + 0x2234]     // [eax+0x10]为背包物品地址 是一个数组 长度为[eax+0x30]
			mov edx, [eax + 0x10]       // [eax+0x10]物品地址指针
			mov dword ptr[pItem], edx
			mov edx, [eax + 0x30]       // 物品数量
			mov dword ptr[dwCount], edx
		}

		//printf("pItem:%08X dwCount:%d\n", pItem, dwCount);
		for (DWORD i = 0; i < dwCount; i++, pItem++) {
			if (i >= save_length)
				break;
			save[i] = (GameSelfItem*)*pItem;
		}
	}
	catch (...) {
		printf("Item::ReadSelfItems失败\n");
	}

	return dwCount;
}

// 根据物品类型获取物品ID
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

// 根据物品类型获取物品数量
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

// 使用物品
DWORD Item::UseSelfItem(DWORD item_id)
{
	if (!item_id)
		return 0;

	Game::Call_UseItem(item_id);
	return 1;
}

// 使用物品
DWORD Item::UseSelfItemByType(ITEM_TYPE type, DWORD use_count)
{
	DWORD dwCount = 0;
	for (DWORD i = 0; i < use_count; i++) {
		if (UseSelfItem(GetSelfItemIdByType(type)))
			dwCount++;
		if (i > 0)
			Sleep(100);
	}
	return dwCount;
}

// 丢弃物品
DWORD Item::DropSelfItem(DWORD item_id)
{
	WaitCanDrop();

	m_i64DropTime = getmillisecond();
	Game::Call_DropItem(item_id);
	return 0;
}

// 丢弃拥有物品
DWORD Item::DropSelfItemByType(ITEM_TYPE type, DWORD live_count)
{
	DWORD dwDropNum = 0; // 丢弃的数量
	DWORD dwNum = 0;     // 拥有的数量
	GameSelfItem* items[120];
	DWORD dwCount = ReadSelfItems(items, 120);
	for (DWORD i = 0; i < dwCount; i++) {
		if (items[i] && items[i]->Type == type) {
			if (++dwNum > live_count) { // 拥有的数量大于要保留的数量
				dwDropNum++;
				DropSelfItem(items[i]->Id);
			}
		}
			
	}
	return dwDropNum;
}

// 是否可以扔物品
bool Item::IsCanDrop()
{
	if (!m_i64DropTime)
		return true;

	__int64 ms = getmillisecond();
	return ms >= (m_i64DropTime + 800);
}

// 等待到可以扔物品
void Item::WaitCanDrop()
{
	while (!IsCanDrop()) {
		Sleep(100);
	}
}


// 读取地面物品
DWORD Item::ReadGroundItems(GameGroundItem** save, DWORD save_length)
{
	if (!m_pGame->m_GameAddr.ItemPtr)
		return 0;

	DWORD  dwCount = 0;
	try {
		DWORD* pItemsBegin = PtrVToDwordPtr(m_pGame->m_GameAddr.ItemPtr);   // 地面物品列表指针首地址
		DWORD* pItemsEnd = PtrVToDwordPtr(m_pGame->m_GameAddr.ItemPtr + 4); // 地面物品列表指针末地址
		dwCount = pItemsEnd - pItemsBegin;
		if (dwCount == 0 || dwCount > 128)
			return 0;

		printf("[%d]地面物品数量：%d\n", (int)time(nullptr), dwCount);
		dwCount = 0;
		//printf("\n---------------------------\n");
		for (DWORD* p = pItemsBegin; p < pItemsEnd; p++) {
			GameGroundItem* pItem = (GameGroundItem*)(*p);
			if (!pItem || pItem->Id == 0xFFFFFFFF || !pItem->X || !pItem->Y)
				continue;

			save[dwCount] = pItem;
			if (++dwCount == save_length)
				break;
			printf("[%d]物品ID:%08X 物品类型:%08X X:%08X(%d) Y:%08X(%d)\n", (int)time(nullptr), pItem->Id, pItem->Type, pItem->X, pItem->X, pItem->Y, pItem->Y);

			//Call_PickUpItem(pItem);
		}
		//printf("\n---------------------------\n");
	}
	catch (...) {
		printf("ReadGroundItems失败\n");
	}
	
	return dwCount;
}

// 地面是否还有此物品
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

// 捡物品
DWORD Item::PickUpItem(ITEM_TYPE* items, DWORD length)
{
	DWORD dwPickUpCount = 0; // 捡了多少物品
	GameGroundItem* pItems[32];
	DWORD dwCount = ReadGroundItems(pItems, sizeof(pItems) / sizeof(GameGroundItem*));
	try {
		printf("周围物品数量:%d\n", dwCount);
		for (DWORD i = 0; i < dwCount; i++) {
			for (DWORD j = 0; j < length; j++) {
				if (items[j] == pItems[i]->Type) {     // 是要捡起来的物品
					Game::Call_PickUpItem(pItems[i]);  // 捡起来
					while (!IsPickUp(pItems[i]->Id)) { // 等待物品捡起来
						Sleep(50);
					}
					dwPickUpCount++;
					break;
				}
			}
		}
	}
	catch (...) {
		printf("Item::PickUpItem失败\n");
	}

	return dwPickUpCount;
}

// 是否已捡起来
bool Item::IsPickUp(DWORD item_id)
{
	try {
		GameGroundItem* pItems[32];
		DWORD dwCount = ReadGroundItems(pItems, sizeof(pItems) / sizeof(GameGroundItem*));
		for (DWORD i = 0; i < dwCount; i++) {
			if (item_id == pItems[i]->Id)
				return false;
		}
	}
	catch (...) { }
	return true;
}
