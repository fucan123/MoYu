#pragma once
#include "GameStruct.h"
#include <Windows.h>

class Game;

class Item
{
public:
	// ...
	Item(Game* p);

	// ��ȡ�Լ�ӵ����Ʒ
	DWORD ReadSelfItems(GameSelfItem** save, DWORD save_length);
	// ������Ʒ���ͻ�ȡ��ƷID
	DWORD GetSelfItemIdByType(ITEM_TYPE type);
	// ������Ʒ���ͻ�ȡ��Ʒ����
	DWORD GetSelfItemCountByType(ITEM_TYPE type);
	// ʹ����Ʒ
	DWORD UseSelfItem(DWORD item_id);
	// ʹ����Ʒ
	DWORD UseSelfItemByType(ITEM_TYPE type, DWORD use_count=1);
	// ������Ʒ
	DWORD DropSelfItem(DWORD item_id);
	// ����ӵ����Ʒ
	DWORD DropSelfItemByType(ITEM_TYPE type, DWORD live_count);
	// �Ƿ��������Ʒ
	bool IsCanDrop();
	// �ȴ�����������Ʒ
	void WaitCanDrop();
	// ��ȡ������Ʒ
	DWORD ReadGroundItems(GameGroundItem** save, DWORD save_length);
	// �����Ƿ��д���Ʒ
	bool GroundHasItem(DWORD item_id);
	// ����Ʒ
	DWORD PickUpItem(ITEM_TYPE* items, DWORD length);
	// �Ƿ��Ѽ�����
	bool IsPickUp(DWORD item_id);
public:
	// ��Ϸ��
	Game* m_pGame;

	// ����Ʒʱ��
	__int64 m_i64DropTime;
};