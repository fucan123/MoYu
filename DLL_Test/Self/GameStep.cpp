#include "GameStep.h"
#include "Game.h"
#include <ShlObj_core.h>
#include <stdio.h>
#include <memory.h>
#include <My/Common/mystring.h>
#include <My/Common/OpenTextFile.h>
#include <My/Common/Explode.h>

#define ZeroStep(p) memset(p, 0, sizeof(_step_))
#define InitStep(n) _step_ n; ZeroStep(&n);

#define MAX_STEP 512

// ...
GameStep::GameStep()
{
	//InitSteps();
}

// ��ȡ��ǰ��ִ�еĲ���
_step_* GameStep::Current(vector<_step_*>& link)
{
	if (m_iStepIndex >= link.size())
		return nullptr;

	return link[m_iStepIndex];
}

// �������ִ�в��� ������һ��
_step_* GameStep::CompleteExec(vector<_step_*>& link)
{
	m_iStepIndex++;
	//INLOGVARN(32, "Index:%d", m_iStepIndex);
	return Current(link);
}

// ��õ�ǰ���������
STEP_CODE GameStep::CurrentCode(vector<_step_*>& link)
{
	_step_* pStep = Current(link);
	return pStep ? pStep->OpCode : OP_UNKNOW;
}

// �����һ���������
STEP_CODE GameStep::NextCode(vector<_step_*>& link)
{
	return OP_UNKNOW;
}

// ��ʼ������
bool GameStep::InitSteps(const char* file)
{
	char path[255];
	SHGetSpecialFolderPathA(0, path, CSIDL_DESKTOPDIRECTORY, 0);
	
	char fb_file[255];
	sprintf_s(fb_file, "%s\\MoYu\\%s", path, file);
	
	printf("ˢ���������ļ�:%s\n", fb_file);
	OpenTextFile hfile;
	if (!hfile.Open(fb_file)) {
		printf("�Ҳ���'%s'�ļ�������", fb_file);
		return false;
	}

	// ��ʼ�����ȥ��������
	InitGoLeiMingSteps();

	int i = 0, index = 0;
	int length = 0;
	char data[128];
	while ((length = hfile.GetLine(data, 128)) > -1) {
		//printf("length:%d\n", length);
		if (length == 0) {
			continue;
		}
		char log[64];

		int ret = ParseStep(trim(data), m_Step);
		if (ret == 0)
			break;
		if (ret > 0) {
			if (ret == 2) {
				index = i;
			}
			i++;
		}
	}
	printf("ִ������������%d\n", m_Step.size());
	ResetStep(index);

	hfile.Close();
	return true;
}

// ��ʼ��ȥ��������
int GameStep::InitGoLeiMingSteps()
{
	ParseStep("�ƶ� 66,60", m_GoLeiMingStep);
	ParseStep("NPC �޵¡���", m_GoLeiMingStep);
	ParseStep("�ȴ� 2", m_GoLeiMingStep);
	ParseStep("��ť 0x9F1", m_GoLeiMingStep);
	ParseStep("�ȴ� 10", m_GoLeiMingStep);
	ParseStep("NPC ������", m_GoLeiMingStep);
	ParseStep("ѡ�� 0x00", m_GoLeiMingStep);
	ParseStep("�ȴ� 10", m_GoLeiMingStep);
	printf("��ʼ�������͵�������½�������:%d\n", m_GoLeiMingStep.size());
	return m_GoLeiMingStep.size();
}

