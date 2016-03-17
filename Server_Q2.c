#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/param.h>
#include <errno.h>
#include <strings.h>

#define QLEN 5

void *dvd_tcp(void*);
void *dvd_udp(void*);

typedef struct Request
{
		socklen_t fromlen;
		struct sockaddr_in from;
		char buf[256];
		int cli_sock;
}request;

typedef struct ConnPurchase
{
	int cli_count;	//Used to count TCP/UDP client connections
	int tcp_pur;	//Used to count no of DVDs purchased by TCP client
	int udp_pur;	//Used to count no of DVDs purchased by UDP client
}connpur;
	    
	        
	    pthread_mutex_t dvd_lock; 	 
	    
		char *start[] ={"Item No", "Title" , "Quantity"};
	    
	    int itemno[3] = {1001,1002,1003};
	    char *title[] = {"Star wars", "Harry Potter","Inside out"};
	    int quantity[3] = {100,80,50};	
			    
int main (int argc, char *argv[])
{
    int tsock, usock, ssock;         
    u_short servport;
    struct sockaddr_in servaddr,cliaddr;    
 
    socklen_t size;
	char recvbuf[1000];
	connpur cli_conn;
	
    fd_set rfds; 
    FD_ZERO(&rfds); 
    int nfds = getdtablesize(); 
    
    
    pthread_t thread1,thread2;
    pthread_mutex_t mut_lock=PTHREAD_MUTEX_INITIALIZER;
            
    if (argc != 2)     
    {
	    fprintf(stderr, "Usage: %s <Server Port> \n", argv[0]);
        exit(1);
    }

    servport = atoi(argv[1]);  //convert string to integer


    if ((tsock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
       	printf("TCP Socket creation failure\n");
       	exit(1);
    }


    if ((usock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
       	printf("UDP Socket creation failure\n");
       	exit(1);
    }
          
    servaddr.sin_family = AF_INET;         
    servaddr.sin_port =htons(servport);     
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    bzero(&(servaddr.sin_zero),8); 

    if (bind(tsock, (struct sockaddr *)&servaddr, sizeof(struct sockaddr))== -1) 
    {
	    printf("Bind failure of TCP socket\n");
	    exit(1);
    }

	if (bind(usock, (struct sockaddr *)&servaddr, sizeof(struct sockaddr))== -1) 
    {
	    printf("Bind failure of UDP socket\n");
	    exit(1);
    }

	if (listen(tsock, QLEN) == -1) 
	{
		printf("Listening failure to TCP socket");
		exit(1);
	}
    	
    printf("\n Waiting for client connections \n");
    fflush(stdout);
    cli_conn.cli_count=0;
    while(1)
	{   
		printf("The number of connections in total are:%d\n",cli_conn.cli_count);
		printf("===============================================================\n");
		FD_SET(tsock, &rfds); //add tsock to set
		FD_SET(usock, &rfds); //add usock to set
		
		if (select(nfds, &rfds, (fd_set *)0, (fd_set *)0,(struct timeval *)0) < 0)
	    {
		printf("select() Failure\n");
	    	exit(1);
		}

		if(FD_ISSET(tsock,&rfds))
		{
			size = sizeof(struct sockaddr_in);
			ssock = accept(tsock, (struct sockaddr *)&cliaddr,&size);
			if (ssock < 0) {
				if (errno == EINTR)
					continue;
				}
			
			printf("Connection from TCP client successful (%s , %d)\n\n",inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));
			
					pthread_mutex_lock(&mut_lock);
					cli_conn.cli_count++;
					pthread_mutex_unlock(&mut_lock);
	
			pthread_create(&thread1, NULL, (void *(*)(void*))dvd_tcp, (void *)&ssock);

		}
		if (FD_ISSET(usock, &rfds)) 
		{			
			request req;
			
			size=sizeof(struct sockaddr_in);
			bzero(recvbuf,sizeof(recvbuf));
			int n = recvfrom(usock,recvbuf,sizeof(recvbuf),0,(struct sockaddr *)&cliaddr,&size);
			//printf("The value of req.buf is %s\n",req.buf);
			if (n < 0)
			 {
				perror("ERROR reading from socket");
			 }
			 
					pthread_mutex_lock(&mut_lock);
					cli_conn.cli_count++;
					pthread_mutex_unlock(&mut_lock);
					
			pthread_attr_t uta;
			(void) pthread_attr_init(&uta);
			(void) pthread_attr_setdetachstate(&uta,PTHREAD_CREATE_DETACHED);
			
			req.cli_sock=usock;
			req.from=cliaddr;
			strncpy(req.buf,recvbuf,sizeof(req.buf));
			
			printf("Connection from UDP client successful(%s , %d)\n\n",inet_ntoa(req.from.sin_addr),ntohs(req.from.sin_port));

			pthread_create(&thread2,&uta,(void *(*)(void*))dvd_udp, (void *)&req);
			
		}
	}
	pthread_join(thread1,NULL);
	pthread_join(thread2,NULL);
	close(tsock);
    return 0;
}

void *dvd_tcp(void *arg1)
{
		int tcpsock = *(int*)arg1;
		int n;
//----------------PLACE HERE-------------------
		char buffer[256];
	    char buffer1[1000];
//---------------------------------------------		
	    
	    bzero(buffer,256);
     
		n = read(tcpsock,buffer,255);

		if (n < 0) 
		 {
			perror("ERROR reading from socket");
		 }
//---------------LIST CONDITION---------------------------		 
		 if(strcmp(buffer,"list") == 0)
		 {
			  pthread_mutex_lock(&dvd_lock);
			  sprintf(buffer1,"%s\t\t%s\t\t\t%s\n",start[0],start[1],start[2]);
			  int i;
			  for (i=0;i<3;i++)
			  {
				  sprintf(buffer1,"%s%d\t\t%s\t\t%d\n",buffer1,itemno[i],title[i],quantity[i]);
			  }
			n= write(tcpsock,buffer1,strlen(buffer1));
			pthread_mutex_unlock(&dvd_lock);
		 }
//---------------ORDER CONDITION-------------------------------
		else if(strncmp(buffer,"order",5)==0)
			 {
				 //printf("Order Received: %s\n",buffer);
				 pthread_mutex_lock(&dvd_lock);
				 char *token;
				 token=strtok(buffer,"#");
				 int ord_itemno,ord_quantity;
				 int flag=0;
				 while(token!=NULL)
				 {
					 if(1==flag)
					 {
						 ord_itemno=atoi(token);
					 }
					 else if(2==flag)
					 {
						 ord_quantity= atoi(token);
					 }			 
					 flag++;
					 token=strtok(NULL,"#");
				 }
				 int i,exist_flag=0;
				 for(i=0;i<3;i++)
				 {
					 
					 if(ord_itemno==itemno[i])
					 {
						 exist_flag=1;
						 if(quantity[i]>0)
						 {
							 quantity[i]-=ord_quantity;
							 //printf("The new quantity is:%d",quantity[i]);
							 strcpy(buffer1,"OK");
							 n= write(tcpsock,buffer1,strlen(buffer1));
						 }
						 else if(quantity[i]<=0)
						 {
							 strcpy(buffer1,"Not Available");
							 n= write(tcpsock,buffer1,strlen(buffer1));
						 }
					 }								 
				 }
				 if(exist_flag!=1)
				 {
					strcpy(buffer1,"Please ask for a valid Item number.Usage:order[ItemNo][Quantity]");
					n= write(tcpsock,buffer1,strlen(buffer1));
				}
				
				if(exist_flag==1)
				 {
				printf("\nPurchase Record TCP:\n Item No: %d Quantity: %d\n", ord_itemno,ord_quantity);
				printf("===============================================================\n");

				 }
				 pthread_mutex_unlock(&dvd_lock);
			 }
			 
		 close(tcpsock);
}

void *dvd_udp(void *arg2)
{
	char buffer[256];
	char buffer1[1000];
	
	request *req_udp=arg2;
	int n=0;
	
	bzero(buffer,sizeof(buffer));
	
	
	int udpsock = req_udp->cli_sock;
	//printf("the value of udpsock is %d\n",udpsock);-------------------FOR DEBUGGING
	struct sockaddr_in cliaddr;
	cliaddr=req_udp->from;
	//printf("The client ip address is %s",inet_ntoa(cliaddr.sin_addr));-------------------FOR DEBUGGING
	
	socklen_t clilen;
	clilen = sizeof(cliaddr);
	
	strcpy(buffer,req_udp->buf);
	//printf("The value of buffer is %s",buffer);-------------------FOR DEBUGGING
		
//---------------UDP LIST CONDITION---------------------------------		 
		if(strcmp(buffer,"list") == 0)
		 {
	  		pthread_mutex_lock(&dvd_lock);
			  bzero(buffer1,sizeof(buffer1));
			  sprintf(buffer1,"%s\t\t%s\t\t\t%s\n",start[0],start[1],start[2]);
			  int i;
			  for (i=0;i<3;i++)
			  {
				  sprintf(buffer1,"%s%d\t\t%s\t\t%d\n",buffer1,itemno[i],title[i],quantity[i]);
			  }
			 //printf("The value of buffer1 is %s\n",buffer1);
			n=sendto(udpsock,buffer1,strlen(buffer1),0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
					pthread_mutex_unlock(&dvd_lock);

		}
//--------------UDP ORDER CONDITION-----------------------------------
else if(strncmp(buffer,"order",5)==0)
	 {
		 		pthread_mutex_lock(&dvd_lock);
		 bzero(buffer1,sizeof(buffer1));
		 //printf("Order Received: %s\n",buffer);-------------------FOR DEBUG

		 char *token;
		 token=strtok(buffer,"#");
		 int ord_itemno,ord_quantity;
		 int flag=0;
		 while(token!=NULL)
		 {
			 if(1==flag)
			 {
				 ord_itemno=atoi(token);
			 }
			 else if(2==flag)
			 {
				 ord_quantity= atoi(token);
			 }
			 flag++;
			 token=strtok(NULL,"#");
		 }
		 int i,exist_flag=0;
		 for(i=0;i<3;i++)
		 {

			 if(ord_itemno==itemno[i])
			 {
				 exist_flag=1;
				 if(quantity[i]>0)
				 {
					 quantity[i]-=ord_quantity;
					 //printf("The new quantity is:%d",quantity[i]);
					 strcpy(buffer1,"OK");
					 n= sendto(udpsock,buffer1,strlen(buffer1),0,(struct sockaddr *) &cliaddr,clilen);
				 }
				 else if(quantity[i]<=0)
				 {
					 strcpy(buffer1,"Not Available");
					 n= sendto(udpsock,buffer1,strlen(buffer1),0,(struct sockaddr*) &cliaddr,clilen);
				 }
			 }
		 }
		 if(exist_flag!=1)
		 {
			strcpy(buffer1,"Please ask for a valid Item number. Usage:order[ItemNo][Quantity]");
			n= sendto(udpsock,buffer1,strlen(buffer1),0,(struct sockaddr*) &cliaddr,clilen);
		}

		if(exist_flag==1)
		 {
			printf("\nPurchase Record UDP:\n Item No: %d Quantity: %d\n", ord_itemno,ord_quantity);
			printf("===============================================================\n");
		 }
		pthread_mutex_unlock(&dvd_lock);
	 }
}
