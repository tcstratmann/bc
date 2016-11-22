#include "MergeToolSocketWin.h"
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

const int DEFAULT_PORT = 5006;


MergeToolSocketWin::MergeToolSocketWin()
{
    //ctor
    printf("MergeToolSocketWin init\n");
    send();
}

MergeToolSocketWin::~MergeToolSocketWin()
{
    //dtor
}

 // Microsoft Development Environment 2003 - Version 7.1.3088

// Copyright (r) 1987-2002 Microsoft Corporation. All Right Reserved

// Microsoft .NET Framework 1.1 - Version 1.1.4322

// Copyright (r) 1998-2002 Microsoft Corporation. All Right Reserved

//

// Run on Windows XP Pro machine, version 2002, SP 2

//

// <windows.h> already included

// WINVER = 0x0501 for Xp already defined in windows.h

// Server program, using TCP


void MergeToolSocketWin::send()

{

WORD wVersionRequested;

WSADATA wsaData;

int wsaerr;



// Using MAKEWORD macro, Winsock version request 2.2

wVersionRequested = MAKEWORD(2, 2);



wsaerr = WSAStartup(wVersionRequested, &wsaData);

if (wsaerr != 0)

{

    /* Tell the user that we could not find a usable WinSock DLL.*/

    printf("Server: The Winsock dll not found!\n");

    exit(0);

}

else

{

       printf("Server: The Winsock dll found!\n");

       printf("Server: The status: %s.\n", wsaData.szSystemStatus);

}



/* Confirm that the WinSock DLL supports 2.2.*/

/* Note that if the DLL supports versions greater    */

/* than 2.2 in addition to 2.2, it will still return */

/* 2.2 in wVersion since that is the version we      */

/* requested.                                        */

if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2 )

{

    /* Tell the user that we could not find a usable WinSock DLL.*/

    printf("Server: The dll do not support the Winsock version %u.%u!\n", LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));

    WSACleanup();

    exit(0);

}

else

{

       printf("SServer: The dll supports the Winsock version %u.%u!\n", LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));

       printf("Server: The highest version this dll can support: %u.%u\n", LOBYTE(wsaData.wHighVersion), HIBYTE(wsaData.wHighVersion));

}



//////////Create a socket////////////////////////

//Create a SOCKET object called m_socket.

SOCKET m_socket;



// Call the socket function and return its value to the m_socket variable.

// For this application, use the Internet address family, streaming sockets, and

// the TCP/IP protocol.

// using AF_INET family, TCP socket type and protocol of the AF_INET - IPv4

m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);



// Check for errors to ensure that the socket is a valid socket.

if (m_socket == INVALID_SOCKET)

{

    printf("Server: Error at socket(): %ld\n", WSAGetLastError());

    WSACleanup();

    exit(0);

}

else

{ printf("Server: socket() is OK!\n"); }



////////////////bind//////////////////////////////

// Create a sockaddr_in object and set its values.

sockaddr_in service;



// AF_INET is the Internet address family.

service.sin_family = AF_INET;

// "127.0.0.1" is the local IP address to which the socket will be bound.

service.sin_addr.s_addr = inet_addr("127.0.0.1");

// 55555 is the port number to which the socket will be bound.

service.sin_port = htons(DEFAULT_PORT);



// Call the bind function, passing the created socket and the sockaddr_in structure as parameters.

// Check for general errors.

if (bind(m_socket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)

{

    printf("Server: bind() failed: %ld.\n", WSAGetLastError());

    closesocket(m_socket);

    exit(0);

}

else

{

    printf("Server: bind() is OK!\n");

}



// Call the listen function, passing the created socket and the maximum number of allowed

// connections to accept as parameters. Check for general errors.

if (listen(m_socket, 10) == SOCKET_ERROR)

    printf("Server: listen(): Error listening on socket %ld.\n", WSAGetLastError());

else

{

   printf("Server: listen() is OK, I'm waiting for connections...\n");

}



// Create a temporary SOCKET object called AcceptSocket for accepting connections.

SOCKET AcceptSocket;



// Create a continuous loop that checks for connections requests. If a connection

// request occurs, call the accept function to handle the request.

printf("Server: Waiting for a client to connect...\n" );

printf("***Hint: Server is ready...run your client program...***\n");

// Do some verification...

while (1)

{

    AcceptSocket = SOCKET_ERROR;

    while (AcceptSocket == SOCKET_ERROR)

    {

        AcceptSocket = accept(m_socket, NULL, NULL);

    }



// else, accept the connection...

// When the client connection has been accepted, transfer control from the

// temporary socket to the original socket and stop checking for new connections.

printf("Server: Client Connected!\n");

m_socket = AcceptSocket;

break;

}



int bytesSent;

int bytesRecv = SOCKET_ERROR;

char sendbuf[200] = "This string is a test data from server";

// initialize to empty data...

char recvbuf[200] = "";



// Send some test string to client...

printf("Server: Sending some test data to client...\n");

bytesSent = write(m_socket, sendbuf, strlen(sendbuf));



if (bytesSent == SOCKET_ERROR)

       printf("Server: send() error %ld.\n", WSAGetLastError());

else

{

       printf("Server: send() is OK.\n");

       printf("Server: Bytes Sent: %ld.\n", bytesSent);

}



// Receives some test string from client...and client

// must send something lol...

bytesRecv = recv(m_socket, recvbuf, 200, 0);



if (bytesRecv == SOCKET_ERROR)

       printf("Server: recv() error %ld.\n", WSAGetLastError());

else

{

       printf("Server: recv() is OK.\n");

       printf("Server: Received data is: \"%s\"\n", recvbuf);

       printf("Server: Bytes received: %ld.\n", bytesRecv);

}



WSACleanup();

exit(0);

}
