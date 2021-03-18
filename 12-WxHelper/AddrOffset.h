#pragma once

/**
 * �����ı���Ϣ
 */
#define SEND_MSG_HOOK_ADDRESS 0x3A0CA0	//HOOK��Ϣ���ڴ��ַƫ��
#define SEND_MSG_BUFFER 0x5A8	//HOOK��Ϣ���ڴ��ַƫ��


 /*
  * ������Ϣ
  */
  //#define RECV_MSG_ADDRESS  0x66553C50
  //#define RECV_MSG_AT_BACK_OFFSET_ADDR  0x3CD5AB
  //
  //#define RECV_MSG_JUMP_BACK  
  //DWORD g_hookOffsetAddr = 0x3CD5A5;
  //DWORD g_jumBackOffsetAddr = 0x3CD5AB;


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
#define MT_SEND_TEXTMSG	11036	//�����ı���Ϣ
#define MT_SEND_CHATROOM_ATMSG	11037	//����Ⱥ@��Ϣ
#define MT_SEND_FILEMSG	11041	//�����ļ���Ϣ



#define MT_DATA_WXID_MSG	11029	//��ȡ����������Ϣ
#define MT_DATA_FRIENDS_MSG	11030	//��ȡ�����б���Ϣ
#define MT_DATA_CHATROOMS_MSG	11031	//��ȡȺ���б���Ϣ
#define MT_DATA_CHATROOM_MEMBERS_MSG	11032	//��ȡȺ��Ա��Ϣ
#define MT_DATA_PUBLICS_MSG	11033	//��ȡ���ں���Ϣ

#define MT_RECV_TEXT_MSG	11046	//�����ı���Ϣ
#define MT_RECV_PICTURE_MSG	11047	//����ͼƬ��Ϣ
#define MT_RECV_VOICE_MSG	11048	//������Ƶ��Ϣ
#define MT_RECV_FRIEND_MSG	11049	//�������������Ϣ
#define MT_RECV_CARD_MSG	11050	//������Ƭ��Ϣ
#define MT_RECV_VIDEO_MSG	11051	//������Ƶ��Ϣ
#define MT_RECV_EMOJI_MSG	11052	//���ձ�����Ϣ
#define MT_RECV_LOCATION_MSG	11053	//����λ����Ϣ
#define MT_RECV_LINK_MSG	11054	//����������Ϣ
#define MT_RECV_FILE_MSG	11055	//�����ļ���Ϣ
#define MT_RECV_MINIAPP_MSG	11056	//����С������Ϣ
#define MT_RECV_WCPAY_MSG	11057	//���պ���ת����Ϣ
#define MT_RECV_SYSTEM_MSG	11058	//����ϵͳ��Ϣ
#define MT_RECV_REVOKE_MSG	11059	//���ճ�����Ϣ
#define MT_RECV_OTHER_MSG	11060	//��������δ֪��Ϣ


typedef enum _EnRawMsgType
{
    WX_MSG_TEXT = 1,       // 0x1     �ı�
    WX_MSG_PICTURE = 3,    // 0x3     ͼƬ
    WX_MSG_VOICE = 34,     // 0x22    ����
    WX_MSG_FRIEND = 37,    // 0x25    �Ӻ������� "from_wxid" : "fmessage"
    WX_MSG_CARD = 42,      // 0x2A    ��Ƭ
    WX_MSG_VIDEO = 43,     // 0x2B    ��Ƶ
    WX_MSG_EMOJI = 47,     // 0x2F    ����
    WX_MSG_LOCATION = 48,  // 0x30    λ��
    WX_MSG_APP = 49,       // 0x31    Ӧ�����ͣ������Ͳο�EnRawAppMsgType
    WX_MSG_SYSTEM = 10000, // 0x2710  ϵͳ��Ϣ
    WX_MSG_REVOKE = 10002, // 0x2712  ������Ϣ
} EnRawMsgType;

// XML XPATH·�� /msg/appmsg/type/text()
typedef enum _EnRawAppMsgType
{
    WX_APPMSG_LINK = 5,     // ���ӣ�����Ⱥ���룩
    WX_APPMSG_FILE = 6,     // �ļ�
    WX_APPMSG_MUTIL = 19,   // �ϲ���Ϣ
    WX_APPMSG_MINIAPP = 33, // С����
    WX_APPMSG_WCPAY = 2000, // ת��
} EnRawAppMsgType;