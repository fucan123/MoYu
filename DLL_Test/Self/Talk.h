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
	// NPC�Ի�ѡ����
	void NPCTalk(DWORD no);
	// NPC�Ի�״̬[�Ի����Ƿ��]
	bool NPCTalkStatus();
	// �ȴ��Ի����
	bool WaitTalkBoxOpen();
public:
	// ��Ϸ��
	Game* m_pGame;
};