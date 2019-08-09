#include "Move.h"
#include "Game.h"
#include <stdio.h>
#include <My/Common/mystring.h>

// ...
Move::Move(Game * p)
{
	m_pGame = p;
	InitData();
}

// 初始化数据
void Move::InitData()
{
	ClearMove();
}

// 移动
bool Move::Run(DWORD x, DWORD y)
{
	if (x == m_dwMvX && y == m_dwMvY) {
		if ((getmillisecond() - m_i64MvTime) < 1200)
			return false;
	}

	printf("Move::Run:%d,%d 自己位置：%d,%d\n", x, y, m_dwX, m_dwY);
	SetMove(x, y);
	Game::Call_Run(x, y);

	return true;
}

// 设置移动位置
void Move::SetMove(DWORD x, DWORD y)
{
	m_pGame->ReadCoor(&m_dwLastX, &m_dwLastY); // 读取当前坐标

	m_dwMvX = x;
	m_dwMvY = y;
	m_i64MvTime = getmillisecond();
}

// 清除移动数据
void Move::ClearMove()
{
	m_dwX = 0;
	m_dwY = 0;
	m_dwLastX = 0;
	m_dwLastY = 0;
	m_dwMvX = 0;
	m_dwMvY = 0;

	m_i64IsEndTime = 0;
	m_i64IsMvTime = 0;
	m_i64MvTime = 0;
}

// 是否达到终点
bool Move::IsMoveEnd()
{
	__int64 ms = getmillisecond();
	if (ms < (m_i64IsEndTime + 500)) // 小于500豪秒 不判断
		return false;

	m_i64IsEndTime = ms;
	m_pGame->ReadCoor(&m_dwX, &m_dwY); // 读取当前坐标
	return m_dwX == m_dwMvX && m_dwY == m_dwMvY;
}

// 是否移动
bool Move::IsMove()
{
	__int64 ms = getmillisecond();
	if (ms < (m_i64IsMvTime + 800)) // 小于500豪秒 不判断
		return true;

	m_i64IsMvTime = ms;

	if (m_pGame->m_GameAddr.MovSta) { // 此地址保存移动状态 0-未移动 1-在移动
		return PtrToDword(m_pGame->m_GameAddr.MovSta) == 1;
	}

	if (!m_dwLastX || !m_dwLastY)
		return false;

	if (ms < (m_i64MvTime + 2500)) // 小于2500豪秒 不计算
		return true;

	m_pGame->ReadCoor(&m_dwX, &m_dwY); // 读取当前坐标
	if (m_dwLastX == m_dwX && m_dwLastY == m_dwY) // 没有移动 1秒内坐标没有任何变化
		return false;

	m_dwLastX = m_dwX;
	m_dwLastY = m_dwY;
	m_i64MvTime = getmillisecond();
	return true;
}

