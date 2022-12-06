#include <iostream>
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#define BACKLOG 5
#define SIZE 8192

using namespace std;

// sendfile
int sendFile(FILE *fp, int newSock)
{
    // To send the file size to client
    // (so that once it receives that much number of bytes it can close writing to file)

    fseek(fp, 0, SEEK_END);                           // pointing to the end of file using fseek()
    const long fileSize = ftell(fp);
    cout << "Filesize (in Bytes) = " << fileSize << endl;

    // bring the fp to beginning of file
    rewind(fp);

    // converting to char array from integer because buffer stores char
    int n = fileSize;
    int i = 0;
    string temp_str = to_string(n);                    // converting number to a string
    char const *number_array = temp_str.c_str();
    cout << "number_array = " << number_array << endl;

    //(sending the size of file to client)
    int retVal = send(newSock, number_array, strlen(number_array), 0);
    cout << "retVal after sending filesize to client= " << retVal << endl;
    int buffSize = 59353;
    char writeBuffer[59353] = {0};

    // read contents of file until it reaches eof
    while (!feof(fp))
    {
        cout << "" << endl;
        // read the contents of file using fread()
        int bytesRead = fread(writeBuffer, 1, buffSize, fp);

        // sending file to client
        int retVal = send(newSock, writeBuffer, bytesRead, 0);
        cout << "retVal after sending file to client = " << retVal << endl;
        memset(&writeBuffer, 0, bytesRead);
        if (retVal < 0)
        {
            cout << "Error in sending file" << endl;
        }
        Sleep(50);
    }
    cout << "Requested file sent to client: " << newSock << endl;
    fclose(fp);
    return 1;
}

void processRequest(int sockFd)
{
    char buffer[SIZE];
    int clnAddrLen = sizeof(struct sockaddr);

    // accepting client's request
    int clnSockFd = accept(sockFd, NULL, &clnAddrLen);
    const char *message = "Processing request";

    if (clnSockFd < 0)
    {
        cout << "Error in accepting the connection" << endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else
    {
        // sending ack to client via msg informing that request is processed
        send(clnSockFd, message, strlen(message), 0);
        FILE *fp;

        // in below recv(), filename of the requested object is received.
        int retVal = recv(clnSockFd, buffer, SIZE, 0);
        if (retVal < 0)
        {
            cout << "Error occured" << endl;
        }
        else
        {
            cout << "New Message received from client is: " << buffer;
            buffer[strlen(buffer) - 1] = '\0';

            // read the contents of the file
            fp = fopen(buffer, "rb");
            if (fp == NULL)
            {
                cout << "File does not exist";
                perror("fopen");
            }
            memset(&buffer, 0, SIZE);
            sendFile(fp, clnSockFd);
        }
    }
}

int main()
{
    
    int port = 8004;
    int nRet = 0;

    int sockFd, newSock;
    struct sockaddr_in server_addr, new_addr;
    int addr_size = sizeof(struct sockaddr);

    char buff[SIZE];

    //
    WSADATA ws;

    if (WSAStartup(MAKEWORD(2, 2), &ws) < 0)
    {
        cout << "WSA Startup Failed" << endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << "WSA Startup Initialised" << endl;
    }

    // create socket
    sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd < 0)
    {
        cout << "Error in socket creation" << endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << "Socket created successfully with id : " << sockFd << endl;
    }

    struct sockaddr_in srv;
    srv.sin_family = AF_INET;
    srv.sin_port = htons(port);
    srv.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(&(srv.sin_zero), 0, 8);

    // binding socket to address
    nRet = bind(sockFd, (sockaddr *)&srv, sizeof(sockaddr));
    if (nRet < 0)
    {
        cout << "failed to bind to local port" << endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << "successfully binded to local port" << endl;
    }

    // listen to connections
    nRet = listen(sockFd, BACKLOG);
    if (nRet < 0)
    {
        cout << "Failed to listen" << endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << "started listening to local port" << endl;
    }

    processRequest(sockFd);

    return 0;
}
