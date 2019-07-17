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
	int flag; // 0-��Ļ���겻��Ҫת�� 1-������Ϸ������Ҫת��
};
// ��Ϸִ�в���
struct _step_
{
	STEP_CODE OpCode;   // ������

	DWORD     X;           // Ҫ������λ��X
	DWORD     Y;           // Ҫ������λ��Y
	DWORD     NPCId;       // Ҫ�Ի���NPCID 
	CHAR      NPCName[32]; // Ҫ�Ի���NPC����
	DWORD     SelectNo;    // �Ի�ѡ������ 0��ʼ
	MagicType Magic;       // ����
	DWORD     WaitMs;      // �ȴ����ٺ�����Ƿ�ȴ�������ȴ���ܿ����ж�������ȴ
	DWORD     OpCount;     // ��������
	__int64   ExecTime;    // ִ��ʱ��
	bool      Exec;        // ����ִ��
};

class Explode;
class GameStep
{
public:
	// ...
	GameStep();
	// ��ȡ����ִ�еĲ���
	_step_* Current();
	// �������ִ�в��� ������һ��
	_step_* CompleteExec();
	// ��õ�ǰ���������
	STEP_CODE CurrentCode();
	// �����һ���������
	STEP_CODE NextCode();
	// ��ȡ��ִ���˶��ʱ��
	int GetHasExecMs(_step_* step = nullptr);
	// ����ִ��״̬
	void SetExec(bool v, _step_* step=nullptr);
	// ����ִ�в�������
	void ResetStep(int index = 0);
	// ��ʼ������
	bool InitSteps();
private:
	// ת��ʵ��ָ��
	STEP_CODE TransFormOP(const char* data);
	// ת��ʵ������
	bool TransFormPos(const char* str, _step_& step);
	// ת��ʵ�ʼ���
	bool TransFormMagic(Explode& line, _step_& step);
	// ת��ʵ�ʼ���
	MagicType TransFormMagic(const char* str);
	// ת��Wait����
	bool    TransFormWait(Explode& line, _step_& step);
	// ��Ӳ���
	void AddStep(_step_& step);
public:
	// ��Ϸ��������
	Link<_step_*> m_Step;
	// ��Ϸ��������
	int m_iStepCount = 0;
	// ��ǰִ�в�������
	int m_iStepIndex = 0;
	// ��Ϸ���˲������
	_step_* m_Steps;
};