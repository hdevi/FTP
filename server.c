/***********************************************************************************

	Author: Himanshu Devi
	Description: FTP Server Implementation in C. RAW FTP commands are implemented.
	 
************************************************************************************/

#include<stdio.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<time.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<error.h>
#include<pthread.h>


typedef struct client_info
{
 int socket_port;
 int client_no;
 uint16_t data_port;		 			
}client_t;


#define IP_ADDR "127.0.0.1"
#define PORT 8080
#define MAX 5

int cmd_fd,data_fd;
char rec_buf[50];
char bytes[200];
char ack[200];
char str[200];

void sigint_handler(int sig)
{
	//close server socket
	shutdown(cmd_fd, SHUT_RDWR);
	printf("closed server listening socket....\n");
	_exit(0);
	
}


char* command_list(char* temp[])
{
	client_t new_client;
	char *cmd,*temp1;
	temp1 = &temp[0][0];

	if(strcmp(temp1,"LIST") == 0)
		cmd = "ls";

	else if(strcmp(temp1,"PWD") == 0)
		cmd = "pwd";

	else if(strcmp(temp1,"DELE") == 0)
		cmd = "rm";

	else if(strcmp(temp1,"MKD") == 0)
		cmd = "mkdir";

	else if(strcmp(temp1,"RMD") == 0)
		cmd = "rmdir";
		
	return cmd;
}

/*
	Implementation of basic shell 
*/
void shell(client_t *arg)
{
	
	char str[512];
	char *ptr, *args[32];
	char buf[50];
	int i, err, ret, status;
	FILE* file;
	client_t new_client = *((client_t*)arg);
	int new_socket = new_client.socket_port;
	int client_no = new_client.client_no;
	int data_socket = new_client.data_port;
	memset(ack,0,sizeof(ack));

	while(1)
	{
		send(new_socket,"cmd> ",6,0);
		memset(ack,0,sizeof(ack));
		memset(rec_buf,0,sizeof(rec_buf));
		recv(new_socket,rec_buf,50,0);
		int len = strlen(rec_buf);
		memset(str,0,sizeof(str));
		memcpy(str,rec_buf,len-2);
		
		//seperating space in string  
		i=0;
		ptr = strtok(str, " ");
		args[i++] = ptr;
		do
		{
			ptr = strtok(NULL, " ");
			args[i++] = ptr;
		}while(ptr!= NULL);

		// internal commands
		if(strcmp(args[0], "QUIT") == 0)
			_exit(0);
	
		else if(strcmp(args[0], "CWD") == 0)
			chdir(args[1]);
	
		else if(strcmp(args[0], "RETR") == 0)
		{
			printf("Reading file %s ....\n",args[1]);
			file = fopen(args[1],"r");
			while(!feof(file))
			{
				fread(buf,sizeof(char),200,file);
				send(data_socket,buf,200,0);
			}
			sprintf(ack,"%s File sent on Data port\n",args[1]);	
			send(new_socket,ack,200,0);
			fclose(file);
		}

		// external commands
		else 
		{
			char* temp[3];
			char* retu;
			temp[0] = args[0];

			args[0] = command_list(temp);

			ret = fork();
			if(ret == 0)
			{
				dup2(new_socket,STDOUT_FILENO);
				err = execvp(args[0], args);
				if(err < 0)
				{
					perror("bad command");
					_exit(1);
				}
			}
			else
			{
				waitpid(-1, &status, 0);
			}
		}
		
		send(new_socket,ack,200,0);
	}
	
}

/*
	subroutine for each thread which performs following actions
	creates socket for data transfer using random port
	Authenticates User
*/

