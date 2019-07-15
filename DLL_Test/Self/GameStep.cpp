#include "GameStep.h"
#include "Game.h"
#include <stdio.h>
#include <memory.h>
#include <My/Common/mystring.h>
#include <My/Common/OpenTextFile.h>
#include <My/Common/Explode.h>

#define ZeroStep(p) memset(p, 0, sizeof(_step_))
#define InitStep(n) _step_ n; ZeroStep(&n); n.Pos.flag=1;n.ClickNum=1; 

#define MAX_STEP 512


// ...
GameStep::GameStep()
{
	//InitSteps();
}

// 获取当前可执行的步骤
_step_* GameStep::Current()
{
	NODE<_step_*>* node = m_Step.Current();
	//INLOGVARN(32, "Current Index:%d", m_iStepIndex);
	return node ? node->value : nullptr;
}

// 完成正在执行步骤 返回下一个
_step_* GameStep::CompleteExec()
{
	_step_* pStep = Current();
	if (!pStep)
		return nullptr;

	pStep->Exec = false;
	m_Step.Next();
	//INLOGVARN(32, "Index:%d", m_iStepIndex);
	return Current();
}

// 获得当前步骤操作码
STEP_CODE GameStep::CurrentCode()
{
	_step_* pStep = Current();
	return pStep ? pStep->OpCode : OP_UNKNOW;
}

// 获得下一步骤操作码
STEP_CODE GameStep::NextCode()
{
	m_Step.Store();
	_step_* pStep = Current();
	m_Step.ReStore();

	return pStep ? pStep->OpCode : OP_UNKNOW;
}

// 获取已执行了多久时间
int GameStep::GetHasExecMs(_step_* step)
{
	if (!step)
		step = Current();

	return step ? getmillisecond() - step->ExecTime : 0;
}

// 设置执行状态
void GameStep::SetExec(bool v, _step_* step)
{
	if (!step)
		step = Current();
	if (step) {
		step->Exec = v;
		step->ExecTime = v ? getmillisecond() : 0;
	}	
}

// 初始化步骤
bool GameStep::InitSteps()
{
	OpenTextFile file;
	if (!file.Open("步骤.txt")) {
		INLOG("找不到[步骤.txt]文件！！！");
		return false;
	}

	int i = 0, index = 0;
	int length = 0;
	char data[128];
	while ((length = file.GetLine(data, 128)) > -1) {
		//printf("length:%d\n", length);
		if (length == 0) {
			continue;
		}
		char log[64];

		//Explode arr("#", data);
		//char* text = arr.GetValue(0);
		Explode explode(" ", trim(data));
		if (explode.GetCount() >= 2) {
			char* cmd = explode.GetValue(0);
			if (*cmd == '-') {
				index = i;
				cmd++;
			}
			if (*cmd == '=') {
				break;
			}
				
			InitStep(step);
			step.OpCode = TransFormOP(cmd);
			switch (step.OpCode)
			{
			case OP_MOVE:
				if (!TransFormPos(explode[1], step))
					continue;
				break;
			case OP_NPC:
				step.NPCId = explode.GetValue2Int(1);
				break;
			case OP_SELECT:
				step.SelectNo = explode.GetValue2Int(1);
				break;
			case OP_MAGIC:
				TransFormMagic(explode, step);
				break;
			case OP_CRAZY:
			case OP_CLEAR:
				step.Magic = TransFormMagic(explode[1]);
				break;
			case OP_WAIT:
				TransFormWait(explode, step);
				break;
			default:
				continue;
				break;
			}

			_step_* pStep = new _step_;
			memcpy(pStep, &step, sizeof(_step_));
			m_Step.Add(pStep);
			i++;
		}
		//memcpy(p->cmd, data, length + 1); // 保存原字符命令
	}
	//printf("流程数量：%d\n", m_Link.Count());
	ResetStep(index);
	return true;
}

// 转成实际指令
STEP_CODE GameStep::TransFormOP(const char* data)
{
	// 以下为建筑类型
	if (strcmp(data, "移动") == 0)
		return OP_MOVE;
	if (strcmp(data, "NPC") == 0)
		return OP_NPC;
	if (strcmp(data, "选择") == 0)
		return OP_SELECT;
	if (strcmp(data, "选项") == 0)
		return OP_SELECT;
	if (strcmp(data, "技能") == 0)
		return OP_MAGIC;
	if (strcmp(data, "狂甩") == 0)
		return OP_CRAZY;
	if (strcmp(data, "清怪") == 0)
		return OP_CLEAR;
	if (strcmp(data, "等待") == 0)
		return OP_WAIT;
}

// 转成实际坐标
bool GameStep::TransFormPos(const char* str, _step_& step)
{
	Explode explode(",", str);
	if (explode.GetCount() == 2) {
		step.X = explode.GetValue2Int(0);
		step.Y = explode.GetValue2Int(1);
	}

	return explode.GetCount() == 2;
}

// 转成实际技能
bool GameStep::TransFormMagic(Explode& line, _step_ & step)
{
	int index = 2;
	step.Magic = TransFormMagic(line[1]);
	if (strstr(line[2], ",")) { // 参数是坐标
		TransFormPos(line[2], step);
		index++;
	}
	step.WaitMs = line.GetValue2Int(index) * 1000;
}

// 转成实际技能
MagicType GameStep::TransFormMagic(const char* str)
{
	if (strcmp(str, "星陨") == 0)
		return 星陨;
	if (strcmp(str, "影魂契约") == 0)
		return 影魂契约;
	if (strcmp(str, "诸神裁决") == 0)
		return 诸神裁决;
	if (strcmp(str, "虚无空间") == 0)
		return 虚无空间;
	if (strcmp(str, "凤珠") == 0)
		return 凤珠;
	if (strcmp(str, "电击术") == 0)
		return 电击术;

	return 未知技能;
}

// 转成Wait数据
bool GameStep::TransFormWait(Explode& line, _step_& step)
{
	int index = 1;
	MagicType magic = TransFormMagic(line[1]);
	if (magic) { // 等待技能冷却
		step.Magic = magic;
		index++;
	}
	step.WaitMs= line.GetValue2Int(index) * 1000;

	return 0;
}

// 添加步骤
void GameStep::AddStep(_step_& step)
{
	//memset(&m_Steps[m_iStepIndex], 0, sizeof(_step_));
	memcpy(&m_Steps[m_iStepIndex], &step, sizeof(_step_));
	//INLOGVARN(32, "添加步骤:%d Exec:%d", step.OpCode, step);
	m_iStepCount = ++m_iStepIndex;
}

// 重置执行步骤索引
void GameStep::ResetStep(int index)
{
	m_Step.Reset(index);
}