#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "pch.h"
#include "mongoose.h"
#include "MSocketServer.h"
#include "string"
#include <list>
#include<thread>
#include "utils.h"
#include "AddrOffset.h"

using namespace std;
using namespace rapidjson;

static sig_atomic_t s_signal_received = 0;
static const char* s_http_port = "10080";
static struct mg_serve_http_opts s_http_server_opts;

//struct mg_mgr g_mgr;
struct mg_connection* g_nc;



static void signal_handler(int sig_num) {
	signal(sig_num, signal_handler);  // Reinstantiate signal handler
	s_signal_received = sig_num;
}

static int is_websocket(const struct mg_connection* nc) {
	return nc->flags & MG_F_IS_WEBSOCKET;
}


/*
 * ��������websocket��ҵ��
 */
static void broadcast(struct mg_connection* nc, const struct mg_str msg) {

    OutputDebugString(TEXT("broadcast..... "));

    ///*
    // * ����sockt���ݣ�������Ϣ
    // */
    //Document document;
    //char buffer[1000] = { 0 };
    //memcpy(buffer, msg.p, msg.len);


    //if (document.ParseInsitu(buffer).HasParseError()) {
    //    //printf("parse error...\n");
    //    //OutputDebugString(TEXT("broadcast...parse error..."));
    //    //TODO ��ʽ����
    //    return;
    //}

    //// ���е����󶼱������type�ֶ�
    //int type = document["type"].GetInt();

    //string text = "type��\t";
    //text.append(Dec2Hex(type));
    //OutputDebugString(String2LPCWSTR(text));

    ////1�������ı�����
    //if (type == MT_SEND_TEXTMSG) {

    //    wchar_t* content = UTF8ToUnicode(document["content"].GetString());
    //    wchar_t* toWxid = UTF8ToUnicode(document["to_wxid"].GetString());

    //    //HANDLE hThread = CreateThread(NULL, 0, ThreadSendMsgText, NULL, 0, NULL);

    //    OutputDebugString(TEXT("broadcast.....333 "));

    //    return;

    //}
    //else if (type == MT_SEND_FILEMSG) {

    //    //�����ļ�
    //    wchar_t* file = UTF8ToUnicode(document["file"].GetString());
    //    wchar_t* toXxid = UTF8ToUnicode(document["to_wxid"].GetString());


    //    return;
    //}


    OutputDebugString(TEXT("broadcast.....222222 "));



    //2�������ļ�����


    //3���յ���Ϣ���ͻط�����

}

static void ev_handler(struct mg_connection* nc, int ev, void* ev_data) {
	switch (ev) {
	case MG_EV_WEBSOCKET_HANDSHAKE_DONE: {
		/*
		 * New websocket connection.
		 * ��clint���ӵ�server֮��hook��Ϣ���գ���Ϣ���͵�client
		 */
		OutputDebugString(TEXT("MG_EV_WEBSOCKET_HANDSHAKE_DONE..... \n"));


		g_nc = nc;
		printf("MG_EV_WEBSOCKET_HANDSHAKE_DONE\n");
		//mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, "{\"from server HANDSHAKE\"}", 0x20);
		break;
	}
	case MG_EV_WEBSOCKET_FRAME: {

		OutputDebugString(TEXT("MG_EV_WEBSOCKET_FRAME..... \n"));

		struct websocket_message* wm = (struct websocket_message*)ev_data;
        struct mg_str msg = { (char*)wm->data, wm->size };


		//�����յ�����Ϣ
		broadcast(nc, msg);

		//mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, "{\"from server FRAME\"}", 0x20);

		//Sleep(2000);
		break;
	}
	case MG_EV_HTTP_REQUEST: {
		mg_serve_http(nc, (struct http_message*)ev_data, s_http_server_opts);
		break;
	}
	case MG_EV_CLOSE: {
		/* Disconnect. Tell everybody. */
		if (is_websocket(nc)) {
			//broadcast(nc, mg_mk_str("-- left"));
		}
		break;
	}
	}
}


VOID StartSocketServer()
{

	OutputDebugString(TEXT("StartSocketServer"));

	struct mg_mgr mgr;
	struct mg_connection* nc;

	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);
	//setvbuf(stdout, NULL, _IOLBF, 0);
	//setvbuf(stderr, NULL, _IOLBF, 0);

	mg_mgr_init(&mgr, NULL);

	nc = mg_bind(&mgr, s_http_port, ev_handler);
	mg_set_protocol_http_websocket(nc);
	s_http_server_opts.document_root = ".";  // Serve current directory
	s_http_server_opts.enable_directory_listing = "yes";

	while (s_signal_received == 0) {
		mg_mgr_poll(&mgr, 200);
	}

	//g_nc = nc;

	return;
}

VOID StopSocketServer(struct mg_mgr* mgr) {

	OutputDebugString(TEXT("StopSocketServer"));

	mg_mgr_free(mgr);
}


//�ı���Ϣ�ṹ��
struct StructWxid
{
    //���͵��ı���Ϣָ��
    wchar_t* pWxid;
    //�ַ�������
    DWORD length;
    //�ַ�����󳤶�
    DWORD maxLength;

    //��������ռλ����
    DWORD fill1;
    DWORD fill2;
};


//BOOL SendMsgText(LPCSTR lpJsonData) {
//
//
//	//��������nc
//	mg_send_websocket_frame(g_nc, WEBSOCKET_OP_TEXT, lpJsonData, 0x20);
//
//	return TRUE;
//}