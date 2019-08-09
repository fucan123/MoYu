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

// ��ʼ������
void Item::InitData()
{
	m_i64DropTime = 0;
}

// ��ȡ�Լ�ӵ����Ʒ
DWORD Item::ReadSelfItems(GameSelfItem ** save, DWORD save_length)
{
	// ��������->CE������Ʒ����->�·��ʶϵ�, ��û�ַ->��ַ�·��ʶϵ�õ�ƫ��
	DWORD* pItem = NULL, dwCount = 0;
	__asm
	{
		mov eax, dword ptr ds : [BASE_DS_OFFSET]
		mov eax, [eax]
		mov eax, [eax + 0x2234]     // [eax+0x10]Ϊ������Ʒ��ַ ��һ������ ����Ϊ[eax+0x30]
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

// ��ȡ��ƷID
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
DWORD Item::UseSelfItem(const char * name, DWORD to_x, DWORD to_y)
{
	return UseSelfItem(GetSelfItem(name, to_x, to_y), (to_x!=0 && to_y!=0));
}

// ʹ����Ʒ
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

// ʹ����Ʒ
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

// ������Ʒ
DWORD Item::SellSelfItem(ConfItemInfo* items, DWORD length)
{
	DWORD dwSellCount = 0;
	GameSelfItem* self_items[120];
	DWORD dwCount = ReadSelfItems(self_items, 120);       // ������Ʒ
	printf("������Ʒ����:%d\n", dwCount);
	for (DWORD i = 0; i < dwCount; i++) {
		if (!self_items[i])
			continue;

		for (DWORD j = 0; j < length; j++) {
			bool is_sell = strstr(self_items[i]->Name, items[j].Name) != nullptr
				|| items[j].Type == self_items[i]->Type;
			if (is_sell) {    // ��Ҫ��������Ʒ
				printf("������Ʒ:%s %08X\n", items[j].Name, self_items[i]->Id);
				Game::Call_SellItem(self_items[i]->Id); // ����
				dwSellCount++;
				Sleep(800);
				break;
			}
		}
	}
	return dwSellCount;
}

// ������Ʒ
DWORD Item::CheckInSelfItem(ConfItemInfo* items, DWORD length)
{
	DWORD dwCheckInCount = 0;
	GameSelfItem* self_items[120];
	DWORD dwCount = ReadSelfItems(self_items, 120);       // ������Ʒ
	printf("������Ʒ����:%d\n", dwCount);
	for (DWORD i = 0; i < dwCount; i++) {
		if (!self_items[i])
			continue;

		for (DWORD j = 0; j < length; j++) {
			bool is_check_in = strstr(self_items[i]->Name, items[j].Name) != nullptr
				|| items[j].Type == self_items[i]->Type;
			if (is_check_in) {    // ��Ҫ�������Ʒ
				printf("������Ʒ:%s %08X\n", items[j].Name, self_items[i]->Id);
				Game::Call_CheckInItem(self_items[i]->Id); // ����
				dwCheckInCount++;
				Sleep(800);
				break;
			}
		}
	}
	return dwCheckInCount;
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
	return ms >= (m_i64DropTime + 800);
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

	DWORD  dwCount = 0;

	DWORD* pItemsBegin = PtrVToDwordPtr(m_pGame->m_GameAddr.ItemPtr);   // ������Ʒ�б�ָ���׵�ַ
	DWORD* pItemsEnd = PtrVToDwordPtr(m_pGame->m_GameAddr.ItemPtr + 4); // ������Ʒ�б�ָ��ĩ��ַ
	dwCount = pItemsEnd - pItemsBegin;
	//printf("[%d]������Ʒ������%d\n", (int)time(nullptr), dwCount);
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
		//printf("[%08X]��Ʒ:%s ID:%08X ��Ʒ����:%08X X:%d Y:%d\n", pItem, name,  pItem->Id, pItem->Type,  pItem->X, pItem->Y);

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
DWORD Item::PickUpItem(ConfItemInfo* items, DWORD length)
{
	DWORD dwPickUpCount = 0; // ���˶�����Ʒ
	GameGroundItem* pItems[32];
	DWORD dwCount = ReadGroundItems(pItems, sizeof(pItems) / sizeof(GameGroundItem*));

	printf("��Χ��Ʒ����:%d\n", dwCount);
	for (DWORD i = 0; i < dwCount; i++) {
		for (DWORD j = 0; j < length; j++) {
			bool pick_up = false;
			if (pItems[i]->Name) { // �ܶ�ȡ��Ʒ����
				char* name = pItems[i]->Name + 0x54; // ��Ҫ���ϴ�ƫ��
				pick_up = strstr(name, items[j].Name) != nullptr;
			}
			else {
				pick_up = items[j].Type == pItems[i]->Type;
			}
				
			if (pick_up) {     // ��Ҫ����������Ʒ
				printf("����:%s %08X\n", items[j].Name, pItems[i]->Id);
				Game::Call_PickUpItem(pItems[i]);  // ������
				DWORD x = 0, y = 0;
				while (!IsPickUp(pItems[i]->Id)) { // �ȴ���Ʒ������
					if (++x == 10) {
						Game::Call_PickUpItem(pItems[i]);  // ������
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

// �Ƿ��Ѽ�����
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
