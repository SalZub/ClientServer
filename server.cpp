#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#define TRUE 1
using namespace std;
volatile int c;
struct process
	{
	int pid=0;
	string pname;
	char status='a';
	time_t startT;
	time_t endT;
	};
struct node{
	process data;
	node * next;
};
class list{
	private:
	node * head, * tail;
	public:
	list()
		{
		head=NULL;
		tail=NULL;
		}
	void insert(process p)
		{
		node * temp=new node;
		temp->data=p;
		temp->next=NULL;
		if(head==NULL)
		{
			head=temp;
			tail=temp;
			temp=NULL;
		}
		else
		{
			tail->next=temp;
			tail=temp;
		}	
		}
	void kill1()
	{
		node * temp=new node;
		temp=head;
		while(temp!=NULL)
		{
		kill(temp->data.pid, SIGTERM);//sending sigterm to a client's process will send sigchild to client handler
		temp=temp->next;
		}
		//kill(-1,SIGTERM);
	}
	/*void killpid(int pid)
	{
		/*node * temp=new node;
		temp=head;
		//printf("%d",ptr);
		//fflush(stdout);
		while(temp!=NULL)
		{	if(temp->data.pid==pid)
			{
				kill(temp->data.pid,SIGTERM);
				break;
			}
			temp=temp->next;
		}
	}*/
	void killpname(string ptr)
		{
			node * temp=new node;
			temp=head;
			while(temp!=NULL)
			{
			if(temp->data.pname==ptr)
			{
				kill(temp->data.pid,SIGTERM);
				break;
			}
			temp=temp->next;
			}
		}
	string display()
	{	string str="";
		node * temp=new node;
		temp=head;
		while(temp!=NULL)
		{
			str+=std::to_string(temp->data.pid);
			str+="\t";
			str+=(temp->data.pname);
			str+="\t";
			str+=ctime(&(temp->data.startT));
			str+="\t";	
			str+=temp->data.status;

str+="\t";
			if(temp->data.status=='u'){	
			
			str+=ctime(&(temp->data.endT));
			str+="\t";
			str+=std::to_string(difftime(temp->data.endT, temp->data.startT));
			str+="\n";
			}
			temp=temp->next;	
		}
		return str;

	}
	void endtime()
	{	
		int status;
		node * temp = new node;
		temp = head;
		while(temp!=NULL)
			{
			int w=waitpid(temp->data.pid, &status, WNOHANG);
			if(w==-1)
			{	
				if(errno==ECHILD){
					temp=temp->next;continue;}
				else
				{

					perror("wait: ");
					exit(0);
				}
			}
			else if (w==0)
				continue;
			else if(w>0)
			{
				time(&temp->data.endT);
				temp->data.status='u';
				//set as status
			}
			temp=temp->next;
			}
		c=0;
	}
};
list pList; //linkedlist
void sig_handlerCH(int signo)
{
	write(STDOUT_FILENO,"sigchild",8);
	if (signo==SIGCHLD)
		c=1;

	//else if (signo==SIGTERM){}


};
void sig_handlerS(int signo)
{
};

struct connection
	{
char buff[1024];
int sp=0;
	int handlerID;
	string cIP;
	int msgsock;
	string status="on";
	time_t startT;
	time_t endT;
void display(){
if(status=="on")
sp=sprintf(buff,"%d\t%s\t%d\t%s\t%s\t%s\n",handlerID, cIP.c_str(),msgsock,status.c_str(),ctime(&(startT)),ctime(&(endT)));
else
sp=sprintf(buff,"%d\t%s\t%d\t%s\t%s\n",handlerID, cIP.c_str(),msgsock,status.c_str(),ctime(&(startT)));
write(STDOUT_FILENO,buff,sp);}
	};

