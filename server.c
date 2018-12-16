#include<stdio.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<sys/socket.h>
#include<error.h>


#define IP_ADDR "127.0.0.1"
#define PORT 8080

int sock_fd;

void sigint_handler(int sig)
{
	//close server socket
	shutdown(sock_fd, SHUT_RDWR);
	printf("closed server listening socket....\n");
	_exit(0);
	
}

int main(void)
{
	int i = 0;
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
		exit(0);
	}	
	
	printf("Server Listening....\n");
	
	while(1)
	{
		
		//memset(&serv_addr, 0, sizeof(struct sockaddr_in));
		//accept the client connection 
		connfd = accept(sock_fd,(struct sockaddr*)&serv_addr,(socklen_t *)&addr_len);
		if(connfd < 0)
		{	
			perror("Error: ");
			exit(0);
		}
		printf("Client Accepted.....\n");
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
	}
	
	close(connfd);
	
	return 0;
}
