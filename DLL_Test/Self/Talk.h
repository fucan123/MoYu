#pragma once
#include "GameStruct.h"
#include <Windows.h>

class Game;

class Talk
{
public:
	// ...
	Talk(Game* p);

	// NPC�Ի�״̬[�Ի����Ƿ��]
	bool NPCTalkStatus();
public:
	// ��Ϸ��
	Game* m_pGame;
};