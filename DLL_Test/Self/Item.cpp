#include "Item.h"
#include "Game.h"
#include <stdio.h>
#include <time.h>
#include <My/Common/mystring.h>

// ...
Item::Item(Game* p)
{
	m_pGame = p;
	InitData();
}

// 初始化数据
void Item::InitData()
{
	m_i64DropTime = 0;
}

// 读取自己拥有物品
DWORD Item::ReadSelfItems(GameSelfItem ** save, DWORD save_length)
{
	// 搜索方法->CE查找物品数量->下访问断点, 获得基址->基址下访问断点得到偏移
	DWORD* pItem = NULL, dwCount = 0;
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

// 获取物品ID
DWORD Item::GetSelfItem(const char* name, DWORD to_x, DWORD to_y)
{
	DWORD dwId = 0;
	GameSelfItem* items[120];
	DWORD dwCount = ReadSelfItems(items, 120);
	for (DWORD i = 0; i < dwCount; i++) {
		if (!items[i])
			continue;

		if (strstr(items[i]->Name, name)) {
			bool result = true;
			if (to_x && to_y) {
				result = items[i]->ToX == to_x && items[i]->ToY == to_y;
			}
			if (result) {
				dwId = items[i]->Id;
				break;
			}
		}

	}
	return dwId;
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
DWORD Item::UseSelfItem(const char * name, DWORD to_x, DWORD to_y)
{
	return UseSelfItem(GetSelfItem(name, to_x, to_y), (to_x!=0 && to_y!=0));
}

// 使用物品
DWORD Item::UseSelfItem(DWORD item_id, bool flag)
{
	if (!item_id)
		return 0;

	if (!flag)
		Game::Call_UseItem(item_id);
	else
		Game::Call_TransmByMemoryStone(item_id);
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

// 售卖物品
DWORD Item::SellSelfItem(ConfItemInfo* items, DWORD length)
{
	DWORD dwSellCount = 0;
	GameSelfItem* self_items[120];
	DWORD dwCount = ReadSelfItems(self_items, 120);       // 背包物品
	printf("背包物品数量:%d\n", dwCount);
	for (DWORD i = 0; i < dwCount; i++) {
		if (!self_items[i])
			continue;

		for (DWORD j = 0; j < length; j++) {
			bool is_sell = strstr(self_items[i]->Name, items[j].Name) != nullptr
				|| items[j].Type == self_items[i]->Type;
			if (is_sell) {    // 是要售卖的物品
				printf("卖出物品:%s %08X\n", items[j].Name, self_items[i]->Id);
				Game::Call_SellItem(self_items[i]->Id); // 出售
				dwSellCount++;
				Sleep(800);
				break;
			}
		}
	}
	return dwSellCount;
}

// 存入物品
DWORD Item::CheckInSelfItem(ConfItemInfo* items, DWORD length)
{
	DWORD dwCheckInCount = 0;
	GameSelfItem* self_items[120];
	DWORD dwCount = ReadSelfItems(self_items, 120);       // 背包物品
	printf("背包物品数量:%d\n", dwCount);
	for (DWORD i = 0; i < dwCount; i++) {
		if (!self_items[i])
			continue;

		for (DWORD j = 0; j < length; j++) {
			bool is_check_in = strstr(self_items[i]->Name, items[j].Name) != nullptr
				|| items[j].Type == self_items[i]->Type;
			if (is_check_in) {    // 是要存入的物品
				printf("存入物品:%s %08X\n", items[j].Name, self_items[i]->Id);
				Game::Call_CheckInItem(self_items[i]->Id); // 存入
				dwCheckInCount++;
				Sleep(800);
				break;
			}
		}
	}
	return dwCheckInCount;
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

	DWORD* pItemsBegin = PtrVToDwordPtr(m_pGame->m_GameAddr.ItemPtr);   // 地面物品列表指针首地址
	DWORD* pItemsEnd = PtrVToDwordPtr(m_pGame->m_GameAddr.ItemPtr + 4); // 地面物品列表指针末地址
	dwCount = pItemsEnd - pItemsBegin;
	//printf("[%d]地面物品数量：%d\n", (int)time(nullptr), dwCount);
	if (dwCount == 0 || dwCount > 128)
		return 0;

	dwCount = 0;
	//printf("\n---------------------------\n");
	for (DWORD* p = pItemsBegin; p < pItemsEnd; p++) {
		GameGroundItem* pItem = (GameGroundItem*)(*p);
		if (!pItem || !pItem->Id || pItem->Id == 0xFFFFFFFF || !pItem->X || !pItem->Y)
			continue;

		save[dwCount] = pItem;
		if (++dwCount == save_length)
			break;

		char* name = pItem->Name + 0x54;
		//printf("[%08X]物品:%s ID:%08X 物品类型:%08X X:%d Y:%d\n", pItem, name,  pItem->Id, pItem->Type,  pItem->X, pItem->Y);

		//Call_PickUpItem(pItem);
	}
	//printf("\n---------------------------\n");
	
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
DWORD Item::PickUpItem(ConfItemInfo* items, DWORD length)
{
	DWORD dwPickUpCount = 0; // 捡了多少物品
	GameGroundItem* pItems[32];
	DWORD dwCount = ReadGroundItems(pItems, sizeof(pItems) / sizeof(GameGroundItem*));

	printf("周围物品数量:%d\n", dwCount);
	for (DWORD i = 0; i < dwCount; i++) {
		for (DWORD j = 0; j < length; j++) {
			bool pick_up = false;
			if (pItems[i]->Name) { // 能读取物品名称
				char* name = pItems[i]->Name + 0x54; // 需要加上此偏移
				pick_up = strstr(name, items[j].Name) != nullptr;
			}
			else {
				pick_up = items[j].Type == pItems[i]->Type;
			}
				
			if (pick_up) {     // 是要捡起来的物品
				printf("捡物:%s %08X\n", items[j].Name, pItems[i]->Id);
				Game::Call_PickUpItem(pItems[i]);  // 捡起来
				DWORD x = 0, y = 0;
				while (!IsPickUp(pItems[i]->Id)) { // 等待物品捡起来
					if (++x == 10) {
						Game::Call_PickUpItem(pItems[i]);  // 捡起来
						x = 0;
					}
					if (++y == 50) {
						break;
					}
					Sleep(500);
				}
				dwPickUpCount++;
				break;
			}
		}
	}

	return dwPickUpCount;
}

// 是否已捡起来
bool Item::IsPickUp(DWORD item_id)
{
	GameGroundItem* pItems[32];
	DWORD dwCount = ReadGroundItems(pItems, sizeof(pItems) / sizeof(GameGroundItem*));
	for (DWORD i = 0; i < dwCount; i++) {
		if (item_id == pItems[i]->Id)
			return false;
	}
	return true;
}
