#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <openssl/md5.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#define MAX_MD5LENGTH 256
#define MAX_FILELENGTH 1000
#define MAX_FILENAME 100
int
main(int argc, char * argv[])
{
	// declare variables
	FILE *fp;
	struct hostent *hp;
	struct sockaddr_in sin;
	struct timeval tv;
	char *host;
	char md5server[100];
	char md5client[MD5_DIGEST_LENGTH];
	char buf[MAX_FILELENGTH];
	char *filename;
	int s, len, server_port;
	double start_time, end_time, nBytes, throughput;
  
	// Inputs: client host port filename
  	if (argc==4) {
    		host = argv[1];
		server_port = atoi(argv[2]);
		filename = argv[3];
	} else {
		fprintf(stderr, "usage: simplex-talk host\n");
		exit(1);
	}

	/* translate host name into peer's IP address */
	hp = gethostbyname(host);
	if (!hp) {
		fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
		exit(1);
	}

  	/* build address data structure */
  	bzero((char *)&sin, sizeof(sin));
  	sin.sin_family = AF_INET;
  	bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
  	sin.sin_port = htons(server_port);
    
  	/* active open */
  	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    		perror("simplex-talk: socket"); exit(1);
  	}
  	printf("Welcome to your first TCP client! To quit, type \'Exit\'\n");

  	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
    		perror("simplex-talk: connect");
    		close(s); exit(1);
  	}

  	/* send file length and file name to server */
	len = strlen(filename)+1;
    	if(send(s, filename, len, 0)==-1){
    		perror("client send error!"); exit(1);
    	}

	/* receive file size and check if valid */
	char file_size[30];
	if ((recv(s,file_size,sizeof(file_size),0))==-1){
		perror("Client receiver error!"); exit(1);
	}
	if (atoi(file_size)<0){
		perror("File does not exist"); exit(1);
	}
	printf("%s\n",file_size);

	/* If the file size is valid keep receiving MD5 hash of file */
	memset(md5server,'\0',sizeof(md5server));
	if (recv(s,md5server,sizeof(md5server),0)==-1){
		perror("Client receiver error!"); exit(1);
	}
	printf("len:%i\n",strlen(md5server));
	md5server[strlen(md5server)] = '\0';
	/*int i;
	for (i=0; i<sizeof(md5server); i++){
		printf("%c",md5server[i]);
	}
	printf("\n"); */
	printf("%s\n",md5server);

	/* Calculate starting time */
	gettimeofday(&tv,NULL);
	start_time = tv.tv_usec;
	//printf(
	
	/* receive the file from the server */
	if ((nBytes=recv(s,buf,sizeof(buf),0))==-1) {
		perror("Client receiver error!"); exit(1);
	}

	/* Calculate end time and throughput of file transfer */
	gettimeofday(&tv,NULL);
	end_time = tv.tv_usec; //in microsecond
	double RTT = (end_time-start_time) * pow(10,-6); //RTT in seconds
	throughput = (nBytes*pow(10,-6))/RTT;
	printf("Throughput: %d\n",throughput);

	/* close server connection */
  	close(s);

	/* covert buf to MD5 hash */

	/* compare md5 from server (MD5server) to md5 form client (MD5client)*/
	
}
