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
	DWORD GetSelfItemCountByType(ITEM_TYPE type);
	// 使用物品
	DWORD UseSelfItem(DWORD item_id);
	// 使用物品
	DWORD UseSelfItemByType(ITEM_TYPE type, DWORD use_count=1);
	// 丢弃物品
	DWORD DropSelfItem(DWORD item_id);
	// 丢弃拥有物品
	DWORD DropSelfItemByType(ITEM_TYPE type, DWORD live_count);
	// 是否可以扔物品
	bool IsCanDrop();
	// 等待到可以扔物品
	void WaitCanDrop();
	// 读取地面物品
	DWORD ReadGroundItems(GameGroundItem** save, DWORD save_length);
	// 地面是否还有此物品
	bool GroundHasItem(DWORD item_id);
	// 捡物品
	DWORD PickUpItem(ITEM_TYPE* items, DWORD length);
	// 是否已捡起来
	bool IsPickUp(DWORD item_id);
public:
	// 游戏类
	Game* m_pGame;

	// 扔物品时间
	__int64 m_i64DropTime;
};