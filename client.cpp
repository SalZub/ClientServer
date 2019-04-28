//g++ -pthread -o client client.cpp -lstdc++
//./client IP PORT
//client
//critical section: reading and writing on the terminal 
//before exiting informs parent
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
int w=0;
int r=0;
//flockfile(stdin);
//flockfile(stdout);
//flockfile(*STDOUT_FILENO);
//void funlockfile(FILE *stream);
//pthread_mutex_t lock; 
/*static void * syncthreads(void * arg){
	struct pollfd fds[2];
	int timeout = (-1 * 60 * 1000); //time in milliseconds before poll returns a 0 if no events occur on all fds
	fds[0].fd = STDOUT_FILENO;
	fds[0].events = POLLOUT;//a macro that means object will have data on it, view manpages or book for more detail
	fds[1].fd = STDIN;
	fds[1].events = POLLIN;
	while(1){
		int rc = poll(fds, 2, timeout);
		if (rc==-1)
		if(errno != EINTR){
		perror("Poll"); exit(0);}
		else continue;
		if (rc == 0){
		write(STDOUT_FILENO, "Poll() Timed Out", 17);
		break;
		}
		if(fds[0].revents == POLLOUT)
		{  //writing to stdout
			
		}
		if(fds[1].revents == POLLIN)
		{ //reading from stdin
		//data available on second fd
		}
	}

}*/
static void * thread1_func(void *arg)
       {	// flockfile(stream);
		//flockfile(stdout);
	//fcntl(STDIN_FILENO, O_NONBLOCK);
		int sock = *((int *)arg);
		while(1) {
			char buf[1024];
			bzero(buf, sizeof(buf));
			//pthread_mutex_lock(&lock);
			//if(w==1)sleep(1);
			//while(tryflockfile(stdout)!=0){	
			//r=1;
			//}
			//r=0;
			int fr = read(STDIN_FILENO,buf,1024);

			//pthread_mutex_unlock(&lock); 
			if(fr==-1)
				{
					perror("enter again. user input: ");
					exit(0);
				}
			if (write(sock, buf, fr) < 0)
				{	
					if(errno==EPIPE)
					{	
						//conn closed
							close(sock);
							exit(0);
					}
					else
					{
					perror("writing on stream socket");
					//exit(0);
					}
           			}
			//funlockfile(stdout);
}//while
		pthread_exit(NULL);
       }
int main(int argc, char *argv[])
	{
	//pthread_mutex_init(&lock,NULL);
	//flockfile(stdin);
	//flockfile(stdout);
	//fcntl(STDIN_FILENO, O_NONBLOCK);
	int sock,fr,fr2,fw2;
	struct sockaddr_in server;
	struct hostent *hp;
	char buf[1024];
	char buff2[1024];

	/* Create socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("opening stream socket");
		exit(1);
	}
	/* Connect socket using name specified by command line. */
	server.sin_family = AF_INET;
	hp = gethostbyname(argv[1]);
	if (hp == 0) {
		fprintf(stderr, "%s: unknown host\n", argv[1]);
		exit(2);
	}
	bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
	server.sin_port = htons(atoi(argv[2]));

	if (connect(sock,(struct sockaddr *) &server,sizeof(server)) < 0) {
		perror("connecting stream socket");
		exit(1);
	}
	//one thread eads from user and writes to socket: thread1

        pthread_t tid1;        //ID returned by pthread_create()
     	//pthread_attr_t attr1,attr2;

	//int *arg=malloc(sizeof(*arg));
	//*arg=sock;
	int t1=pthread_create(&tid1, NULL, &thread1_func, (void *)&sock);
	//the other thread reads from socket and writes to STDOUT: thread2
	pthread_join(t1,NULL);
	while(1){
			fr2=read(sock, buff2,1024);

			if(fr2==0){
				//server has died or disconnected
					close(sock);
					exit(0);
				}
			if(fr2==-1)
				perror("socket read: ");
				/*if(r==1)sleep(1);
				while(ftrylock(stdout)!=0){
				w=1;
				}
				w=0;*/
			fw2=write(STDOUT_FILENO,buff2,fr2);
			//funlockfile(stdout);
			
					
		}
	pthread_exit(NULL);
	}
