// Tutorial followed: https://learn.microsoft.com/en-us/windows/win32/winsock/getting-started-with-winsock (server application, the client is our browser)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

SOCKET ListenSocket = INVALID_SOCKET;
SOCKET ClientSocket = INVALID_SOCKET;
struct addrinfo* result = NULL, * ptr = NULL, hints;

void myserver_StartUpWSA(WSADATA wsaData)
{
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return;
	}
	printf("WSAStartup succeeded\n");
}

void myserver_GetAddrInfo() {
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	int iResult = getaddrinfo(NULL, "80", &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return;
	}
	printf("Got adddress into \n");
}

void myserver_CheckSocket()
{
	if (ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return;
	}
	printf("Socket checked. No errors\n");
}

void myserver_BindSocket() {
	int iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}
	printf("Socket bound. iResult = %d\n", iResult);
}

void myserver_Listen()
{
	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}
	printf("Listening\n");
}

void myserver_ReceiveSendData() {
	char recvbuf[512];
	
	int iResult, iSendResult;
	int recvbuflen = 512;

	// Receive until the peer shuts down the connection
	do {
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		
		if (iResult > 0) {
			printf("Received request. RecvBuffer data: \n%s\n", recvbuf);
			printf("Bytes received: %d\n", iResult);
			// Echo the buffer back to the sender
			char* content = recvbuf;
			int c_size = strlen(content);
			char sizeString[256] = {0};
			sprintf(sizeString, "%d", c_size);
			char sndbuf[1024] = {0};
			// concats data to send to client into single string with variables, concats into sndbuf string
			strcat_s(sndbuf, sizeof sndbuf, "HTTP/1.1 200 OK\nContent-Length:");
			strcat_s(sndbuf, sizeof sndbuf, sizeString);
			strcat_s(sndbuf, sizeof sndbuf, "\nConnection: close\n\n");
			strcat_s(sndbuf, sizeof sndbuf, "Your client request is:\n\n");
			strcat_s(sndbuf, sizeof sndbuf, content);
			iSendResult = send(ClientSocket, sndbuf, strlen(sndbuf), 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return;
			}
			printf("Bytes sent: %d\n", iSendResult);
		}	
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return;
		}

	} while (iResult > 0);
}

void myserver_AccceptConnection()
{
	// Accept a client socket
	printf("Accepting connection\n");
	
	// accept is a loop function that waits for any connection
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}
	myserver_ReceiveSendData();
	// remove comment to always listen to connections, recursively. might be a bad practice
	// myserver_AccceptConnection();
}

myserver_Shutdown() {
	printf("Shutdowning\n");
	int iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}
	closesocket(ClientSocket);
	WSACleanup();
}

int main()
{
	printf("This is a test on a winsock server. It directly sends response to a request only once, before shutting down. The format expects that you will open localhost:80 to communicate with server.\n");
	WSADATA wsaData = {0};
	myserver_StartUpWSA(wsaData);
	myserver_GetAddrInfo();
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	myserver_CheckSocket();
	myserver_BindSocket();
	myserver_Listen();
	myserver_AccceptConnection();
	myserver_Shutdown();

	return 0;
}