#pragma once
//#include <minwindef.h>

static DWORD  g_WinBaseDddress = 0;
static HANDLE g_hProcess = 0;

/**
 * ������Ϣ
 */
#define USERINFO_WXID 0x18A3584	
#define USERINFO_NICKNAME 0x18A35FC	
#define USERINFO_BIG_PIC 0x18A38C4	


 /**
  * ��ȡ��ϵ��
  */
#define CONTACT_ADDR  0x1886B38
#define CONTACT_OFFSET1  0x28
#define CONTACT_OFFSET2  0x8C


  /**
   * ��ȡ��ϵ��
   */
#define CHATROOM_ADDR  0x1886B38
#define CHATROOM_OFFSET1  0x840
#define CHATROOM_OFFSET2  0x88


   ////΢��ID�Ľṹ��
   //struct UserInfoStructRPC
   //{
   //    TCHAR wxid[50];
   //    TCHAR nickname[100];
   //    TCHAR avatar[200];
   //};





   /*
	* ��Ϣ
	*/
#define MT_DATA_OWNER_MSG 11028	//��ȡ�ҵ���Ϣ��Ϣ
#define MT_DATA_FRIENDS_MSG	11030	//��ȡ�����б���Ϣ
#define MT_DATA_CHATROOMS_MSG	11031	//��ȡȺ����Ϣ
#define MT_SEND_TEXTMSG	11036	//�����ı���Ϣ
#define MT_SEND_CHATROOM_ATMSG	11037	//����Ⱥ@��Ϣ
#define MT_SEND_FILEMSG	11041	//�����ļ���Ϣ

#define MT_UPLOAD_DLL 10023	//ж��dll