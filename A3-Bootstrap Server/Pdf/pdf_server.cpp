#include <iostream>
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <fstream>
#include <vector>
#include <windows.h>
#include <tchar.h>

#define BOOTPORT 6000
#define PORT 8006
#define BACKLOG 5
#define SIZE 8192

using namespace std;

// sendfile
int sendFile(FILE *fp, int newSock)
{
    // To send the file size to client
    // (so that once it receives that much number of bytes it can close writing to file)

    fseek(fp, 0, SEEK_END); // pointing to the end of file using fseek()
    const long fileSize = ftell(fp);
    cout << "Filesize (in Bytes) = " << fileSize << endl;

    // bring the fp to beginning of file
    rewind(fp);

    // converting to char array from integer because buffer stores char
    int n = fileSize;
    int i = 0;
    string temp_str = to_string(n); // converting number to a string
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
        // cout << "" << endl;
        //  read the contents of file using fread()
        int bytesRead = fread(writeBuffer, 1, buffSize, fp);

        // sending file to client
        int retVal = send(newSock, writeBuffer, bytesRead, 0);
        // cout << "retVal after sending file to client = " << retVal << endl;
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

// function that reads the contents of the directory
void open_directory(const string &name, vector<string> &v)
{
    string pattern(name);
    pattern.append("\\*");
    WIN32_FIND_DATA data;
    HANDLE handleFind;
    if ((handleFind = FindFirstFile(_T(pattern.c_str()), &data)) != INVALID_HANDLE_VALUE)
    {
        do
        {
            v.push_back(data.cFileName);
        } while (FindNextFile(handleFind, &data) != 0);
        FindClose(handleFind);
    }
}
// processing client's request
void processRequest(int clnSockFd)
{
    char buffer[SIZE] = {0};
    memset(buffer, 0, SIZE);
    FILE *fp;

    // in below recv(), filename of the requested object is received.
    int retVal = recv(clnSockFd, buffer, SIZE, 0);
    if (retVal < 0)
    {
        cout << "Error occured" << WSAGetLastError() << endl;
        exit(0);
    }
    else
    {
        cout << "New Message received from client is: " << buffer;

        // appending a prefix message to differentiate the requests from client
        string msg = "SENDFILE";

        // getting access token
        string bufferContents = string(buffer);
        string atoken = bufferContents.substr(bufferContents.find_first_of(",") + 1, bufferContents.length());
        cout << "atoken = " << atoken << endl;

        if (strncmp("SENDFILE", buffer, 8) == 0)
        {
            if ((strcmp("efgh", atoken.c_str()) == 0))
            {
                // if strcmp ==0 it means request is obtained from client, so now open and read the files from directory
                vector<string> extensions;
                open_directory(".", extensions);

                // appending all the available files to a string
                string files = "";
                for (int i = 0; i < extensions.size(); i++)
                {
                    // access only .mp4 files and store it in a string called "files"

                    string substring = extensions[i].substr(extensions[i].find(".") + 1, extensions[i].length());
                    if (strcmp(substring.c_str(), "pdf") == 0)
                    {
                        files += extensions[i];
                        // if it is the last file then "," should not be there
                        if (i != extensions.size() - 1)
                        {
                            files += ",";
                        }
                    }
                }

                // send the string to client by copying it to the buffer
                strcpy(buffer, files.c_str());
                retVal = send(clnSockFd, buffer, strlen(buffer), 0);
                cout << " buffer having file names : " << buffer << endl;
                if (retVal <= 0)
                {
                    cout << "Error in sending" << endl;
                    WSACleanup();
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                cout << " Invalid client, as access tokens are not matched " << endl;
            }
        }
        else
        {
            // filepath received so send the file
            // read the contents of the file
            fp = fopen(buffer, "rb");
            if (fp == NULL)
            {
                cout << "File does not exist" << endl;
                perror("fopen");
            }
            memset(&buffer, 0, SIZE);
            sendFile(fp, clnSockFd);
        }
    }
}

int main()
{
    int nRet = 0;
    int sockFd, newSock, sockFdTcp;
    struct sockaddr_in server_addr, new_addr;
    int addr_size = sizeof(struct sockaddr);

    // char buff[SIZE];

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
    sockFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
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
    srv.sin_port = htons(BOOTPORT);
    srv.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(&(srv.sin_zero), 0, 8);

    // sending fileserver details to bootstrap server
    string fServer = "servicename:PDFserver,servicetype:pdf,ipaddress:127.0.0.1,portnum:8006,serviceaccesstoken:efgh";
    char buffer[SIZE] = {0};
    strcpy(buffer, fServer.c_str()); // converting fServer to a constant char array and copying it to the buffer

    int srvLen = sizeof(srv);
    newSock = sendto(sockFd, buffer, strlen(buffer), 0, (const sockaddr *)&srv, srvLen);
    cout << "newSock = " << newSock;

    // create socket for TCP connection
    sockFdTcp = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFdTcp < 0)
    {
        cout << "Error in TCP socket creation" << endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << "TCP socket created successfully with id : " << sockFdTcp << endl;
    }

    // new structure for connecting clients
    struct sockaddr_in srv1;
    srv1.sin_family = AF_INET;
    srv1.sin_port = htons(PORT);
    srv1.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(&(srv1.sin_zero), 0, 8);

    // binding socket to address
    nRet = bind(sockFdTcp, (sockaddr *)&srv1, sizeof(sockaddr));
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

    // structure for connecting to bootport
    //  struct sockaddr_in srv;
    srv.sin_family = AF_INET;
    srv.sin_port = htons(BOOTPORT);
    srv.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(&(srv.sin_zero), 0, 8);

    // listen to connections
    nRet = listen(sockFdTcp, BACKLOG);
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

    memset(buffer, 0, SIZE);
    int clnAddrLen = sizeof(struct sockaddr);

    // accepting client's request
    int clnSockFd = accept(sockFdTcp, NULL, &clnAddrLen);
    cout << "Accepted Connection " << clnSockFd << endl;
    const char *message = "Processing request";

    if (clnSockFd < 0)
    {
        cout << "Error in accepting the connection" << endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    // sending ack to client via msg informing that request is processed
    send(clnSockFd, message, strlen(message), 0);
    while (true)
    {
        // process the client's request
        processRequest(clnSockFd);
    }

    return 0;
}