#include <string>
#include <iostream>
#include "SocketHandler.h"

namespace pro_cal
{
	/**
	* check if the server socket started, if not then call startSocketServer methode
	*
	* @param ServerSocket SOCKET which will be checked and started
	* @param serverIP char array with the ip address from the server
	* @param socketPort char array with the port from the server
	* @return bool true if the server socket started successfully
	*/
	bool checkAndStartServerSocket(SOCKET& ServerSocket, const char* serverIP, const char* socketPort) {
		int serverStarted = 1;
		if (ServerSocket == INVALID_SOCKET) {
			serverStarted = startSocketServer(ServerSocket, serverIP, socketPort);
		}
		if (ServerSocket != INVALID_SOCKET && serverStarted > 0) {
			return true;
		}
		else {
			return false;
		}
	}

	/**
	* check if the UDP/TCP client socket started, if not then call startSocketClient methode
	*
	* @param ClientSocket SOCKET which will be checked and started
	* @param serverIP char array with the ip address from the server
	* @param serverPort char array with the port from the server
	* @param protocol IPPROTO set the used protocol UDP = IPPROTO_UDP or TCP = IPPROTO_TCP
	* @return bool true if the client socket started successfully
	*/
	bool checkAndStartClientSocket(SOCKET& ClientSocket, const char* serverIP, const char* serverPort, const IPPROTO protocol) {
		int clientStarted = 1;
		if (ClientSocket == INVALID_SOCKET) {
			clientStarted = startSocketClient(ClientSocket, serverIP, serverPort, protocol);
		}
		if (clientStarted != INVALID_SOCKET && ClientSocket > 0) {
			return true;
		}
		else {
			return false;
		}
	}

	/**
	* start a non-blocking udp server socket
	* 1. Initialize Winsock
	* 2. Resolve the server address and port
	* 3. Create a SOCKET for connecting to server
	* 4. Setup the listening socket
	* 5. If iMode!=0, set non-blocking mode to enabled.
	*
	* @param ServerSocket SOCKET which will be checked and started
	* @param serverIP char array with the ip address from the server
	* @param socketPort char array with the port from the server
	* @return int 1 if the server socket started successfully, -1 if an error occurred
	*/
	int startSocketServer(SOCKET& ServerSocket, const char* serverIP, const char* socketPort)
	{
		WSADATA wsaData;
		int iResult;

		struct addrinfo *result = NULL;
		struct addrinfo hints;

		// Initialize Winsock
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			std::cout << "WSAStartup failed with error: " << iResult << std::endl;
			return -1;
		}

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;
		//hints.ai_flags = AI_PASSIVE;

		// Resolve the server address and port
		iResult = getaddrinfo(serverIP, socketPort, &hints, &result);
		if (iResult != 0) {
			std::cout << "getaddrinfo failed with error: " << iResult << std::endl;
			WSACleanup();
			return -1;
		}

		// Create a SOCKET for connecting to server
		ServerSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (ServerSocket == INVALID_SOCKET) {
			handleSocketError(ServerSocket, result, "socket failed with error: ", WSAGetLastError());
			return -1;
		}

		// Setup the listening socket
		iResult = bind(ServerSocket, result->ai_addr, (int)result->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			handleSocketError(ServerSocket, result, "bind failed with error: ", WSAGetLastError());
			return -1;
		}

		// If iMode!=0, non-blocking mode is enabled.
		u_long iMode = 1;
		iResult = ioctlsocket(ServerSocket, FIONBIO, &iMode);
		if (iResult == SOCKET_ERROR){
			std::cout << "Setting non blocking failed";
		}

