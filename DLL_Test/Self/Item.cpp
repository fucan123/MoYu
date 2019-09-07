#include "GameClient.h"
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
	ZeroMemory(m_Yao, sizeof(m_Yao));
	ZeroMemory(m_Bao, sizeof(m_Bao));
}

// ��ȡҩ����
DWORD Item::GetYaoCount(int* p_index)
{
	int count = 0;
	for (int i = 0; i < YAO_BAO_L; i++) {
		if (m_Yao[i] != 0) {
			if (p_index)
				*p_index = i;

			count++;
		}	
	}
	return count;
}

// ��ȡ������
DWORD Item::GetBaoCount(int* p_index)
{
	int count = 0;
	for (int i = 0; i < YAO_BAO_L; i++) {
		if (m_Bao[i] != 0) {
			if (p_index)
				*p_index = i;

			count++;
		}
	}
	return count;
}

// ʹ��ҩ
DWORD Item::UseYao()
{
	DWORD id = 0;
	int index = 0;
	DWORD count = GetYaoCount(&index);
	printf("ҩ����:%d[%d]\n", count, index);
	if (count == 0) {
		ReadYaoBao(0);
		count = GetYaoCount(&index);
		printf("ҩ����2:%d[%d]\n", count, index);
		if (count == 0) {
			printf("ʹ��ҩ��\n");
			UseBao();
		}
	}
	if (count > 0) {
		id = m_Yao[index];
		m_pGame->Call_UseItem(id);
		m_Yao[index] = 0;
	}
	
	return id;
}

// ʹ��ҩ��
DWORD Item::UseBao()
{
	DWORD id = 0;
	int index = 0;
	DWORD count = GetBaoCount(&index);
	printf("ҩ������:%d[%d]\n", count, index);
	if (count == 0) {
		ReadYaoBao(1);
		count = GetBaoCount(&index);
		printf("ҩ������2:%d[%d]\n", count, index);
	}
	if (count > 0) {
		id = m_Bao[index];
		m_pGame->Call_UseItem(id);
		m_Bao[index] = 0;
	}

	return id;
}

// ��ȡҩ��
int Item::ReadYaoBao(int flag)
{
	DWORD* arr = flag == 0 ? m_Yao : m_Bao;
	ITEM_TYPE type = flag == 0 ? ��Ч����ҩˮ : ��Ч���ư�;
	ZeroMemory(arr, sizeof(DWORD) * YAO_BAO_L);

	GameSelfItem* items[120];
	DWORD dwCount = ReadSelfItems(items, 120);
	DWORD index = 0;
	for (DWORD i = 0; i < dwCount; i++) {
		SelfItem info;
		if (!GetSelfItemInfo(items[i], info)) {
			printf("�޷���ȡ������Ʒ��Ϣ(%d) %08X\n", GetLastError(), items[i]);
			continue;
		}
		//printf("����:%08X=%08X %s\n", info.Type, type, info.Name);
		if (items[i] && info.Type == type) {
			if (index >= YAO_BAO_L)
				break;

			arr[index++] = info.Id;
		}
	}

	printf("��ȡ:%s(%d)\n", flag == 0 ? "��Ч����ҩˮ" : "��Ч���ư�", index);
	return index;
}

// ��ȡ�Լ�ӵ����Ʒ
DWORD Item::ReadSelfItems(GameSelfItem ** save, DWORD save_length)
{
	// ��������->CE������Ʒ����->�·��ʶϵ�, ��û�ַ->��ַ�·��ʶϵ�õ�ƫ��
	DWORD* pItem = NULL, dwCount = 0;
#if 0
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
#else
#if 0
	DWORD p = PtrToDword(BASE_DS_OFFSET);
	p = PtrToDword(p);
	p = PtrToDword(p + 0x2234);
	pItem = (DWORD*)PtrToDword(p + 0x10);
	dwCount = PtrToDword(p + 0x30);
#endif
	DWORD p;
	if (m_pGame->ReadDwordMemory(g_pObjHero, p)) {
		if (m_pGame->ReadDwordMemory(p + 0x224C, p)) {
			m_pGame->ReadDwordMemory(p + 0x10, (DWORD&)pItem);
			m_pGame->ReadDwordMemory(p + 0x30, (DWORD&)dwCount);
		}
	}
#endif

	if (!pItem)
		return 0;

	//printf("pItem:%08X dwCount:%d\n", pItem, dwCount);
	for (DWORD i = 0; i < dwCount; i++, pItem++) {
		if (i >= save_length)
			break;

		if (!m_pGame->ReadDwordMemory((DWORD)pItem, (DWORD&)save[i])) {
			save[i] = nullptr;
		}
	}

	return dwCount;
}

