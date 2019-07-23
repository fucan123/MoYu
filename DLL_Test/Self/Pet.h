#pragma once
#include "GameStruct.h"

// 宠物结构体
typedef struct game_pet {
	DWORD Id;   // ID
	DWORD Life; // 血量
} GamePet;

class Game;

// 宠物类
class Pet
{
public:
	// ...
	Pet(Game* p);

	// 宠物出征
	bool  PetOut(DWORD* nums, DWORD length, bool fuck=true);
	// 复活所有没有血量宠物
	DWORD Revive();
	// 读取拥有宠物
	DWORD ReadPetList(DWORD life=1);
public:
	// 游戏类
	Game* m_pGame;
	
	// 拥有宠物数量
	DWORD   m_dwPetCount;
	// 宠物结构列表
	GamePet m_GamePet[8];
};