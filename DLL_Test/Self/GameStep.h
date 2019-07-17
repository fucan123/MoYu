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
	OP_NPC     = 2,
	OP_SELECT  = 3,
	OP_MAGIC   = 4,
	OP_CRAZY   = 5,
	OP_CLEAR   = 6,
	OP_WAIT    = 7,
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

	DWORD     X;           // 要操作的位置X
	DWORD     Y;           // 要操作的位置Y
	DWORD     NPCId;       // 要对话的NPCID 
	CHAR      NPCName[32]; // 要对话的NPC名称
	DWORD     SelectNo;    // 对话选择索引 0开始
	MagicType Magic;       // 技能
	DWORD     WaitMs;      // 等待多少毫秒或是否等待技能冷却或技能可以有多少秒冷却
	DWORD     OpCount;     // 操作次数
	__int64   ExecTime;    // 执行时间
	bool      Exec;        // 已在执行
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
	bool TransFormPos(const char* str, _step_& step);
	// 转成实际技能
	bool TransFormMagic(Explode& line, _step_& step);
	// 转成实际技能
	MagicType TransFormMagic(const char* str);
	// 转成Wait数据
	bool    TransFormWait(Explode& line, _step_& step);
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