// ��ȡ�Լ���Ʒ��Ϣ
bool Item::GetSelfItemInfo(PVOID addr, SelfItem& item)
{
	ZeroMemory(item.Name, sizeof(item.Name));
	DWORD ptr = (DWORD)addr;
	if (!m_pGame->ReadDwordMemory(ptr + 4, item.Id))
		return false;
	if (!m_pGame->ReadDwordMemory(ptr + 12, item.Type))
		return false;
	if (!m_pGame->ReadMemory((PVOID)(ptr + 16), item.Name, sizeof(item.Name)))
		return false;
	if (!m_pGame->ReadDwordMemory(ptr + 784, item.ToX))
		return false;
	if (!m_pGame->ReadDwordMemory(ptr + 788, item.ToY))
		return false;

	return true;
}

// ��ȡ��ƷID
DWORD Item::GetSelfItem(const char* name, DWORD to_x, DWORD to_y, DWORD allow_to)
{
	DWORD dwId = 0;
	GameSelfItem* items[120];
	DWORD dwCount = ReadSelfItems(items, 120);
	for (DWORD i = 0; i < dwCount; i++) {
		if (!items[i])
			continue;

		SelfItem info;
		if (!GetSelfItemInfo(items[i], info)) {
			printf("�޷���ȡ������Ʒ��Ϣ(%d) %08X\n", GetLastError(), items[i]);
			continue;
		}

		if (strstr(info.Name, name)) {
			bool result = true;
			if (to_x && to_y) {
				if (allow_to) { // ������Χ
					int cx = (int)info.ToX - (int)to_x;
					int cy = (int)info.ToY - (int)to_y;
					result = abs(cx) <= allow_to && abs(cy) <= allow_to;
				}
				else {
					result = info.ToX == to_x && info.ToY == to_y;
				}
			}
			if (result) {
				dwId = info.Id;
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
		SelfItem info;
		if (!GetSelfItemInfo(items[i], info)) {
			printf("�޷���ȡ������Ʒ��Ϣ(%d) %08X\n", GetLastError(), items[i]);
			continue;
		}
		if (items[i] && info.Type == type) {
			dwId = info.Id;
			break;
		}
	}
	return dwId;
}

// ������Ʒ���ƻ�ȡ��Ʒ����
DWORD Item::GetSelfItemCountByName(const char * name)
{
	DWORD dwNum = 0;
	GameSelfItem* items[120];
	DWORD dwCount = ReadSelfItems(items, 120);
	for (DWORD i = 0; i < dwCount; i++) {
		SelfItem info;
		if (!GetSelfItemInfo(items[i], info)) {
			printf("�޷���ȡ������Ʒ��Ϣ(%d) %08X\n", GetLastError(), items[i]);
			continue;
		}
		if (strcmp(info.Name, name) == 0)
			dwNum++;
	}
	return dwNum;
}

// ������Ʒ���ͻ�ȡ��Ʒ����
DWORD Item::GetSelfItemCountByType(ITEM_TYPE type)
{
	DWORD dwNum = 0;
	GameSelfItem* items[120];
	DWORD dwCount = ReadSelfItems(items, 120);
	for (DWORD i = 0; i < dwCount; i++) {
		SelfItem info;
		if (!GetSelfItemInfo(items[i], info)) {
			printf("�޷���ȡ������Ʒ��Ϣ(%d) %08X\n", GetLastError(), items[i]);
			continue;
		}
		if (items[i] && info.Type == type)
			dwNum++;
	}
	return dwNum;
}

// ʹ����Ʒ
DWORD Item::UseSelfItem(const char * name, DWORD to_x, DWORD to_y, DWORD allow_to)
{
	return UseSelfItem(GetSelfItem(name, to_x, to_y, allow_to), (to_x!=0 && to_y!=0));
}

// ʹ����Ʒ
DWORD Item::UseSelfItem(DWORD item_id, bool flag)
{
	if (!item_id)
		return 0;

	if (!flag)
		m_pGame->Call_UseItem(item_id);
	else
		m_pGame->Call_TransmByMemoryStone(item_id);
	return item_id;
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

		SelfItem info;
		if (!GetSelfItemInfo(self_items[i], info)) {
			printf("�޷���ȡ������Ʒ��Ϣ(%d) %08X\n", GetLastError(), items[i]);
			continue;
		}
		for (DWORD j = 0; j < length; j++) {
			bool is_sell = strstr(info.Name, items[j].Name) != nullptr
				|| items[j].Type == info.Type;
			if (is_sell) {    // ��Ҫ��������Ʒ
				printf("������Ʒ:%s\n", items[j].Name);
				m_pGame->Call_SellItem(info.Id); // ����
				dwSellCount++;
				Sleep(600);
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

		SelfItem info;
		if (!GetSelfItemInfo(self_items[i], info)) {
			printf("�޷���ȡ������Ʒ��Ϣ(%d) %08X\n", GetLastError(), items[i]);
			continue;
		}
		for (DWORD j = 0; j < length; j++) {
			bool is_check_in = strstr(info.Name, items[j].Name) != nullptr
				|| items[j].Type == info.Type;
			if (is_check_in) {    // ��Ҫ�������Ʒ
				printf("������Ʒ:%s\n", items[j].Name);
				m_pGame->Call_CheckInItem(info.Id); // ����
				dwCheckInCount++;
				Sleep(600);
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
	m_pGame->Call_DropItem(item_id);
	return 0;
}

// ����ӵ����Ʒ
DWORD Item::DropSelfItemByName(const char* name, DWORD live_count)
{
	DWORD dwDropNum = 0; // ����������
	DWORD dwNum = 0;     // ӵ�е�����
	GameSelfItem* items[120];
	DWORD dwCount = ReadSelfItems(items, 120);
	for (DWORD i = 0; i < dwCount; i++) {
		SelfItem info;
		if (!GetSelfItemInfo(items[i], info)) {
			printf("�޷���ȡ������Ʒ��Ϣ(%d) %08X\n", GetLastError(), items[i]);
			continue;
		}

		if (items[i] && strstr(info.Name, name)) {
			if (++dwNum > live_count) { // ӵ�е���������Ҫ����������
				dwDropNum++;
				DropSelfItem(info.Id);
				Sleep(200);
			}
		}

	}
	return dwDropNum;
}

// ����ӵ����Ʒ
DWORD Item::DropSelfItemByType(ITEM_TYPE type, DWORD live_count)
{
	DWORD dwDropNum = 0; // ����������
	DWORD dwNum = 0;     // ӵ�е�����
	GameSelfItem* items[120];
	DWORD dwCount = ReadSelfItems(items, 120);
	for (DWORD i = 0; i < dwCount; i++) {
		SelfItem info;
		if (!GetSelfItemInfo(items[i], info)) {
			printf("�޷���ȡ������Ʒ��Ϣ(%d) %08X\n", GetLastError(), items[i]);
			continue;
		}

		if (items[i] && info.Type == type) {
			if (++dwNum > live_count) { // ӵ�е���������Ҫ����������
				dwDropNum++;
				DropSelfItem(info.Id);
				Sleep(200);
			}
		}
			
	}
	return dwDropNum;
}

// ��ȡ�ֿ���Ʒ����
DWORD Item::GetSaveItemCount(const char* name)
{
	return ReadSaveItems(nullptr, 0, name);
}

// ȡ����Ʒ
DWORD Item::CheckOutItem(ConfItemInfo* items, DWORD length)
{
	DWORD dwCheckOutCount = 0;
	GameSelfItem* self_items[120];
	DWORD dwCount = ReadSaveItems(self_items, 120);       // ������Ʒ
	printf("�ֿ���Ʒ����:%d\n", dwCount);
	for (DWORD i = 0; i < dwCount; i++) {
		if (!self_items[i])
			continue;

		for (DWORD j = 0; j < length; j++) {
			bool is_check_out = strstr(self_items[i]->Name, items[j].Name) != nullptr
				|| items[j].Type == self_items[i]->Type;
			if (is_check_out) {    // ��Ҫ�������Ʒ
				printf("ȡ����Ʒ:%s %08X\n", items[j].Name, self_items[i]->Id);
				m_pGame->Call_CheckOutItem(self_items[i]->Id); // ȡ��
				dwCheckOutCount++;
				Sleep(600);
				break;
			}
		}
	}
	return dwCheckOutCount;
}

// ȡ����Ʒ
DWORD Item::CheckOutItem(const char* name, DWORD out_count)
{
	GameSelfItem* items[120];
	DWORD dwCount = ReadSaveItems(items, sizeof(items), name);
	if (out_count && out_count > dwCount) { // ��������
		return 0;
	}
	if (out_count == 0) {
		out_count = dwCount;
	}
	for (DWORD i = 0; i < out_count; i++) {
		printf("%d.ȡ����Ʒ:%s %08X\n", i+1, items[i]->Name, items[i]->Id);
		m_pGame->Call_CheckOutItem(items[i]->Id);
		Sleep(800);
	}
	return dwCount;
}

// ��ȡ�ֿ���Ʒ����
DWORD Item::GetSaveItemAllCount()
{
	// ������ʽ: �ҵ��ֿ�����ָ�룬�¶εõ���ַ��һ·׷��ecx
#if 0
	ASM_STORE_ECX();
	__asm
	{
		mov ecx, g_pObjHero
		mov ecx, dword ptr ds : [ecx]
		// ������CHero::DelPackageItem������(Ӧ���ǵ�һ��)
		mov ecx, dword ptr ds : [ecx + 0x252C]
		// ������CPackage::DelItem������(Ӧ���ǵ�һ��)
		add ecx, 4
		// �������¶�ֱ�ӿɵõ���ƫ��
		mov eax, [ecx + 0x2C]
	}
	ASM_RESTORE_ECX();
#else
	DWORD p = PtrToDword(g_pObjHero);
	p = PtrToDword(p + 0x2544);
	return PtrToDword(p + 4 + 0x2C);
#endif
}

// ��ȡ�ֿ���Ʒ
DWORD Item::ReadSaveItems(GameSelfItem** save, DWORD save_length, const char* name)
{
	if (!m_pGame->m_GameCall.GetPkageItemByIndex)
		return 0;

	DWORD dwIndex = 0;
	// ������ʽ: ȡ��Ʒ�����¶�, һ·׷�ݿ��ҵ� call dword ptr ds:[CHero::GetPackageItemByIndex]
	DWORD dwCount = GetSaveItemAllCount();
	//DWORD* f = (DWORD*)this->ReadSaveItems;
	printf("�ֿ���Ʒ����:%d %08X\n", dwCount, &Item::ReadSaveItems);
	if (dwCount > 180)
		return 0;

	for (DWORD i = 0; i < dwCount; i++) {
		if (save_length > 0 && i >= save_length)
			break;

		GameSelfItem* p = (GameSelfItem*)m_pGame->Call_GetPackageItemByIndex(i);
		bool is_save = name ? strstr(p->Name, name) != nullptr : true;
		if (save && is_save)
			save[dwIndex] = p;
		if (is_save)
			dwIndex++;

		//printf("�ֿ���Ʒ[%08X]:%s[%08X]\n", p, p->Name, p->Id);
	}
	return dwIndex;
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

// ��ȡ������Ʒ��Ϣ
bool Item::GetGroundItemInfo(PVOID addr, GameGroundItem& item)
{
	return m_pGame->ReadMemory(addr, &item, sizeof(GameGroundItem));
}


// ��ȡ������Ʒ
DWORD Item::ReadGroundItems(GameGroundItem** save, DWORD save_length)
{
	if (!m_pGame->m_GameAddr.ItemPtr) {
		printf("û���ҵ�������Ʒָ���ַ\n");
		return 0;
	}

	DWORD  dwCount = 0;

	DWORD* pItemsBegin = nullptr; // ������Ʒ�б�ָ���׵�ַ
	DWORD* pItemsEnd   = nullptr; // ������Ʒ�б�ָ��ĩ��ַ
	if (!m_pGame->ReadDwordMemory(m_pGame->m_GameAddr.ItemPtr, (DWORD&)pItemsBegin)) {
		printf("�޷���ȡ������Ʒ�׵�ַ(%d) %08X\n", GetLastError(), m_pGame->m_GameAddr.ItemPtr);
		return 0;
	}
	if (!m_pGame->ReadDwordMemory(m_pGame->m_GameAddr.ItemPtr + 4, (DWORD&)pItemsEnd)) {
		printf("�޷���ȡ������Ʒβ��ַ(%d) %08X\n", GetLastError(), m_pGame->m_GameAddr.ItemPtr);
		return 0;
	}
	if (!pItemsBegin || !pItemsEnd) {
		printf("������Ʒ�б��ַΪnullptr\n");
		return 0;
	}

	dwCount = pItemsEnd - pItemsBegin;
	//printf("[%d]������Ʒ������%d\n", (int)time(nullptr), dwCount);
	if (dwCount == 0 || dwCount > 128)
		return 0;

	dwCount = 0;
	//printf("\n---------------------------\n");
	for (DWORD* p = pItemsBegin; p < pItemsEnd; p++) {
		GameGroundItem* pItem = nullptr;
		if (!m_pGame->ReadDwordMemory((DWORD)p, (DWORD&)pItem)) {
			printf("�޷���ȡ������Ʒ��ַ\n");
			continue;
		}
		if (!pItem) {
			printf("������Ʒ��ַΪnullptr\n");
			continue;
		}

		GameGroundItem item;
		if (!GetGroundItemInfo(pItem, item)) {
			printf("�޷���ȡ������Ʒ��Ϣ\n");
			continue;
		}
		if (item.Id == 0x00 || item.Id == 0xFFFFFFFF || item.X == 0x00 || item.Y == 0x00)
			continue;

		save[dwCount] = pItem;
		if (++dwCount == save_length)
			break;

		char* name = item.Name + 0x54;
		//printf("[%08X]��Ʒ:%s ID:%08X ��Ʒ����:%08X X:%d Y:%d\n", pItem, name, item.Id, item.Type, item.X, item.Y);

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
		GameGroundItem item;
		if (!GetGroundItemInfo(pItems[i], item)) {
			printf("�޷���ȡ������Ʒ��Ϣ\n");
			continue;
		}
		if (!item.Id || item.Id == 0xFFFFFFFF)
			continue;

		for (DWORD j = 0; j < length; j++) {
			bool pick_up = false;
			char* name = item.Name + 0x54; // ��Ҫ���ϴ�ƫ��
			if (item.Name) { // �ܶ�ȡ��Ʒ����
				char item_name[32] = { 0 };
				if (m_pGame->ReadMemory(name, item_name, sizeof(item_name))) {
					pick_up = strstr(item_name, items[j].Name) != nullptr;
				}
			}
			else {
				pick_up = items[j].Type == item.Type;
			}
				
			if (pick_up) {     // ��Ҫ����������Ʒ
				printf("����:%s\n", items[j].Name);
				m_pGame->Call_PickUpItem(item.Id, item.X, item.Y);  // ������
				DWORD x = 0, y = 0;
				while (!IsPickUp(item.Id)) { // �ȴ���Ʒ������
					if (++x == 15) {
						//m_pGame->Call_PickUpItem(pItems[i]);  // ������
						//x = 0;
						break;
					}
					if (++y == 50) {
						break;
					}
					Sleep(500);
				}
				dwPickUpCount++;

				m_pGame->m_pClient->SendPickUpItem(items[j].Name);
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
		GameGroundItem item;
		if (!GetGroundItemInfo(pItems[i], item)) {
			printf("�޷���ȡ������Ʒ��Ϣ\n");
			continue;
		}
		if (item_id == item.Id)
			return false;
	}
	return true;
}
