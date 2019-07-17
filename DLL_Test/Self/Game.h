#pragma once
#include "GameStruct.h"

class GameProc;
class Item;
class Talk;
class Move;
class GuaiWu;
class Magic;

class Game
{
public:
	// ...
	Game();
	// >>>
	~Game();
	// 初始化
	bool Init();
	// 运行
	void Run();
	// 停止
	void Stop(bool v = true);
	// 获得游戏窗口
	HWND FindGameWnd();
	// 获取所有模块地址
	void FindAllModAddr();
	// 获取游戏所有CALL
	void FindAllCall();
	// 获取移动CALL
	DWORD FindRunCall();
	// 获取模块地址
	DWORD FindModAddr(LPCWSTR name);
	// 读取坐标
	bool ReadCoor(DWORD* x=NULL, DWORD* y=NULL);
	// 读取生命值
	bool ReadLife(int& life, int& life_max);
	// 读取药包数量
	bool ReadQuickKey2Num(int* nums, int length);
	// 读取包包物品
	bool ReadBag(DWORD* bag, int length);
	// 人物是否在移动
	bool IsMove();
	// 攻击怪物
	void AttackGuaiWu();
	// 获取坐标地址
	bool FindCoorAddr();
	// 获取移动状态地址
	bool FindMoveStaAddr();
	// 获取对话框状态地址
	bool FinDTalkBoxStaAddr();
	// 获取生命地址
	bool FindLifeAddr();
	// 获取快捷键上面物品数量地址
	bool FindQuickKeyAddr();
	// 获取背包代码
	bool FindBagAddr();
	// 获得地面物品地址的保存地址
	bool FindItemPtr();
	// 获取NPC二级对话ESI寄存器数值
	bool FindCallNPCTalkEsi();
	// 在某个模块里面搜索
	DWORD SearchInMod(LPCTSTR name, DWORD* codes, DWORD length, DWORD* save, DWORD save_length = 1, DWORD step = 1);
	// 搜索特征码
	DWORD SearchCode(DWORD* codes, DWORD length, DWORD* save, DWORD save_length=1, DWORD step=4);
	// 读取游戏内存
	bool ReadGameMemory(DWORD flag=0x01);
	// 打印日记
	void InsertLog(char* text);
	// 打印日记
	void InsertLog(wchar_t* text);
public:
	// 游戏窗口
	HWND  m_hWnd = NULL;
	// 游戏进程ID
	DWORD m_dwPid = 0;
	// 游戏权限句柄
	HANDLE   m_hGameProcess = NULL;
	// 游戏模块地址
	static GameModAddr m_GameModAddr;
	// 游戏地址
	static GameAddr m_GameAddr;
	// 游戏CALL
	static GameCall m_GameCall;
	// 游戏窗口信息
	static GameWnd m_GameWnd;

	// 当前X坐标
	DWORD m_dwX;
	// 当前Y坐标
	DWORD m_dwY;

	// 怪物数量
	DWORD        m_dwGuaiWuCount;
	// 怪物指针[数组 会new]
	GameGuaiWu** m_pGuaiWus;

	// 是否读取完毕
	bool  m_bIsReadEnd;
	// 读取内存块的大小
	DWORD m_dwReadSize;
	// 读取基地址
	DWORD m_dwReadBase;
	// 读取内容临时内存
	BYTE* m_pReadBuffer;

	// 游戏过程处理类
	GameProc* m_pGameProc;
	// 物品类
	Item*     m_pItem;
	// 对话类
	Talk*     m_pTalk;
	// 移动类
	Move*     m_pMove;
	// 怪物类
	GuaiWu*   m_pGuaiWu;
	// 技能类
	Magic*    m_pMagic;

	// 自身
	static Game* self;
public:
	// 正在执行的CALL
	static CallStep m_CallStep;
	// 是否正在执行CALL
	bool static IsCalling();
	// CALL是否已完成
	bool static CallIsComplete();
	// 设置CALL STEP数据
	void static SetCallStep(CallStepType type, DWORD v1, DWORD v2=0);
	// 清除CALL STEP数据
	void static ClearCallStep();
	// 人物移动函数
	void static Call_Run(int x, int y);
	// 喊话CALL
	void static Call_Talk(const char* msg, int type=0);
	// NPC对话
	void static Call_NPC(int npc_id);
	// NPC二级对话
	void static Call_NPCTalk(int no);
	// 使用物品
	void static Call_UseItem(int item_id);
	// 扔物品
	void static Call_DropItem(int item_id);
	// 捡物品
	void static Call_PickUpItem(GameGroundItem* p);
	// 放技能
	void static Call_Magic(int magic_id, int guaiwu_id);
	// 放技能
	void static Call_Magic(int magic_id, int x, int y);
};