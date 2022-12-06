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
#define PORT 6000
#define MAXSERVERS 8
#define SIZE 8192

using namespace std;

struct ServerData
{
    string servicename;
    string servicetype;
    string ipaddress;
    string portnum;
    string accesstoken;
};

struct sockaddr_in srv, clnt;
ServerData serverData[MAXSERVERS];
int servNums = 0;

//finds the substring between ":" and ","
string tokens(string msg)
{
    int index1 = msg.find_first_of(":");
    int index2 = msg.find_first_of(",");

    string ans = msg.substr(index1 + 1, index2 - index1 - 1);

    return ans;
}

//parses the message and finds details of the request
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

//if the request is from client, send the details of all the fileservers to it
void processingClient(int sockFd)
{
    cout<<" Processing Client !!"<<endl;
    char client_buffer[SIZE];
    memset(&(client_buffer), 0, SIZE);

    struct ServerData *ptr = serverData;
    int i = 0;
    string str = "";
    while ((i++) < servNums && ptr != NULL)
    {
        str += "servicename:" + ptr->servicename + ",servicetype:" + ptr->servicetype + ",ipaddress:" + ptr->ipaddress + ",portnumber:" + ptr->portnum + ",serviceaccesstoken:" + ptr->accesstoken + "$";
        ptr++;
    }
    strcpy(client_buffer, str.c_str());
    int structLen = sizeof(clnt);
    // sendto
    int newSock = sendto(sockFd, client_buffer, strlen(client_buffer), 0, (const sockaddr *)&clnt, structLen);
    cout << "newSock = " << newSock;
}

//prints the details of the fileservers
void printServerDataInfo()
{
    struct ServerData *ptr = serverData;
    int i = 0;
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

int main()
{
    int nRet = 0;

    int sockFd, newSock;
    struct sockaddr_in server_addr, new_addr;
    int addr_size = sizeof(struct sockaddr);

    char buff[SIZE];

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

    // create UDP socket
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

    // struct sockaddr_in srv, clnt;
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    srv.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(&(srv.sin_zero), 0, 8);

    int srvLen = sizeof(srv);
    // binding socket to address
    nRet = bind(sockFd, (sockaddr *)&srv, srvLen);
    if (nRet < 0)
    {
        cout << "failed to bind to local port" << WSAGetLastError() << endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << "successfully binded to local port" << endl;
    }

    while (true)
    {
        int clntLen = sizeof(clnt);
        memset(buff, 0, SIZE);
        newSock = recvfrom(sockFd, buff, SIZE, 0, (struct sockaddr *)&clnt, &clntLen);

        // if request is from client 
        string str = "requesting to Bootstrap server";
        if (strcmp(buff, str.c_str()) == 0) 
        {
            cout << "Buffer = " << buff<<endl;
            processingClient(sockFd);

        }
        else
        {
            // parsemsg
            parseMsg(string(buff));

            printServerDataInfo();
        }
    }
    return 0;
}
