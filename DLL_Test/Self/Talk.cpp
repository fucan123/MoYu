#include "Talk.h"
#include "Game.h"
#include <stdio.h>

// ...
Talk::Talk(Game * p)
{
	m_pGame = p;
}

// NPC�Ի�״̬[�Ի����Ƿ��]
bool Talk::NPCTalkStatus()
{
	PtrToDword(ADDR_TALKBOX_REAL + CHD_TALBOX_STATUS) != 0;
}