// ��������
int GameStep::ParseStep(const char* data, vector<_step_*>& link)
{
	Explode explode(" ", data);
	if (explode.GetCount() < 2)
		return -1;
	if (*data == '=')
		return 0;

	int result = 1;
	char* cmd = explode.GetValue(0);
	if (*cmd == '-') {
		result = 2;
		cmd++;
	}

	InitStep(step);
	try {
		step.OpCode = TransFormOP(cmd);
		switch (step.OpCode)
		{
		case OP_MOVE:
		case OP_MOVEFAR:
			if (!TransFormPos(explode[1], step)) {
				printf("GameStep::InitSteps.TransFormPosʧ��\n");
				result = -1;
			}
			break;
		case OP_NPC:
			strcpy(step.NPCName, explode[1]);
			break;
		case OP_SELECT:
			step.SelectNo = explode.GetValue2Int(1);
			step.OpCount = explode.GetValue2Int(2);
			if (step.OpCount == 0)
				step.OpCount = 1;
			break;
		case OP_MAGIC:
		case OP_MAGIC_PET:
			TransFormMagic(explode, step);
			break;
		case OP_CRAZY:
		case OP_CLEAR:
			strcpy(step.Magic, explode[1]);
			break;
		case OP_PICKUP:
		case OP_CHECKIN:
			break;
		case OP_USEITEM:
			strcpy(step.Name, explode[1]);     // ��Ʒ����
			if (explode.GetCount() > 2) {
				if (strstr(explode[2], ",")) { // �Ƿ���֤��������
					TransFormPos(explode[2], step);
				}
				if (explode.GetCount() > 3) {
					step.Extra[0] = explode.GetValue2Int(3);
				}
			}
			break;
		case OP_DROPITEM:
			strcpy(step.Name, explode[1]);     // ��Ʒ����
			if (explode.GetCount() > 2) {
				step.Extra[0] = explode.GetValue2Int(2);
			}
			break;
		case OP_SELL:
			break;
		case OP_BUTTON:
			step.ButtonId = explode.GetValue2Int(1);
			break;
		case OP_WAIT:
			TransFormWait(explode, step);
			break;
		default:
			result = -1;
			break;
		}
	}
	catch (...) {
		result = -1;
		printf("��ȡ���̴���:%s\n", data);
	}
	if (result) {
		_step_* pStep = new _step_;
		memcpy(pStep, &step, sizeof(_step_));
		link.push_back(pStep);
	}
	return result;
}

// ת��ʵ��ָ��
STEP_CODE GameStep::TransFormOP(const char* data)
{
	if (data == nullptr)
		return OP_UNKNOW;
	// ����Ϊ��������
	if (strcmp(data, "�ƶ�") == 0)
		return OP_MOVE;
	if (strcmp(data, "����") == 0)
		return OP_MOVEFAR;
	if (strcmp(data, "NPC") == 0)
		return OP_NPC;
	if (strcmp(data, "ѡ��") == 0)
		return OP_SELECT;
	if (strcmp(data, "ѡ��") == 0)
		return OP_SELECT;
	if (strcmp(data, "����") == 0)
		return OP_MAGIC;
	if (strcmp(data, "�輼") == 0)
		return OP_MAGIC_PET;
	if (strcmp(data, "��˦") == 0)
		return OP_CRAZY;
	if (strcmp(data, "���") == 0)
		return OP_CLEAR;
	if (strcmp(data, "����") == 0)
		return OP_PICKUP;
	if (strcmp(data, "����") == 0)
		return OP_CHECKIN;
	if (strcmp(data, "ʹ��") == 0)
		return OP_USEITEM;
	if (strcmp(data, "����") == 0)
		return OP_DROPITEM;
	if (strcmp(data, "����") == 0)
		return OP_SELL;
	if (strcmp(data, "����") == 0)
		return OP_SELL;
	if (strcmp(data, "��ť") == 0)
		return OP_BUTTON;
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
	//printf("TransFormMagic\n");
	strcpy(step.Magic, line[1]);
	if (line.GetCount() > 2) {
		int index = 2;
		if (strstr(line[2], ",")) { // ����������
			TransFormPos(line[2], step);
			index++;
		}
		step.WaitMs = line.GetValue2Int(index) * 1000;
	}
	//printf("TransFormMagic End\n");

	return true;
}

// ת��ʵ�ʼ���
MagicType GameStep::TransFormMagic(const char* str)
{
	if (str == nullptr)
		return δ֪����;
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
	if (strcmp(str, "��������") == 0)
		return ��������;

	return δ֪����;
}

// ת��Wait����
bool GameStep::TransFormWait(Explode& line, _step_& step)
{
	int index = 1;
	int test = line.GetValue2Int(1);
	if (test == 0) { // �ȴ�������ȴ
		strcpy(step.Magic, line[1]);
		index++;
	}
	int v = line.GetValue2Int(index);
	step.WaitMs = v < 150 ? v * 1000 : v;

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
	if (index > 0) {
		m_iStepStartIndex = index;
	}
	m_iStepIndex = index;
}