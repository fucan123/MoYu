#include "Talk.h"
#include "Game.h"
#include <stdio.h>
#include <My/Common/mystring.h>

// ...
Talk::Talk(Game * p)
{
	m_pGame = p;
}

// NPC
void Talk::NPC(DWORD npc_id)
{
	Game::Call_NPC(npc_id);
}

// NPC对话选择项
void Talk::NPCTalk(DWORD no)
{
	Game::Call_NPCTalk(no);
}

// NPC对话状态[对话框是否打开]
bool Talk::NPCTalkStatus()
{
	PtrToDword(ADDR_TALKBOX_REAL + CHD_TALBOX_STATUS) != 0;
}

// 等待对话框打开
bool Talk::WaitTalkBoxOpen()
{
	__int64 ms = getmillisecond();
	while (!NPCTalkStatus()) {
		if (getmillisecond() > (ms + 3000)) { // 超过3秒
			return false;
		}
		Sleep(100);
	}
	return true;
}
