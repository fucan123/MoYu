#pragma once
#include "GameStruct.h"
#include <My/Common/Link.hpp>

typedef unsigned char      u_char;
typedef unsigned int       u_int;
typedef unsigned long long qword;

#define STEP_IS_MOVE(step) (step.OpCode==OP_MOVE)
#define STEP_IS_CLICK(step) (step.OpCode==OP_CLICK)
#define STEP_IS_SELECT(step) (step.OpCode==OP_SELECT)
#define STEP_IS_MOUMOVE(step) (step.OpCode==OP_MOUMOVE)
#define STEP_IS_WAIT(step) (step.OpCode==OP_KEY)
#define STEP_IS_WAIT(step) (step.OpCode==OP_WAIT)

enum STEP_CODE
{
	OP_UNKNOW  = 0,
	OP_MOVE    = 1,
	OP_CLICK   = 2,
	OP_SELECT  = 3,
	OP_MOUMOVE = 4,
	OP_KEYDOWN = 5,
	OP_WAIT    = 6,
};
struct Point
{
	int x;
	int y;
	int flag; // 0-屏幕坐标不需要转换 1-基于游戏坐标需要转换
};
// 游戏执行步骤
struct _step_
{
	STEP_CODE OpCode;   // 操作码
	Point     Pos;      // 操作位置
	int       ClickNum; // 点击次数
	int       SelectNo; // 选项编号
	int       WaitMs;   // 等待毫秒
	u_char    Keys[16]; // 按下哪些键
	__int64   ExecTime; // 执行时间
	bool      Exec;     // 已在执行


	DWORD     MvX;        // 要移动的位置X
	DWORD     MvY;        // 要移动的位置Y
	DWORD     NPCId;      // 要对话的NPCID 
	DWORD     TalkIndex;  // 对话选择索引 0开始
	DWORD     TalkCount;  // 对话次数
	MagicType Magic;      // 技能
	DWORD     WaitMagic;  // 是否等待技能冷却
	DWORD     WaitSecond; // 等待多少秒或技能可以有多少秒冷却
	DWORD     WaitType;   // 0-WaitSecond为等待多少秒 其他值为技能[等待技能冷却,WaitSecond为允许可以有多少秒冷却]
};

class Explode;
class GameStep
{
public:
	// ...
	GameStep();
	// 获取正在执行的步骤
	_step_* Current();
	// 完成正在执行步骤 返回下一个
	_step_* CompleteExec();
	// 获得当前步骤操作码
	STEP_CODE CurrentCode();
	// 获得下一步骤操作码
	STEP_CODE NextCode();
	// 获取已执行了多久时间
	int GetHasExecMs(_step_* step = nullptr);
	// 设置执行状态
	void SetExec(bool v, _step_* step=nullptr);
	// 重置执行步骤索引
	void ResetStep(int index = 0);
	// 初始化步骤
	bool InitSteps();
private:
	// 转成实际指令
	STEP_CODE TransFormOP(const char* data);
	// 转成实际坐标
	bool TransFormPos(Explode& line, _step_& step);
	// 转成实际按键
	bool TransFormKey(Explode& line, _step_& step);
	// 转成虚拟按键
	u_char    TransFormKey(const char* data);
	// 添加移动步骤
	void AddMove(int x, int y, int flag=1);
	// 添加移动步骤
	void AddMove(int x, int y, u_char* keys, int length, int flag = 1);
	// 添加点击步骤
	void AddClick(int x, int y, int num=1, int flag = 1);
	// 添加选择步骤
	void AddSelect(int x, int y, int no);
	// 添加鼠标移动步骤
	void AddMouMove(int x, int y);
	// 添加按键步骤
	void AddKey(u_char key);
	// 添加按键步骤
	void AddKey(u_char* keys, int length);
	// 添加等待步骤
	void AddWait(u_int ms);
	// 添加步骤
	void AddStep(_step_& step);
public:
	// 游戏步骤链表
	Link<_step_*> m_Step;
	// 游戏步骤数量
	int m_iStepCount = 0;
	// 当前执行步骤索引
	int m_iStepIndex = 0;
	// 游戏按此步骤进行
	_step_* m_Steps;
};