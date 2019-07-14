#include "GameProc.h"
#include "Game.h"
#include "GameStep.h"

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
	char log[64];
	//INLOGVARN(32, "����%.2f", asin(0.5f));
#if RUNRUN == 0
	return;
#endif
	if (!m_pGameStep->InitSteps()) {
		INLOG("��ʼ������ʧ�ܣ��޷��������У�����");
		return;
	}
	ReadQuickKey2Num();
	INLOGVARP(log, "��ݼ�����Ʒ����:%d %d", m_QuickKey2Nums[0], m_QuickKey2Nums[1]);
	int n = 0;
	int exec_time = time(nullptr) + 5;
	while (true) {
		int c = exec_time - time(nullptr);
		if (c >= 0 && n != c) {
			INLOGVARN(64, "׼����ʼִ�У�������.[%d��]", c);
		}
		n = c;
		if (c < 0) {
			break;
		}
		Sleep(500);
	}
	//Drv_MouseMovAbsolute(m_pGame->m_GameWnd.Rect.left, m_pGame->m_GameWnd.Rect.top);
	//return;
	//INLOGVARN(32, "Step:%08x Stop:%d", step, m_bStop);
	int start_time = time(nullptr);
	_step_* step;
	while (step = m_pGameStep->Current()) {
		while (m_bStop) {
			Sleep(1000);
		}

		m_pStep = step;

		//INLOGVARN(64, "ִ��ָ��:[%d] Exec:%d", step->OpCode, step->Exec);
		if (ReadCoor()) {
			switch (step->OpCode)
			{
			case OP_MOVE:
				Move(step->Pos.x, step->Pos.y);
				break;
			case OP_CLICK:
				Click(step->Pos.x, step->Pos.y, step->ClickNum);
				break;
			case OP_SELECT:
				Select(step->Pos.x, step->Pos.y, step->SelectNo);
				break;
			case OP_MOUMOVE:
				MouMove(step->Pos.x, step->Pos.y);
				break;
			case OP_KEYDOWN:
				KeyDown(step->Keys);
				break;
			case OP_WAIT:
				Wait(step->WaitMs);
				break;
			default:
				break;
			}

			if (IsNeedAddLife()) {
				AddLife();
			}
		}
		else {
			INLOG("��ȡ����ʧ�ܣ�����");
		}

		int ms = 100;
		if (m_pGameStep->CurrentCode() == OP_MOVE || m_pGameStep->NextCode() == OP_MOVE) {
			ms = 25;
		}
		Sleep(ms);
		//break;
	}

	int second = time(nullptr) - start_time;
	INLOGVARP(log, "��ɣ�����ʱ:%02d��%02d��", second/60, second%60);
}

// �ƶ�
void GameProc::Move(int x, int y)
{
	if (m_iCoorX == x && m_iCoorY == y) {
		INLOG("->�ƶ������.");
		m_pGameStep->CompleteExec();
		return;
	}

	
	if (m_pStep->Exec) {
		if (!IsNeedReMove()) {
			return;
		}

		m_pGameStep->SetExec(false, m_pStep);
		
		//INLOG(">>����ִ����.");
		
	}

	if (!m_pStep->Exec) {
		CalcRealMovCoor();

		char log[64];
		INLOGVARP(log, "->[%d,%d]�ƶ���[%d,%d] �յ�:[%d,%d]", m_iCoorX, m_iCoorY, m_iMovCoorX, m_iMovCoorY, x, y);
		ClickEvent(m_iMovCoorX, m_iMovCoorY, 1, m_pStep->Pos.flag);
		m_pGameStep->SetExec(true);
	}
	//INLOGVARP(log, ">>�ƶ�״̬:%d", m_pGame->IsMove());
}

// ���
void GameProc::Click(int x, int y, int num)
{
	//Sleep(2000);
	INLOG("->��ʼ���");
	if (!m_pStep->Pos.flag) {
		x = m_pGame->m_GameWnd.Rect.left + x;
		y = m_pGame->m_GameWnd.Rect.top + y;
	}
	if (ClickEvent(x, y, num, m_pStep->Pos.flag)) {
		m_pGameStep->CompleteExec();
		INLOG("->��ɵ��.");
		Sleep(2000);
	}
}

