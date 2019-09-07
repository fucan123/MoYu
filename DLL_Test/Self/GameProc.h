#pragma once
#include "GameStruct.h"
#include <My/Common/Link.hpp>
#include <Windows.h>
#include <vector>
#include <fstream>

using namespace std;

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
	// ȡ���������İ�ť
	void CancelAllButton();
	// ִ��
	void Exec();
	// ����
	void Run();
	// �������
	bool ViteInTeam(const char* name);
	// ���
	void InTeam();
	// ���������
	void ViteInFB();
	// ͬ�������
	void AgreeInFB();
	// ���ȥ������½����
	void GoLeiMing();
	// ȥ��ȡ����
	void GoGetXiangLian();
	// ȥ�����ſ�
	void GoFBDoor();
	// ������
	bool GoInFB();
	// ������
	void GoOutFB(const char* name, bool notify=true);
	// ִ�и�������
	void ExecInFB();
	// ִ������
	bool ExecStep(vector<_step_*>& link);
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
	// ��Ǯ
	void SaveMoney();
	// ����
	DWORD CheckIn(bool in=true);
	// ʹ����Ʒ
	void UseItem();
	// ����Ʒ
	void DropItem();
	// ������Ʒ
	void SellItem();
	// ��ť
	void Button();
	// ��ť
	bool Button(int button_id, DWORD sleep_ms=0, const char* name=nullptr);
	// �ȴ�
	void Wait();
	// �ȴ�
	void Wait(DWORD ms);
	// ����
	void ReBorn();
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
	int  IsNeedAddLife();
	// ��Ѫ
	void AddLife();
	// ������Ϣ�������
	void SendMsg(const char* v, const char* v2=nullptr);
	// ֹͣ
	void Stop(bool v=true);
	// ���ռ��ļ�
	void OpenLogFile();
	// д���ռ�
	void WriteLog(const char* log, bool flush=false);
public:
	// ��Ϸָ��
	Game*     m_pGame;
	// ��Ϸ����ָ��
	GameStep* m_pGameStep;
	// ����ִ�еĲ���
	_step_* m_pStep;
	// ����ִ�еĲ���[����]
	_step_* m_pStepCopy;
	// ѡ���ĸ������ļ�
	int  m_iChkFile = 2;
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
		DWORD MvX;         // �ƶ�X
		DWORD MvY;         // �ƶ�Y
		DWORD NPCId;       // NPCId
		CHAR  NPCName[32]; // NPC����
		CHAR  Magic[32];   // ����
		bool  IsOut;       // �Ƿ������
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

	// �Ƿ��������븱��
	bool m_bLockGoFB = false;
	// �Ƿ��ڸ�������
	bool m_bAtFB = false;
	// ��������ҩ����
	int  m_iNeedYao = 6;
	// �������ư�����
	int  m_iNeedBao = 6;

	// �Ƿ����BOSS
	bool m_bLastBoss = false;
	// �Ƿ��ͳ�ȥ����
	bool m_bSendOut = false;

	ofstream m_LogFile;
};
