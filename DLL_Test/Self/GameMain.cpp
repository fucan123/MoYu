#include "GameMain.h"
#include <stdio.h>
#include <time.h>

// �ر�
#define OP_CLOSE        0x00
// ��Ч�Ķ˿�
#define OP_ACTIVE_PORT  0x01
// ��������
#define OP_SEND_DATA    0x02
// �����ӵ�
#define OP_SEND_FIRE    0x03
// ����������
#define OP_SEND_FISH    0x04

// ���÷����DLL·��
#define OP_SETDSDLLPATH 0x10