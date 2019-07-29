#pragma once
#include "GameStruct.h"
#include <Windows.h>

#define ONE_COOR_PIX     50   // 一坐标占用像数
#define ONE_MAX_MOV_COOR 8    // 一次最大可移动8距离坐标
#define ONE_MOV_MAX_MS   1000 // 一次移动最大允许时间(毫秒)

#define BAG_NUM         40       // 包包容量
#define WUPIN_YAOBAO    0x0B5593 // 药包
#define WUPIN_XIANGLIAN 0x0B5B24 // 爱娜祈祷项链
#define WUPIN_YAO       0x0F6982 // 药
#define WUPIN_LINGYAO   0x0F943E // 速效圣兽灵药

#define RUNRUN 1
struct _step_;

class Game;
class GameStep;
class GameProc
{
public:
	// !!!
	GameProc(Game* pGame);
	// 运行
	void Run();
	// 步骤是否已执行完毕
	bool StepIsComplete();
	// 移动
	void Move();
	// NCP
	void NPC();
	// 选择
	void Select();
	// 技能
	void Magic();
	// 狂甩
	void Crazy();
	// 清怪
	void Clear();
	// 捡物
	void PickUp();
	// 等待
	void Wait();
	// 等待
	void Wait(DWORD ms);
	// 是否在副本
	bool IsInFB();
	// 读取人物坐标
	bool ReadCoor();
	// 读取人物血量
	bool ReadLife();
	// 读取快捷键上面物品数量
	bool ReadQuickKey2Num();
	// 读取包包物品
	bool ReadBag();
	// 是否需要加血量
	bool IsNeedAddLife();
	// 加血
	void AddLife();
	// 停止
	void Stop(bool v=true);
public:
	// 游戏指针
	Game*     m_pGame;
	// 游戏步骤指针
	GameStep* m_pGameStep;
	// 正在执行的步骤
	_step_* m_pStep;
	// 是否停止
	bool  m_bStop = false;
	// 是否暂停
	bool  m_bPause = false;
	// 是否重新开始
	bool  m_bReStart = false;
	// 是否狂甩
	bool m_bIsCrazy = false;
	// 狂甩技能
	MagicType m_CrazyMagic;
	// 上一次执行步骤的相关信息
	struct {
		DWORD     MvX;   // 移动X
		DWORD     MvY;   // 移动Y
		DWORD     NPCId; // NPCId
		MagicType Magic; // 技能
	} m_stLastStepInfo;
	// 人物坐标
	DWORD   m_iCoorX = 0;
	DWORD   m_iCoorY = 0;
	// 人物移动到的坐标
	int   m_iMovCoorX = 0;
	int   m_iMovCoorY = 0;
	// 人物血量
	int   m_iLife = 0;
	int   m_iLifeMax = 0;
	// 快捷键上物品数量
	int   m_QuickKey2Nums[2];
	// 包包物品
	DWORD m_dwBag[BAG_NUM];
	// 加血时间
	__int64 m_i64AddLifeTime = 0;
};
