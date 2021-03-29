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
#define PENDING_REQ 15
using namespace std;
using std::thread;
using std::vector;

int messaging(int clientfd,struct sockaddr_in* clientaddr);

int main(int argc, char** argv){
	signal(SIGPIPE,SIG_IGN);
    vector<struct sockaddr_in *> clnt_addrarray;
    vector<std::string> messageVec;//thread간 메세지 안겹치도록 저장
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
	if((serv_sock = socket(PF_INET, SOCK_STREAM, 0))==-1)
    {// socket generated
        printf("Server : Can't open stream socket\n");
        exit(0);
    }
    if(setsockopt(serv_sock,SOL_SOCKET,SO_REUSEADDR,(void*)&on,sizeof(on))!=0)
    {
        perror("setsockopt()error");
        return -1;
    }
    memset(&serv_addr,0,sizeof(serv_addr));//string.h 0으로초기화
    
    serv_addr.sin_family=AF_INET; //adress family
    serv_addr.sin_addr.s_addr=INADDR_ANY;//any IP address
	serv_addr.sin_port=htons(atoi(argv[1])); //htons host to network  littleendian bigendian~네트워크에선 공용어 쓰자
	
	if(bind(serv_sock,(struct sockaddr *)&serv_addr, sizeof(serv_addr))==-1) //binding해야 해당 소켓에서 packet 받기가능
    {
        printf("Server : can't bind local address.\n");
        return -1;
    }

    if(listen(serv_sock, PENDING_REQ)<0){//최대 PENDING_REQ개까지의 pending request
        printf("Server  : Can't listen connect.\n");
        exit(0);
    }
	
	memset(buffer,0x00,sizeof(buffer));
    printf("Server:waiting connection request.\n");
    clnt_addr_size=sizeof(clnt_addr);//둘다 int 형인데..

	while(1){

        clnt_sock=accept(serv_sock,(struct sockaddr *)&clnt_addr,(socklen_t*)&clnt_addr_size);//sock
        //returns the new socket to connect with client from serverside 
        if(clnt_sock<0)
        {
            printf("Server: accept failed.\n");


        }
        clnt_addrarray.push_back(&clnt_addr);
    
        inet_ntop(AF_INET,&clnt_addr.sin_addr.s_addr, temp,sizeof(temp));//accept address보고싶으면temp출력
		//cout<<temp<<endl;
        threadarr.push_back(thread(messaging,clnt_sock,clnt_addrarray[i]));// 쓰레드 생성하는법
        i++;
        count++;
    }
	
	close(serv_sock);
	return 0;

}

int messaging(int clientfd,struct sockaddr_in* clientaddr)
{
    double timelasted;
    int ti,tempti;
    char temp[1000];
    char cladd[20];//client address
    char seradd[20];//server address
	char buff[1024];//readin 10kbyte
	std::string strbuff;
	int infd,outfd,i,nread;
    FILE* clnt_read;
    FILE* clnt_write;
    FILE* indexht,*secretht;
	int fileLength=2048;
	char method[15];
    char file_name[MAX_LENGTH_FILE_PATH];
    char protocol[BUF_LEN];//="HTTP/1.1 200 OK\r\n";
    char server[]="Server:UbuntuB Web Server \r\n";
    char connection[]="Connection: keep-alive\r\n";
    char cnt_len[BUF_LEN];//="Content-length:2048\r\n";
    char keepalive[]="Keep-Alive: timeout=5, max=100\r\n";
    char cnt_type[BUF_LEN];
	char message[] ="hello world";	
	while(true){

		clnt_read=fdopen(dup(clientfd),"r");
	    memset(buff,0x00,sizeof(buff));
	    memset(cnt_type,0x00,sizeof(cnt_type));
	    memset(protocol,0x00,sizeof(protocol));
	    memset(cnt_len,0x00,sizeof(cnt_len));
		nread=read(clientfd,buff,sizeof(buff)); //받은 packet을 읽는다.user번호 읽어오기?
	    buff[nread]='\0';
		strbuff=buff;
		if(nread==EOF || nread==0)
		{
			//reached end of cliendfd
			fclose(clnt_read);
			break;
		}
		sprintf(cnt_type,"Content-type:%s\r\n\r\n","text/html");//html 헤더제작
        fileLength=2048;
        sprintf(cnt_len,"Content-length:%d\r\n",fileLength);//대충 2048byte보내기.
        sprintf(protocol,"HTTP/1.1 %s\r\n","200 OK");
		fclose(clnt_read); //request 를 다 읽었음
		
		strcpy(method,strtok(buff," /"));//get 받아오고
        if(strcmp(method,"GET")!=0)
        {
            //post 같은걸로들어왔나봐 -> buffer에 읽었던 content length만큼 한번더 패킷을 읽기 (nread=read(clientfd,buff,content_length))
            //fclose(clnt_write);
            continue;
        }
		clnt_write=fdopen(dup(clientfd),"w"); //여기에 쓰면 패킷이보내짐
		sprintf(cnt_len,"Content-length:%d\r\n",12);
		fputs(protocol,clnt_write);
        fputs(server,clnt_write);
        fputs(cnt_len,clnt_write);
        fputs(keepalive,clnt_write);
        fputs(cnt_type,clnt_write);//default==html//\r\n\r\n 
		//이걸 차례대로 보내주면, 된다. 일단 hello world 보내면 contentlength=12이겠지?그래서 위에설정 12
		fputs(message,clnt_write);
		fflush(clnt_write);//전송시켜주기
		fclose(clnt_write);
		
		break; //end
	
	}//while true 끝


	close(clientfd);

	
	return 0;
	

}



