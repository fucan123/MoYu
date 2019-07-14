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
	int flag; // 0-��Ļ���겻��Ҫת�� 1-������Ϸ������Ҫת��
};
// ��Ϸִ�в���
struct _step_
{
	STEP_CODE OpCode;   // ������
	Point     Pos;      // ����λ��
	int       ClickNum; // �������
	int       SelectNo; // ѡ����
	int       WaitMs;   // �ȴ�����
	u_char    Keys[16]; // ������Щ��
	__int64   ExecTime; // ִ��ʱ��
	bool      Exec;     // ����ִ��


	DWORD     MvX;        // Ҫ�ƶ���λ��X
	DWORD     MvY;        // Ҫ�ƶ���λ��Y
	DWORD     NPCId;      // Ҫ�Ի���NPCID 
	DWORD     TalkIndex;  // �Ի�ѡ������ 0��ʼ
	DWORD     TalkCount;  // �Ի�����
	MagicType Magic;      // ����
	DWORD     WaitMagic;  // �Ƿ�ȴ�������ȴ
	DWORD     WaitSecond; // �ȴ���������ܿ����ж�������ȴ
	DWORD     WaitType;   // 0-WaitSecondΪ�ȴ������� ����ֵΪ����[�ȴ�������ȴ,WaitSecondΪ��������ж�������ȴ]
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
	bool TransFormPos(Explode& line, _step_& step);
	// ת��ʵ�ʰ���
	bool TransFormKey(Explode& line, _step_& step);
	// ת�����ⰴ��
	u_char    TransFormKey(const char* data);
	// ����ƶ�����
	void AddMove(int x, int y, int flag=1);
	// ����ƶ�����
	void AddMove(int x, int y, u_char* keys, int length, int flag = 1);
	// ��ӵ������
	void AddClick(int x, int y, int num=1, int flag = 1);
	// ���ѡ����
	void AddSelect(int x, int y, int no);
	// �������ƶ�����
	void AddMouMove(int x, int y);
	// ��Ӱ�������
	void AddKey(u_char key);
	// ��Ӱ�������
	void AddKey(u_char* keys, int length);
	// ��ӵȴ�����
	void AddWait(u_int ms);
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