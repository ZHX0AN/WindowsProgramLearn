#pragma once
//#include <minwindef.h>

static DWORD  g_WinBaseDddress = 0;
static HANDLE g_hProcess = 0;

/**
 * 个人信息
 */
#define USERINFO_WXID 0x18A3584	
#define USERINFO_NICKNAME 0x18A35FC	
#define USERINFO_BIG_PIC 0x18A38C4	


 /**
  * 获取联系人
  */
#define CONTACT_ADDR  0x1886B38
#define CONTACT_OFFSET1  0x28
#define CONTACT_OFFSET2  0x8C


  /**
   * 获取联系人
   */
#define CHATROOM_ADDR  0x1886B38
#define CHATROOM_OFFSET1  0x840
#define CHATROOM_OFFSET2  0x88


   ////微信ID的结构体
   //struct UserInfoStructRPC
   //{
   //    TCHAR wxid[50];
   //    TCHAR nickname[100];
   //    TCHAR avatar[200];
   //};





   /*
	* 消息
	*/
#define MT_DATA_OWNER_MSG 11028	//获取我的信息消息
#define MT_DATA_FRIENDS_MSG	11030	//获取好友列表消息
#define MT_DATA_CHATROOMS_MSG	11031	//获取群组信息
#define MT_SEND_TEXTMSG	11036	//发送文本消息
#define MT_SEND_CHATROOM_ATMSG	11037	//发送群@消息
#define MT_SEND_FILEMSG	11041	//发送文件消息

#define MT_UPLOAD_DLL 10023	//卸载dll