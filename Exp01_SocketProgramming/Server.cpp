#include "pch.h"
#include "Server.h"

// Library Functions
using std::cout; using std::cin; using std::endl;
using std::memset;
using std::istringstream;
using std::fstream;
using std::mutex; using std::lock_guard;

// Data Structures
using std::string; using std::to_string; using std::getline;
using std::queue; using std::array; using std::unordered_set;

/* Static Storage */

Server::output_buf_t routing_output_buf;
mutex routing_output_buf_lock;

/* Method Implementations */

Server::Server(
    const volatile bool &running_indicator,
    const char *static_directory,
    const char *address,
    const char *port,
    Fl_Text_Buffer *output_buffer,
    const size_t max_connections,
    const size_t recv_buf_size
) : running(running_indicator)
{
    // initialize receive buffer
    this->config.recv_buf_size = recv_buf_size;
    this->recv_buf = new char[this->config.recv_buf_size];
    memset(this->recv_buf, 0, this->config.recv_buf_size);

    // Configurations
    this->config.address = string(address);
    this->config.port = string(port);
    this->config.max_connections = max_connections;
    this->config.static_directory = string(static_directory);

    cout << endl;
    cout << "Server Configurations\n";
    cout << "=====================\n";
    cout << "Address: " << this->config.address << "\n"
        << "Port: " << this->config.port << "\n"
        << "Max Connections: " << this->config.max_connections << "\n"
        << "Static File Directory: " << this->config.static_directory
        << endl;
    cout << endl;

    this->_InitWinsock();
    this->_CreateListenSocket();
    this->_StartListen();

    this->output_buffer = output_buffer;

    // all ok!
    return;
}


Server::~Server()
{
    if (this->recv_buf != 0) {
        delete[] this->recv_buf;
        this->recv_buf = 0;
    }

    // clean WinSock
    WSACleanup();   // TODO: okay to cleanup multiple times?

    return;
}


void Server::StopServer(void) {
    shutdown(this->srv_listen_sock, SD_SEND);
    closesocket(this->srv_listen_sock);
    return;
}


void Server::_InitWinsock(
    WORD major_version,
    WORD minor_version
)
{
    int winret;
    // initialize WinSock
    WSADATA wsa_data;
    winret = WSAStartup(
        MAKEWORD(major_version, minor_version),
        &wsa_data
    );
    if (winret != 0) {
        throw std::runtime_error(
            string("WinSock startup failed: ")
            + to_string(winret)
        );
    }
    // WinSock startup ok
    // cout << "WinSock startup succeded!" << endl;
    return;
}


