#include <iostream>
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BACKLOG 5
#define SIZE 1024

using namespace std;

int writeFile(int sockFd, char *filename, char length[])
{
     char buffer[SIZE];
    
    FILE *fp = fopen(filename, "wb");

    cout << "filename =  " << filename << endl;
    int fileSize = atoi(length);                //converting char to int

    int totalBytesRcvd = 0;
    cout << "fileSize =  " << fileSize << endl;

    //recv the bytes until it reaches filesize
    while (totalBytesRcvd < fileSize)
    {
        memset(&buffer, 0, SIZE);

        // recv() to receive contents of file
        int bytesRcvd = recv(sockFd, buffer, SIZE, 0);

        totalBytesRcvd += bytesRcvd;
        cout << "totalBytesRcvd = " << totalBytesRcvd << endl;
        cout << "bytesRcvd = " << bytesRcvd << endl;

        // writing contents to new file
        int bytesWritten = fwrite(buffer, 1, bytesRcvd, fp);
    }

    fclose(fp);
    return 1;
}

int main()
{
    char *ip = "127.0.0.1";
    int port = 8004;
    int nRet;

    int sockFd;
    struct sockaddr_in server_addr;

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

    struct sockaddr_in clnt;
    clnt.sin_family = AF_INET;
    clnt.sin_port = htons(port);
    clnt.sin_addr.s_addr = inet_addr(ip);
    memset(&(clnt.sin_zero), 0, 8);

    // establish a connection between server & client
    nRet = connect(sockFd, (struct sockaddr *)&clnt, sizeof(clnt));
    if (nRet == -1)
    {
        cout << "Error in connecting" << endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    else
    {
        cout << "Connected to server" << endl;
        char buffer[SIZE];
        memset(&buffer, '\0', SIZE);

        // recv() for message saying request is getting processed
        recv(sockFd, buffer, SIZE, 0);
        cout << "Message received from server is : " << buffer << endl;

        cout << "Enter filepath for the request object: " << endl;
        fgets(buffer, SIZE, stdin);

        cout << "Buff after receiving filepath= " << buffer << endl;

        int bufferLen = strlen(buffer);

        // the below send call will send file name to server required by client
        send(sockFd, buffer, strlen(buffer), 0);
        char filename[SIZE];
        buffer[strlen(buffer) - 1] = '\0';
        strcpy(filename, buffer);                       // copying requested object name from buffer to filename

        memset(&buffer, 0, SIZE);

        // recv() to know size of file
        int retVal = recv(sockFd, buffer, SIZE, 0);
        cout << "retval after receiving size of file : " << retVal << endl;
        cout << "Length of the file = " << buffer << endl;

        //function for receiving file from server
        retVal = writeFile(sockFd, filename, buffer);
    }
}