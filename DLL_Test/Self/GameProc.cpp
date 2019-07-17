#include "GameProc.h"
#include "Game.h"
#include "GameStep.h"
#include "Move.h"
#include "Item.h"
#include "GuaiWu.h"
#include "Talk.h"
#include "Magic.h"

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

// 运行
void GameProc::Run()
{
	char log[64];
	//INLOGVARN(32, "反：%.2f", asin(0.5f));
#if RUNRUN == 0
	return;
#endif
	if (!m_pGameStep->InitSteps()) {
		printf("初始化步骤失败，无法继续运行！！！\n");
		return;
	}
	//ReadQuickKey2Num();
	//INLOGVARP(log, "快捷键上物品数量:%d %d", m_QuickKey2Nums[0], m_QuickKey2Nums[1]);
	int n = 0;
	int exec_time = time(nullptr) + 5;
	while (true) {
		int c = exec_time - time(nullptr);
		if (c >= 0 && n != c) {
			printf("准备开始执行，请别操作.[%d秒]\n", c);
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

		switch (step->OpCode)
		{
		case OP_MOVE:
			printf("正在移动:%d.%d\n", step->X, step->Y);
			Move();
			break;
		case OP_NPC:
			printf("NPC:%s\n", step->NPCName);
			NPC();
			break;
		case OP_SELECT:
			printf("选项:%d\n", step->SelectNo);
			Select();
			break;
		case OP_MAGIC:
			printf("技能:%08X\n", step->Magic);
			Magic();
			break;
		case OP_CRAZY:
			//KeyDown(step->Keys);
			break;
		case OP_WAIT:
			printf("等待:%d %08X\n", step->WaitMs/1000, step->Magic);
			Wait();
			break;
		default:
			break;
		}

		while (true) {
			Sleep(100);
			if (IsNeedAddLife()) {
				printf("加血.\n");
				AddLife();
			}
			if (StepIsComplete()) { // 已完成此步骤
				break;
			}
		}
		m_pGameStep->CompleteExec();

#if 0
		int ms = 100;
		if (m_pGameStep->CurrentCode() == OP_MOVE || m_pGameStep->NextCode() == OP_MOVE) {
			ms = 25;
		}
		Sleep(ms);
		//break;
#endif
	}

	int second = time(nullptr) - start_time;
	printf("完成，总用时:%02d分%02d秒\n", second/60, second%60);
}

// 步骤是否已执行完毕
bool GameProc::StepIsComplete()
{
	bool result = false;
	switch (m_pStep->OpCode)
	{
	case OP_MOVE:
		if (m_pGame->m_pMove->IsMoveEnd()) { // 已到指定位置
			result = true;
			goto end;
		}
		if (!m_pGame->m_pMove->IsMove()) {   // 已经停止移动
			m_pGame->m_pMove->Run(m_pStep->X, m_pStep->Y);
		}
		break;
	case OP_NPC:
		result = m_pGame->m_pTalk->NPCTalkStatus();
		if (!result) { // 对话框没有打开
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

// 移动
void GameProc::Move()
{
	m_pGame->m_pMove->Run(m_pStep->X, m_pStep->Y);
}

// 点击
void GameProc::NPC()
{
	// https://31d7f5.link.yunpan.360.cn/lk/surl_yL2uvtfBesv#/-0
	m_pGame->m_pTalk->NPC(m_pStep->NPCName);
}

// 选择
void GameProc::Select()
{
	for (DWORD i = 0; i < m_pStep->OpCount; i++) {
		m_pGame->m_pTalk->NPCTalk(m_pStep->SelectNo);
		if (i > 0) {
			Sleep(500);
			m_pGame->m_pTalk->WaitTalkBoxOpen();
		}
	}
}

// 鼠标移动
void GameProc::Magic()
{
	if (m_pStep->WaitMs) { // 需要等待冷却
		while (!m_pGame->m_pMagic->CheckCd(m_pStep->Magic)) {
			Sleep(100);
		}
	}

	DWORD x = m_pStep->X, y = m_pStep->Y;
	if (!x || !y) { // 释放的位置坐标
		m_pGame->ReadCoor(&x, &y);
	}
	m_pGame->m_pMagic->UseMagic(m_pStep->Magic, x, y);
}

// 按键
void GameProc::KeyDown(unsigned char* keys)
{
	while (*keys) {
		INLOGVARN(16, "->按键：%d", *keys);

		Drv_KeyDown(*keys);
		Sleep(25);
		Drv_KeyUp(*keys);

		keys++;
		Sleep(800);
	}
	m_pGameStep->CompleteExec();
}

// 等待
void GameProc::Wait()
{
	if (m_pStep->Magic) { // 等待技能冷却
		while (!m_pGame->m_pMagic->CheckCd(m_pStep->Magic, m_pStep->WaitMs)) {
			Sleep(100);
		}
	}
	else { // 等待
		Wait(m_pStep->WaitMs);
	}
}

// 等待
void GameProc::Wait(DWORD ms)
{
	int n = 0;
	__int64 now_time = getmillisecond();
	while (true) {
		int c = getmillisecond() - now_time;
		c = (int)ms - c;

		if (c >= 0 && n != c) {
			printf("等待%d秒，还剩%d秒\n", ms / 1000, c / 1000);
		}
		if (c <= 0) {
			break;
		}
		n = c;
		Sleep(500);
	}
}

// 需要重新移动
bool GameProc::IsNeedReMove()
{
	if (!m_pStep->Exec)
		return true;

	if (m_iCoorX == m_iMovCoorX && m_iCoorY == m_iMovCoorY) {   // 移动到指定坐标
		//INLOG(">>移动到指定位置.");
		return true;
	}
	if (m_pGameStep->GetHasExecMs(m_pStep) >= ONE_MOV_MAX_MS) { // 移动时间超过允许时间
		return true;
	}
	if (!m_pGame->IsMove()) { // 已停止
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
				INLOGVARP(log, "->点击[%d,%d] 屏幕坐标[%d,%d] 第%d/%d次", x, y, cx, cy, i, num);
			}
			else {
				INLOGVARP(log, "->点击[%d,%d] 屏幕坐标[%d,%d]", x, y, cx, cy);
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
			INLOGVARP(log, "->移动鼠标失败[%d,%d]", cx, cy);
		}
		//Sleep(25);
	}
	return true;
}

// 计算实际要移动要的坐标
void GameProc::CalcRealMovCoor()
{
	
}

// 制作点击坐标x
int GameProc::MakeClickX(int& x, int& y, int dist_x)
{
	float left_x = (dist_x - m_iCoorX) * ONE_COOR_PIX * 1.25f;     // 要点击坐标差值
	float mul_x = (90.f - 30.f) / 90.f;
	float mul_y = 1.f - mul_x;

	x = mul_x * left_x;
	y = mul_y * left_x;

	return 0;
}

// 制作点击坐标y
int GameProc::MakeClickY(int& x, int& y, int dist_y)
{
	float left_y = (dist_y - m_iCoorY) * -1 * ONE_COOR_PIX * 1.25f;     // 要点击坐标差值
	float mul_x = (90.f - 30.f) / 90.f;
	float mul_y = 1.f - mul_x;

	x += mul_x * left_y;
	y += mul_y * left_y * -1;

	//INLOGVARN(64, "!!!Click Y->%d,%d", x, y);
	return 0;
}

// 制作点击坐标
int GameProc::MakeClickCoor(int& x, int& y, int dist_x, int dist_y)
{
	int role_x_spos = m_pGame->m_GameWnd.Width / 2 + m_pGame->m_GameWnd.Rect.left; // 人物在屏幕的坐标
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

// 读取人物坐标
bool GameProc::ReadCoor()
{
	return m_pGame->ReadCoor(&m_iCoorX, &m_iCoorY);
}

// 读取人物血量
bool GameProc::ReadLife()
{
	return m_pGame->ReadLife(m_iLife, m_iLifeMax);
}

// 读取快捷键上面物品数量
bool GameProc::ReadQuickKey2Num()
{
	return m_pGame->ReadQuickKey2Num(m_QuickKey2Nums, sizeof(m_QuickKey2Nums));
}

// 读取包包物品
bool GameProc::ReadBag()
{
	return m_pGame->ReadBag(m_dwBag, sizeof(m_dwBag));
}

// 是否需要加血量
bool GameProc::IsNeedAddLife()
{
	if (!ReadLife())
		return false;
	if (m_iLife == 0)
		return false;

	return (m_iLife + 2500) <= m_iLifeMax;
}

// 加血
void GameProc::AddLife()
{
	bool add = false;
	DWORD count = m_pGame->m_pItem->GetSelfItemCountByType(速效治疗药水);
	if (count > 0) { // 有药水
		m_pGame->m_pItem->UseSelfItemByType(速效治疗药水);
		add = true;
		count--;
	}
	if (count == 0) { // 没有药水了
		if (m_pGame->m_pItem->GetSelfItemCountByType(速效治疗包) > 0) {
			if (add) {
				Sleep(200);
			}	
			m_pGame->m_pItem->UseSelfItemByType(速效治疗包); // 使用药包开出药水
			if (!add) {
				Sleep(200);
				m_pGame->m_pItem->UseSelfItemByType(速效治疗药水);
			}
		}
	}
}

// 停止
void GameProc::Stop(bool v)
{
	INLOGVARN(32, "Set Stop:%d", v);
	m_bStop = v;
}


