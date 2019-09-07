#pragma once
#include "GameStruct.h"

#define MAX_MAGIC_COUNT 100

// ���ܵ����ĸ�CALL
enum MagicCallType{
	MCT_POS  = 0x00, // ���ô�����CALL
	MCT_GWID = 0x01, // ���ô�����IDCALL
};

typedef struct game_magic
{
	DWORD First;         // ��֪����ʲô
	DWORD Id;            // ����ID
	DWORD UnKnow;        // ��֪��
	CHAR* Name;          // ��������
	BYTE  UNknow2[0x4C]; // ��֪��Ϊʲô
	DWORD Cd;            // ��ȴʱ��
} GameMagic;

typedef struct _magic_
{
	DWORD Id;
	DWORD Cd;
	CHAR  Name[32];
} _Magic_;

typedef struct magic_info
{
	CHAR          Name[32];   // ��������
	DWORD         Id;         // ����Id
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
	bool UseMagic(const char* name, DWORD x, DWORD y, DWORD gwid=0);
	// �����ȴʱ��[allow_ms=�������ʣ����ȴ]
	bool CheckCd(const char* name, DWORD allow_ms=0, DWORD* magic_id=nullptr);
	// �����ȴʱ��[allow_ms=�������ʣ����ȴ]
	bool CheckCd(DWORD id, DWORD allow_ms=0);
	// �����ȴʱ��[allow_ms=�������ʣ����ȴ]
	bool CheckCd(MagicInfo* p, DWORD allow_ms = 0);
	// ��ȡ������Ϣ
	MagicInfo* GetMagicInfo(const char* name);
	// ��ȡ������Ϣ
	MagicInfo* GetMagicInfo(DWORD Id);
	// �����Ƿ�Ϊ���꼼��
	bool MagicIsPos(const char* name);
	// ��ȡ��������
	int ReadMagicLength(bool flag);
	// ��ȡ����
	bool ReadMagic(const char* name=nullptr, _Magic_* save = nullptr, bool flag=true);
private:
	// ��Ӽ�����Ϣ
	MagicInfo* AddMagicInfo(CHAR* name, DWORD id, DWORD cd);
public:
	// ��Ϸ��
	Game* m_pGame;
	// ��������
	DWORD m_dwMagicNum;
	// ������Ϣ�б�����
	DWORD m_dwMagicInfoCount;
	// ������Ϣ�б�
	MagicInfo m_ListMagicInfo[MAX_MAGIC_COUNT];
	// ���ڹ����Ĺ���Id
	DWORD m_dwAttackGWId;

	CHAR m_Test[0x1000];
};