#pragma once

#include <WinSock2.h>
#include <string>

const long Config_AsyncLoopUsec = 50000;

const char Config_Address[] = "127.0.0.1";
const char Config_Port[] = "5002";

const size_t Config_RecvBufferSize = 2048;
const size_t Config_MaxConnections = 10;
const u_long Config_BlockMode = 1;      // non-blocking

const std::string Config_StaticRootPath = "C:/Users/smdsbz/Desktop";
