#include <iostream>
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#define BACKLOG 5
#define PORT 9000
#define BOOTPORT 6000
#define SIZE 1024
#define MAXSERVERS 8
#define IPADDR "127.0.0.1"

using namespace std;

int servNums = 0;
struct sockaddr_in videosrv, pdfsrv, imagesrv, textsrv;

struct ServerData
{
    string servicename;
    string servicetype;
    string ipaddress;
    string portnum;
    string accesstoken;
};

struct sockaddr_in srv, clnt;
int sockVideo, sockPdf, sockImage, sockText;
ServerData serverData[MAXSERVERS];

// function to find tokens
string tokens(string msg)
{
    int index1 = msg.find_first_of(":");
    int index2 = msg.find_first_of(",");

    string ans = msg.substr(index1 + 1, index2 - index1 - 1);

    return ans;
}

//
void parseMsg(string msg)
{
    string servicename = tokens(msg);
    msg = msg.substr(msg.find_first_of(",") + 1, msg.length());

    string servicetype = tokens(msg);
    msg = msg.substr(msg.find_first_of(",") + 1, msg.length());

    string ipaddress = tokens(msg);
    msg = msg.substr(msg.find_first_of(",") + 1, msg.length());

    string portnum = tokens(msg);
    msg = msg.substr(msg.find_first_of(",") + 1, msg.length());

    string accesstoken = tokens(msg);
    msg = msg.substr(msg.find_first_of(",") + 1, msg.length());

    serverData[servNums].servicename = servicename;
    serverData[servNums].servicetype = servicetype;
    serverData[servNums].ipaddress = ipaddress;
    serverData[servNums].portnum = portnum;
    serverData[servNums].accesstoken = accesstoken;
    servNums++;
}

// tokenize at the deliminator "$"
void fillServerData(char buffer[])
{
    char *ptr = strtok(buffer, "$");
    while (ptr != NULL)
    {
        parseMsg(ptr);
        ptr = strtok(NULL, "$");
    }
}

