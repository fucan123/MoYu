#include "GameProc.h"
#include "Game.h"
#include "GameStep.h"
#include "Move.h"
#include "Item.h"
#include "GuaiWu.h"
#include "Talk.h"
#include "Magic.h"
#include "Pet.h"

#include <My/Common/mystring.h>
#include <My/Common/func.h>
#include <My/Driver/KbdMou.h>
#include <psapi.h>
#include <stdio.h>
#include <time.h>

// !!!
GameProc::GameProc(Game* pGame)
{
	m_pGame = pGame;
	m_pGameStep = new GameStep;
}

// ����
void GameProc::Run()
{
	m_bStop = false;
	m_bPause = false;
	m_bReStart = false;

	char log[64];
	//INLOGVARN(32, "����%.2f", asin(0.5f));
#if RUNRUN == 0
	return;
#endif
	if (!m_pGameStep->InitSteps()) {
		printf("��ʼ������ʧ�ܣ��޷��������У�����\n");
		return;
	}
	//ReadQuickKey2Num();
	//INLOGVARP(log, "��ݼ�����Ʒ����:%d %d", m_QuickKey2Nums[0], m_QuickKey2Nums[1]);
	int n = 0;
	int exec_time = time(nullptr) + 5;
	while (true) {
		int c = exec_time - time(nullptr);
		if (c >= 0 && n != c) {
			printf("׼����ʼִ�У�������.[%d��]\n", c);
		}
		n = c;
		if (c < 0) {
			break;
		}
		Sleep(500);
	}

	DWORD dwPetNo[] = { 0, 1, 3 };
	m_pGame->m_pPet->PetOut(dwPetNo, sizeof(dwPetNo)/sizeof(DWORD), true);

	printf("��ʼ\n");
	//Drv_MouseMovAbsolute(m_pGame->m_GameWnd.Rect.left, m_pGame->m_GameWnd.Rect.top);
	//return;
	//INLOGVARN(32, "Step:%08x Stop:%d", step, m_bStop);
	int start_time = time(nullptr);
	_step_* step;
	while (step = m_pGameStep->Current()) {
		m_pStep = step;

		switch (step->OpCode)
		{
		case OP_MOVE:
			printf("�����ƶ�:%d.%d\n", step->X, step->Y);
			Move();
			break;
		case OP_NPC:
			printf("NPC:%s\n", step->NPCName);
			NPC();
			break;
		case OP_SELECT:
			printf("ѡ��:%d\n", step->SelectNo);
			Select();
			break;
		case OP_MAGIC:
			printf("����:%08X\n", step->Magic);
			Magic();
			break;
		case OP_CRAZY:
			Crazy();
			break;
		case OP_CLEAR:
			Clear();
			break;
		case OP_PICKUP:
			printf("��ʰ��Ʒ\n");
			PickUp();
			break;
		case OP_WAIT:
			printf("�ȴ�:%d %08X\n", step->WaitMs/1000, step->Magic);
			Wait();
			break;
		default:
			break;
		}

		do {
			do {
				if (m_bStop || m_bReStart)
					goto end;
				if (m_bPause)
					Sleep(500);
			} while (m_bPause);
				
			Sleep(100);
			if (IsNeedAddLife()) {
				AddLife();
			}
			do {
				// ����һЩҩƷ
				m_pGame->m_pItem->DropSelfItemByType(��Ч����ҩˮ, 6);
				m_pGame->m_pItem->DropSelfItemByType(��Ч���ư�,   8);
				// ��������û��Ѫ������
				m_pGame->m_pPet->Revive();
			} while (false);

			if (StepIsComplete()) { // ����ɴ˲���
				if (m_pGame->m_pTalk->NPCTalkStatus()) // �Ի����Ǵ򿪵�
					m_pGame->m_pTalk->NPCTalk(0xff);   // �ص���
				if (m_pGame->m_pTalk->TipBoxStatus())  // ��ʾ���Ƿ��
					m_pGame->m_pTalk->CloseTipBox();   // �ص���

				m_pGameStep->CompleteExec();
				break;
			}
		} while (true);
	}
end:
	int second = time(nullptr) - start_time;
	printf("��ɣ�����ʱ:%02d��%02d��\n", second/60, second%60);
	if (m_bReStart) {
		m_pGameStep->m_Step.Release(1);
		printf("GameProc::Run���¼���...\n");
		Run();
	}
}

