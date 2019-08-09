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

// ��ʼ������
void Move::InitData()
{
	ClearMove();
}

// �ƶ�
bool Move::Run(DWORD x, DWORD y)
{
	if (x == m_dwMvX && y == m_dwMvY) {
		if ((getmillisecond() - m_i64MvTime) < 1200)
			return false;
	}

	printf("Move::Run:%d,%d �Լ�λ�ã�%d,%d\n", x, y, m_dwX, m_dwY);
	SetMove(x, y);
	Game::Call_Run(x, y);

	return true;
}

// �����ƶ�λ��
void Move::SetMove(DWORD x, DWORD y)
{
	m_pGame->ReadCoor(&m_dwLastX, &m_dwLastY); // ��ȡ��ǰ����

	m_dwMvX = x;
	m_dwMvY = y;
	m_i64MvTime = getmillisecond();
}

// ����ƶ�����
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

// �Ƿ�ﵽ�յ�
bool Move::IsMoveEnd()
{
	__int64 ms = getmillisecond();
	if (ms < (m_i64IsEndTime + 500)) // С��500���� ���ж�
		return false;

	m_i64IsEndTime = ms;
	m_pGame->ReadCoor(&m_dwX, &m_dwY); // ��ȡ��ǰ����
	return m_dwX == m_dwMvX && m_dwY == m_dwMvY;
}

// �Ƿ��ƶ�
bool Move::IsMove()
{
	__int64 ms = getmillisecond();
	if (ms < (m_i64IsMvTime + 800)) // С��500���� ���ж�
		return true;

	m_i64IsMvTime = ms;

	if (m_pGame->m_GameAddr.MovSta) { // �˵�ַ�����ƶ�״̬ 0-δ�ƶ� 1-���ƶ�
		return PtrToDword(m_pGame->m_GameAddr.MovSta) == 1;
	}

	if (!m_dwLastX || !m_dwLastY)
		return false;

	if (ms < (m_i64MvTime + 2500)) // С��2500���� ������
		return true;

	m_pGame->ReadCoor(&m_dwX, &m_dwY); // ��ȡ��ǰ����
	if (m_dwLastX == m_dwX && m_dwLastY == m_dwY) // û���ƶ� 1��������û���κα仯
		return false;

	m_dwLastX = m_dwX;
	m_dwLastY = m_dwY;
	m_i64MvTime = getmillisecond();
	return true;
}

