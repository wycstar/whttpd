#include "def.h"

STATUS_CODE parseRequest(char header[], REQUEST &request, int clientFD);
void initNet(int &FD, bool isDynamicPort, int &port);
void makeResponseHeader(int code,int &clientFD, REQUEST request, int contentLen);

inline void DISPLAY_ERROR(const char *tips)
{
    cout << tips << endl;
    exit(1);
}

inline void closeSocket(int socket)
{
	shutdown(socket, SHUT_RDWR);
	close(socket);
	socket = -1;
}

STATUS_CODE parseRequest(char header[], REQUEST &request, int clientFD)
{
    istringstream headerString(header);
    string line;
    size_t methodEnd = 0;
    getline(headerString, line);
    if(string::npos != (methodEnd = line.find_first_of(' '))){
        request.method = line.substr(0, methodEnd);
        size_t pathEnd = 0;
        if(string::npos != (pathEnd = line.find_last_of('/'))){
            request.uri = line.substr(methodEnd + 1, pathEnd - methodEnd - 6);
            request.version = line.substr(pathEnd + 1, line.size() - pathEnd - 2);
            while (getline(headerString, line)){
                if (string::npos != line.find(":")){
                    request.header.insert(make_pair(line.substr(0, line.find_first_of(":")), line.substr(line.find(":") + 2)));}}}
        else{
            makeResponseHeader(400, clientFD, request, 0);}}
    else{
        makeResponseHeader(400, clientFD, request, 0);}
    return 1;
}

void makeResponseHeader(int code,int &clientFD, REQUEST request, int contentLen)
{
    stringstream header;
    switch(code)
    {
    case 200:{
        int cutLen = request.header["Accept"].find_first_of(',');
        header << "HTTP/" << request.version << "200 OK\r\n"
               << "Content-Type: " << request.header["Accept"].substr(0, cutLen) << "\r\n"
               << "Content-Length: " << contentLen << "\r\n"
               << "\r\n";
        send(clientFD, header.str().c_str(), header.str().length(), 0);
        header.flush();}
        break;
    case 400:{
        const char* content = "<h1>Bad Request!</h1>";
        header << "HTTP/" << request.version << "Bad Request\r\n"
               << "Content-Type: text/html\r\n"
               << "Content-Length: " << strlen(content) << "\r\n"
               << "\r\n"
               << content;
        send(clientFD, header.str().c_str(), header.str().length(), 0);
        header.flush();
        closeSocket(clientFD);}
        break;
    case 404:{
        const char* content = "<h1>404 Not Found!</h1>";
        header << "HTTP/" << request.version << "404 NOT FOUND\r\n"
               << "Content-Type: text/html\r\n"
               << "Content-Length: " << strlen(content) << "\r\n"
               << "\r\n"
               << content;
        send(clientFD, header.str().c_str(), header.str().length(), 0);
        header.flush();
        closeSocket(clientFD);}
        break;
    default:
        return;
    }
    return;
}

void initNet(int &FD, bool isDynamicPort, int &port)
{
    int isReuse = 1;
    FD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(-1 == FD){
        DISPLAY_ERROR("Invalid Socket!");}
    setsockopt(FD, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char *>(&isReuse), sizeof(int));
    sockaddr_in name;
    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    if (-1 == bind(FD, reinterpret_cast<sockaddr *>(&name), sizeof(sockaddr_in)))
        DISPLAY_ERROR("BIND ERROR!");
    if(isDynamicPort){
        int addrLen = sizeof(sockaddr_in);
        if(-1 == getsockname(FD, (struct sockaddr *)&name, &addrLen))
            DISPLAY_ERROR("Get Sock Name ERROR!");
        port = ntohs(name.sin_port);}
    if (-1 == listen(FD, 50000))
        DISPLAY_ERROR("LISTEN ERROR!");
}

void sendResponse(REQUEST &request, int &clientFD)
{
    struct stat st;
    stringstream ss;
    int readLenth;
    ss << basePath << (strcmp(request.uri.c_str(), "/") != 0 ? request.uri : "/index.html");
    cout << ss.str().c_str() << endl;
    ifstream file(ss.str().c_str());
    if (!file.is_open()){
        makeResponseHeader(404, clientFD, request, 0);
        return;}
    stat(ss.str().c_str(), &st);
    makeResponseHeader(200, clientFD, request, st.st_size);
    char *sendBuf = new char[128 * 1024]();
    while((readLenth = (file.read(sendBuf, 128*1024)).gcount()) > 0){
        send(clientFD, sendBuf, readLenth, 0);}
    if(file.is_open())
        file.close();
    delete [] sendBuf;
    closeSocket(clientFD);
    return;
}

int fork(int clientFD)
{
	char *recvBuf = new char[REQUEST_SIZE]();
	std::fill(recvBuf, recvBuf + REQUEST_SIZE, 0);
	recv(clientFD, recvBuf, REQUEST_SIZE, 0);
	REQUEST request;
	parseRequest(recvBuf, request, clientFD);
	delete [] recvBuf;
	sendResponse(request, clientFD);
	return OK;
}

int main()
{
    int FD = 0;
    int port = 15000;
    initNet(FD, false, port);
    sockaddr_in clientName;
    int *clientFD = new int[MAX_CLIENT];
    std::fill_n(clientFD, MAX_CLIENT, -1);
    int clientNameLen = sizeof(clientName);
    int clientID = 0;
    while (1)
    {
       
        if (-1 == (clientFD[clientID] = accept(FD,
											   reinterpret_cast<sockaddr *>(&clientName),
											   &clientNameLen)))
            DISPLAY_ERROR("ACCEPT ERROR!\n");
        taskSpawn("tForkProcess", 150, VX_FP_TASK, 20000, (FUNCPTR)fork, clientFD[clientID], 0, 0, 0, 0, 0, 0, 0, 0, 0);
        clientID = (++clientID) < 1000 ? clientID : 0;
    }
}