// �����Ƿ���ִ�����
bool GameProc::StepIsComplete()
{
	if (m_bIsCrazy) {
		m_pGame->m_pGuaiWu->Clear(m_CrazyMagic, 8, 8); // ���5*5��Χ�ڵĹ���
	}

	bool result = false;
	switch (m_pStep->OpCode)
	{
	case OP_MOVE:
		if (m_pGame->m_pMove->IsMoveEnd()) { // �ѵ�ָ��λ��
			result = true;
			goto end;
		}
		if (!m_pGame->m_pMove->IsMove()) {   // �Ѿ�ֹͣ�ƶ�
			m_pGame->m_pMove->Run(m_pStep->X, m_pStep->Y);
		}
		break;
	case OP_NPC:
		result = m_pGame->m_pTalk->NPCTalkStatus();
		result = true;
		if (!result) { // �Ի���û�д�
			m_pGame->m_pTalk->NPC(m_pStep->NPCName);
		}
		break;
	case OP_SELECT:
		result = true;
		break;
	case OP_MAGIC:
		result = true;
		break;
	case OP_CRAZY:
		result = true;
		break;
	case OP_CLEAR:
		printf("���:%08X\n", m_pStep->Magic);
		result = m_pGame->m_pGuaiWu->Clear(m_pStep->Magic, 8, 8); // ���5*5��Χ�ڵĹ���
		break;
	case OP_PICKUP:
		result = true;
		break;
	case OP_WAIT:
		result = true;
		break;
	default:
		result = true;
		break;
	}
end:
	return result;
}

// �ƶ�
void GameProc::Move()
{
	m_stLastStepInfo.MvX = m_pStep->X;
	m_stLastStepInfo.MvY = m_pStep->Y;
	m_pGame->m_pMove->Run(m_pStep->X, m_pStep->Y);
}

// ���
void GameProc::NPC()
{
	// https://31d7f5.link.yunpan.360.cn/lk/surl_yL2uvtfBesv#/-0
	if (strcmp(m_pStep->NPCName, "��һ��")) { // �Ի���һ�����˻���NPC
		m_pGame->m_pTalk->NPC(m_stLastStepInfo.NPCId);
	}
	else {
		m_stLastStepInfo.NPCId = m_pGame->m_pTalk->NPC(m_pStep->NPCName);
	}
}

// ѡ��
void GameProc::Select()
{
	if (!m_stLastStepInfo.NPCId) // û����NPC�Ի�, û��Ҫ��ѡ��
		return;

	for (DWORD i = 1; i <= m_pStep->OpCount; i++) {
		Sleep(500);
		if (i < m_pStep->OpCount) { // �������һ��
			if (!m_pGame->m_pTalk->WaitTalkBoxOpen()) {        // �ȴ��Ի���򿪳�ʱ
				m_pGame->m_pTalk->NPC(m_stLastStepInfo.NPCId); // ������NPC�Ի�
			}
		}
		m_pGame->m_pTalk->NPCTalk(m_pStep->SelectNo);

		if (i > 1) {
			if (IsNeedAddLife()) {
				AddLife();
			}
		}
	}
}

// ����
void GameProc::Magic()
{
	if (m_pStep->WaitMs) { // ��Ҫ�ȴ���ȴ
		while (!m_pGame->m_pMagic->CheckCd(m_pStep->Magic)) {
			Sleep(100);
		}
	}

	DWORD x = m_pStep->X, y = m_pStep->Y;
	if (!x || !y) { // �ͷŵ�λ������
		m_pGame->ReadCoor(&x, &y);
	}
	m_pGame->m_pMagic->UseMagic(m_pStep->Magic, x, y);
}

// ��˦
void GameProc::Crazy()
{
	m_bIsCrazy = m_pStep->Magic == δ֪���� ? false : true;
	m_CrazyMagic = m_pStep->Magic;
	if (m_bIsCrazy) {
		printf("������˦ģʽ\n");
	}
	else {
		printf("��˦ģʽ����\n");
	}
}

// ���
void GameProc::Clear()
{
}

// ����
void GameProc::PickUp()
{
	ITEM_TYPE items[] = { ��Чʥ����ҩ };
	m_pGame->m_pItem->PickUpItem(items, sizeof(items) / sizeof(ITEM_TYPE));
}

// �ȴ�
void GameProc::Wait()
{
	if (m_pStep->Magic) { // �ȴ�������ȴ
		while (!m_pGame->m_pMagic->CheckCd(m_pStep->Magic, m_pStep->WaitMs)) {
			Sleep(100);
		}
	}
	else { // �ȴ�
		Wait(m_pStep->WaitMs);
	}
}

// �ȴ�
void GameProc::Wait(DWORD ms)
{
	int n = 0;
	__int64 now_time = getmillisecond();
	while (true) {
		int c = getmillisecond() - now_time;
		c = (int)ms - c;

		if (c >= 0 && n != c) {
			printf("�ȴ�%d�룬��ʣ%d��\n", ms / 1000, c / 1000);
		}
		if (c <= 0) {
			break;
		}
		n = c;
		Sleep(500);
	}
}

// ��Ҫ�����ƶ�
bool GameProc::IsNeedReMove()
{
	return false;
}

