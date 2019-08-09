#pragma once
#include "GameStruct.h"
#include <Windows.h>

#define ONE_COOR_PIX     50   // һ����ռ������
#define ONE_MAX_MOV_COOR 8    // һ�������ƶ�8��������
#define ONE_MOV_MAX_MS   1000 // һ���ƶ��������ʱ��(����)

#define BAG_NUM         40       // ��������
#define WUPIN_YAOBAO    0x0B5593 // ҩ��
#define WUPIN_XIANGLIAN 0x0B5B24 // ����������
#define WUPIN_YAO       0x0F6982 // ҩ
#define WUPIN_LINGYAO   0x0F943E // ��Чʥ����ҩ

#define RUNRUN 1
struct _step_;

class Game;
class GameStep;
class GameProc
{
public:
	// !!!
	GameProc(Game* pGame);

	// ��ʼ������
	void InitData();
	// ����
	void Run();
	// �����Ƿ���ִ�����
	bool StepIsComplete();
	// �ƶ�
	void Move();
	// NCP
	void NPC();
	// ѡ��
	void Select();
	// ����
	void Magic();
	// ����-����
	void MagicPet();
	// ��˦
	void Crazy();
	// ���
	void Clear();
	// ����
	void PickUp();
	// ����
	void CheckIn();
	// ʹ����Ʒ
	void UseItem();
	// ������Ʒ
	void SellItem();
	// �ȴ�
	void Wait();
	// �ȴ�
	void Wait(DWORD ms);
	// �Ƿ��ڸ���
	bool IsInFB();
	// ��ȡ��������
	bool ReadCoor();
	// ��ȡ����Ѫ��
	bool ReadLife();
	// ��ȡ��ݼ�������Ʒ����
	bool ReadQuickKey2Num();
	// ��ȡ������Ʒ
	bool ReadBag();
	// �Ƿ���Ҫ��Ѫ��
	bool IsNeedAddLife();
	// ��Ѫ
	void AddLife();
	// ֹͣ
	void Stop(bool v=true);
public:
	// ��Ϸָ��
	Game*     m_pGame;
	// ��Ϸ����ָ��
	GameStep* m_pGameStep;
	// ����ִ�еĲ���
	_step_* m_pStep;
	// ѡ���ĸ������ļ�
	int  m_iChkFile = 1;
	// �Ƿ�ֹͣ
	bool  m_bStop = false;
	// �Ƿ���ͣ
	bool  m_bPause = false;
	// �Ƿ����¿�ʼ
	bool  m_bReStart = false;
	// �Ƿ��˦
	bool m_bIsCrazy = false;
	// ��˦����
	char m_CrazyMagic[32];

	// �Ƿ��������
	DWORD m_dwKairi = 0;
	// ��������ʱ��
	__int64 m_i64SearchKairi;

	// ��һ��ִ�в���������Ϣ
	struct {
		DWORD MvX;       // �ƶ�X
		DWORD MvY;       // �ƶ�Y
		DWORD NPCId;     // NPCId
		CHAR  Magic[32]; // ����
	} m_stLastStepInfo;
	// ��������
	DWORD   m_iCoorX = 0;
	DWORD   m_iCoorY = 0;
	// �����ƶ���������
	int   m_iMovCoorX = 0;
	int   m_iMovCoorY = 0;
	// ����Ѫ��
	int   m_iLife = 0;
	int   m_iLifeMax = 0;
	// ��ݼ�����Ʒ����
	int   m_QuickKey2Nums[2];
	// ������Ʒ
	DWORD m_dwBag[BAG_NUM];
	// ��Ѫʱ��
	__int64 m_i64AddLifeTime = 0;
};
