#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#define SERVER_PORT 41017
#define MAX_PENDING 5
#define MAX_LINE 256

int file_exists(char * filename)
{
	FILE * file;
	int size;
	if (file = fopen(filename, "r"))
	{
		fseek(file,0,SEEK_END);
		size = ftell(file);
		fseek(file,0,SEEK_SET);
		fclose(file);
		return size;
	}
	return -1;
}


int main()
{
	struct sockaddr_in sin;
	char buf[MAX_LINE];
	int len;
	int s, new_s;
	int opt = 1;
	int file_size;
	char file_size_s[100];

	/* build address data structure */
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(SERVER_PORT);

	/* setup passive open */
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("simplex-talk: socket");
		exit(1);
	}

	// set socket option
	if ((setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)& opt, sizeof(int)))<0){
		perror ("simplex-talk:setscokt");
		exit(1);
	}
	if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
		perror("simplex-talk: bind"); exit(1);
	}
	if ((listen(s, MAX_PENDING))<0){
		perror("simplex-talk: listen"); exit(1);
	} 

	printf("Welcome to the first TCP Server!\n");
	/* wait for connection, then receive and print text */
	while(1) {
		if ((new_s = accept(s, (struct sockaddr *)&sin, &len)) < 0) {
			perror("simplex-talk: accept");
			exit(1);
		}


		if((len=recv(new_s, buf, sizeof(buf), 0))==-1){
			perror("Server Received Error!");
			exit(1);
		}
		if (len==0) break;
		printf("TCP Server Received:%s", buf);

		file_size = file_exists(buf);
		sprintf(file_size_s,"%i",file_size);
		send(new_s,file_size_s,sizeof(file_size_s)/sizeof(char),0);

		if(file_size > 0)
		{

		} else {
			perror("File does not exist");
			exit(1);
		}

		printf("Client finishes, close the connection!\n");
		close(new_s);
	}
}
