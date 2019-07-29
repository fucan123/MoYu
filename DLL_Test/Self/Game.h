#pragma once
#include "GameStruct.h"

// ����ģ���з���������Ҫ����Ϣ
typedef struct search_mod_func_msg
{
	const char*   Name;    // ����������
	const char*   Substr;  // �����а����򲻰������ַ���
	DWORD         Flag;    // 0-������ 1-����
	DWORD*        Save;    // �����ַ
	const char*   Remark;  // ˵��
} SearchModFuncMsg;

class GameProc;
class Item;
class Talk;
class Move;
class GuaiWu;
class Magic;
class Pet;

class Game
{
public:
	// ...
	Game();
	// >>>
	~Game();
	// ��ʼ��
	bool Init();
	// �����Ƿ���ħ��
	bool IsMoYu();
	// ����
	void Run();
	// ֹͣ
	void Stop(bool v = true);
	// �����Ϸ����
	HWND FindGameWnd();
	// ��ȡ����ģ���ַ
	void FindAllModAddr();
	// ��ȡ��Ϸ����CALL
	void FindAllCall();
	// ��ȡ�ر���ʾ����
	DWORD FindCloseTipBoxCall();
	// ��ȡģ���ַ
	DWORD FindModAddr(LPCWSTR name);
	// �Ƚϵ�¼�ʺ�
	bool CmpLoginAccount(const char* name);
	// ��ȡ����
	bool ReadCoor(DWORD* x=NULL, DWORD* y=NULL);
	// ��ȡ����ֵ
	bool ReadLife(int& life, int& life_max);
	// ��ȡҩ������
	bool ReadQuickKey2Num(int* nums, int length);
	// ��ȡ������Ʒ
	bool ReadBag(DWORD* bag, int length);
	// ��������
	void AttackGuaiWu();
	// ��ȡ�����ַ
	bool FindCoorAddr();
	// ��ȡ�ƶ�״̬��ַ
	bool FindMoveStaAddr();
	// ��ȡ�Ի���״̬��ַ
	bool FindTalkBoxStaAddr();
	// ��ȡ�Ƿ�ѡ���������״̬��ַ
	bool FindTeamChkStaAddr();
	// ��ȡ��ʾ��״̬��ַ
	bool FindTipBoxStaAddr();
	// ��ȡ������ַ
	bool FindLifeAddr();
	// ��ȡ��ݼ�������Ʒ������ַ
	bool FindQuickKeyAddr();
	// ��ȡ��������
	bool FindBagAddr();
	// ��õ�����Ʒ��ַ�ı����ַ
	bool FindItemPtr();
	// ��ȡNPC�����Ի�ESI�Ĵ�����ֵ
	bool FindCallNPCTalkEsi();
	// ��ȡ�����б����ַ
	bool FindPetPtrAddr();
	// ��ĳ��ģ������������
	DWORD SearchFuncInMode(SearchModFuncMsg* info, HANDLE hMod);
	// ��ĳ��ģ����������
	DWORD SearchInMod(LPCTSTR name, DWORD* codes, DWORD length, DWORD* save, DWORD save_length = 1, DWORD step = 1);
	// ����������
	DWORD SearchCode(DWORD* codes, DWORD length, DWORD* save, DWORD save_length=1, DWORD step=4);
	// ��ȡ��Ϸ�ڴ�
	bool ReadGameMemory(DWORD flag=0x01);
	// ��ӡ�ռ�
	void InsertLog(char* text);
	// ��ӡ�ռ�
	void InsertLog(wchar_t* text);
public:
	// ��Ϸ����
	HWND  m_hWnd = NULL;
	// ��Ϸ����ID
	DWORD m_dwPid = 0;
	// ��ϷȨ�޾��
	HANDLE   m_hGameProcess = NULL;
	// ��Ϸģ���ַ
	static GameModAddr m_GameModAddr;
	// ��Ϸ��ַ
	static GameAddr m_GameAddr;
	// ��ϷCALL
	static GameCall m_GameCall;
	// ��Ϸ������Ϣ
	static GameWnd m_GameWnd;

	// ��ǰX����
	DWORD m_dwX;
	// ��ǰY����
	DWORD m_dwY;

	// ��������
	DWORD        m_dwGuaiWuCount;
	// ����ָ��[���� ��new]
	GameGuaiWu** m_pGuaiWus;

	// �Ƿ��ȡ���
	bool  m_bIsReadEnd;
	// ��ȡ�ڴ��Ĵ�С
	DWORD m_dwReadSize;
	// ��ȡ����ַ
	DWORD m_dwReadBase;
	// ��ȡ������ʱ�ڴ�
	BYTE* m_pReadBuffer;

	// ��Ϸ���̴�����
	GameProc* m_pGameProc;
	// ��Ʒ��
	Item*     m_pItem;
	// �Ի���
	Talk*     m_pTalk;
	// �ƶ���
	Move*     m_pMove;
	// ������
	GuaiWu*   m_pGuaiWu;
	// ������
	Magic*    m_pMagic;
	// ������
	Pet*      m_pPet;

	// ����
	static Game* self;
public:
	// ����ִ�е�CALL
	static CallStep m_CallStep;
	// �Ƿ�����ִ��CALL
	bool static IsCalling();
	// CALL�Ƿ������
	bool static CallIsComplete();
	// ����CALL STEP����
	void static SetCallStep(CallStepType type, DWORD v1, DWORD v2=0);
	// ���CALL STEP����
	void static ClearCallStep();
	// �����ƶ�����
	void static Call_Run(int x, int y);
	// ����CALL
	void static Call_Talk(const char* msg, int type=0);
	// NPC�Ի�
	void static Call_NPC(int npc_id);
	// NPC�����Ի�
	void static Call_NPCTalk(int no);
	// �ر���ʾ��
	void static Call_CloseTipBox(int close=1);
	// ʹ����Ʒ
	void static Call_UseItem(int item_id);
	// ����Ʒ
	void static Call_DropItem(int item_id);
	// ����Ʒ
	void static Call_PickUpItem(GameGroundItem* p);
	// �ż���
	void static Call_Magic(int magic_id, int guaiwu_id);
	// �ż���
	void static Call_Magic(int magic_id, int x, int y);
	// �������
	void static Call_PetOut(int pet_id);
	// �����ٻ�
	void static Call_PetIn(int pet_id);
	// �������
	void static Call_PetFuck(int pet_id);
	// �Ƿ�ѡ���������
	void static Call_CheckTeam(int v=1);
};