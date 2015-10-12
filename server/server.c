#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <openssl/md5.h>
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

int main(int argc, char* argv[])
{
	struct sockaddr_in sin;
	char buf[MAX_LINE];
	int len;
	int s, new_s;
	int opt = 1;
	int file_size;
	char file_size_s[100];
	int server_port;
	char* file_buffer;
	int file_description;
	unsigned char result[MD5_DIGEST_LENGTH];
	char hex[100];

	if (argc==2)
	{
		server_port = atoi(argv[1]);
	} else {
		fprintf(stderr,"usage: simplex-talk host\n");
		exit(1);
	}


	/* build address data structure */
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(server_port);

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

		file_size = file_exists(buf);
		printf("%i\n",file_size);
		sprintf(file_size_s,"%i",file_size);
		send(new_s,file_size_s,sizeof(file_size_s),0);

		if(file_size > 0)
		{
			//get MD5 from file
			file_description = open(buf,O_RDONLY);
			file_buffer = mmap(0,file_size, PROT_READ, MAP_SHARED, file_description, 0);
			MD5((unsigned char*) file_buffer, file_size, result);
			munmap(file_buffer, file_size);


			memset(hex,'\0',sizeof(hex));
			int i,j;
			char str[2*MD5_DIGEST_LENGTH+2];
			memset(str,'\0',sizeof(str));
			char str2[2];
			for(i=0; i<MD5_DIGEST_LENGTH; i++) {
				printf("%02x",result[i]);
				sprintf(str2,"%02x",result[i]);
				str[i*2]=str2[0];
				str[(i*2)+1]=str2[1];
			}
			str[2*MD5_DIGEST_LENGTH]='\0';
			printf("\n");
			printf("%s",str);
			printf("\n");
			printf("%i",strlen(str));


			//send MD5
			send(new_s, str,sizeof(str),0);

		} else {
			perror("File does not exist\n");
			exit(1);
		}

		close(new_s);
	}
}
