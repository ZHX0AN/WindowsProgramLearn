#pragma once

/**
 * �����ı���Ϣ
 */
#define SEND_MSG_HOOK_ADDRESS 0x3A0C20	//HOOK��Ϣ���ڴ��ַƫ��
#define SEND_MSG_BUFFER 0x5A8	//HOOK��Ϣ���ڴ��ַƫ��

 /**
  * �����ļ�
  */
#define SEND_FILE_PARAM 0x1550368
#define SEND_FILE_ADDR0 0x574180

#define SEND_FILE_ADDR1 0x5741C0
#define SEND_FILE_ADDR2 0x5741C0
#define SEND_FILE_ADDR3 0x66CB0
#define SEND_FILE_ADDR4 0x2B7600

#define BUFF_SIZE 0x350



/*
* socket��Ϣ����
*/
#define MT_SEND_TEXTMSG 1	//�����ı���Ϣ
#define MT_SEND_FILEMSG	11041	//�����ļ���Ϣ




static DWORD g_WinBaseDddress = 0;