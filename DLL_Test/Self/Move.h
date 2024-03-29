#pragma once
#include "GameStruct.h"

class Game;

class Move
{
public:
	// ...
	Move(Game* p);

	// 初始化数据
	void InitData();
	// 移动
	bool Run(DWORD x, DWORD y);
	// 设置移动位置
	void SetMove(DWORD x, DWORD y);
	// 清除移动数据
	void ClearMove();
	// 是否达到终点
	bool IsMoveEnd();
	// 是否移动
	bool IsMove();
public:
	// 游戏类
	Game* m_pGame;
	// 当前位置X
	DWORD m_dwX;
	// 当前位置Y
	DWORD m_dwY;
	// 上次位置X
	DWORD m_dwLastX;
	// 上次位置Y
	DWORD m_dwLastY;
	// 移动位置X
	DWORD m_dwMvX;
	// 移动位置Y
	DWORD m_dwMvY;
	// 判断移动结束时间
	__int64 m_i64IsEndTime;
	// 判断是否移动时间
	__int64 m_i64IsMvTime;
	// 移动时间
	__int64 m_i64MvTime;

	CHAR m_Test[0x1000];
};