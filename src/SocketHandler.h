#ifndef  __SOCKETHANDLER_H__
#define  __SOCKETHANDLER_H__
#define WIN32_LEAN_AND_MEAN
#define DEFAULT_BUFLEN 512
#define DATA_BUFLEN 4096
#define IMAGE_BUFLEN 10485760
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

namespace pro_cal
{
	bool checkAndStartServerSocket(SOCKET& ServerSocket, const char* serverIP, const char* socketPort);
	bool checkAndStartClientSocket(SOCKET& ClientSocket, const char* serverIP, const char* serverPort, const IPPROTO protocol);
	int startSocketClient(SOCKET& ClientSocket, const char* serverIP, const char* serverPort, const IPPROTO protocol);
	int startSocketServer(SOCKET& ServerSocket, const char* serverIP, const char* socketPort);
	int closeSocket(SOCKET& Socket);
	int sendSocketMsgTo(SOCKET& Socket, const std::string msg, sockaddr_in& toAddr);
	int receiveSocketMsgFrom(SOCKET& Socket, std::string& msg, sockaddr_in& fromAddr);
	int sendSocketMsg(SOCKET& Socket, const std::string msg);
	int receiveSocketMsg(SOCKET& Socket, std::string& msg);
	int receiveSocketDataMsg(SOCKET& Socket, char* data);
	void handleSocketError(SOCKET& Socket, struct addrinfo *result, std::string errMsg, int errNum);
}
#endif  /*__SOCKETHANDLER_H__*/