#include "Move.h"
#include "Game.h"
#include <stdio.h>
#include <My/Common/mystring.h>

// ...
Move::Move(Game * p)
{
	m_pGame = p;
	ClearMove();
}

// 移动
void Move::Run(DWORD x, DWORD y)
{
	SetMove(x, y);
	Game::Call_Run(x, y);
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
	m_i64MvTime = 0;
}

// 是否达到终点
bool Move::IsMoveEnd()
{
	m_pGame->ReadCoor(&m_dwX, &m_dwY); // 读取当前坐标
	return m_dwX == m_dwMvX && m_dwY == m_dwMvY;
}

// 是否移动
bool Move::IsMove()
{
	if (m_pGame->m_GameAddr.MovSta) { // 此地址保存移动状态 0-未移动 1-在移动
		try {
			return PtrToDword(m_pGame->m_GameAddr.MovSta) == 1;
		}
		catch (...) {
			printf("Move::IsMove函数|m_pGame->m_GameAddr.MovSta地址错误\n");
		}
	}

	if (!m_dwLastX || !m_dwLastY)
		return false;

	__int64 ms = getmillisecond();
	if (ms < (m_i64MvTime + 250)) // 小于250豪秒 不计算
		return true;

	m_pGame->ReadCoor(&m_dwX, &m_dwY); // 读取当前坐标
	if (m_dwLastX == m_dwX && m_dwLastY == m_dwY) // 没有移动 1秒内坐标没有任何变化
		return false;

	m_dwLastX = m_dwX;
	m_dwLastY = m_dwY;
	m_i64MvTime = getmillisecond();
	return true;
}

