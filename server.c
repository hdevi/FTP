#include<stdio.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<time.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<error.h>
#include<pthread.h>


typedef struct client_info
{
 int socket_port;
 int client_no;
 int data_port;		 			
}client_t;

typedef enum Commands{USER,PASS,CWD,RETR,QUIT,BYE}command_t;

#define IP_ADDR "127.0.0.1"
#define PORT 8080
#define MAX 5

int sock_fd;
char rec_buf[50];
char bytes[200];
char ack[200];
char str[200];

void sigint_handler(int sig)
{
	//close server socket
	shutdown(sock_fd, SHUT_RDWR);
	printf("closed server listening socket....\n");
	_exit(0);
	
}


void* command(void* arg)
{
	int ret,r;
	command_t input;
	srand((unsigned)time(NULL));
	r=rand();
	printf("%d\n",r);
	client_t new_client = *((client_t*)arg);
	int new_socket = new_client.socket_port;
	int client_no = new_client.client_no;
	new_client.data_port = r;
	memset(ack,0,sizeof(ack));
	sprintf(ack,"220 Running client: %d connected to the Server, Data Port assigned(%d)\nLogin Himan Server: ",client_no,new_client.data_port);
	send(new_socket,ack,200,0);
	do
	{
		memset(ack,0,sizeof(ack));
		memset(rec_buf,0,sizeof(rec_buf));
		recv(new_socket,rec_buf,50,0);
		int len = strlen(rec_buf);
		memset(str,0,sizeof(str));
		memcpy(str,rec_buf,len-2);
		printf("%s %d %lu\n",str,len-2,strlen(str));
		if(strcmp(str,"anonymous") == 0)
		{
			sprintf(ack,"331 USER logged in as Anonymous. Need Password(Password)\n");
			send(new_socket,ack,200,0);
		}
		else if(strcmp(str,"Password") == 0)
		{
			sprintf(ack,"230 Anonymous successfully logged in\n");
			send(new_socket,ack,200,0);
		}
		else if(strcmp(str,"CWD") == 0)
		{
			
		}
		else if(strcmp(str,"RETR") == 0)
		{
			
		}
		else if(strcmp(str,"QUIT") == 0)
		{
			
		}
		else
		{
			
		}

		sleep(1);

	}while(strcmp(str,"bye") != 0);

	printf("Exiting socket\n");
	close(new_socket);
	pthread_exit(NULL);
	
}

int main(void)
{
	int i = 0,pid;
	int addr_len,ret;
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
	if(listen(sock_fd,MAX) != 0)
	{
		perror("Error: \n");
		_exit(0);
	}	
	
	printf("Server Listening....\n");
	pthread_t cli[MAX];
	client_t info[MAX];
	int j = -1;
	
	while(1)
	{
		int connfd;	
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
		//printf("connfd = %d | j = %d\n",connfd,j);
		//printf("Creating Thread[%d]: \n",j);
		info[j].socket_port = connfd;
		info[j].client_no = j;
		if(pthread_create(&cli[j],NULL,command,&info[j]) != 0)
			printf("Thread creation failed....\n");
		
		if(j > MAX)
		{
			j = -1;
			while(j < MAX)
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
