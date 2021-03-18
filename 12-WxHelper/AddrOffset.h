#pragma once

/**
 * 发送文本消息
 */
#define SEND_MSG_HOOK_ADDRESS 0x3A0CA0	//HOOK消息的内存地址偏移
#define SEND_MSG_BUFFER 0x5A8	//HOOK消息的内存地址偏移


 /*
  * 接收消息
  */
  //#define RECV_MSG_ADDRESS  0x66553C50
  //#define RECV_MSG_AT_BACK_OFFSET_ADDR  0x3CD5AB
  //
  //#define RECV_MSG_JUMP_BACK  
  //DWORD g_hookOffsetAddr = 0x3CD5A5;
  //DWORD g_jumBackOffsetAddr = 0x3CD5AB;


   /**
    * 发送文件
    */
#define SEND_FILE_PARAM 0x1550368
#define SEND_FILE_ADDR0 0x574180

#define SEND_FILE_ADDR1 0x5741C0
#define SEND_FILE_ADDR2 0x5741C0
#define SEND_FILE_ADDR3 0x66CB0
#define SEND_FILE_ADDR4 0x2B7600

#define BUFF_SIZE 0x350




    /*
    * socket消息类型
    */
#define MT_SEND_TEXTMSG	11036	//发送文本消息
#define MT_SEND_CHATROOM_ATMSG	11037	//发送群@消息
#define MT_SEND_FILEMSG	11041	//发送文件消息



#define MT_DATA_WXID_MSG	11029	//获取单个好友消息
#define MT_DATA_FRIENDS_MSG	11030	//获取好友列表消息
#define MT_DATA_CHATROOMS_MSG	11031	//获取群聊列表消息
#define MT_DATA_CHATROOM_MEMBERS_MSG	11032	//获取群成员消息
#define MT_DATA_PUBLICS_MSG	11033	//获取公众号消息

#define MT_RECV_TEXT_MSG	11046	//接收文本消息
#define MT_RECV_PICTURE_MSG	11047	//接收图片消息
#define MT_RECV_VOICE_MSG	11048	//接收视频消息
#define MT_RECV_FRIEND_MSG	11049	//接收申请好友消息
#define MT_RECV_CARD_MSG	11050	//接收名片消息
#define MT_RECV_VIDEO_MSG	11051	//接收视频消息
#define MT_RECV_EMOJI_MSG	11052	//接收表情消息
#define MT_RECV_LOCATION_MSG	11053	//接收位置消息
#define MT_RECV_LINK_MSG	11054	//接收链接消息
#define MT_RECV_FILE_MSG	11055	//接收文件消息
#define MT_RECV_MINIAPP_MSG	11056	//接收小程序消息
#define MT_RECV_WCPAY_MSG	11057	//接收好友转账消息
#define MT_RECV_SYSTEM_MSG	11058	//接收系统消息
#define MT_RECV_REVOKE_MSG	11059	//接收撤回消息
#define MT_RECV_OTHER_MSG	11060	//接收其他未知消息


typedef enum _EnRawMsgType
{
    WX_MSG_TEXT = 1,       // 0x1     文本
    WX_MSG_PICTURE = 3,    // 0x3     图片
    WX_MSG_VOICE = 34,     // 0x22    语音
    WX_MSG_FRIEND = 37,    // 0x25    加好友请求 "from_wxid" : "fmessage"
    WX_MSG_CARD = 42,      // 0x2A    名片
    WX_MSG_VIDEO = 43,     // 0x2B    视频
    WX_MSG_EMOJI = 47,     // 0x2F    表情
    WX_MSG_LOCATION = 48,  // 0x30    位置
    WX_MSG_APP = 49,       // 0x31    应用类型，子类型参考EnRawAppMsgType
    WX_MSG_SYSTEM = 10000, // 0x2710  系统消息
    WX_MSG_REVOKE = 10002, // 0x2712  撤回消息
} EnRawMsgType;

// XML XPATH路径 /msg/appmsg/type/text()
typedef enum _EnRawAppMsgType
{
    WX_APPMSG_LINK = 5,     // 链接（包含群邀请）
    WX_APPMSG_FILE = 6,     // 文件
    WX_APPMSG_MUTIL = 19,   // 合并消息
    WX_APPMSG_MINIAPP = 33, // 小程序
    WX_APPMSG_WCPAY = 2000, // 转帐
} EnRawAppMsgType;