		freeaddrinfo(result);
		return 1;
	}

	/**
	* start an tcp/udp client socket
	* 1. Initialize Winsock
	* 2. Resolve the server address and port
	* 3. Attempt to connect to an address until one succeeds
	* 4. Create a SOCKET for connecting to server
	* 5. Connect to server.
	*
	* @param ServerSocket SOCKET which will be checked and started
	* @param serverIP char array with the ip address from the server
	* @param socketPort char array with the port from the server
	* @param protocol IPPROTO set the used protocol UDP = IPPROTO_UDP or TCP = IPPROTO_TCP
	* @return int 1 if the server socket started successfully, -1 if an error occurred
	*/
	int startSocketClient(SOCKET& ClientSocket, const char* serverIP, const char* serverPort, const IPPROTO protocol)
	{
		WSADATA wsaData;
		struct addrinfo *result = NULL,
			*ptr = NULL,
			hints;

		// Initialize Winsock
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			std::cout << "WSAStartup failed with error: " << iResult << std::endl;
			return -1;
		}

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = protocol == IPPROTO_UDP ? SOCK_DGRAM : SOCK_STREAM;
		hints.ai_protocol = protocol; //IPPROTO_UDP, IPPROTO_TCP

		// Resolve the server address and port
		iResult = getaddrinfo(serverIP, serverPort, &hints, &result);
		if (iResult != 0) {
			std::cout << "getaddrinfo failed with error: " << iResult << std::endl;
			WSACleanup();
			return -1;
		}

		// Attempt to connect to an address until one succeeds
		for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
			// Create a SOCKET for connecting to server
			ClientSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			if (ClientSocket == INVALID_SOCKET) {
				std::cout << "socket failed with error: " << WSAGetLastError() << std::endl;
				WSACleanup();
				return -1;
			}

			// Connect to server.
			iResult = connect(ClientSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iResult == SOCKET_ERROR) {
				closesocket(ClientSocket);
				ClientSocket = INVALID_SOCKET;
				continue;
			}
			break;
		}

		freeaddrinfo(result);

		if (ClientSocket == INVALID_SOCKET) {
			std::cout << "Unable to connect to server!" << std::endl;
			WSACleanup();
			return -1;
		}
		return 1;
	}

	/**
	* close an open client or server socket
	*
	* @param Socket SOCKET which will be closed
	* @return int 1 if the server socket closed successfully, -1 if an error occurred
	*/
	int closeSocket(SOCKET& Socket)
	{
		// shutdown the connection since we're done
		int iResult = shutdown(Socket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			handleSocketError(Socket, nullptr, "shutdown failed with error: ", WSAGetLastError());
			return -1;
		}

		// cleanup
		closesocket(Socket);
		WSACleanup();
		return 1;
	}

	/**
	* send a message over an open socket to an specific socket address
	*
	* @param Socket SOCKET to use for sending the message
	* @param msg std::string the message to be send
	* @param toAddr sockaddr_in the address from the message receiver
	* @return int 1 if the server socket closed successfully, -1 if an error occurred
	*/
	int sendSocketMsgTo(SOCKET& Socket, const std::string msg, sockaddr_in& toAddr)
	{
		int sizeOfTo = sizeof(toAddr);
		int iSendResult = sendto(Socket, msg.c_str(), msg.length(), 0, (struct sockaddr *)&toAddr, sizeOfTo);
		if (iSendResult == SOCKET_ERROR) {
			handleSocketError(Socket, nullptr, "send failed with error: ", WSAGetLastError());
			return -1;
		}
		std::cout << "Msg sent: " << msg.c_str() << std::endl;
		return 1;
	}


	/**
	* receive a message over an open socket from an specific socket address
	*
	* @param Socket SOCKET to use for receiving the message
	* @param msg std::string where the received message will be assigned 
	* @param toAddr sockaddr_in the address from the message sender
	* @return int 1 if the server socket closed successfully, -1 if an error occurred
	*/
	int receiveSocketMsgFrom(SOCKET& Socket, std::string& msg, sockaddr_in& fromAddr)
	{
		int result = 0;
		int sizeOfFrom = sizeof(fromAddr);
		char recMsgBuff[DEFAULT_BUFLEN];
		result = recvfrom(Socket, recMsgBuff, DEFAULT_BUFLEN, 0, (struct sockaddr *)&fromAddr, &sizeOfFrom);
		if (result > 0) {
			//add null character, if you want to use with printf/puts or other string handling functions
			recMsgBuff[result] = '\0';
			msg.assign(recMsgBuff);
		}
		return result;
	}

	/**
	* send a message over an open socket to the defined server from a client socket
	*
	* @param Socket SOCKET to use for sending the message
	* @param msg std::string the message to be send
	* @return int 1 if the server socket closed successfully, -1 if an error occurred
	*/
	int sendSocketMsg(SOCKET& Socket, const std::string msg)
	{
		// Send an initial buffer
		int result = send(Socket, msg.c_str(), msg.length(), 0);
		if (result == SOCKET_ERROR) {
			handleSocketError(Socket, nullptr, "send failed with error: ", WSAGetLastError());
			return -1;
		}
		return 1;
	}

	/**
	* receive a message over an open socket from the defined server socket
	*
	* @param Socket SOCKET to use for receiving the message
	* @param msg std::string where the received message will be assigned
	* @return int 1 if the server socket closed successfully, -1 if an error occurred
	*/
	int receiveSocketMsg(SOCKET& Socket, std::string& msg)
	{
		int result = 0;
		char recMsgBuff[DEFAULT_BUFLEN];
		result = recv(Socket, recMsgBuff, DEFAULT_BUFLEN, 0);
		if (result > 0)
		{
			recMsgBuff[result] = '\0';
			msg.assign(recMsgBuff);
		}
		return result;
	}

	/**
	* receive data over an open socket from the defined server socket
	*
	* @param Socket SOCKET to use for receiving the message
	* @param data char array where the received data will be assigned
	* @return int 1 if the server socket closed successfully, -1 if an error occurred
	*/
	int receiveSocketDataMsg(SOCKET& Socket, char* data)
	{
		int size_recv, total_size = 0;
		char chunk[DATA_BUFLEN];
		while (true)
		{
			memset(chunk, 0, DATA_BUFLEN);  //clear the variable
			size_recv = recv(Socket, data + total_size, DATA_BUFLEN, 0);
			if (size_recv > 0)
			{
				total_size += size_recv;
			}
			else
			{
				break;
			}

		}
		return total_size;
	}

	/**
	* print the passed error message, then close and clean up the socket variables
	*
	* @param Socket SOCKET which will be closed
	* @param result addrinfo pointer which will be cleaned 
	* @param errMsg td::string which will be printed
	* @param errNum int which will be printed
	*/
	void handleSocketError(SOCKET& socket, struct addrinfo *result, std::string errMsg, int errNum) {
		std::cout << errMsg.c_str() << errNum << std::endl;
		if (result != nullptr)
			freeaddrinfo(result);
		if (socket != NULL)
			closesocket(socket);
		WSACleanup();
	}

}