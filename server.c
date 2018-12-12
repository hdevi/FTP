#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<error.h>


#define IP_ADDR 127.0.0.1
#define PORT 8080

int main(void)
{
	int sock_fd;
	struct sockaddr *serv_addr;
	struct sockaddr_in cli;

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
	serv_addr.sin_addr.s_addr = htonl(IP_ADDR);
	serv_addr.sin_port = htons(PORT);

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
	
	len = sizeof(cli);
	
	//accept the client connection 
	connfd = accept(sock_fd,(struct sockaddr*)&cli,&len);
	if(connfd < 0)
	{	
		perror("Error: ");
		exit(0);
	}
	else
		printf("Client Accepted.....\n");
	
	//close socket connection
	close(sock_fd);

	return 0;
}
