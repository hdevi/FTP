#include<stdio.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<error.h>
#include<pthread.h>


#define IP_ADDR "127.0.0.1"
#define PORT 8080

int sock_fd;
char rec_buf[50];
char buffer[16];

void sigint_handler(int sig)
{
	//close server socket
	shutdown(sock_fd, SHUT_RDWR);
	printf("closed server listening socket....\n");
	_exit(0);
	
}


void* file_transfer(void* arg)
{
	int new_socket = *((int*)arg);
	recv(new_socket,rec_buf,50,0);
	sleep(1);
	strcpy(buffer,"Hey,Got you....");
	send(new_socket,buffer,16,0);
	printf("Exiting socket\n");
	close(new_socket);
	pthread_exit(NULL);
	
}

int main(void)
{
	int i = 0,pid;
	int addr_len,connfd,ret;
	struct sigaction sa;
	struct sockaddr_in serv_addr;
	addr_len = sizeof(serv_addr);
	
	//register signal handler for keyboard interrupt
	memset(&sa,0,sizeof(struct sigaction));
	sa.sa_handler = sigint_handler;
	ret = sigaction(SIGINT,&sa,NULL);
	if(ret == -1)
	{
		perror("Error: \n");
		_exit(0);
	}

	//socket endpoint created
	sock_fd = socket(AF_INET,SOCK_STREAM,0);
	
	if(sock_fd == -1)
	{
		perror("Error: \n");
		_exit(0);
	}

	printf("Socket successfuly created.....\n");
	
	memset(&serv_addr,0,sizeof(struct sockaddr_in));
	
	//assign value to structure members
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(IP_ADDR);
	serv_addr.sin_port = htons(PORT);
	printf("Server listening %s : %d\n",IP_ADDR,PORT);

	//bind the address to socket
	if(bind(sock_fd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) != 0)
	{
		perror("Error: \n");
		exit(0);	
	}
	
	printf("socket successfully Binded.....\n");

	//listen if client is ready to connect max 5 clients can connect
	if(listen(sock_fd,5) != 0)
	{
		perror("Error: \n");
		_exit(0);
	}	
	
	printf("Server Listening....\n");
	pthread_t cli[5];
	int j = -1;
	while(1)
	{
		
		//memset(&serv_addr, 0, sizeof(struct sockaddr_in));
		//accept the client connection 
		connfd = accept(sock_fd,(struct sockaddr*)&serv_addr,(socklen_t *)&addr_len);
		if(connfd < 0)
		{	
			perror("Error: ");
			_exit(0);
		}
		printf("Client Accepted.....\n");
		
		++j;
		printf("Creating Thread[%d]: \n",j);

		if(pthread_create(&cli[j],NULL,file_transfer,&connfd) != 0)
			printf("Thread creation failed....\n");
		
		if(j > 5)
		{
			j = -1;
			while(j < 5)
			{
				pthread_join(cli[j++],NULL);
			}
			j = -1;	
		}
		
		/*pid = fork();
		if(pid == 0)
		{
			int i = 0;
			while(i < 10)
			{
				printf("%d\n",i);
				i++;
			}

			//close socket connection
			close(connfd);
			printf("Socket closed....\n");
			_exit(0);
		}*/
	
	}
	
	
	return 0;
}
