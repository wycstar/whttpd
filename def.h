#ifndef BASE_H
#define BASE_H
#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <sys/stat.h>
#include <string.h>
#include <sockLib.h>
#include <netinet/in.h>
#include <dirent.h>
#include <unistd.h>
#include <taskLib.h>

#define STATUS_CODE int
#define STATUS      int

#define MAX_CLIENT 1000
#define REQUEST_SIZE 10000

typedef unsigned short WORD;
typedef unsigned char  BYTE;
typedef unsigned int   UINT;

#define SERVER_STRING "Server: A Low Performance Http Server By WYC 0.1.0\r\n"

using std::ifstream;
using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::istringstream;
using std::getline;
using std::map;
using std::make_pair;
using std::stringstream;

typedef struct _tREQUEST
{
    string method;
    string uri;
    string version;
    map<string, string> header;
    string content;
}REQUEST;

const char* basePath = "/ata00:2/returnContent";

#endif