bool GameProc::ClickEvent(int x, int y, int num, bool make)
{
	char log[64];

	int i = 0;
	while (i++ < num) {
		int cx = x, cy = y;
		if (make) {
			//INLOG("MAKE.");
			MakeClickCoor(cx, cy, x, y);
		}
		if (Drv_MouseMovAbsolute(cx, cy)) {
			if (num > 1) {
				INLOGVARP(log, "->���[%d,%d] ��Ļ����[%d,%d] ��%d/%d��", x, y, cx, cy, i, num);
			}
			else {
				INLOGVARP(log, "->���[%d,%d] ��Ļ����[%d,%d]", x, y, cx, cy);
			}

			Sleep(50);
#if 1
			if (IsNeedAddLife()) {
				AddLife();
			}
#endif
			Drv_MouseLeftDown();
			Sleep(25);
			Drv_MouseLeftUp();
			if (num > 1) {
				Sleep(1500);
			}
		}
		else {
			INLOGVARP(log, "->�ƶ����ʧ��[%d,%d]", cx, cy);
		}
		//Sleep(25);
	}
	return true;
}

// ����ʵ��Ҫ�ƶ�Ҫ������
void GameProc::CalcRealMovCoor()
{
	
}

// �����������x
int GameProc::MakeClickX(int& x, int& y, int dist_x)
{
	float left_x = (dist_x - m_iCoorX) * ONE_COOR_PIX * 1.25f;     // Ҫ��������ֵ
	float mul_x = (90.f - 30.f) / 90.f;
	float mul_y = 1.f - mul_x;

	x = mul_x * left_x;
	y = mul_y * left_x;

	return 0;
}

// �����������y
int GameProc::MakeClickY(int& x, int& y, int dist_y)
{
	float left_y = (dist_y - m_iCoorY) * -1 * ONE_COOR_PIX * 1.25f;     // Ҫ��������ֵ
	float mul_x = (90.f - 30.f) / 90.f;
	float mul_y = 1.f - mul_x;

	x += mul_x * left_y;
	y += mul_y * left_y * -1;

	//INLOGVARN(64, "!!!Click Y->%d,%d", x, y);
	return 0;
}

// �����������
int GameProc::MakeClickCoor(int& x, int& y, int dist_x, int dist_y)
{
	int role_x_spos = m_pGame->m_GameWnd.Width / 2 + m_pGame->m_GameWnd.Rect.left; // ��������Ļ������
	int role_y_spos = m_pGame->m_GameWnd.Height / 2 + m_pGame->m_GameWnd.Rect.top;

	char log[64];
	x = 0, y = 0;
	MakeClickX(x, y, dist_x);
	MakeClickY(x, y, dist_y);
	//INLOGVARP(log, "!!!all[%.2f,%.2f] [%d,%d]", 0, 0, x, y);

	x += role_x_spos;
	y += role_y_spos;

	return 0;// role_spos + coor_left * ONE_COOR_PIX;
}

// ��ȡ��������
bool GameProc::ReadCoor()
{
	return m_pGame->ReadCoor(&m_iCoorX, &m_iCoorY);
}

// ��ȡ����Ѫ��
bool GameProc::ReadLife()
{
	return m_pGame->ReadLife(m_iLife, m_iLifeMax);
}

// ��ȡ��ݼ�������Ʒ����
bool GameProc::ReadQuickKey2Num()
{
	return m_pGame->ReadQuickKey2Num(m_QuickKey2Nums, sizeof(m_QuickKey2Nums));
}

// ��ȡ������Ʒ
bool GameProc::ReadBag()
{
	return m_pGame->ReadBag(m_dwBag, sizeof(m_dwBag));
}

// �Ƿ���Ҫ��Ѫ��
bool GameProc::IsNeedAddLife()
{
	if (getmillisecond() < (m_i64AddLifeTime + 500)) // 1���ڲ��ظ���
		return false;
	if (!ReadLife())
		return false;
	if (m_iLife == 0)
		return false;

	return (m_iLife + 2500) <= m_iLifeMax;
}

// ��Ѫ
void GameProc::AddLife()
{
	bool add = false;
	DWORD count = m_pGame->m_pItem->GetSelfItemCountByType(��Ч����ҩˮ);
	if (count > 0) { // ��ҩˮ
		m_pGame->m_pItem->UseSelfItemByType(��Ч����ҩˮ);
		add = true;
		count--;
	}
	if (count == 0) { // û��ҩˮ��
		if (m_pGame->m_pItem->GetSelfItemCountByType(��Ч���ư�) > 0) {
			if (add) {
				Sleep(200);
			}	
			m_pGame->m_pItem->UseSelfItemByType(��Ч���ư�); // ʹ��ҩ������ҩˮ
			if (!add) {
				Sleep(200);
				m_pGame->m_pItem->UseSelfItemByType(��Ч����ҩˮ);
				add = true;
			}
		}
	}
	if (add == true) {
		printf("��Ѫ.\n");
		m_i64AddLifeTime = getmillisecond();
	}	
}

// ֹͣ
void GameProc::Stop(bool v)
{
	INLOGVARN(32, "Set Stop:%d", v);
	m_bStop = v;
}


