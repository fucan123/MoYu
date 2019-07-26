#pragma once
#include "GameStruct.h"

// NPC��Ϣ
typedef struct game_npc {
	DWORD First[20]; // δ֪��ֵ
	CHAR* Name;      // ����
	DWORD Mid[35];   // λ����ֵ
	DWORD Id;        // ID
} GameNPC;

class Game;

class Talk
{
public:
	// ...
	Talk(Game* p);
	// ��������˵�
	void ClearSearched();
	// NPC
	void NPC(DWORD npc_id);
	// NPC
	DWORD NPC(const char* name);
	// NPC�Ի�ѡ����
	void NPCTalk(DWORD no);
	// NPC�Ի�״̬[�Ի����Ƿ��]
	bool NPCTalkStatus();
	// ��ʾ���Ƿ��
	bool TipBoxStatus();
	// �ر���ʾ��
	void CloseTipBox(int close=0);
	// �ȴ��Ի����
	bool WaitTalkBoxOpen();
	// ��ȡNPCID
	DWORD GetNPCId(const char* name);
	// ��ȡ��ΧNPC
	bool ReadNPC();
public:
	// ��Ϸ��
	Game* m_pGame;

	// �Ƿ�����NPCID
	bool m_bIsSearchNPC;
	// ������NCP����
	char m_chSearchNPCName[32];
	// ��������NPCId
	DWORD m_dwSearchNPCId;
	// �Ѿ������˵�NPCId��Ŀ
	DWORD m_dwIsSearchCount;
	// �Ѿ������˵�NPCId�б�
	DWORD m_ListIsSearch[32];
};