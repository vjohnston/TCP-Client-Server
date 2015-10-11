#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define MAX_LINE 256
#define MAX_FILENAME 100
int
main(int argc, char * argv[])
{
	FILE *fp;
	struct hostent *hp;
	struct sockaddr_in sin;
	char *host;
	char md5hash[MAX_LINE];
	char *filename;
	int s;
	int len;
	int server_port;
  
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
	char file_size[20];
	if ((recv(s,file_size,sizeof(file_size),0))==-1){
		perror("client receiver error!"); exit(1);
	}
	if (atoi(file_size)<0){
		perror("File does not exist"); exit(1);
	}
	
	/* If the file size is valid keep receiving MD5 hash of file */
	
	

  	close(s);
}
