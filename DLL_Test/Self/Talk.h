#pragma once
#include "GameStruct.h"

class Game;

class Talk
{
public:
	// ...
	Talk(Game* p);

	// NPC
	void NPC(DWORD npc_id);
	// NPC对话选择项
	void NPCTalk(DWORD no);
	// NPC对话状态[对话框是否打开]
	bool NPCTalkStatus();
	// 等待对话框打开
	bool WaitTalkBoxOpen();
public:
	// 游戏类
	Game* m_pGame;
};