// write contents of the file
int writeFile(int sockFd, char *filename, char length[])
{
    char buffer[SIZE] = {0};

    FILE *fp = fopen(filename, "wb");

    cout << "filename =  " << filename << endl;
    int fileSize = atoi(length); // converting char to int

    int totalBytesRcvd = 0;
    cout << "fileSize =  " << fileSize << endl;

    // recv the bytes until it reaches filesize
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

// function to print fileserver information
void printServerInfo()
{
    struct ServerData *ptr = serverData;
    int i = 0;
    cout << "servNums = " << servNums << endl;
    while ((i++) < servNums && ptr != NULL)
    {
        cout << endl;
        cout << "servicename:" << ptr->servicename << endl;
        cout << "servicetype:" << ptr->servicetype << endl;
        cout << "ipaddress:" << ptr->ipaddress << endl;
        cout << "portnumber:" << ptr->portnum << endl;
        cout << "serviceaccesstoken:" << ptr->accesstoken << endl;
        ptr++;
    }
}

// process the response sent by fileserver
void FileServerResponse(char answer[], ServerData *ptr)
{
    vector<string> fileList;
    cout << " Response obtained from sender is :" << answer << endl;
    // tokenize at "," to obtain the list of files and push it to the vector(which stores list of files in a directory)
    char *p = strtok(answer, ",");
    while (p != NULL)
    {
        fileList.push_back(string(p));
        p = strtok(NULL, ",");
    }

    // storing the list of files in a pair along with its service type
    pair<string, vector<string>> list;
    list.first = ptr->servicetype;
    list.second = fileList;

    //if the access token did not match, output "Invalid Client" message
    if (strcmp(answer, "Invalid Client"))
        cout << "Invalid Client as the access tokens do not match " << endl;
        
    //  if access tokens are matched, print the list of files
    else
    {
        cout << "The files of " << list.first << "are : " << endl;
        for (int i = 0; i < fileList.size(); i++)
        {
            cout << fileList[i] << endl;
        }
    }

    // among the list of files, choose a particular servicetype only
    char buffer[SIZE] = {0};
    memset(buffer, 0, SIZE);
    for (int i = 0; i < fileList.size(); i++)
    {
        if (list.first.compare("video") == 0)
        {
            string fName = list.second[0];
            strcpy(buffer, fName.c_str());
            int newSock = send(sockVideo, buffer, strlen(buffer), 0);
            if (newSock <= 0)
            {
                cout << "Error in sending video file " << WSAGetLastError() << endl;
                WSACleanup();
                exit(EXIT_FAILURE);
            }
            char fileName[SIZE] = {0};
            strcpy(fileName, buffer);
            newSock = recv(sockVideo, buffer, SIZE, 0);
            writeFile(sockVideo, fileName, buffer);
        }
        else if (list.first.compare("pdf") == 0)
        {
            string fName = list.second[0];
            strcpy(buffer, fName.c_str());
            int newSock = send(sockPdf, buffer, strlen(buffer), 0);
            if (newSock <= 0)
            {
                cout << "Error in sending pdf file " << WSAGetLastError() << endl;
                WSACleanup();
                exit(EXIT_FAILURE);
            }

            char fileName[SIZE] = {0};
            memset(fileName, 0, SIZE);
            strcpy(fileName, buffer);
            newSock = recv(sockPdf, buffer, SIZE, 0);
            writeFile(sockPdf, fileName, buffer);
        }

        else if (list.first.compare("image") == 0)
        {
            string fName = list.second[0];
            strcpy(buffer, fName.c_str());
            int newSock = send(sockImage, buffer, strlen(buffer), 0);
            if (newSock <= 0)
            {
                cout << "Error in sending image file " << WSAGetLastError() << endl;
                WSACleanup();
                exit(EXIT_FAILURE);
            }
            char fileName[SIZE] = {0};
            memset(fileName, 0, SIZE);
            strcpy(fileName, buffer);
            newSock = recv(sockImage, buffer, SIZE, 0);
            writeFile(sockImage, fileName, buffer);
        }

        else if (list.first.compare("text") == 0)
        {
            string fName = list.second[0];
            strcpy(buffer, fName.c_str());
            int newSock = send(sockText, buffer, strlen(buffer), 0);
            if (newSock <= 0)
            {
                cout << "Error in sending text file " << WSAGetLastError() << endl;
                WSACleanup();
                exit(EXIT_FAILURE);
            }
            char fileName[SIZE] = {0};
            memset(fileName, 0, SIZE);
            strcpy(fileName, buffer);
            newSock = recv(sockText, buffer, SIZE, 0);
            writeFile(sockText, fileName, buffer);
        }
    }
}

// creating socket to connect fileservers via TCP
int createSockets(struct sockaddr_in srv, ServerData *ptr)
{
    int sockFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
    clnt.sin_port = htons(stoi(ptr->portnum));
    clnt.sin_addr.s_addr = inet_addr(ptr->ipaddress.c_str());
    memset(&(clnt.sin_zero), 0, 8);

    // connecting with fileservers
    int nRet = connect(sockFd, (struct sockaddr *)&clnt, sizeof(clnt));
    if (nRet == -1)
    {
        cout << "Error in connecting" << endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else
    {
        char buffer[SIZE] = {0};
        nRet = recv(sockFd, buffer, SIZE, 0);
        cout << "buffer : " << buffer;
        cout << "Successfully connected to " << ptr->servicename << endl;
    }
    return sockFd;
}

//
int main()
{

    int nRet;
    int sockFd, sockFdTcp;
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

    // create socket for Bootstrapserver communicating via UDP
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

    struct sockaddr_in clnt;
    clnt.sin_family = AF_INET;
    clnt.sin_port = htons(BOOTPORT);
    clnt.sin_addr.s_addr = inet_addr(IPADDR);
    memset(&(clnt.sin_zero), 0, 8);

    // sending a small message to BootstrapServer indicating that the request is from client
    string msg = "requesting to Bootstrap server";
    char buffer[SIZE] = {0};
    strcpy(buffer, msg.c_str()); // converting fServer to a constant char array and copying it to the buffer

    int clntLen = sizeof(clnt);
    int newSock = sendto(sockFd, buffer, strlen(buffer), 0, (struct sockaddr *)&clnt, clntLen);
    // cout << "newSock = " << newSock;
    memset(buffer, 0, SIZE);
    newSock = recvfrom(sockFd, buffer, SIZE, 0, (struct sockaddr *)&clnt, &clntLen);
    // cout << "buffer = " << buffer;
    fillServerData(buffer);
    cout << endl;
    printServerInfo();

    // creating sockets for all 4 fileservers
    struct ServerData *ptr = serverData;
    int i = 0;

    while ((i++) < servNums && ptr != NULL)
    {
        newSock = recv(sockVideo, buffer, SIZE, 0);
        if (strcmp(ptr->servicename.c_str(), "Videoserver") == 0)
        {
            sockVideo = createSockets(videosrv, ptr);
            // sending access token to the file server via buffer
            string prefix = "SENDFILE,abcd";
            char buffer[SIZE] = {0};
            strcpy(buffer, prefix.c_str());
            newSock = send(sockVideo, buffer, strlen(buffer), 0);

            // receive
            memset(buffer, 0, SIZE);
            newSock = recv(sockVideo, buffer, SIZE, 0);
            if (newSock <= 0)
            {
                cout << " Error in receiving files :" << endl;
                WSACleanup();
                exit(EXIT_FAILURE);
            }

            // parsing server's response
            FileServerResponse(buffer, ptr);
        }
        else if (strcmp(ptr->servicename.c_str(), "PDFserver") == 0)
        {
            sockPdf = createSockets(pdfsrv, ptr);
            // sockVideo = createSockets(videosrv, ptr);
            string prefix = "SENDFILE,efgh";
            char buffer[SIZE] = {0};
            strcpy(buffer, prefix.c_str());
            newSock = send(sockPdf, buffer, strlen(buffer), 0);

            // receive
            memset(buffer, 0, SIZE);
            newSock = recv(sockPdf, buffer, SIZE, 0);
            if (newSock <= 0)
            {
                cout << " Error in receiving files :" << endl;
                WSACleanup();
                exit(EXIT_FAILURE);
            }

            // parsing server's response
            FileServerResponse(buffer, ptr);
        }
        else if (strcmp(ptr->servicename.c_str(), "Imageserver") == 0)
        {
            sockImage = createSockets(imagesrv, ptr);
            // sockVideo = createSockets(videosrv, ptr);
            string prefix = "SENDFILE,ijkl";
            char buffer[SIZE] = {0};
            strcpy(buffer, prefix.c_str());
            newSock = send(sockImage, buffer, strlen(buffer), 0);

            // receive
            memset(buffer, 0, SIZE);
            newSock = recv(sockImage, buffer, SIZE, 0);
            if (newSock <= 0)
            {
                cout << " Error in receiving files :" << endl;
                WSACleanup();
                exit(EXIT_FAILURE);
            }

            // parsing server's response
            FileServerResponse(buffer, ptr);
        }
        else if (strcmp(ptr->servicename.c_str(), "Textserver") == 0)
        {
            sockText = createSockets(textsrv, ptr);
            // sockVideo = createSockets(videosrv, ptr);
            string prefix = "SENDFILE,mnop";
            char buffer[SIZE] = {0};
            strcpy(buffer, prefix.c_str());
            newSock = send(sockText, buffer, strlen(buffer), 0);

            // receive
            memset(buffer, 0, SIZE);
            newSock = recv(sockText, buffer, SIZE, 0);
            if (newSock <= 0)
            {
                cout << " Error in receiving files :" << endl;
                WSACleanup();
                exit(EXIT_FAILURE);
            }

            // parsing server's response
            FileServerResponse(buffer, ptr);
        }
        ptr++;
    }
    return 0;
}