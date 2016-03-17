#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
	char str[100];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    //printf("Please enter the message: ");
    bzero(buffer,256);
    //fgets(buffer,255,stdin);
    //int x;
    //for (x=3; x<argc;x++){
    //n = write(sockfd,argv[x],strlen(argv[x]));
    if(strcmp(argv[3],"list")==0)
    {
		n = write(sockfd,argv[3],strlen(argv[3]));
	}
	
	if(strcmp(argv[3],"order")==0)
	{
		strcpy(str,argv[3]);
		strcat(str,"#");
	int x;
	  for (x=4; x<argc;x++)
	  {
		strcat(str,argv[x]);
		strcat(str,"#");
		
	  }
	  //printf("The value of str is:%s",str);
	  n=write(sockfd,str,strlen(str));
	}
    if (n < 0) 
    {
	  error("ERROR writing to socket");
	  exit(1);
	 }
 
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) 
      {   
		  error("ERROR reading from socket");
		  exit(0);
	  }
    printf("%s\n",buffer);
    //close(sockfd);
    return 0;
}
