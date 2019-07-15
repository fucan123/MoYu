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

// ��ȡ��ǰ��ִ�еĲ���
_step_* GameStep::Current()
{
	NODE<_step_*>* node = m_Step.Current();
	//INLOGVARN(32, "Current Index:%d", m_iStepIndex);
	return node ? node->value : nullptr;
}

// �������ִ�в��� ������һ��
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

// ��õ�ǰ���������
STEP_CODE GameStep::CurrentCode()
{
	_step_* pStep = Current();
	return pStep ? pStep->OpCode : OP_UNKNOW;
}

// �����һ���������
STEP_CODE GameStep::NextCode()
{
	m_Step.Store();
	_step_* pStep = Current();
	m_Step.ReStore();

	return pStep ? pStep->OpCode : OP_UNKNOW;
}

// ��ȡ��ִ���˶��ʱ��
int GameStep::GetHasExecMs(_step_* step)
{
	if (!step)
		step = Current();

	return step ? getmillisecond() - step->ExecTime : 0;
}

// ����ִ��״̬
void GameStep::SetExec(bool v, _step_* step)
{
	if (!step)
		step = Current();
	if (step) {
		step->Exec = v;
		step->ExecTime = v ? getmillisecond() : 0;
	}	
}

// ��ʼ������
bool GameStep::InitSteps()
{
	OpenTextFile file;
	if (!file.Open("����.txt")) {
		INLOG("�Ҳ���[����.txt]�ļ�������");
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
		//memcpy(p->cmd, data, length + 1); // ����ԭ�ַ�����
	}
	//printf("����������%d\n", m_Link.Count());
	ResetStep(index);
	return true;
}

// ת��ʵ��ָ��
STEP_CODE GameStep::TransFormOP(const char* data)
{
	// ����Ϊ��������
	if (strcmp(data, "�ƶ�") == 0)
		return OP_MOVE;
	if (strcmp(data, "NPC") == 0)
		return OP_NPC;
	if (strcmp(data, "ѡ��") == 0)
		return OP_SELECT;
	if (strcmp(data, "ѡ��") == 0)
		return OP_SELECT;
	if (strcmp(data, "����") == 0)
		return OP_MAGIC;
	if (strcmp(data, "��˦") == 0)
		return OP_CRAZY;
	if (strcmp(data, "���") == 0)
		return OP_CLEAR;
	if (strcmp(data, "�ȴ�") == 0)
		return OP_WAIT;
}

// ת��ʵ������
bool GameStep::TransFormPos(const char* str, _step_& step)
{
	Explode explode(",", str);
	if (explode.GetCount() == 2) {
		step.X = explode.GetValue2Int(0);
		step.Y = explode.GetValue2Int(1);
	}

	return explode.GetCount() == 2;
}

// ת��ʵ�ʼ���
bool GameStep::TransFormMagic(Explode& line, _step_ & step)
{
	int index = 2;
	step.Magic = TransFormMagic(line[1]);
	if (strstr(line[2], ",")) { // ����������
		TransFormPos(line[2], step);
		index++;
	}
	step.WaitMs = line.GetValue2Int(index) * 1000;
}

// ת��ʵ�ʼ���
MagicType GameStep::TransFormMagic(const char* str)
{
	if (strcmp(str, "����") == 0)
		return ����;
	if (strcmp(str, "Ӱ����Լ") == 0)
		return Ӱ����Լ;
	if (strcmp(str, "����þ�") == 0)
		return ����þ�;
	if (strcmp(str, "���޿ռ�") == 0)
		return ���޿ռ�;
	if (strcmp(str, "����") == 0)
		return ����;
	if (strcmp(str, "�����") == 0)
		return �����;

	return δ֪����;
}

// ת��Wait����
bool GameStep::TransFormWait(Explode& line, _step_& step)
{
	int index = 1;
	MagicType magic = TransFormMagic(line[1]);
	if (magic) { // �ȴ�������ȴ
		step.Magic = magic;
		index++;
	}
	step.WaitMs= line.GetValue2Int(index) * 1000;

	return 0;
}

// ��Ӳ���
void GameStep::AddStep(_step_& step)
{
	//memset(&m_Steps[m_iStepIndex], 0, sizeof(_step_));
	memcpy(&m_Steps[m_iStepIndex], &step, sizeof(_step_));
	//INLOGVARN(32, "��Ӳ���:%d Exec:%d", step.OpCode, step);
	m_iStepCount = ++m_iStepIndex;
}

// ����ִ�в�������
void GameStep::ResetStep(int index)
{
	m_Step.Reset(index);
}