void Server::_CreateListenSocket(void)
{
    // get sockaddr infos
    struct addrinfo hints, *result;
    memset((void *)&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    int winret;
    winret = getaddrinfo(
        NULL,
        this->config.port.c_str(),
        &hints,
        &result
    );
    if (winret != 0) {
        WSACleanup();
        throw std::runtime_error(
            string("getaddrinfo() failed: ")
            + to_string(winret)
        );
    }
    // cout << "getaddrinfo() passed!" << endl;

    // create listen socket
    this->srv_listen_sock = socket(
        result->ai_family,
        result->ai_socktype,
        result->ai_protocol
    );
    if (this->srv_listen_sock == INVALID_SOCKET) {
        WSACleanup();
        freeaddrinfo(result);
        throw std::runtime_error(
            string("Socket creation failed: ")
            + string("INVALID_SOCKET")
        );
    }
    // cout << "Socket created successfully!" << endl;

    // bind listen socket to port
    winret = bind(
        this->srv_listen_sock,
        result->ai_addr,
        (int)result->ai_addrlen
    );
    if (winret == SOCKET_ERROR) {
        freeaddrinfo(result);
        closesocket(this->srv_listen_sock);
        WSACleanup();
        throw std::runtime_error(
            string("Socket bind failed: ")
            + to_string(winret)
        );
    }
    // bind okay
    // cout << "Socket bind succeded!" << endl;
    freeaddrinfo(result);   // result freed for its no longer needed

    return;
}


void Server::_StartListen(void)
{
    int winret;
    // start listen
    winret = listen(this->srv_listen_sock, this->config.max_connections);
    if (winret == SOCKET_ERROR) {
        closesocket(this->srv_listen_sock);
        WSACleanup();
        throw std::runtime_error("Socket listen failed!");
    }
    // cout << "Socket is now listening!" << endl;

    return;
}


string Server::_RecvPack(
    SOCKET sock,
    const size_t recv_buf_size,
    const string &termseq
)
{
    size_t sockbuf_len = recv_buf_size;
    char *sockbuf = new char[sockbuf_len];
    memset(sockbuf, 0, sockbuf_len);
    string recv_buf_str;
    int winret;
    // recv from distant side of pipe
    do {
        winret = recv(
            sock,
            sockbuf,
            sockbuf_len,
            0
        );
        // encoutered terminal sequence, signal for end of distant send
        recv_buf_str += sockbuf;
        if (string_ends_with(recv_buf_str, termseq)) {
            break;
        }
        else {
            recv_buf_str += sockbuf;
        }
    } while (winret > 0);
    // check if recv failed
    if (winret < 0) {
        delete[] sockbuf;
        throw std::runtime_error(
            string("recv failed: ")
            + to_string(winret)
        );
    }
    // check if distant side closed socket
    if (winret == 0) {
        winret = shutdown(sock, SD_SEND);
        if (winret == SOCKET_ERROR) {
            throw std::runtime_error(
                string("Socket shutdown failed: ")
                + to_string(winret)
            );
        }
        closesocket(sock);
        // cout << "Received this before remote shutdown:\n"
        //     << recv_buf_str
        //     << endl;
        delete[] sockbuf;
        throw std::runtime_error(
            string("Remote side of pipe shutdown!")
        );
    }
    delete[] sockbuf;
    return recv_buf_str;
}


void Server::_SendPack(
    SOCKET sock,
    string send_str
)
{
    int winret;
    do {
        winret = send(
            sock,
            &send_str[0],
            send_str.size(),    // NOTE: non-ascii counted as well!
            0
        );
        // check if successfully sent
        if (winret == SOCKET_ERROR) {
            throw std::runtime_error(
                string("send failed: ")
                + to_string(winret)
            );
        }
        // successfully send, remove sent part from send buffer
        send_str.erase(0, winret);
    } while (!send_str.empty());
    return;
}


string Server::_GetIPFromSock(const SOCKET &sock)
{
    char ip_cstr[INET_ADDRSTRLEN];
    struct sockaddr name;
    socklen_t namelen = sizeof(name);
    getpeername(
        sock,
        &name,
        &namelen
    );
    inet_ntop(
        AF_INET,
        &name,
        ip_cstr,
        INET_ADDRSTRLEN
    );
    return string(ip_cstr);
}


UINT Server::StartServiceLoop(LPVOID self)
{
    Server *this_ = (Server *)self;
    int winret;
    while (true) {
        // shrink thread pool
        for (auto it = this_->client_thread_pool.begin();
                it != this_->client_thread_pool.end();) {
            if (WaitForSingleObject((*it)->m_hThread, 0) == WAIT_OBJECT_0) {
                this_->client_thread_pool.erase(it);
                // NOTE: the following line must be added, or memory issue raised
                it = this_->client_thread_pool.begin();
            }
            else {
                ++it;
            }
        }
        // if stopping service
        if (this_->running == false) {
            if (!this_->client_thread_pool.empty()) {
                cout << "Server not running but thread pool's not empty : "
                    << this_->client_thread_pool.size() << std::endl;
                continue;
            }
            return 0;
        }
        // clear routing_output_buf
        Fl::lock();
        string output_str = std::string(this_->output_buffer->text());
        {
            lock_guard<mutex> lock(::routing_output_buf_lock);
            while (!::routing_output_buf.empty()) {
                output_buf_item_t record = ::routing_output_buf.front();
                ::routing_output_buf.pop();
                output_str += (
                    string("From ") + record[0]
                    + string(", requesting ") + record[1] + string("\n")
                );
            }
        }
        this_->output_buffer->text(output_str.c_str());
        Fl::unlock();
        // init select() timeout
        fd_set listen_set;
        FD_ZERO(&listen_set);
        FD_SET(this_->srv_listen_sock, &listen_set);
        // blocked in select, wait for accecpt to be satisfied
        // accept() will be performed in _ClientJob() thread
        timeval timeout_setting;
        timeout_setting.tv_sec = 0;
        timeout_setting.tv_usec = Config_AsyncLoopUsec;
        winret = select(
            0,
            &listen_set,
            0,
            0,
            &timeout_setting
        );
        if (winret <= 0) {     // no in-comming client sockets after timeout
            continue;
        }
        // make a copy of current server configuration
        // NOTE: param should be freed in Server::_ClientJobWithAccept()
        Server::_ClientJobParam *param = new Server::_ClientJobParam(
            &this_->srv_listen_sock,
            this_->config,
            ::routing_output_buf,
            ::routing_output_buf_lock
        );
        CWinThread *pctrd = AfxBeginThread(
            Server::_ClientJobWithAccept,
            (LPVOID)param
        );
        pctrd->m_bAutoDelete = FALSE;
        // add new to thread pool
        this_->client_thread_pool.insert(pctrd);
        // // listen for incomming connection
        // SOCKET *cli_sock_ptr = new SOCKET;
        // *cli_sock_ptr = accept(
        //     this->srv_listen_sock,
        //     NULL,
        //     NULL
        // );
        // if (*cli_sock_ptr == INVALID_SOCKET) {
        //     closesocket(*cli_sock_ptr);
        //     delete cli_sock_ptr;
        //     WSACleanup();
        //     throw std::runtime_error("Incomming connection acception failed!");
        // }
        // // connection accepted, print connection info
        // cout << "Connection accepted, from "
        //     << this->_GetIPFromSock(*cli_sock_ptr) << endl;
        // // NOTE: Server::_ClientJob SHOULD delete param!
        // Server::_ClientJobParam *param = new Server::_ClientJobParam(
        //     cli_sock_ptr,
        //     this->config
        // );
        // AfxBeginThread(
        //     Server::_ClientJob,
        //     (LPVOID)param
        // );
    }
    return 0;
}


UINT Server::_ClientJobWithAccept(LPVOID param_ptr)
{
    // NOTE: accept() is guaranteed to be non-blocked by previous select()
    Server::_ClientJobParam param = *(Server::_ClientJobParam *)param_ptr;
    SOCKET cli_sock = accept(
        *param.sock_ptr,
        0,
        0
    );
    string recv_str_full = Server::_RecvPack(
        cli_sock,
        param.srv_config.recv_buf_size,
        "\r\n"
    );
    string cli_ip = Server::_GetIPFromSock(cli_sock);
    string url = HttpParser::GetURL(recv_str_full);
    {
        lock_guard<mutex> lock(param.output_lock);
        std::cout << "From " << cli_ip << ", requesting " << url << std::endl;
        ::routing_output_buf.push({ cli_ip, url });
    }
    // send message to remote
    string send_buf_str = Server::_RouteAndReturn(
        recv_str_full,
        param.srv_config.static_directory
    );
    Server::_SendPack(cli_sock, send_buf_str);
    // no keep-alive, shutdown immediately
    shutdown(cli_sock, SD_SEND);
    closesocket(cli_sock);
    delete param_ptr;
    return 0;
}


// UINT Server::_ClientJob(LPVOID param_ptr)
// {
//     Server::_ClientJobParam param = *(Server::_ClientJobParam *)param_ptr;
//     SOCKET *cli_sock_ptr = param._sock_ptr;
//     // recv message from remote
//     string recv_str_full = Server::_RecvPack(
//         *cli_sock_ptr,
//         param.srv_config.recv_buf_size,
//         "\r\n"
//     );
//     // send message to remote
//     string send_buf_str = Server::_RouteAndReturn(
//         recv_str_full,
//         param.srv_config.static_directory
//     );
//     Server::_SendPack(*cli_sock_ptr, send_buf_str);
//     // no keep-alive, shutdown immediately
//     shutdown(*cli_sock_ptr, SD_SEND);
//     closesocket(*cli_sock_ptr);
//     delete param_ptr;
//     return 0;
// }


string Server::_RouteAndReturn(
    const string &package,
    const string &static_directory
) {
    string url = HttpParser::GetURL(package);
    if (url.empty()) {
        cout << "Package received not recognized as HTTP header!" << endl;
        return string();
    }
    // routing
    string content_type = HttpParser::GetContentTypeFromURL(url);
    string content;
    if (url == string("/")) {
        content = "<h1>NyanNyanNyanNyanNyanNyanNyanNyanNyanNyanNyan~~~</h1>";
        content.shrink_to_fit();
    }
    else {      // requesting files
        try {
            // NOTE: binary data stored in `string` buffer, use `string::capacity()` to get
            //       data size!
            content = read_file(static_directory + url);
        }
        catch (std::runtime_error &e) {
            content = "<h1>Oops, file cannot be found on server!</h1>";
            content.shrink_to_fit();
        }
    }
    return (
        string("HTTP/1.1 200 OK\r\n")
        + string("Content-Type: ") + content_type + string("\r\n")
        + string("Content-Length: ") + to_string(content.size()) + string("\r\n")
        + string("\r\n")    // blank line
        + content
    );
}
