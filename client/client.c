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
#define MAX_MD5LENGTH 100
#define MAX_FILELENGTH 2000
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
	char buf[MAX_FILELENGTH];
	char md5server[MAX_MD5LENGTH];
	unsigned char md5client[MD5_DIGEST_LENGTH];
	char *filename;
	int s, len, server_port;
	float start_time, end_time, nBytes, throughput;
  
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

	/* If the file size is valid keep receiving MD5 hash of file */
	memset(md5server,'\0',sizeof(md5server));
	while(strlen(md5server)==0){
		recv(s,md5server,sizeof(md5server),0);
	}
	md5server[strlen(md5server)] = '\0';

	/* Calculate starting time */
	gettimeofday(&tv,NULL);
	start_time = tv.tv_usec;
	
	/* receive the file from the server. Get line by line and add to buffer */
	int n;
	char line[100];
	memset(buf,'\0',sizeof(buf));
	memset(line,'\0',sizeof(line));
	while ((n=recv(s,line,sizeof(line),0))>0) {
		strcat(buf,line);
		nBytes += n;
		memset(line,'\0',sizeof(line));
	}

	/* Calculate end time and throughput of file transfer */
	gettimeofday(&tv,NULL);
	end_time = tv.tv_usec; //in microsecond
	float RTT = (end_time-start_time) * pow(10,-6); //RTT in seconds
	throughput = (nBytes*pow(10,-6))/RTT;

	/* close server connection */
  	close(s);

	/* covert buf to MD5 hash */
	MD5((unsigned char*) buf, atoi(file_size), md5client);
	munmap(buf, atoi(file_size));

	int i,j;
	char str[2*MD5_DIGEST_LENGTH+2];
	memset(str,'\0',sizeof(str));
	char str2[2];
	for(i=0; i<MD5_DIGEST_LENGTH; i++) {
		sprintf(str2,"%02x",md5client[i]);
		str[i*2]=str2[0];
		str[(i*2)+1]=str2[1];
	}
        str[2*MD5_DIGEST_LENGTH]='\0';

	char md5str[strlen(str)+1];
	memcpy(md5str,str,strlen(str));
	md5str[strlen(str)] = '\0';

	/* compare md5 from server (MD5server) to md5 form client (MD5client)*/
	if (strcmp(md5server,md5str)!=0){
		printf("File hashes do not match - bad transfer\n");
		exit(1);
	} else {
		printf("Hash matches\n");
		printf("%.0f bytes transferred in %f seconds. Throughput: %f Megabytes/sec. File MDSum: %s.\n",nBytes,RTT,throughput,md5server);
	}
	exit(1);
}
