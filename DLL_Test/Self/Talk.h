#pragma once
#include "GameStruct.h"

// NPC信息
typedef struct game_npc {
	DWORD First[20]; // 未知数值
	CHAR* Name;      // 名称
	DWORD Mid[35];   // 位置数值
	DWORD Id;        // ID
} GameNPC;

class Game;

class Talk
{
public:
	// ...
	Talk(Game* p);
	// 清除搜索了的
	void ClearSearched();
	// NPC
	void NPC(DWORD npc_id);
	// NPC
	DWORD NPC(const char* name);
	// NPC对话选择项
	void NPCTalk(DWORD no);
	// NPC对话状态[对话框是否打开]
	bool NPCTalkStatus();
	// 提示框是否打开
	bool TipBoxStatus();
	// 关闭提示框
	void CloseTipBox(int close=0);
	// 等待对话框打开
	bool WaitTalkBoxOpen();
	// 获取NPCID
	DWORD GetNPCId(const char* name);
	// 读取周围NPC
	bool ReadNPC();
public:
	// 游戏类
	Game* m_pGame;

	// 是否搜索NPCID
	bool m_bIsSearchNPC;
	// 搜索的NCP名称
	char m_chSearchNPCName[32];
	// 搜索到的NPCId
	DWORD m_dwSearchNPCId;
	// 已经搜索了的NPCId数目
	DWORD m_dwIsSearchCount;
	// 已经搜索了的NPCId列表
	DWORD m_ListIsSearch[32];
};