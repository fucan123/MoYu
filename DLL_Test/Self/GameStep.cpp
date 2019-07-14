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
			case OP_CLICK:
			case OP_SELECT:
				step.SelectNo = explode.GetValue2Int(3);
			case OP_MOUMOVE:
				INLOGVARP(log, "%s", data);
				if (!TransFormPos(explode, step)) {
					continue;
				}
				break;
			case OP_KEYDOWN:
				TransFormKey(explode, step);
				INLOGVARP(log, "按键：0x%02x", step.Keys[0]&0xff);
				break;
			case OP_WAIT:
				step.WaitMs = explode.GetValue2Int(1) * 1000;
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
	m_Steps = new _step_[MAX_STEP];

#if 0
	AddMove(267, 358);  // 移动
	AddClick(267, 356);
	//AddClick(700, 360, 1, 0); // 点击[柱子]
	AddSelect(425, 355, 1);        // 选择第一个选项[关闭魔法障壁]
	AddMove(272, 362);
	AddMove(267, 358);
	AddMouMove(272, 362);

	AddMove(906, 1060);
	AddClick(995, 260, 1, 0);
	AddSelect(415, 390, 1);
	AddMove(903, 1017);
	AddKey(VK_F4);
	AddMove(828, 1000);
	AddKey(VK_F3);
	AddMove(797, 940);
#endif
}

// 转成实际指令
STEP_CODE GameStep::TransFormOP(const char* data)
{
	// 以下为建筑类型
	if (strcmp(data, "移动") == 0)
		return OP_MOVE;
	if (strcmp(data, "点击") == 0)
		return OP_CLICK;
	if (strcmp(data, "选择") == 0)
		return OP_SELECT;
	if (strcmp(data, "鼠标") == 0)
		return OP_MOUMOVE;
	if (strcmp(data, "鼠标移动") == 0)
		return OP_MOUMOVE;
	if (strcmp(data, "按键") == 0)
		return OP_KEYDOWN;
	if (strcmp(data, "等待") == 0)
		return OP_WAIT;
}

// 转成实际坐标
bool GameStep::TransFormPos(Explode& line, _step_& step)
{
	Explode explode(",", line.GetValue(1));
	if (explode.GetCount() == 2) {
		step.Pos.x = explode.GetValue2Int(0);
		step.Pos.y = explode.GetValue2Int(1);
	}

	if (step.OpCode != OP_MOVE) {
		if (line.GetCount() >= 3) {
			Explode extra(",", line.GetValue(2));
			if (extra.GetCount() == 2) {
				step.Pos.flag = extra.GetValue2Int(1);
				step.ClickNum = extra.GetValue2Int(0);
			}
			//INLOGVARN(32, "FLAG:%d", step.Pos.flag);
		}
	}

	return explode.GetCount() == 2;
}

// 转成实际按键
bool GameStep::TransFormKey(Explode& line, _step_ & step)
{
	int size = sizeof(step.Keys) / sizeof(u_char);
	Explode explode(",", line.GetValue(1));
	size = explode.GetCount() < size ? explode.GetCount() : size;
	for (int i = 0; i < size; i++) {
		step.Keys[i] = TransFormKey(explode.GetValue(i));
	}
	return true;
}

// 转成虚拟按键
u_char GameStep::TransFormKey(const char* data)
{
	if (!data)
		return 0;

	if (strlen(data) == 1) {
		if (*data >= '0' && *data <= '9')
			return *data;
		if (*data >= 'A' && *data <= 'Z')
			return *data;
		if (*data >= 'a' && *data <= 'z')
			return *data - 32;
	}
	
	if (strcmp(data, "F1") == 0)
		return VK_F1;
	if (strcmp(data, "F2") == 0)
		return VK_F2;
	if (strcmp(data, "F3") == 0)
		return VK_F3;
	if (strcmp(data, "F4") == 0)
		return VK_F4;
	if (strcmp(data, "F5") == 0)
		return VK_F5;
	if (strcmp(data, "F6") == 0)
		return VK_F6;
	if (strcmp(data, "F7") == 0)
		return VK_F7;
	if (strcmp(data, "F8") == 0)
		return VK_F8;

	return 0;
}

// 添加移动步骤
void GameStep::AddMove(int x, int y, int flag)
{
	AddMove(x, y, nullptr, 0, flag);
}

// 添加移动步骤
void GameStep::AddMove(int x, int y, u_char* keys, int length, int flag)
{
	InitStep(step);
	step.OpCode = OP_MOVE;
	step.Pos.x = x;
	step.Pos.y = y;
	step.Pos.flag = flag;
	if (keys && length) {
		memcpy(step.Keys, keys, length * sizeof(u_char));
	}
	AddStep(step);
}

// 添加点击步骤
void GameStep::AddClick(int x, int y, int num, int flag)
{
	InitStep(step);
	step.OpCode = OP_CLICK;
	step.Pos.x = x;
	step.Pos.y = y;
	step.Pos.flag = flag;
	step.ClickNum = num;
	AddStep(step);
}

// 添加选择步骤
void GameStep::AddSelect(int x, int y, int no)
{
	InitStep(step);
	step.OpCode = OP_SELECT;
	step.Pos.x = x;
	step.Pos.y = y;
	step.Pos.flag = 0;
	step.SelectNo = no;
	AddStep(step);
}

// 添加鼠标移动步骤
void GameStep::AddMouMove(int x, int y)
{
	InitStep(step);
	step.OpCode = OP_MOUMOVE;
	step.Pos.x = x;
	step.Pos.y = y;
	AddStep(step);
}

// 添加按键步骤
void GameStep::AddKey(u_char key)
{
	InitStep(step);
	step.OpCode = OP_KEYDOWN;
	step.Keys[0] = key;
	AddStep(step);
}

// 添加按键步骤
void GameStep::AddKey(u_char * keys, int length)
{
	InitStep(step);
	step.OpCode = OP_KEYDOWN;
	memcpy(step.Keys, keys, length * sizeof(u_char));
	AddStep(step);
}

// 添加等待步骤
void GameStep::AddWait(u_int ms)
{
	InitStep(step);
	step.OpCode = OP_WAIT;
	step.WaitMs = ms;
	AddStep(step);
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