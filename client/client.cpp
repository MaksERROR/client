#include <iostream>
#include <WS2tcpip.h>
#include <winsock2.h>
#include <iostream>
#include <string>
#pragma comment(lib, "ws2_32.lib")
using namespace std;
bool retry=1;
const char Local_IP[16] = "192.168.0.106";
const char LOCAL_PORT[4] = "140";
SOCKET Connection = INVALID_SOCKET;
WSAData wsaData;
ADDRINFO hints;
ADDRINFO* addrrez = NULL;
WORD DLLVersion = MAKEWORD(2, 2);

bool Closing(int s_type = 0, int function = 0)
{
	switch (s_type)
	{
	case(1)://
		if (function != 0)
		{
			cout << "startup err: " << function << endl;
			return 1;
		}
		break;

	case(2):
		if (function != 0)
		{
			cout << "getaddrinfo err: " << function << endl;
			freeaddrinfo(addrrez);
			WSACleanup();
			return 1;
		}
		break;

	case(3)://recv err
		if (function <= 0)
		{
			cout << ((function == 0) ? "recv error connection closed..." : "recv error") << endl;
			closesocket(Connection);
			return 0;
		}
		break;

	case(4)://send err
		if (function == SOCKET_ERROR)
		{
			cout << "send err:" << function << endl;
			closesocket(Connection);
			freeaddrinfo(addrrez);
			WSACleanup();
			return 1;
		}
		break;

	case(5):// Not CONNECTED WITH SERVER
		if (function == SOCKET_ERROR)
		{
			cout << "Unable connect to server..." << endl;
			closesocket(Connection);
			Connection = INVALID_SOCKET;
			freeaddrinfo(addrrez);
			WSACleanup();
			return 1;
		}
		break;

	default:
		break;
	}
	return 0;
}


void ClientHandler() {
	int msg_size = 0;

	while (retry) {

		retry = !Closing(3, recv(Connection, (char*)&msg_size, sizeof(int), 0));
		if (!retry)
		{
			return;
		}
		char* msg = new char[msg_size + 1];
		msg[msg_size] = '\0';

		retry = !Closing(3, recv(Connection, msg, msg_size, 0));

		cout << msg << endl;
		delete[] msg;
	}
}

int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "");
	system("chcp 1251");

	int rez = 0;
	
	if (Closing(1, WSAStartup(DLLVersion, &wsaData)))
		return 1;
	
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;//tcp ip

	if (Closing(2, getaddrinfo(Local_IP, LOCAL_PORT, &hints, &addrrez)))
		return 1;

	Connection = socket(addrrez->ai_family, addrrez->ai_socktype, addrrez->ai_protocol);
	if (Connection == INVALID_SOCKET)
	{
		cout << "Ssocket create err" << endl;
		freeaddrinfo(addrrez);
		WSACleanup();
		return 1;
	}

	if (Closing(5, connect(Connection, addrrez->ai_addr, (int)addrrez->ai_addrlen)))
		return 1;

	cout << "Connected with the server!\n";

	CreateThread(
		NULL, 
		NULL, 
		(LPTHREAD_START_ROUTINE)ClientHandler, 
		NULL, 
		NULL, 
		NULL);

	string msg1;

	while (retry) 
	{
		getline(cin, msg1);
		size_t msg_size = msg1.size();

		if(Closing(4,send(Connection, (char*)&msg_size, sizeof(int), 0)))
			return 1;
		
		if (Closing(4, send(Connection, msg1.c_str(), msg_size, 0)))
			return 1;

		Sleep(10);
	}

	freeaddrinfo(addrrez);
	WSACleanup();

	return 0;
}