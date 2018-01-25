#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
int sockfd, newsockfd,port;
struct sockaddr_in srv_addr, cli_addr;
socklen_t cli_len;
char file_name [256]; 
char addr [INET_ADDRSTRLEN]; 
/*read file name from socket */
void fname(int sockfd, char* file_name)
{
	char recv_str[256]; 
	ssize_t rcvd_bytes; 
	rcvd_bytes = recv(sockfd, recv_str, sizeof(recv_str), 0);
	if ( rcvd_bytes < 0)
	{
		perror("CAN'T RECIEVE FILENAME");
		exit(0);
	}
	sscanf (recv_str, "%s\n", file_name); 
	printf("filename::%s\n",file_name);
}
/*sending file*/
void sendfile(int sockfd, char *file_name)
{
	ssize_t read_bytes, sent_bytes,filesize=0;
	char send_buf[256]; 
	char * errmsg= "FILE NOT FOUND\n";
	FILE* f = fopen(file_name,"r");
	fseek(f,0,SEEK_SET);
	if(f==NULL) 
	{
		perror(file_name);
		if( (sent_bytes = send(sockfd, errmsg ,strlen(errmsg), 0)) < 0 )
		{
			perror("CAN'T SEND");
			exit(0);
		}
	}
	else 
	{
		printf("SENDING FILE: %s\n", file_name);
		while( (read_bytes = fread(send_buf,1,sizeof(send_buf),f)) > 0 )
		{
			if( (sent_bytes = send(sockfd, send_buf, read_bytes, 0))< read_bytes )
			{
				perror("CAN'T SEND");
				exit(0);
			}
			filesize += sent_bytes;
		}
		fclose(f);
	} 
	printf("Sent %ld bytes \n",filesize);
}
void* fun(void *vargp)
{
	if (newsockfd < 0)
	{
		perror("ACCEPTING ERROR");
		exit(1);
	}
	inet_ntop(AF_INET, &(cli_addr.sin_addr),addr, INET_ADDRSTRLEN);//convert numeric IP to command line
	printf("CLIENT CONNECTED VIA %s:%d\n",addr, ntohs(cli_addr.sin_port) );
	fname(newsockfd, file_name);
	sendfile(newsockfd, file_name);
	close(newsockfd);
	return NULL;
}
/*recieving file*/
void recvfile(int sock, char* file_name)
{
	char send_str [256]; 
	FILE* f; 
	ssize_t sent_bytes, rcvd_bytes, filesize=0;
	char recv_str[256]; 
	size_t send_strlen; 
	sprintf(send_str, "%s\n", file_name); 
	send_strlen = strlen(send_str); 
	if( (sent_bytes = send(sock, file_name, send_strlen, 0)) < 0 ) 
	{
		perror("CAN'T SEND");
		exit(0);
	}
	f = fopen(file_name, "w+");
	if (f==NULL)
	{
		perror("CAN'T CREATE FILE");
		exit(0);
	}
	while ( (rcvd_bytes = recv(sock, recv_str, 256, 0)) > 0 )
	{
		filesize += rcvd_bytes;
		if (fwrite( recv_str,1, rcvd_bytes,f) < 0 )
		{
			perror("FILE CAN'T BE WRITTEN");
			exit(0);
		}
	}
	printf("%s\n",file_name);
	fclose(f);
	printf("Client Received: %ld bytes \n", filesize);
}
void* serv(void *vargp)
{
	memset(&srv_addr, 0, sizeof(srv_addr)); 
	memset(&cli_addr, 0, sizeof(cli_addr));
	 
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	port = (int)(vargp);
	srv_addr.sin_port = htons(port);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if ( sockfd < 0)
	{
		perror("SOCKET NOT CREATED");
		exit(0);
	}
	if( bind(sockfd, (struct sockaddr*) &srv_addr, sizeof(srv_addr)) < 0 )
	{
		perror("BINDING ERROR");
		exit(0);
	}
	printf("LISTENING ON PORT %d ...\n", port);
	if( listen(sockfd, 5) < 0 )
	{
		perror("LISTENING ERROR");
		exit(0);
	}
	pthread_t tid[3];
	while(1)
	{
		for (int i = 0; i < 3; i++)
		{
			cli_len = sizeof(cli_addr);
			newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr,&cli_len);
			pthread_create(&tid[i], NULL, fun, (void*)newsockfd);
		}
	}
    pthread_exit(NULL);
    close(sockfd);
}

