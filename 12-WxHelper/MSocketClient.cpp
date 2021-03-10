#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "MSocketClient.h"
#include "mongoose.h"
#include "AddrOffset.h"
#include "string"
#include "utils.h"

static int s_done = 0;
static int s_is_connected = 0;

using namespace std;
using namespace rapidjson;

extern "C" _declspec(dllexport) VOID SendMsgText(LPCSTR lpJsonData);


//mg_connection* g_nc;

//DWORD WINAPI ThreadSendMsgText(PVOID lpParameter);



static void ev_handler(struct mg_connection* nc, int ev, void* ev_data) {
    (void)nc;

    //OutputDebugString(TEXT("ev_handler...."));
    string text = "ev_handler ev：\t";
    text.append(Dec2Hex(ev));
    OutputDebugString(String2LPCWSTR(text));

    switch (ev) {
    case MG_EV_CONNECT: {
        int status = *((int*)ev_data);
        if (status != 0) {
            printf("-- Connection error: %d\n", status);
        }
        break;
    }
    case MG_EV_WEBSOCKET_HANDSHAKE_DONE: {
        struct http_message* hm = (struct http_message*)ev_data;
        if (hm->resp_code == 101) {
            //printf("-- Connected\n");
            //握手成功
            OutputDebugString(TEXT("ev_handler....Connected"));
            s_is_connected = 1;
        }
        else {
            printf("-- Connection failed! HTTP code %d\n", hm->resp_code);
            /* Connection will be closed after this. */
        }
        break;
    }
    case MG_EV_POLL: {
        char msg[500];
        int n = 0;

        INPUT_RECORD inp[100];
        HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
        DWORD i, num;
        if (!PeekConsoleInput(h, inp, sizeof(inp) / sizeof(*inp), &num)) break;
        for (i = 0; i < num; i++) {
            if (inp[i].EventType == KEY_EVENT &&
                inp[i].Event.KeyEvent.wVirtualKeyCode == VK_RETURN) {
                break;
            }
        }
        if (i == num) break;
        if (!ReadConsole(h, msg, sizeof(msg), &num, NULL)) break;
        /* Un-unicode. This is totally not the right way to do it. */
        for (i = 0; i < num * 2; i += 2) msg[i / 2] = msg[i];
        n = (int)num;

        if (n <= 0) break;
        while (msg[n - 1] == '\r' || msg[n - 1] == '\n') n--;
        mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, msg, n);
        break;
    }
    case MG_EV_WEBSOCKET_FRAME: {
        OutputDebugString(TEXT("ev_handler...MG_EV_WEBSOCKET_FRAME."));

        struct websocket_message* wm = (struct websocket_message*)ev_data;
        struct mg_str msg = { (char*)wm->data, wm->size };
        //printf("%.*s\n", (int)wm->size, wm->data);

        



        //发送消息
        //mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, "{\"from client FRAME\"}", 0x15);

        break;
    }
    case MG_EV_CLOSE: {
        if (s_is_connected) printf("-- Disconnected\n");
        s_done = 1;
        break;
    }
    }
}

int SocketClient() {

    OutputDebugString(TEXT("SocketClient...."));


    struct mg_mgr mgr;
    struct mg_connection* nc;
    const char* chat_server_url = "ws://127.0.0.1:10080";

    mg_mgr_init(&mgr, NULL);
    OutputDebugString(TEXT("SocketClient....2222"));
    nc = mg_connect_ws(&mgr, ev_handler, chat_server_url, "ws_chat", NULL);
    if (nc == NULL) {
        //fprintf(stderr, "Invalid address\n");
        return 1;
    }

    //g_nc = nc;

    OutputDebugString(TEXT("SocketClient....3333"));
    while (!s_done) {
        mg_mgr_poll(&mgr, 100);
    }
    OutputDebugString(TEXT("SocketClient....4444"));
    mg_mgr_free(&mgr);
    OutputDebugString(TEXT("SocketClient....5555"));
    return 0;
}



////新线程，开启socket
//DWORD WINAPI ThreadSendMsgText(PVOID lpParameter) {
//
//    OutputDebugString(TEXT("WeChatHelper ThreadProc... "));
//    return 0;
//
//}

