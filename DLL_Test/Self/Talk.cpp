#include "Talk.h"
#include "Game.h"
#include <stdio.h>
#include <My/Common/mystring.h>

// ...
Talk::Talk(Game * p)
{
	m_pGame = p;
	m_bIsSearchNPC = false;
	ClearSearched();
}

// 清除搜索了的
void Talk::ClearSearched()
{
	m_dwIsSearchCount = 0;
}

// NPC
void Talk::NPC(DWORD npc_id)
{
	Game::Call_NPC(npc_id);
}

// NPC
DWORD Talk::NPC(const char* name)
{
	printf("Talk::NPC:%s\n", name);
	DWORD npc_id = 0;
	for (int i = 1; i <= 3; i++) {
		printf("第%d次寻找NPC\n", i);
		npc_id = GetNPCId(name);
		if (npc_id) {
			NPC(npc_id);
			if (WaitTalkBoxOpen()) {
				break;
			}
			else {
				printf("NPC可能无效.\n");
			}
		}
		Sleep(500);
	}
	return npc_id;
}

// NPC对话选择项
void Talk::NPCTalk(DWORD no)
{
	Game::Call_NPCTalk(no);
}

// NPC对话状态[对话框是否打开]
bool Talk::NPCTalkStatus()
{
	try {
		return PtrToDword(m_pGame->m_GameAddr.TalKBoxSta) != 0;
	}
	catch (...) {
		printf("Talk::NPCTalkStatus失败\n");
		return false;
	}
}

// 是否选择邀请队伍
bool Talk::CheckTeamSta()
{
	// +29C0是TeamChkSta偏移头 +100是选择框状态
	return PtrToDword(m_pGame->m_GameAddr.TalKBoxSta + 0x29C0 + 0x100) != 0;
}

// 提示框是否打开
bool Talk::TipBoxStatus()
{
	try {
		return PtrToDword(m_pGame->m_GameAddr.TipBoxSta + 0xA0) != 0;
	}
	catch (...) {
		printf("Talk::TipBoxStatus失败\n");
		return false;
	}
}

// 关闭提示框
void Talk::CloseTipBox(int close)
{
	Game::Call_CloseTipBox(close);
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

// 获取NPCID
DWORD Talk::GetNPCId(const char* name)
{
	strcpy(m_chSearchNPCName, name);
	m_bIsSearchNPC = true;
	m_dwSearchNPCId = 0;
	m_pGame->ReadGameMemory(0x20);
	m_bIsSearchNPC = false;

	return m_dwSearchNPCId;
}

// 读取怪物
bool Talk::ReadNPC()
{
	//printf("搜索NPC\n");
	// 搜索方法:找到NPCID->CE搜索查找合适地址
	// 4:0x03E5D678 4:* 4:* 4:* 4:* 4:0x00 4:0x00 4:0xFFFFFFDB
	// 4:0x03D8C650 4:* 4:* 4:* 4:* 4:0x00 4:0x00 4:0xFFFFFFDB 4:0x00
	DWORD codes[] = {
		0x1234D678, 0x00000011, 0x00000011, 0x00000011,
		0x00000011, 0x00000000, 0x00000000, 0xFFFFFFDB,
	};

	DWORD address[16];
	DWORD count = m_pGame->SearchCode(codes, sizeof(codes) / sizeof(DWORD), address, 16);
	if (count) {
		//printf("NPC列表:%d\n", count);
		DWORD num = 0, near_index = 0xff, near_dist = 0;
		for (DWORD i = 0; i < count; i++) {
			try {
				DWORD dwRepeatId = 0;
				char* name = (char*)((DWORD)address[i] + 0x50);
				DWORD id = PtrToDword(address[i] + 0xD0);
				GameNPC* pNPC = (GameNPC*)address[i];
				if (1 || id) {
					printf("%02d[%08X].%s[%08X]\n", i + 1, pNPC, name, id);
					num++;
				}

				if (m_bIsSearchNPC) {
					if (id && strcmp(m_chSearchNPCName, name) == 0) {
						bool no_search = true;
						for (DWORD j = 0; j < m_dwIsSearchCount; j++) { // 已经搜索了的 不再使用
							if (m_ListIsSearch[j] == id) {
								printf("NPCID:%08X重复\n", id);
								no_search = false;
								dwRepeatId = id;
								break;
							}
						}
						if (no_search) {
							printf("NPCID:%08X\n", id);
							m_dwSearchNPCId = id;
							m_bIsSearchNPC = false;
							m_ListIsSearch[m_dwIsSearchCount++] = id;
							return false;
						}
					}
				}
				if (dwRepeatId)
					m_dwSearchNPCId = dwRepeatId;
			}
			catch (void*) {
			}
		}

	}

	return true;
}