void* command(void* arg)
{
	int ret;
	uint16_t random_port;
	srand((unsigned)time(NULL));
	random_port = rand();
	client_t new_client = *((client_t*)arg);
	int new_socket = new_client.socket_port;
	int client_no = new_client.client_no;
	int data_addr_len,conn_data_fd;
	struct sockaddr_in data_serv_addr;
	data_addr_len = sizeof(data_serv_addr);
	printf("DATA Port %u\n",new_client.data_port);
	
	//socket endpoint created for data transfer
	data_fd = socket(AF_INET,SOCK_STREAM,0);
	if(data_fd == -1)
	{
		perror("Error: \n");
		_exit(0);
	}

	printf("Data Socket successfuly created.....\n");
	memset(&data_serv_addr,0,sizeof(struct sockaddr_in));
	
	//assign value to structure members
	data_serv_addr.sin_family = AF_INET;
	data_serv_addr.sin_addr.s_addr = inet_addr(IP_ADDR);
	data_serv_addr.sin_port = htons(random_port);
	printf("Server's Data Port listening %s : %d\n",IP_ADDR,random_port);
	
	//bind the address to data socket
	if(bind(data_fd,(struct sockaddr*)&data_serv_addr,sizeof(data_serv_addr)) != 0)
	{
		perror("Error: \n");
		_exit(0);	
	}
	
	printf("Data socket successfully Binded.....\n");

	memset(ack,0,sizeof(ack));
	sprintf(ack,"220 Running client: %d connected to the Server, Data Port assigned(%d)\nLogin Himan Server: ",client_no,random_port);
	send(new_socket,ack,200,0);

	//listen if client is ready to connect max 5 clients can connect
	if(listen(data_fd,MAX) != 0)
	{
		perror("Error: \n");
		_exit(0);
	}	

	printf("server listening on Data port: %d\n",random_port);

	do
	{
			
		memset(ack,0,sizeof(ack));
		memset(rec_buf,0,sizeof(rec_buf));
		recv(new_socket,rec_buf,50,0);
		int len = strlen(rec_buf);
		memset(str,0,sizeof(str));
		memcpy(str,rec_buf,len-2);
		if(strcmp(str,"anonymous") == 0)
		{
			sprintf(ack,"331 USER logged in as Anonymous. Need Password(Password)\n");
			send(new_socket,ack,200,0);
		}
		else if(strcmp(str,"Password") == 0)
		{
			sprintf(ack,"230 Anonymous successfully logged in. Connect at (%d port) for Data Transfer\n",random_port);
			send(new_socket,ack,200,0);
			while(1)
			{
				conn_data_fd = accept(data_fd,(struct sockaddr*)&data_serv_addr,(socklen_t *)&data_addr_len);
				if(conn_data_fd < 0)
				{	
					perror("Error: ");
					_exit(0);
				}

				new_client.data_port = conn_data_fd;
				shell(&new_client);
			}				
		}
		else
		{	
			sprintf(ack,"430 Invalid Login\n");
			send(new_socket,ack,200,0);
		}

		sleep(1);

	}while(strcmp(str,"bye") != 0);

	printf("Exiting socket\n");
	close(new_socket);
	close(new_client.data_port);
	pthread_exit(NULL);
	
}

/*
	main is entry point function 
	Registers signal handler 
	creates socket for commands transfer
	Accepts multiple client by creating multiple threads 
*/
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

	//socket endpoint created for command transfer
	cmd_fd = socket(AF_INET,SOCK_STREAM,0);
	
	if(cmd_fd == -1)
	{
		perror("Error: \n");
		_exit(0);
	}

	printf("Command Socket successfuly created.....\n");
	memset(&serv_addr,0,sizeof(struct sockaddr_in));
	
	//assign value to structure members
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(IP_ADDR);
	serv_addr.sin_port = htons(PORT);
	printf("Server listening %s : %d\n",IP_ADDR,PORT);

	//bind the address to socket
	if(bind(cmd_fd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) != 0)
	{
		perror("Error: \n");
		_exit(0);	
	}
	
	printf("socket successfully Binded.....\n");

	//listen if client is ready to connect max 5 clients can connect
	if(listen(cmd_fd,MAX) != 0)
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
	
		//accept the client connection 
		connfd = accept(cmd_fd,(struct sockaddr*)&serv_addr,(socklen_t *)&addr_len);
		if(connfd < 0)
		{	
			perror("Error: ");
			_exit(0);
		}
		printf("Client Accepted.....\n");
		++j;
		info[j].socket_port = connfd;
		info[j].client_no = j;
		
		//creating the threads for each client 
		if(pthread_create(&cli[j],NULL,command,&info[j]) != 0)
			printf("Thread creation failed....\n");
		
		if(j > MAX)
		{
			j = -1;
			while(j < MAX)
			{	
				//waiting for thread to finish
				pthread_join(cli[j++],NULL);
			}
			j = -1;	
		}
		
	
	}
	
	
	return 0;
}
