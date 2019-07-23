#pragma once
#include "GameStruct.h"

#define MAX_MAGIC_COUNT 0x10

// ���ܵ����ĸ�CALL
enum MagicCallType{
	MCT_POS  = 0x00, // ���ô�����CALL
	MCT_GWID = 0x01, // ���ô�����IDCALL
};
typedef struct magic_info
{
	MagicType     Type;       // ����
	MagicCallType CallType;   // CALL���� 0-����
	DWORD         Cd;         // ������ȴʱ��[����]
	__int64       UseTime;    // ʹ��ʱ��[����]
	__int64       CdTime;     // ��ȴʱ��
	
} MagicInfo;

class Game;

class Magic
{
public:
	// ...
	Magic(Game* p);
	// ʹ�ü���
	bool UseMagic(MagicType type, DWORD v, DWORD v2);
	// �����ȴʱ��[allow_ms=�������ʣ����ȴ]
	bool CheckCd(MagicType type, DWORD allow_ms=0);
	// ��ȡ������Ϣ
	MagicInfo* GetMagicInfo(MagicType type);
private:
	// ��Ӽ�����Ϣ
	void AddMagicInfo(MagicType type, MagicCallType call_type, DWORD cd);
public:
	// ��Ϸ��
	Game* m_pGame;
	// ������Ϣ�б�����
	DWORD m_dwMagicInfoCount;
	// ������Ϣ�б�
	MagicInfo m_ListMagicInfo[MAX_MAGIC_COUNT];
	// ���ڹ����Ĺ���Id
	DWORD m_dwAttackGWId;
};