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

	// ��ʼ������
	void InitData();
	// ��������˵�
	void ClearSearched();
	// NPC
	void NPC(DWORD npc_id);
	// NPC
	DWORD NPC(const char* name);
	// NPC�Ի�ѡ����
	void NPCTalk(DWORD no, bool close = true);
	// NPC�Ի�״̬[�Ի����Ƿ��]
	bool NPCTalkStatus();
	// �Ƿ�ѡ���������
	bool CheckTeamSta();
	// ��ʾ���Ƿ��
	bool TipBoxStatus();
	// �ر���ʾ��
	void CloseTipBox(int close=0);
	// �ȴ��Ի����
	bool WaitTalkBoxOpen();
	// ��ȡNPC��Ϣ
	bool GetNPCInfo(PVOID addr, Player& info, bool isid=false);
	// ��ȡNPCID
	DWORD GetNPCId(const char* name);
	// ��ȡNPC����
	DWORD ReadNPCCount();
	// ��ȡNPC
	DWORD ReadNPC(const char* name=nullptr, GamePlayer** save=nullptr, DWORD save_count=0, bool no_repeat=true);
	// ��ȡ��ǰ��ɫѪ��
	DWORD GetLife(GamePlayer* p);
	// ��ȡԶ������������Ϣ
	DWORD ReadRemotePlayer(const char* name = nullptr);
	// ��ȡ��ΧNPC
	bool ReadNPC_Old();
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

	DWORD m_Test[0x1000];
};