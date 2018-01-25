#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include<pthread.h>
#include <arpa/inet.h>
#include<iostream>
#include<vector>
#include <fstream>
#include <iterator>
#include<cctype>
#include <cstring>
using namespace std;
char dumy[20]="FILE NOT FOUND";
char op[30]="FILESHARED";
char op1[30]="FILEREMOVED";
int sockfd, newsockfd,port,n;
struct sockaddr_in srv_addr, cli_addr;
socklen_t cli_len;
char filename [30];   //repofile
char clientfile[30]; //clientfile
char buffer[256];   //client info
char addr [INET_ADDRSTRLEN]; 
vector<string> v; //repofile
vector<string> t; //client file
void* fun(void *vargp)
{
	if (newsockfd < 0)
	{
		perror("ERROR IN ACCEPT");
		exit(1);
	}
	inet_ntop(AF_INET, &(cli_addr.sin_addr),addr, INET_ADDRSTRLEN);//convert numeric IP to command line
	printf("CONNECTED TO CLIENT %s:%d\n",addr, ntohs(cli_addr.sin_port) );
	char alias[20],client[50];
	memset(alias, 0, sizeof(alias));
	memset(alias, 0, sizeof(client));
	read(newsockfd,client,49);    //reading client info from client
	t.push_back(client);
	ofstream output(clientfile, ios::app);
	ostream_iterator<string> output_it(output, "\n");
	copy(t.begin(), t.end(), output_it); 
	output.close();
	t.clear();
	int e=0;
	while(client[e]!=':') //alias name
	{
		alias[e]=client[e];
		e++;
	}
	alias[e]='\0';
	while(1)
	{
		memset(buffer, 0, sizeof(buffer));
     	n = read(newsockfd,buffer,255);
		char cmd[10];
		char path[30];
		char file[20];
		char join[50];
		char flag[1];
		memset(flag, 0, sizeof(flag));
		memset(join, 0, sizeof(join));
		memset(cmd, 0, sizeof(cmd));
		memset(path, 0, sizeof(path));
		memset(file, 0, sizeof(file));
		int i=0;
		while(buffer[i]!=' ' && buffer[i]!='\0')   //command     
		{
			cmd[i]=buffer[i];
			i++;
		}
		cmd[i]='\0';
		printf("%s\n",cmd);
		i=i+2;
		int j=0;
		while(buffer[i]!='"' && buffer[i-2]!='\0') //path
		{
			path[j]=buffer[i];
			i++;
			j++;
		}
		path[j]='\0';
		i=strlen(buffer)-1;
		j=0;
		while(buffer[i]!='/' && buffer[i]!='\0') 
		{
			i--;	
		}
		i++;
		while(buffer[i]!='"' && buffer[i]!='\0') //file
		{
			file[j]=buffer[i];
			j++;
			i++;
		}
		file[j]='\0';
		strcat(join,file);
		strcat(join,":");
		strcat(join,path);
		strcat(join,":");
		strcat(join,alias);
		if(strcmp(cmd,"exit1")==0)
		{
			vector<string> w1;
			string line;
			ifstream input(clientfile,ios::app);
			while(getline(input,line))
				w1.push_back(line);
			input.close();       
			for (vector<string> ::iterator it=w1.begin();it!=w1.end();it++)
			{
				
				string z=*it;
				if(strcmp(client,z.c_str())==0)
				{
					w1.erase(it);
					ofstream output(clientfile);
					ostream_iterator<string> output_it(output, "\n");
					copy(w1.begin(), w1.end(), output_it); 
					send(newsockfd,flag,strlen(flag),0);
					close(newsockfd);
					return NULL;
				}
			}
		}/*client_file entry deleted*/
		else if(strcmp(cmd,"share")==0) ///share
		{
			v.push_back(join);
			ofstream output_file(filename, ios::app);
			ostream_iterator<string> output_iterator(output_file, "\n");
			copy(v.begin(), v.end(), output_iterator); 
			v.clear(); 
			send(newsockfd,op,strlen(op),0);  
			                                  
		}/*share*/
		else if(strcmp(cmd,"del")==0) ///del
		{
			vector<string> w;
			string line;
			ifstream inputfile(filename,ios::app);
			while(getline(inputfile,line))
				w.push_back(line);
			inputfile.close();       
			for (vector<string> ::iterator it=w.begin();it!=w.end();it++)
			{
				
				string z=*it;
				if(strcmp(join,z.c_str())==0)
				{
					w.erase(it);
					ofstream output_file(filename);
					ostream_iterator<string> output_iterator(output_file, "\n");
					copy(w.begin(), w.end(), output_iterator); 
					break;
				}
			}
			send(newsockfd,op1,strlen(op1),0); 
		}/*del*/
		else if(strcmp(cmd,"search")==0 || strcmp(cmd,"get")==0 )
		{
			vector<string> w2;
			string line;
			ifstream inputfile(filename,ios::app);
			while(getline(inputfile,line))
				w2.push_back(line);
			inputfile.close();       
			for (vector<string> ::iterator it=w2.begin();it!=w2.end();it++)
			{
				
				string z=*it;
				char fi[10];
				int i=0;
				while(z[i]!=':')
				{
					fi[i]=z[i];
					i++;
				}
				fi[i]='\0';
				if(strcmp(fi,path)==0)
				{
					char alias2[20];
					int x=0;
					int y=z.length()-1;
					while(z[y]!=':' && z[y]!='\0')
					{
						y--;
					}
					y++;
					int j=0;
					while(z[y]!='\0')
					{
						alias2[j]=z[y];
						j++;
						y++;
					}
					alias2[j]='\0';
					vector<string> w1;
					string line;
					ifstream input(clientfile,ios::app);
					while(getline(input,line))
						w1.push_back(line);
					input.close();       
					for (vector<string> ::iterator it=w1.begin();it!=w1.end();it++)
					{
						string z=*it;
						char fi[10];
						int i=0;
						while(z[i]!=':')
						{
							fi[i]=z[i];
							i++;
						}
						fi[i]='\0';
						if(strcmp(alias2,fi)==0)
						{
							send(newsockfd,z.c_str(),z.length(),0);
						}
					}
				}
			}
		}/*search*/
		else
				send(newsockfd,dumy,strlen(dumy),0);
	}
	close(newsockfd);
	return NULL;
}

int main(int argc, char* argv[])
{
	
	memset(&srv_addr, 0, sizeof(srv_addr)); 
	memset(&cli_addr, 0, sizeof(cli_addr)); 
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (argc < 5)
	{
		printf("usage: %s <IP address_server> [port number_server] [repofile] [client file] [server root]\n", argv[0]);
		exit(0);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if ( inet_pton(AF_INET, argv[1], &(srv_addr.sin_addr)) < 1 ) // convert command line argument to numeric IP 
	{
		printf("INVALID IP\n");
		exit(0);
	}
	port = atoi(argv[2]);
	srv_addr.sin_port = htons(port);
	
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
	printf("LISTENING ON PORT %d ...\n", ntohs(srv_addr.sin_port));
	if( listen(sockfd, 5) < 0 )
	{
		perror("LISTENING ERROR");
		exit(0);
	}
	strcpy(filename,argv[3]); // copy repo filename
	strcpy(clientfile,argv[4]); // copy client filename
	pthread_t tid;
		for (int i = 0; i < 3; i++)
		{	
			
			cli_len = sizeof(cli_addr);
			newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr,&cli_len);
			pthread_create(&tid, NULL, fun, NULL);
		}
    		pthread_exit(NULL);
    close(sockfd); 
	return 0;
}

