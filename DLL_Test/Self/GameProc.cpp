#include "GameProc.h"
#include "Game.h"
#include "GameConf.h"
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

// 初始化数据
void GameProc::InitData()
{
	m_bStop = false;
	m_bPause = true;
	m_bReStart = false;
	m_i64SearchKairi = 0;

	ZeroMemory(&m_stLastStepInfo, sizeof(m_stLastStepInfo));
	m_pGame->m_pMove->InitData();
	m_pGame->m_pItem->InitData();
	m_pGame->m_pTalk->InitData();
	m_pGame->m_pGuaiWu->InitData();
}

// 运行
void GameProc::Run()
{
	//return;
	//Game::KeyDown('I');
	//Sleep(100);
	//Game::KeyUp('I');
	char log[64];
	//INLOGVARN(32, "反：%.2f", asin(0.5f));
#if RUNRUN == 0
	return;
#endif
	m_bPause = true;
	printf("选择配置文件：A:魔域副本流程.txt B:魔域副本流程2.txt\n");
	while (m_bPause) {
		Sleep(500);
	}
	m_pGame->m_pGameConf->ReadConf();
	if (!m_pGameStep->InitSteps(m_iChkFile)) {
		printf("初始化步骤失败，无法继续运行！！！\n");
		return;
	}
start:
	InitData();
	if (m_bPause) {
		printf("按C开始\n");
	}
	while (m_bPause) {
		Sleep(500);
	}
	//ReadQuickKey2Num();
	//INLOGVARP(log, "快捷键上物品数量:%d %d", m_QuickKey2Nums[0], m_QuickKey2Nums[1]);
	m_dwKairi = 0;
	int n = 0;
	int exec_time = time(nullptr) + 2;
	while (true) {
		int c = exec_time - time(nullptr);
		if (c >= 0 && n != c) {
			printf("准备开始执行[%02d秒]\n", c);
		}
		n = c;
		if (c < 0) {
			break;
		}
		Sleep(500);
	}

	DWORD dwPetNo[] = { 0, 1, 2 };
	m_pGame->m_pPet->PetOut(m_pGame->m_pGameConf->m_stPetOut.No, m_pGame->m_pGameConf->m_stPetOut.Length, true);

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
			printf("流程->正在移动:%d.%d\n", step->X, step->Y);
			Move();
			break;
		case OP_NPC:
			printf("流程->NPC:%s\n", step->NPCName);
			NPC();
			break;
		case OP_SELECT:
			printf("流程->选项:%d\n", step->SelectNo);
			Select();
			break;
		case OP_MAGIC:
			printf("流程->技能:%s\n", step->Magic);
			Magic();
			break;
		case OP_MAGIC_PET:
			printf("流程->宠物技能:%s\n", step->Magic);
			MagicPet();
			break;
		case OP_CRAZY:
			Crazy();
			break;
		case OP_CLEAR:
			Clear();
			break;
		case OP_PICKUP:
			printf("流程->捡拾物品\n");
			PickUp();
			break;
		case OP_CHECKIN:
			printf("流程->存入物品\n");
			CheckIn();
			break;
		case OP_USEITEM:
			printf("流程->使用物品\n");
			UseItem();
			break;
		case OP_SELL:
			printf("流程->售卖物品\n");
			SellItem();
			break;
		case OP_WAIT:
			printf("流程->等待:%d %s\n", step->WaitMs/1000, step->Magic);
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
				
			Sleep(250);
			__int64 now_ms = getmillisecond();
			if (IsNeedAddLife()) {
				AddLife();
			}
			do {
				// 丢弃一些药品
				m_pGame->m_pItem->DropSelfItemByType(速效治疗药水, 6);
				m_pGame->m_pItem->DropSelfItemByType(速效治疗包,   8);
				if (m_pStep->OpCode != OP_MOVE) {
					// 复活所有没有血量宠物
					m_pGame->m_pPet->Revive();
				}
				if (m_pStep->OpCode != OP_SELECT && m_dwKairi == 1 
					&& ((now_ms - m_i64SearchKairi) >= 1000)) { // 清掉凯瑞
					DWORD krX = 12, krY = 12;
					if (m_pGame->m_pGuaiWu->IsInArea("伟大的凯瑞", krX, krY)) {
						if (m_pGame->m_pMagic->CheckCd("影魂契约")) {
							m_pGame->m_pMagic->UseMagic("影魂契约", krX, krY);
						}
						else if (m_pGame->m_pMagic->CheckCd("星陨")) {
							m_pGame->m_pMagic->UseMagic("星陨", krX, krY);
						}

						m_dwKairi = 2;
						m_i64SearchKairi = now_ms;
						Wait(10);
					}
				}
				
			} while (false);

			if (StepIsComplete()) { // 已完成此步骤
				if (m_pStep->OpCode != OP_NPC && m_pStep->OpCode != OP_SELECT && m_pStep->OpCode != OP_WAIT) {
					if (m_pGame->m_pTalk->NPCTalkStatus()) // 对话框还是打开的
						m_pGame->m_pTalk->NPCTalk(0xff);   // 关掉它
				}
				if (m_pGame->m_pTalk->TipBoxStatus())  // 提示框是否打开
					m_pGame->m_pTalk->CloseTipBox();   // 关掉它

#if 1
				if (m_pStep->X == 425 && m_pStep->Y == 395) { // 开始扫描伟大的凯瑞
					//m_dwKairi = 1;
				}
#endif
				
				m_pGameStep->CompleteExec();
				Sleep(200);
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
	else {
		m_pGameStep->ResetStep();
		m_bPause = true;
		goto start;
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

// NPC
void GameProc::NPC()
{
	// 500 - 930
	// 560 - 1120
	// https://31d7f5.link.yunpan.360.cn/lk/surl_yL2uvtfBesv#/-0
	if (0
		|| strcmp(m_pStep->NPCName, "贪求缚灵柱") == 0
		|| strcmp(m_pStep->NPCName, "怨恨缚灵柱") == 0
		|| strcmp(m_pStep->NPCName, "迷幻缚灵柱") == 0
		|| strcmp(m_pStep->NPCName, "离爱缚灵柱") == 0) {
		printf("NPC特殊:%s\n", m_pStep->NPCName);
		char gw_name[16];
		int length = strstr(m_pStep->NPCName, "缚灵柱") - m_pStep->NPCName;
		
		if (m_dwKairi == 0) {
			//m_dwKairi = 1;
		}
		if (0 && length > 0) {
			DWORD x = 8, y = 8;
			memset(gw_name, 0, sizeof(gw_name));
			memcpy(gw_name, m_pStep->NPCName, length);
			strcat(gw_name, "之魂");
			printf("要搜索的怪物:%s %d,%d\n", gw_name, x, y);
			__int64 t = getmillisecond();
			while (!m_pGame->m_pGuaiWu->IsInArea(gw_name, x, y)) { // 怪物未来
				if ((getmillisecond() - t) > 10000) {
					printf("10秒还未获取到怪物, 忽略\n");
					break;
				}
				Sleep(100);
			}
			m_pGame->m_pMagic->UseMagic(m_stLastStepInfo.Magic, x, y);
			Sleep(100);
		}
	}
	else if (strcmp(m_pStep->NPCName, "上一个") == 0) { // 对话上一个对了话的NPC
		m_pGame->m_pTalk->NPC(m_stLastStepInfo.NPCId);
	}
	else {
		if (strcmp(m_pStep->NPCName, "魔封障壁") == 0) {
			m_dwKairi = 0;
		}
		m_stLastStepInfo.NPCId = m_pGame->m_pTalk->NPC(m_pStep->NPCName);
	}
}

// 选择
void GameProc::Select()
{
	if (!m_stLastStepInfo.NPCId) // 没有与NPC对话, 没必要再选择
		return;

	for (DWORD i = 1; i <= m_pStep->OpCount; i++) {
		if (1 || i < m_pStep->OpCount) { // 不是最后一次
			if (!m_pGame->m_pTalk->WaitTalkBoxOpen()) {        // 等待对话框打开超时
				printf("重新对话NPC\n");
				m_pGame->m_pTalk->NPC(m_stLastStepInfo.NPCId); // 重新与NPC对话
				if (m_pGame->m_pTalk->WaitTalkBoxOpen()); // NPC有效
					//i--;
			}
		}
		printf("第(%d)选择\n", i);
		m_pGame->m_pTalk->NPCTalk(m_pStep->SelectNo);

		if (i > 1) {
			if (IsNeedAddLife()) {
				AddLife();
			}
			Sleep(500);
		}
	}
}

// 技能
void GameProc::Magic()
{
	if (m_pStep->WaitMs) { // 需要等待冷却
		GameMagic* pMagic = m_pGame->m_pMagic->ReadMagic(m_pStep->Magic);
		if (!pMagic)
			return;
		while (!m_pGame->m_pMagic->CheckCd(pMagic->Id)) {
			Sleep(100);
		}
	}

	DWORD x = m_pStep->X, y = m_pStep->Y;
	if (!x || !y) { // 释放的位置坐标
		m_pGame->ReadCoor(&x, &y);
	}
	m_pGame->m_pMagic->UseMagic(m_pStep->Magic, x, y);
	strcpy(m_stLastStepInfo.Magic, m_pStep->Magic);
	Sleep(100);
}

// 技能-宠物
void GameProc::MagicPet()
{
	DWORD x = m_pStep->X, y = m_pStep->Y;
	if (!x || !y) { // 释放的位置坐标
		m_pGame->ReadCoor(&x, &y);
	}
}

// 狂甩
void GameProc::Crazy()
{
	m_bIsCrazy = strstr(m_pStep->Magic, "结束") ? false : true;
	strcpy(m_CrazyMagic, m_pStep->Magic);
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
	for (int i = 0; i < 3; i++) {
		printf("第%d次捡物\n", i);
		m_pGame->m_pItem->PickUpItem(m_pGame->m_pGameConf->m_stPickUp.PickUps, 
			m_pGame->m_pGameConf->m_stPickUp.Length);
		Sleep(1000);
	}
}

// 存物
void GameProc::CheckIn()
{
	HWND child, parent;

	m_pGame->FindButtonWnd(BUTTON_ID_ROLE, child, parent);
	SendMessageW(parent, WM_COMMAND, MAKEWPARAM(BUTTON_ID_ROLE, BN_CLICKED), (LPARAM)child);    // 点击人物按钮
	Sleep(1000);
	m_pGame->FindButtonWnd(BUTTON_ID_VIP, child, parent);
	SendMessageW(parent, WM_COMMAND, MAKEWPARAM(BUTTON_ID_VIP, BN_CLICKED), (LPARAM)child);     // 点击VIP按钮
	Sleep(1000);
	m_pGame->FindButtonWnd(BUTTON_ID_CHECKIN, child, parent);
    SendMessageW(parent, WM_COMMAND, MAKEWPARAM(BUTTON_ID_CHECKIN, BN_CLICKED), (LPARAM)child); // 点击物品仓库按钮
	if (!parent) {
		printf("打开仓库失败\n");
		return;
	}

	Sleep(1500);
	m_pGame->m_pItem->CheckInSelfItem(m_pGame->m_pGameConf->m_stCheckIn.CheckIns,
		m_pGame->m_pGameConf->m_stCheckIn.Length);

	m_pGame->FindButtonWnd(BUTTON_ID_CLOSECKIN, child, parent);
	SendMessageW(parent, WM_COMMAND, MAKEWPARAM(BUTTON_ID_CLOSECKIN, BN_CLICKED), (LPARAM)child); // 点击关闭仓库按钮
}

// 使用物品
void GameProc::UseItem()
{
	m_pGame->m_pItem->UseSelfItem(m_pStep->Name, m_pStep->X, m_pStep->Y);
}

// 售卖物品
void GameProc::SellItem()
{
	if (!m_stLastStepInfo.NPCId) {
		printf("没有找到NPC 无法卖出物品\n");
		return;
	}

	m_pGame->m_pItem->SellSelfItem(m_pGame->m_pGameConf->m_stSell.Sells,
		m_pGame->m_pGameConf->m_stSell.Length);

	HWND child, parent;
	m_pGame->FindButtonWnd(BUTTON_ID_CLOSESHOP, child, parent);
	SendMessageW(parent, WM_COMMAND, MAKEWPARAM(BUTTON_ID_CLOSESHOP, BN_CLICKED), (LPARAM)child); // 点击关闭商店按钮
}

// 等待
void GameProc::Wait()
{
	if (strlen(m_pStep->Magic)) { // 等待技能冷却
		GameMagic* pMagic = m_pGame->m_pMagic->ReadMagic(m_pStep->Magic);
		if (!pMagic)
			return;

		while (!m_pGame->m_pMagic->CheckCd(pMagic->Id, m_pStep->WaitMs)) {
			Sleep(100);
		}
		strcpy(m_stLastStepInfo.Magic, m_pStep->Magic);
	}
	else { // 等待
		Wait(m_pStep->WaitMs);
	}
}

// 等待
void GameProc::Wait(DWORD ms)
{
	if (ms < 1000) {
		printf("等待%d毫秒\n", ms);
		Sleep(ms);
		return;
	}

	int n = 0;
	__int64 now_time = getmillisecond();
	while (true) {
		int c = getmillisecond() - now_time;
		c = (int)ms - c;
		int ls = c / 1000;

		if (c >= 0 && n != ls) {
			printf("等待%d秒，还剩%d秒\n", ms / 1000, ls);
		}
		if (c <= 0) {
			break;
		}
		n = ls;
		Sleep(100);
	}
}

// 是否在副本
bool GameProc::IsInFB()
{
	ReadCoor();
	return (m_iCoorX >= 200 && m_iCoorX <= 930) && (m_iCoorY >= 30 && m_iCoorY <= 1120);
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


