#pragma once
#include "GameStruct.h"
#include <Windows.h>

class Game;

class Talk
{
public:
	// ...
	Talk(Game* p);

	// NPC对话状态[对话框是否打开]
	bool NPCTalkStatus();
public:
	// 游戏类
	Game* m_pGame;
};