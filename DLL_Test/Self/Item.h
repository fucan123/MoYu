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
	DWORD GetSelfItemCountByType(ITEM_TYPE);
	// ��ȡ������Ʒ
	DWORD ReadGroundItems(GameGroundItem** save, DWORD save_length);
	// �����Ƿ��д���Ʒ
	bool GroundHasItem(DWORD item_id);
	// ����Ʒ
	void PickUpItem();
public:
	// ��Ϸ��
	Game* m_pGame;
};