int main(){
	connection cList [5];
	int sockfd1, sockmsgfd1, fd1,bnd,conncount,sp,fw,ad,fr,e2;
	socklen_t length;
	socklen_t lengthc;
	struct sockaddr_in addr;
	struct sockaddr_in client;
	char add[5];
	char * ptr;
	char buff[1024];
	int SHpipe1[2];//SHpipe1[0] for server to write and SHpipe1[1] for handler to read
	int SHpipe2[2];//SHpipe2[0] for handler to write and SHpipe2[1] for server to read
	sockfd1 = socket(AF_INET, SOCK_STREAM,0);
	if(sockfd1==-1)
		{
			perror("socket creation: ");
			exit(0);
		}
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = 0;
	bnd=bind(sockfd1, (struct sockaddr *) &addr, sizeof(addr));
	if(bnd==-1)
		{
			perror("bind: ");
			exit(0);
		}
	length = sizeof(addr);
	if (getsockname(sockfd1, (struct sockaddr *) &addr, (socklen_t*) &length)) {
		perror("getting socket name");
		exit(1);
	}
	/*fw=write(STDOUT_FILENO,"Socket has port#",16);
	if(fw==-1){
		perror("write: ");
		exit(0);
		}	
	ad=ntohs(addr.sin_port);
	sp=sprintf(add,"%d\n",ad);
	if(sp==-1){
		perror("write: ");
		exit(0);
		}
	if(write(STDOUT_FILENO,add,sp)<0){
		perror("write: ");
		exit(0);
		}
	*/
	printf("Socket has port:%d\n",ntohs(addr.sin_port));
	fflush(stdout);
	int lst=listen(sockfd1, 5);
			if(lst==-1)
			{
				perror("listening error: ");
				exit(0);	
			}
	struct pollfd fds[2];
	fds[0].fd = sockfd1;
	fds[0].events = POLLIN;
	fds[1].fd = STDIN_FILENO;
	fds[1].events = POLLIN;
	while(true)/////////////////listen for conns
	{	int i=0,p=0;
		int rc = poll(fds, 2,-1);
		if(rc==-1){
				//if(errno==EINTR)
				//continue;
			perror("POLL:" );
			exit(0);
			}
		if(fds[1].revents == POLLIN) //for main server commands
		{
			fr=read(STDIN_FILENO,buff,1024);
			if(fr==-1)
			{//
				perror("reading: ");
				exit(0);
			}//
			buff[fr-1]='\0';
			ptr=strtok(buff," ");
		if(strcmp(ptr,"exit")==0)
			{//	
				ptr=strtok(NULL," ");
				if(ptr==NULL)//exit all connections
				{//
					for(i=0;i<conncount;i++){//
					if(cList[i].status=="on")
						kill(cList[i].handlerID,SIGTERM);
					}//
				}//
				else//end connection with the given ip			
				{//
					for(i=0;i<conncount;i++){//
					if(cList[i].status=="on" && cList[i].cIP==string(ptr)){//
						kill(cList[i].handlerID,SIGTERM);
						break;		}//
					}//
				
				}//
		
					  
			}//
			else if(strcmp(ptr,"print")==0)
			{	ptr=strtok(NULL," ");
				p=sprintf(buff,"%p.s",ptr); //message
				ptr=strtok(NULL,";");
				ptr=strtok(NULL," ");
				if(ptr==NULL)// broadcast
				{//
					for(i=0;i<conncount;i++){//
						if(cList[i].status=="on")
						write(cList[i].msgsock,buff,p);
					}//
				}//
				else//ip		
				{//
					for(i=0;i<conncount;i++){//
					if(cList[i].status=="on" && cList[i].cIP==string(ptr)){//
						write(cList[i].msgsock,buff,p);
						break;		}//
					}//
				
				}//

			}//
			if(strcmp(ptr,"list")==0)
			{//
				ptr=strtok(NULL," ");
				if(strcmp(ptr,"proc")==0)// 
				{
					for(i=0;i<conncount;i++){
						cList[i].display();
						write(SHpipe1[0],"list",4);
						fr=read(SHpipe2[1],buff,1024);
						write(STDOUT_FILENO,buff,fr);
					}
				}
				else if(strcmp(ptr,"conn")==0)//ip		
				{
					for(i=0;i<conncount;i++){
						cList[i].display();
					}
				
				}
			}//
			else
				write(STDOUT_FILENO,"invalid\n",8);
			
		}//event for server command
		if(fds[0].revents == POLLIN)
		{	write(STDOUT_FILENO,"in ch",5);
			lengthc=sizeof(client);
			sockmsgfd1 = accept(sockfd1,0,0); 
			//if(fd1>0)
			//sleep(1);
			if(sockmsgfd1==-1)
			{//	
				perror("connection error try again: ");
				continue;
			}//	

			if (getsockname(sockmsgfd1, (struct sockaddr *) &client, (socklen_t*) &lengthc)) {//
				perror("getting socket name");
				exit(1);
			}//
			///new connection made	

			time(&(cList[conncount].startT));
			cList[conncount].msgsock=sockmsgfd1;
			cList[conncount].cIP=std::string(inet_ntoa(client.sin_addr));
			
			fd1 = fork();  //client handler change to before accept
			if(fd1==-1)
			{
				perror("fork: ");//handle error
			}
			if(fd1>0){
			cList[conncount].handlerID=fd1;
			conncount++;}
			if(fd1==0) //in client connection manager /server child
			{
				close(SHpipe1[0]);
				close(SHpipe2[1]);
				struct pollfd fdd[2];
				fdd[0].fd = SHpipe1[1];
				fdd[0].events = POLLIN;//a macro that means object will have data on it, view manpages or book for more detail
				fdd[1].fd = sockmsgfd1;
				fdd[1].events = POLLIN;
				while(1){
					int r= poll(fdd, 2, -1);
					if (r==-1){
					perror("poll: ");break;
					}
					if(fdd[0].revents == POLLIN){
							string str=pList.display();
							char cstr[str.size() + 1];
							strcpy(cstr, str.c_str());	// or pass &s[0]
							sp=sprintf(buff,"%s",cstr);
							if(sp==-1){
								perror("sprint: ");
								exit(0);
								}
							fw=write(SHpipe2[0],buff,sp);
							if(fw==-1){//
								perror("write: ");
								exit(0);
								}

					}
					if(fdd[1].revents == POLLIN)
					{	
						if(c==1)
						pList.endtime();
						signal(SIGCHLD,sig_handlerCH);
						int e1,e3,i,fr;
						int result,num=0,fd2;
						int pid,pcount=0,pip;
						char pipread[100];
						string print;
						process p;
						int pipefd[2];//for run between parent and child
									//e1=write(sockmsgfd1,"Enter command or 0 to exit>>\r\n",30);
						bzero(buff, sizeof(buff));
						e2=read(sockmsgfd1, buff, 1024);
						if(e2==-1){//
							perror("reading from socket: ");
							exit(0);
							}///
						write(STDOUT_FILENO,buff,e2);
						buff[e2-1]='\0';
						ptr=strtok(buff," ");
						if(strcmp(ptr,"exit")==0)//client has asked to end the connection
						{	//
					//before exiting must close all processes related to that client or send a sigterm signal to the process
							//pList.kill1();
						    write(STDOUT_FILENO,"client exiting\n",15);
							write(sockmsgfd1,"exiting",7);
								close(sockmsgfd1);

						    exit(0);
						}//
						else if(strcmp(ptr,"list")==0)
						{
							string str=pList.display();
							char cstr[str.size() + 1];
							strcpy(cstr, str.c_str());	// or pass &s[0]
							sp=sprintf(buff,"%s",cstr);
							if(sp==-1){
								perror("sprint: ");
								exit(0);
								}
							fw=write(sockmsgfd1,buff,sp);
							if(fw==-1){//
								perror("write: ");
								exit(0);
								}
						
						}//
						else if(strcmp(ptr,"kill")==0)
						{//
							ptr=strtok(NULL, " ");//check//killall
							if(ptr==NULL)
							{	pList.kill1();
							}
							else if(strcmp(ptr,"all")==0)
							{
								pList.kill1(); //kill all
							}
							else if(strcmp(ptr,"not a number")==0)
							{
								pList.killpname(string(ptr)); //using pname
							}
							else//ptr is a pid
							{	int o;	
								int l=sscanf(ptr,"%d",&o);
								l=sprintf(buff,"%d",o);
						write(STDOUT_FILENO,buff,l);
								kill(o,SIGTERM);
							}
						}//
						else if(strcmp(ptr,"run")==0)
						    {
						    ptr=strtok(NULL, " ");	
						    pip=pipe2(pipefd,O_CLOEXEC);//fd[0] for parent to read and fd[1] for child to write
							if(pip==-1){
								perror("pipe creation: ");
								exit(0);
							}
						    pid=fork();
						    if(pid>0)
							{
								close(pipefd[1]);
								fr=read(pipefd[0],&pipread,100);
								if(fr==0)//exec successful
								{	
										p.pid=pid; 
					//making an entry in the process list making sure if exec was successful
					//sol: if the exec fails notify the parent through a signal with a pid of the process in which exec was called 
					//sol2: create a pipe between parent and child and close it on exec while 
										p.pname=string(ptr);
										time(&p.startT); //current time in the variable
										pList.insert(p);
										sp=sprintf(buff,"%d",pid);
										write(sockmsgfd1,buff,sp);
										write(sockmsgfd1,"\n",1);
								}	
								else  if(fr==-1)//pipe reading error
								{	perror("reading pipe ");
									exit(0);
								}
								else 
								{//exec failure error
									write(sockmsgfd1,pipread,fr);
									//can also get error message fro status of id using wait
									write(sockmsgfd1,"\n",1);
								}
							    }
							    else if(pid==0)
								{	
									
								close(pipefd[0]);
								e3=execlp(ptr,ptr,NULL);
								if(e3==-1){
								    //perror("error");
								    //write(STDOUT_FILENO,"\n",1);
								    //kill(getppid(),SIGUSR1); //tells parent that exec has failed
								    write(pipefd[1],"error occured on exec",21);
								    exit(errno);
									  }
							    	}
							
						   	}//run	
							else if(strcmp(ptr,"add")==0)
							    {
							    result=0;
							    ptr = strtok(NULL," ");
							 if(ptr==NULL){
                    						write(sockmsgfd1,"incomplete command\n",19);
								continue;}
							    while(ptr!=NULL)
								{
								sscanf(ptr,"%d",&num);
								result=result+ num;
								ptr = strtok(NULL," ");
								}
							    char res[1024];
							    fd2= sprintf(res,"%d", result);
							   // write(sockmsgfd1,"                          >>",28);
							    write(sockmsgfd1, res, fd2);
							    write(sockmsgfd1,"\n",1);
							    }
							else if (strcmp(ptr, "sub")==0)
							{    if(ptr==NULL){
                    						write(sockmsgfd1,"incomplete command\n",19);
								continue;}
								result=0;
							    result=atoi(ptr);
								ptr=strtok(NULL," ");
								while(ptr!=NULL)          {
									result = result- atoi(ptr);
									ptr = strtok(NULL," ");
									}
									 char res[1024];
							    fd2= sprintf(res,"%d", result);
							   // write(sockmsgfd1,"                          >>",28);
							    write(sockmsgfd1, res, fd2);
							    write(sockmsgfd1,"\n",1);
							}

						    else if (strcmp(start_arr, "mul")==0)
						    {
							if(ptr==NULL){
                    						write(sockmsgfd1,"incomplete command\n",19);
								continue;}
							result=1;
								while(ptr!=NULL)          {
									result = result * atoi(ptr);
									ptr = strtok(NULL," ");
											}
							 char res[1024];
							    fd2= sprintf(res,"%d", result);
							   // write(sockmsgfd1,"                          >>",28);
							    write(sockmsgfd1, res, fd2);
							    write(sockmsgfd1,"\n",1);

						    }

						    else if(strcmp(start_arr, "div")==0)
						    {  if(ptr==NULL){
                    						write(sockmsgfd1,"incomplete command\n",19);
								continue;}
							   result=atoi(ptr);
							   ptr = strtok(NULL," ");

							   while(ptr!=NULL)          {
								result= result/atoi(ptr);

								ptr= strtok(NULL," ");
									    }
									   char res[1024];
							    fd2= sprintf(res,"%d", result);
							   // write(sockmsgfd1,"                          >>",28);
							    write(sockmsgfd1, res, fd2);
							    write(sockmsgfd1,"\n",1);

						    }
					}//event end
					}	//while
				}//fd==client handler		

		}//if data on fd[0]
		
		}///while true for connections
		
		
		
//before accepting to avoid making the client wait for fork we already fork before accepting to keep a connection handler process ready for client and both the server connection and the client request handler will have the sockmsgfd no need to send through pipe to the parent server

}//for main
