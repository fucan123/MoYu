#pragma once
#include "GameStruct.h"
#include <Windows.h>

class Game;

class Item
{
public:
	// ...
	Item(Game* p);

	// 读取自己拥有物品
	DWORD ReadSelfItems(GameSelfItem** save, DWORD save_length);
	// 根据物品类型获取物品ID
	DWORD GetSelfItemIdByType(ITEM_TYPE type);
	// 根据物品类型获取物品数量
	DWORD GetSelfItemCountByType(ITEM_TYPE);
	// 读取地面物品
	DWORD ReadGroundItems(GameGroundItem** save, DWORD save_length);
	// 地面是否还有此物品
	bool GroundHasItem(DWORD item_id);
	// 捡物品
	void PickUpItem();
public:
	// 游戏类
	Game* m_pGame;
};