#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> //write 있음close랑
#include <fcntl.h> //O_READONLY,O_WRONLY,O_RDWR
#include <mutex>
#include <thread>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <memory.h> //memset? string에 있지않나
#include <istream>
#include <fstream>
#include <signal.h>
// 얘가 multiserver.cpp

#define MAX_LENGTH_FILE_PATH 1024
#define BUF_LEN 1024
using namespace std;
using std::thread;
using std::vector;

int main(int argc, char** argv){
	signal(SIGPIPE,SIG_IGN);
    vector<struct sockaddr_in *> clnt_addrarray;
    vector<std::string> messageVec;//thread간 메세지 안겹치도록
    std::string strtemp;
    vector<thread> threadarr;
    vector<std::chrono::system_clock::time_point> times;//timestamp 저장가능
    int serv_sock;
    int clnt_sock;
    char buffer[BUF_LEN];
    char temp[20];
    int on=1,i=0,count=0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    int clnt_addr_size;
    std::mutex m;
    char message[] ="hello world";

    if(argc !=2)
    {
        printf("Usage: %s <port>\n",argv[0]);
        return -1;
    }
	

}