// ѡ��
void GameProc::Select(int x, int y, int no)
{
	char log[64];
	INLOGVARP(log, "->ѡ���%d��ѡ��[%d,%d] %d", no, x, y, m_pStep->Pos.flag);
	if (!m_pStep->Pos.flag) {
		x = m_pGame->m_GameWnd.Rect.left + x;
		y = m_pGame->m_GameWnd.Rect.top + y;

		y += (no - 1) * 26;
	}
	if (ClickEvent(x, y, m_pStep->ClickNum, m_pStep->Pos.flag)) {
		m_pGameStep->CompleteExec();
		INLOGVARP(log, "->���ѡ��[%d,%d].", x, y);
	}
}

// ����ƶ�
void GameProc::MouMove(int x, int y)
{
	int cx, cy;
	if (!m_pStep->Pos.flag) {
		cx = m_pGame->m_GameWnd.Rect.left + x;
		cy = m_pGame->m_GameWnd.Rect.top + y;

	}
	else {
		MakeClickCoor(cx, cy, x, y);
	}
	
	char log[64];
	INLOGVARP(log, "->�ƶ����[%d,%d] ��Ļ����[%d,%d]", x, y, cx, cy);

	if (Drv_MouseMovAbsolute(cx, cy)) {
		INLOG("->�ƶ�������");
	}
	else {
		INLOGVARP(log, "->�ƶ����ʧ��[%d,%d] ��Ļ����[%d,%d]", x, y, cx, cy);
	}
	m_pGameStep->CompleteExec();
}

// ����
void GameProc::KeyDown(unsigned char* keys)
{
	while (*keys) {
		INLOGVARN(16, "->������%d", *keys);

		Drv_KeyDown(*keys);
		Sleep(25);
		Drv_KeyUp(*keys);

		keys++;
		Sleep(800);
	}
	m_pGameStep->CompleteExec();
}

// �ȴ�
void GameProc::Wait(int ms)
{
	int n = 0;
	__int64 now_time = getmillisecond();
	while (true) {
		int c = getmillisecond() - now_time;
		c = ms - c;

		if (c >= 0 && n != c) {
			INLOGVARN(32, "�ȴ�%d�룬��ʣ%d��", ms/1000, c/1000);
		}
		if (c <= 0) {
			break;
		}
		n = c;
		Sleep(1000);
	}

	m_pGameStep->CompleteExec();
}

// ��Ҫ�����ƶ�
bool GameProc::IsNeedReMove()
{
	if (!m_pStep->Exec)
		return true;

	if (m_iCoorX == m_iMovCoorX && m_iCoorY == m_iMovCoorY) {   // �ƶ���ָ������
		//INLOG(">>�ƶ���ָ��λ��.");
		return true;
	}
	if (m_pGameStep->GetHasExecMs(m_pStep) >= ONE_MOV_MAX_MS) { // �ƶ�ʱ�䳬������ʱ��
		return true;
	}
	if (!m_pGame->IsMove()) { // ��ֹͣ
		return true;
	}
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
	if (!ReadLife())
		return false;
	if (m_iLife == 0)
		return false;

	return (m_iLife + 2500) <= m_iLifeMax;
}

// ��Ѫ
void GameProc::AddLife()
{
	if (!ReadQuickKey2Num())
		return;

	if (m_QuickKey2Nums[0] > 1) {
		INLOG("->��Ѫ.");

		Drv_KeyDown('1');
		Sleep(10);
		Drv_KeyUp('1');
	}
	if (m_QuickKey2Nums[0] <= 3 && m_QuickKey2Nums[1] > 1) {
		INLOG("->ʹ��ҩ��.");
		Sleep(25);
		Drv_KeyDown('2');
		Sleep(10);
		Drv_KeyUp('2');
	}
	Sleep(250);
}

// ֹͣ
void GameProc::Stop(bool v)
{
	INLOGVARN(32, "Set Stop:%d", v);
	m_bStop = v;
}


