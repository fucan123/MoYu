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

// �ƶ�
void Move::Run(DWORD x, DWORD y)
{
	SetMove(x, y);
	Game::Call_Run(x, y);
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
	m_i64MvTime = 0;
}

// �Ƿ�ﵽ�յ�
bool Move::IsMoveEnd()
{
	m_pGame->ReadCoor(&m_dwX, &m_dwY); // ��ȡ��ǰ����
	return m_dwX == m_dwMvX && m_dwY == m_dwMvY;
}

// �Ƿ��ƶ�
bool Move::IsMove()
{
	if (m_pGame->m_GameAddr.MovSta) { // �˵�ַ�����ƶ�״̬ 0-δ�ƶ� 1-���ƶ�
		try {
			return PtrToDword(m_pGame->m_GameAddr.MovSta) == 1;
		}
		catch (...) {
			printf("Move::IsMove����|m_pGame->m_GameAddr.MovSta��ַ����\n");
		}
	}

	if (!m_dwLastX || !m_dwLastY)
		return false;

	__int64 ms = getmillisecond();
	if (ms < (m_i64MvTime + 250)) // С��250���� ������
		return true;

	m_pGame->ReadCoor(&m_dwX, &m_dwY); // ��ȡ��ǰ����
	if (m_dwLastX == m_dwX && m_dwLastY == m_dwY) // û���ƶ� 1��������û���κα仯
		return false;

	m_dwLastX = m_dwX;
	m_dwLastY = m_dwY;
	m_i64MvTime = getmillisecond();
	return true;
}

