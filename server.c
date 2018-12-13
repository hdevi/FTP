#include<stdio.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<error.h>


#define IP_ADDR "192.168.1.22"
#define PORT 8080

int main(void)
{
	int i = 0;
	int sock_fd,addr_len,connfd;
	struct sockaddr_in serv_addr;
	addr_len = sizeof(serv_addr);
	
	//socket endpoint created
	sock_fd = socket(AF_INET,SOCK_STREAM,0);
	
	if(sock_fd == -1)
	{
		perror("Error: \n");
		exit(0);
	}
	else
		printf("Socket successfuly created.....\n");
	
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
	else
		printf("socket successfully Binded.....\n");
	//listen if client is ready to connect max 5 clients can connect
	if(listen(sock_fd,5) != 0)
	{
		perror("Error: \n");
		exit(0);
	}	
	else
		printf("Server Listening....\n");
	
	memset(&serv_addr, 0, sizeof(struct sockaddr_in));
	//accept the client connection 
	connfd = accept(sock_fd,(struct sockaddr*)&serv_addr,(socklen_t *)&addr_len);
	if(connfd < 0)
	{	
		perror("Error: ");
		exit(0);
	}
	else
		printf("Client Accepted.....\n");

		printf("accepted\n");
		i++;
		printf("%d\n",i);
	
	//close socket connection
	close(connfd);
	printf("Socket closed....\n");
	
	shutdown(sock_fd, SHUT_RDWR);
	printf("closed server listening socket....\n");

	return 0;
}
