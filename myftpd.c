#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <dirent.h>

#define PATH_SIZE 256
#define BUF_SIZE 1024

void exMsg(int sock);
int ftp(int sock);
int quit(int sock);
int pwd(int sock);
int cd(int sock, int length);
int dir(int sock, int length);
int get(int sock, int length);
int put(int sock, int length);
int error(int sock);

struct myftph{
	uint8_t 	type;
	uint8_t		code;
	uint16_t	length;
};

int main(int argc, char* argv[])
{
	int pid;
	int sock0, sock;
	struct sockaddr_in server;
	struct sockaddr_in client;
	unsigned int cliAddrLen;
	unsigned short serverPort;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <TCP SERVER PORT>\n", argv[0]);
		exit(1);
	}
	//get server port number
	serverPort = atoi(argv[1]);
	
	//create a socket
	if ((sock0 = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "sock failed\n");
		exit(2);
	}
	//initialize
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(serverPort);

	//bind server addr to the socket
	if (bind(sock0, (struct sockaddr *) &server, sizeof(server)) < 0) {
		fprintf(stderr, "bind failed\n");
		exit(3);
	}
	//wait connection request
	listen(sock0, 5); 

	while (1) {
		//accept connection request
		cliAddrLen = sizeof(client);
		sock = accept(sock0, (struct sockaddr *)&client, &cliAddrLen); 
		fprintf(stdout, "connected client %s\n", 
				inet_ntoa(client.sin_addr));
		write(sock, "HELLO", 5);
		
		if ((pid = fork()) == 0) {
			//child
			for (;;) {
				//exMsg(sock);
				if (ftp(sock) == -1) {
					break;
				}
			}
		} else if (pid == -1) {
			fprintf(stderr, "fork error\n");
			close(sock);
			exit(1);
		} else {
		}
	}
	close(sock0);

	return 0;
}

void exMsg(int sock)
{
	struct myftph header;
	int n;
	for (;;) {
		//receive message
		n = read(sock, &header, sizeof(header));
		fprintf(stderr, "type = %d, code = %d, length = %d\n",
				header.type, header.code, header.length);
		//send message
		write(sock, &header, sizeof(header));
	}
}

int ftp(int sock)
{
	struct myftph header;

	read(sock, &header, sizeof(header));

	fprintf(stderr, "received message\n");
	fprintf(stderr, "header.type = 0x%02x\n", header.type);

	if (header.type == 0x01) {
		return quit(sock);
	} else if (header.type == 0x02) {
		return pwd(sock);
	} else if (header.type == 0x03) {
		return cd(sock, header.length);
	} else if (header.type == 0x04) {
		return dir(sock, header.length);
	} else if (header.type == 0x05) {
		return get(sock, header.length);
	} else if (header.type == 0x06) {
		return put(sock, header.length);
	} else {
		return error(sock);
	}
	return -1;
}

int quit(int sock)
{
	struct myftph header;

	header.type = 0x10;
	header.code = 0x00;
	header.length = 0;

	write(sock, &header, sizeof(header));
	return -1;
}

int pwd(int sock)
{
	struct myftph header;
	char path[PATH_SIZE];

	getcwd(path, PATH_SIZE);
	fprintf(stderr, "path = %s\n", path);

	header.type = 0x10;
	header.code = 0x00;
	header.length = strlen(path);

	write(sock, &header, sizeof(header));

	write(sock, &path, header.length);
	return 0;
}

int cd(int sock, int length)
{
	char path[PATH_SIZE];
	struct myftph header;

	read(sock, &path, length * sizeof(char));
	path[length] = '\0';

	if (access(path, F_OK) == 0) {
		if (chdir(path) == 0) {
			header.type = 0x10;
			header.code = 0x00;
		} else {
			header.type = 0x12;
			header.code = 0x01;
		}
	} else {
			header.type = 0x12;
			header.code = 0x00;
	}
	header.length = 0;
	write(sock, &header, sizeof(header));
	
	return 0;
}	

int dir(int sock, int length)
{
	int i;
	int name;
	DIR *dir;
	struct dirent *ds;
	struct myftph header;
	char path[4 * PATH_SIZE] = ".";

	if (length > 0) {
		read(sock, &path, length * sizeof(char));
		path[length] = '\0';
	}

	dir = opendir(path);
	name = 0;
	for (ds = readdir(dir); ds != NULL; ds = readdir(dir)) {
		for (i = 0; i < strlen(ds->d_name); i++) {
			path[name] = ds->d_name[i];
			name++;
		}
		path[name] = '\n';
		name++;
	}
	name--;
	path[name] = '\0';
	closedir(dir);

	header.type = 0x10;
	header.code = 0x01;
	header.length = strlen(path);
	write(sock, &header, sizeof(header));

	write(sock, &path, header.length * sizeof(char));
	return 0;
}

int get(int sock, int length)
{ 
	int i, n;
	char path[PATH_SIZE];
	char buf[BUF_SIZE];
	struct myftph header;

	FILE *fp;
	
	read(sock, &path, length);
	path[length] = '\0';
	if (access(path, F_OK) == 0) {
		if (access(path, R_OK) == 0) {
			if ((fp = fopen(path, "r")) == NULL) {
				fprintf(stderr, "cannot open file\n");
				return 1;
			}
			header.type = 0x10;
			header.code = 0x01;
			header.length = 0;
			write(sock, &header, sizeof(header));

			for (;;) {
				n = fread(buf, sizeof(char), BUF_SIZE, fp);
				if (n < BUF_SIZE) {
					break;
				}
				header.type = 0x20;
				header.code = 0x01;
				header.length = BUF_SIZE;

				write(sock, &header, sizeof(header));
				write(sock, &buf, BUF_SIZE * sizeof(char));
			}
			fprintf(stderr, "did break\n");
			header.type = 0x20;
			header.code = 0x00;
			header.length = n;
			write(sock, &header, sizeof(header));
			write(sock, &buf, n * sizeof(char));
			fclose(fp);
			return 0;
		} else {
			//no access authorization
			header.type = 0x12;
			header.code = 0x01;
			header.length = 0;
			write(sock, &header, sizeof(header));
			write(sock, &buf, header.length);
			fprintf(stderr, "no access authorization\n");
			return 1;
		}
	} else {
		//no file
		header.type = 0x12;
		header.code = 0x00;
		header.length = 0;
		write(sock, &header, sizeof(header));
		fprintf(stderr, "no such a file\n");
		return 2;
	}
	return 0;
}

int put(int sock, int length)
{
	int i, n;
	char path[PATH_SIZE];
	char buf[BUF_SIZE];
	struct myftph header;
	FILE *fp;

	read(sock, &path, length * sizeof(char));
	path[length] = '\0';
	if ((fp = fopen(path, "w")) == NULL) {
		fprintf(stderr, "cannot open file\n");
		header.type = 0x12;
		header.code = 0x01;
		header.length = 0;
		write(sock, &header, sizeof(header));
		return 1;
	}
	header.type = 0x10;
	header.code = 0x00;
	header.length = 0;
	write(sock, &header, sizeof(header));
	for (;;) {
		read(sock, &header, sizeof(header));
		n = read(sock, &buf, header.length * sizeof(char));

		fwrite(buf, header.length * sizeof(char), 1, fp);

		if (header.code == 0x00) {
			fputc('\0', fp);
			fprintf(stderr, "break\n");
			break;
		}
	}
	fclose(fp);
	return 0;
}

int error(int sock)
{
	return 0;
}

