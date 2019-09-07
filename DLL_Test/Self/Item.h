#pragma once
#include "GameStruct.h"
#include <Windows.h>

#define YAO_BAO_L 16

// ��Ϸ�Լ�ӵ����Ʒ��Ϣ
typedef struct self_item
{
	DWORD Id;       // ID
	DWORD Type;     // ����
	CHAR  Name[32]; // ����
	DWORD ToX;      // �ɴ��͵ĵ�(X)
	DWORD ToY;      // �ɴ��͵ĵ�(Y)
} SelfItem;

class Game;
class Item
{
public:
	// ...
	Item(Game* p);

	// ��ʼ������
	void InitData();
	// ��ȡҩ����
	DWORD GetYaoCount(int* p_index=nullptr);
	// ��ȡ������
	DWORD GetBaoCount(int* p_index=nullptr);
	// ʹ��ҩ
	DWORD UseYao();
	// ʹ��ҩ��
	DWORD UseBao();
	// ��ȡҩ��
	int   ReadYaoBao(int flag);
	// ��ȡ�Լ�ӵ����Ʒ
	DWORD ReadSelfItems(GameSelfItem** save, DWORD save_length);
	// ��ȡ�Լ���Ʒ��Ϣ
	bool GetSelfItemInfo(PVOID addr, SelfItem& item);
	// ��ȡ��ƷID
	DWORD GetSelfItem(const char* name, DWORD to_x=0, DWORD to_y=0, DWORD allow_to=0);
	// ������Ʒ���ͻ�ȡ��ƷID
	DWORD GetSelfItemIdByType(ITEM_TYPE type);
	// ������Ʒ���ƻ�ȡ��Ʒ����
	DWORD GetSelfItemCountByName(const char* name);
	// ������Ʒ���ͻ�ȡ��Ʒ����
	DWORD GetSelfItemCountByType(ITEM_TYPE type);
	// ʹ����Ʒ
	DWORD UseSelfItem(const char* name, DWORD to_x=0, DWORD to_y=0, DWORD allow_to=0);
	// ʹ����Ʒ
	DWORD UseSelfItem(DWORD item_id, bool flag=0);
	// ʹ����Ʒ
	DWORD UseSelfItemByType(ITEM_TYPE type, DWORD use_count=1);
	// ������Ʒ
	DWORD SellSelfItem(ConfItemInfo* items, DWORD length);
	// ������Ʒ
	DWORD CheckInSelfItem(ConfItemInfo* items, DWORD length);
	// ������Ʒ
	DWORD DropSelfItem(DWORD item_id);
	// ����ӵ����Ʒ
	DWORD DropSelfItemByName(const char* name, DWORD live_count);
	// ����ӵ����Ʒ
	DWORD DropSelfItemByType(ITEM_TYPE type, DWORD live_count);
	// ��ȡ�ֿ���Ʒ����
	DWORD GetSaveItemCount(const char* name);
	// ȡ����Ʒ
	DWORD CheckOutItem(ConfItemInfo* items, DWORD length);
	// ȡ����Ʒ
	DWORD CheckOutItem(const char* name, DWORD out_count=0);
	// ��ȡ�ֿ���Ʒ����
	DWORD GetSaveItemAllCount();
	// ��ȡ�ֿ���Ʒ
	DWORD ReadSaveItems(GameSelfItem** save, DWORD save_length, const char* name=nullptr);
	// �Ƿ��������Ʒ
	bool IsCanDrop();
	// �ȴ�����������Ʒ
	void WaitCanDrop();
	// ��ȡ������Ʒ��Ϣ
	bool GetGroundItemInfo(PVOID addr, GameGroundItem& item);
	// ��ȡ������Ʒ
	DWORD ReadGroundItems(GameGroundItem** save, DWORD save_length);
	// �����Ƿ��д���Ʒ
	bool GroundHasItem(DWORD item_id);
	// ����Ʒ
	DWORD PickUpItem(ConfItemInfo* items, DWORD length);
	// �Ƿ��Ѽ�����
	bool IsPickUp(DWORD item_id);
public:
	// ��Ϸ��
	Game* m_pGame;

	// ҩID
	DWORD m_Yao[YAO_BAO_L];
	// ҩ��ID
	DWORD m_Bao[YAO_BAO_L];

	// ��ʱ����
	DWORD m_dwTmp;

	// ����Ʒʱ��
	__int64 m_i64DropTime;

	CHAR m_Test[0x1000];
};