int main(int argc, char* argv[])
{
	memset(&file_name, 0, sizeof(file_name));
	port = atoi(argv[6]);
	pthread_t tid;
	pthread_create(&tid, NULL, serv, (void*)port);
	int sockfd, port,n,portc,port2,clfd;
	struct sockaddr_in srv_addr,cliaddr,cl2addr;
	char buffer[256]; //send to server
	char client[100]; //client info
	char fileinfo[100]; //recieve from server
	if (argc < 7)
	{
		printf("usage: %s <username> <IP address_client> [port number_client] <IP address_server> [port number_server] [download port] [client port]\n", argv[0]);
		exit(0);
	}
	memset(&client, 0, sizeof(client)); 
	memset(&srv_addr, 0, sizeof(srv_addr)); 
	memset(buffer, 0, sizeof(buffer));
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	cliaddr.sin_family = AF_INET; 
	strcat(client,argv[1]);
	strcat(client,":");
	strcat(client,argv[2]);
	strcat(client,":");
	strcat(client,argv[3]);
	strcat(client,":");
	strcat(client,argv[6]);
	if ( inet_pton(AF_INET, argv[2], &(srv_addr.sin_addr)) < 1 ) // convert command line argument to numeric IP 
	{
		printf("INVALID IP1\n");
		exit(0);
	}
	portc = atoi(argv[3]);
	cliaddr.sin_port = htons(portc);
	srv_addr.sin_family = AF_INET; 
	if ( inet_pton(AF_INET, argv[4], &(srv_addr.sin_addr)) < 1 ) // convert command line argument to numeric IP 
	{
		printf("INVALID IP2\n");
		exit(0);
	}
	port = atoi(argv[5]);
	srv_addr.sin_port = htons(port);
	if( connect(sockfd, (struct sockaddr*) &srv_addr, sizeof(srv_addr)) < 0 )
	{
		perror("CONNECT ERROR\n");
		exit(0);
	}
	printf("CONNECTED TO SERVER:%s:%d.....\n",argv[5],srv_addr.sin_port);
	write(sockfd,client,strlen(client));  //sending client info to server
	while(1)
	{
		printf("enter the cmd: ");
		fgets(buffer,255,stdin);
		int i=0;
		char cmd[255];
		while(buffer[i]!=' ')
		{
			cmd[i]=buffer[i];
			i++;
		}
		cmd[i]='\0';
		n=write(sockfd,buffer,strlen(buffer));
		if(n<0)
		{
			perror("ERROR IN WRITE");
			exit(0);
		}
		memset(fileinfo, 0, sizeof(fileinfo));
		n=recv(sockfd,fileinfo,sizeof(fileinfo),0);
		if(n<0)
		{
			perror("ERROR IN READ");
			exit(0);
		}
		if(n==0)
		exit(0);
		printf("SUCCESS:%s\n",fileinfo);
		if(strcmp(cmd,"get")==0)
		{
			int x=0,y=0;
			char f[10]; //extracting filename
			while(buffer[x]!='"' && buffer[i]!='\0')
			{
				x++;
			}
			x++;
			while(buffer[x]!='"' && buffer[i]!='\0')
			{
				f[y]=buffer[x];
				y++;
				x++;
			}
			f[y]='\0';
			int i=0;
			char ip[20]; //extracting ip address
			char port[10]; //extracting port no
			while(fileinfo[i]!=':' && fileinfo[i]!='\0')
			{
				i++;
			}
			i++;
			int j=0;
			while(fileinfo[i]!=':' && fileinfo[i]!='\0')
			{
				ip[j]=fileinfo[i];
				j++;
				i++;
			}
			ip[j]='\0';
			int k=strlen(fileinfo)-1;
			while(fileinfo[k]!=':' && fileinfo[k]!='\0')
			{
				k--;
			}
			k++;
			j=0;
			while(fileinfo[k]!='\0')
			{
				port[j]=fileinfo[k];
				j++;
				k++;
			}
			printf("ip=%s\n",ip);
			port[j]='\0';
			printf("port=%s\n",port);
			clfd = socket(AF_INET, SOCK_STREAM, 0);
			cl2addr.sin_family = AF_INET; 
			if ( inet_pton(AF_INET, ip, &(cl2addr.sin_addr)) < 1 ) // convert command line argument to numeric IP 
			{
				printf("INVALID IP3\n");
				exit(0);
			}
			port2 = atoi(port);
			cl2addr.sin_port = htons(port2);
			if( connect(clfd, (struct sockaddr*) &cl2addr, sizeof(cl2addr)) < 0 )
			{
				perror("CONNECT ERROR\n");
				exit(0);
			}
			printf("CONNECTED TO SERVER:%s:%d.....\n",port,cl2addr.sin_port);
			recvfile(clfd,f);
			close(clfd);
		}
	}
	if(close(sockfd) < 0)
	{
	perror("SOCKET CLOSE ERROR\n");
	exit(0);
	}
	return 0;
}


