#pragma once

// Library Functions
#include <iostream>
#include <exception>
#define _AFXDLL 1
#include <afxwin.h>
#include <afxmt.h>
#include <WinSock2.h>   // C-style WinAPI header
#include <WS2tcpip.h>   // `inet_ntop()`, `inet_ntop()`
#pragma comment(lib, "Ws2_32.lib")
#include <cstring>      // `memset`
#include <sstream>
#include <fstream>
#include <mutex>

// Data Structures
#include <string>
#include <vector>
#include <queue>
#include <array>
#include <unordered_set>

#include "Config.h"     // global configurations
#include "common_utils.h"
#include "HttpParser.h"

#include <FL/Fl.H>
#include <FL/Fl_Text_Display.H>

class Server
{
public:

    SOCKET              srv_listen_sock;
    char               *recv_buf;
    // std::unordered_map  thread_pool;
    const volatile bool &running;

    std::unordered_set<CWinThread *>        client_thread_pool;

    typedef std::array<std::string, 2>      output_buf_item_t;
    typedef std::queue<output_buf_item_t>   output_buf_t;
    // output_buf_t    routing_output_buf;
    // std::mutex      routing_output_buf_lock;

    Fl_Text_Buffer *output_buffer;

public:

    // Configurations
    class Config {
    public:
        std::string address;
        std::string port;
        size_t      max_connections;
        size_t      recv_buf_size;
        std::string static_directory;
    } config;

    class _ClientJobParam {
    public:
        SOCKET         *sock_ptr;
        Server::Config  srv_config;
        output_buf_t   &output_buf;
        std::mutex     &output_lock;
        _ClientJobParam(
            SOCKET *psock,
            Server::Config config,
            output_buf_t output_buf,
            std::mutex &output_lock
        ) : sock_ptr(psock),
            srv_config(config),
            output_buf(output_buf),
            output_lock(output_lock)
        {
            return;
        }
    };

    Server(
        const volatile bool &runing_indicator,
        const char *static_directory = Config_StaticRootPath.c_str(),
        const char *address = Config_Address,
        const char *port = Config_Port,
        Fl_Text_Buffer *output_buffer = 0,
        const size_t max_connections = Config_MaxConnections,
        const size_t recv_buf_size = Config_RecvBufferSize
    );
    void _InitWinsock(
        WORD major_version = 2,
        WORD minor_version = 2
    );
    void _CreateListenSocket(void);
    void _StartListen(void);
    static UINT StartServiceLoop(LPVOID self);
    static std::string _RecvPack(
        SOCKET sock,
        const size_t recv_buf_size,
        const std::string &termseq = ":q"
    );
    static void _SendPack(
        SOCKET sock,
        std::string send_str    // NOTE: copy-constructed
    );
    // static UINT _ClientJob(LPVOID param_ptr);
    static UINT _ClientJobWithAccept(LPVOID param_ptr);
    static std::string _RouteAndReturn(
        const std::string &package,
        const std::string &static_directory
    );
    static std::string _GetIPFromSock(const SOCKET &sock);
    void StopServer(void);
    virtual ~Server();
};
