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

// 运行
void GameProc::Run()
{
	m_bStop = false;
	m_bPause = false;
	m_bReStart = false;

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

	DWORD dwPetNo[] = { 0, 1, 3 };
	m_pGame->m_pPet->PetOut(dwPetNo, sizeof(dwPetNo)/sizeof(DWORD), true);

	printf("开始\n");
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
			Crazy();
			break;
		case OP_CLEAR:
			Clear();
			break;
		case OP_PICKUP:
			printf("捡拾物品\n");
			PickUp();
			break;
		case OP_WAIT:
			printf("等待:%d %08X\n", step->WaitMs/1000, step->Magic);
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
				// 丢弃一些药品
				m_pGame->m_pItem->DropSelfItemByType(速效治疗药水, 6);
				m_pGame->m_pItem->DropSelfItemByType(速效治疗包,   8);
				// 复活所有没有血量宠物
				m_pGame->m_pPet->Revive();
			} while (false);

			if (StepIsComplete()) { // 已完成此步骤
				if (m_pGame->m_pTalk->NPCTalkStatus()) // 对话框还是打开的
					m_pGame->m_pTalk->NPCTalk(0xff);   // 关掉它
				if (m_pGame->m_pTalk->TipBoxStatus())  // 提示框是否打开
					m_pGame->m_pTalk->CloseTipBox();   // 关掉它

				m_pGameStep->CompleteExec();
				break;
			}
		} while (true);
	}
end:
	int second = time(nullptr) - start_time;
	printf("完成，总用时:%02d分%02d秒\n", second/60, second%60);
	if (m_bReStart) {
		m_pGameStep->m_Step.Release(1);
		printf("GameProc::Run重新加载...\n");
		Run();
	}
}

// 步骤是否已执行完毕
bool GameProc::StepIsComplete()
{
	if (m_bIsCrazy) {
		m_pGame->m_pGuaiWu->Clear(m_CrazyMagic, 8, 8); // 清除5*5范围内的怪物
	}

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
		result = true;
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
		printf("清怪:%08X\n", m_pStep->Magic);
		result = m_pGame->m_pGuaiWu->Clear(m_pStep->Magic, 8, 8); // 清除5*5范围内的怪物
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

// 移动
void GameProc::Move()
{
	m_stLastStepInfo.MvX = m_pStep->X;
	m_stLastStepInfo.MvY = m_pStep->Y;
	m_pGame->m_pMove->Run(m_pStep->X, m_pStep->Y);
}

// 点击
void GameProc::NPC()
{
	// https://31d7f5.link.yunpan.360.cn/lk/surl_yL2uvtfBesv#/-0
	if (strcmp(m_pStep->NPCName, "上一个")) { // 对话上一个对了话的NPC
		m_pGame->m_pTalk->NPC(m_stLastStepInfo.NPCId);
	}
	else {
		m_stLastStepInfo.NPCId = m_pGame->m_pTalk->NPC(m_pStep->NPCName);
	}
}

// 选择
void GameProc::Select()
{
	if (!m_stLastStepInfo.NPCId) // 没有与NPC对话, 没必要再选择
		return;

	for (DWORD i = 1; i <= m_pStep->OpCount; i++) {
		Sleep(500);
		if (i < m_pStep->OpCount) { // 不是最后一次
			if (!m_pGame->m_pTalk->WaitTalkBoxOpen()) {        // 等待对话框打开超时
				m_pGame->m_pTalk->NPC(m_stLastStepInfo.NPCId); // 重新与NPC对话
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

// 技能
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

// 狂甩
void GameProc::Crazy()
{
	m_bIsCrazy = m_pStep->Magic == 未知技能 ? false : true;
	m_CrazyMagic = m_pStep->Magic;
	if (m_bIsCrazy) {
		printf("开启狂甩模式\n");
	}
	else {
		printf("狂甩模式结束\n");
	}
}

// 清怪
void GameProc::Clear()
{
}

// 捡物
void GameProc::PickUp()
{
	ITEM_TYPE items[] = { 速效圣兽灵药 };
	m_pGame->m_pItem->PickUpItem(items, sizeof(items) / sizeof(ITEM_TYPE));
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
	if (getmillisecond() < (m_i64AddLifeTime + 500)) // 1秒内不重复加
		return false;
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
				add = true;
			}
		}
	}
	if (add == true) {
		printf("加血.\n");
		m_i64AddLifeTime = getmillisecond();
	}	
}

// 停止
void GameProc::Stop(bool v)
{
	INLOGVARN(32, "Set Stop:%d", v);
	m_bStop = v;
}


