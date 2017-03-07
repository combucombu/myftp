#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>


#define CMD_MAX 256
#define PATH_SIZE 256
#define BUF_SIZE 1024
int getCmd(char *);
int splitCmd(char *command, char* av[]);

int ftp(char *command, int sock);
int quit(int sock);
int pwd(int sock);
int cd(int sock, char* av[]);
int dir(int sock, char* av[], int path_num);
int lpwd();
int lcd(char *av[]);
int ldir(char *av[], int path_num);
int get(int sock, char* av[], int path_num);
int put(int sock, char* av[], int path_num);


struct myftph{
	uint8_t		type;
	uint8_t		code;
	uint16_t	length;
};

int main(int argc, char* argv[])
{
	int n;
	int sock;
	struct sockaddr_in server;
	unsigned short serverPort;
	char *serverIP;
	char command[CMD_MAX];
	char message[32];
	char path[PATH_SIZE];

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <Server IP><Server port>\n", argv[0]);
		exit(1);
	} else {
		serverPort = atoi(argv[2]);
	}

	serverIP = argv[1];
	
	//create a socket
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "sock failed\n");
		exit(2);
	}
	//initialize
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(serverIP);
	server.sin_port = htons(serverPort);

	//connect requset to server
	connect(sock, (struct sockaddr *)&server, sizeof(server));

	//receive date from server
	memset(message, 0, sizeof(message));
	n = read(sock, message, sizeof(message));
	printf("%d, %s\n", n, message);
	//send and receive loop
	for (;;) {
		//get command
		getcwd(path, PATH_SIZE);
		fprintf(stdout, "[myFTP] %s\n$ ", path);
		getCmd(command);
		//launch command
		if (ftp(command, sock) == -1) {
			break;
		}
	}
	close(sock);
	return 0;	
}
int getCmd(char* command)
{
	int i, k;
	char c;
	for (i = 0, k = 0; i < CMD_MAX; i++, k++) {
		if ((c = fgetc(stdin)) == EOF) {
			fprintf(stderr, "Input Erorr\n");
			exit(1);
		} else {
			if (c == '\0') {
				command[k] = c;
				return i;
			} else if (c == '\n') {
				command[k] = '\0';
				return i;
			} else {
				command[k] = c;
			} 
		}
	}
	fprintf(stderr, "Command too long");
	exit(2);
	return i;
}
int splitCmd(char* command, char* av[])
{
	int i, j;
	av[0] = command;
	for (i = 0, j = 1; i < CMD_MAX; i++) {
		if (command[i] == '\0') {
			break;
		} else if (command[i] == ' ' || command[i] == '\t') {
			command[i] = '\0';
			if (command[i + 1] != ' ' && command[i + 1] != '\0') {
	                 av[j++] = &command[i + 1];
			}
		}
	}
	for (i = 0; i < j; i++) {
		fprintf(stderr, "av[%d] = %s\n", i, av[i]);
	}
	return j;
}
int ftp(char* command, int sock)
{
	int ac, path_num;
	char* av[3];
	
	ac = splitCmd(command, av);
	path_num = ac -1;
	
	fprintf(stderr, "path_num = %d\n", path_num);

	if (strcmp(av[0], "quit") == 0) {
		quit(sock);
		return -1;
	} else if (strcmp(av[0], "pwd") == 0) {
		pwd(sock);
		return 0;
	} else if (strcmp(av[0], "cd") == 0) {
		if (path_num == 1) {
			cd(sock, av);
			return 0;
		} else {
			fprintf(stderr, "Usage: cd <PATH>\n");
			return 1;
		}
	} else if (strcmp(av[0], "dir") == 0) {
		dir(sock, av, path_num);
		return 0;
	} else if (strcmp(av[0], "lpwd") == 0) {
		lpwd();
		return 0;
	} else if (strcmp(av[0], "lcd") == 0) {
		if (path_num == 1) {
			lcd(av);
			return 0;
		} else {
			fprintf(stderr, "Usage: lcd <PATH>\n");
			return 1;
		}
	} else if (strcmp(av[0], "ldir") == 0) {
		ldir(av, path_num);
		return 0;
	} else if (strcmp(av[0], "get") == 0) {
		if (path_num > 0) {
			get(sock, av, path_num);
			return 0;
		} else {
			fprintf(stderr, "Usage: get <input file> [output file]\n");
			return 1;
		}
	} else if (strcmp(av[0], "put") == 0) {
		if (path_num > 0) {
			put(sock, av, path_num);
			return 0;
		} else {
			fprintf(stderr, "Usage: put <input file> [output file]\n");
			return 1;
		}
	} else {
		fprintf(stderr, "unkown command: %s\n", av[0]);
		return 1;
	}

	
	return 0;
}
int quit(int sock)
{
	struct myftph header;

	header.type = 0x01;
	header.code = 0x00;
	header.length = 0;

	write(sock, &header, sizeof(header));
	read(sock, &header, sizeof(header));
		
	fprintf(stderr, "received reply\n");
	fprintf(stderr, "header type = 0x%02x\n", header.type);

	return 1;
}

int pwd(int sock)
{
	char path[PATH_SIZE];
	struct myftph header;

	header.type = 0x02;
	header.code = 0x00;
	header.length = 0;

	write(sock, &header, sizeof(header));

	read(sock, &header, sizeof(header));
	read(sock, &path, header.length * sizeof(char));
	path[header.length] = '\0';

	fprintf(stdout, "%s\n", path);
	return 0;
}


int cd(int sock, char* av[])
{
	struct myftph header;
	char path[PATH_SIZE];

	fprintf(stderr, "here is cd\n");

	header.type = 0x03;
	header.code = 0x00;

	strcpy(path, av[1]);
	header.length = strlen(path);
	
	write(sock, &header, sizeof(header));
	
	
	write(sock, &path, header.length * sizeof(char));
	read(sock, &header, sizeof(header));
	if (header.type == 0x10) {
		fprintf(stdout, "changed directory\n");
	} else if (header.type == 0x12) {
		if (header.code == 0x00) {
			fprintf(stderr, "No such a directory: %s\n", path);
			return 1;
		} else if (header.code == 0x01) {
			fprintf(stderr, "No access authorization: %s\n", path);
			return 1;
		}
	}
	return 0;
}

int dir(int sock, char *av[], int path_num)
{
	struct myftph header;
	char path[PATH_SIZE];

	header.type = 0x04;
	header.code = 0x00;

	if (path_num == 1) {
		strcpy(path, av[1]);
		header.length = strlen(path);
	} else {
		header.length = 0;
	}
	write(sock, &header, sizeof(header));
	
	if (path_num > 0) {
		write(sock, &path, header.length * sizeof(char));
	}

	read(sock, &header, sizeof(header));
	read(sock, &path, header.length * sizeof(char));
	path[header.length] = '\0';
	fprintf(stdout, "%s\n", path);
	return 0;
}

int lpwd()
{	
	char path[PATH_SIZE];

	if (access(path, F_OK) == 0) {
		getcwd(path, PATH_SIZE);
	}

	fprintf(stdout, "%s\n", path);	
	return 0;
}

int lcd(char *av[])
{
	char path[PATH_SIZE];

	strcpy(path, av[1]);

	if (access(path, F_OK) == 0) {
		if (chdir(path) == 0) {
			fprintf(stdout, "changed directory\n");
			return 0;
		} else {
			fprintf(stderr, "No access authorization: %s\n", path);
			return 1;
		}
	} else {
		fprintf(stderr, "No such a directory: %s\n", path);
		return 1;
	}
}

int ldir(char *av[], int path_num)
{
	DIR *dir;
	struct dirent *ds;
	char path[PATH_SIZE] = ".";

	if (path_num == 1) {
		strcpy(path, av[1]);
	}
	if (access(path, F_OK) == 0) {
		dir = opendir(path);
		for(ds = readdir(dir); ds != NULL; ds = readdir(dir) ){
			printf("%s\n", ds->d_name);
		}
		closedir(dir);
		fprintf(stderr, "%s", path);
	} else {
		fprintf(stderr, "No directory: %s\n", path);
		return 1;
	}
	return 0;
}

int get(int sock, char *av[], int path_num)
{
	int i, n;
	char path[PATH_SIZE];
	char buf[BUF_SIZE];
	struct myftph header;
	FILE *fp;

	strcpy(path, av[1]);
	header.type = 0x05;
	header.code = 0x00;
	header.length = strlen(path);
	write(sock, &header, sizeof(header));

	write(sock, &path, header.length);
	read(sock, &header, sizeof(header));

	if (header.type == 0x10) {
		if (path_num == 1) {
			if ((fp = fopen(av[1], "w")) == NULL) {
				fprintf(stderr, "cannot open file %s", av[1]);
				return 1;
			}
			fprintf(stderr, "opened %s\n", av[2]);
		} else if (path_num ==2) {
			if ((fp = fopen(av[2], "w")) == NULL) {
				fprintf(stderr, "cannot open file %s", av[2]);
				return 2;
			}
			fprintf(stderr, "opened %s\n", av[2]);
		}
		for (i = 1;; i++) {
			if (i % 10000 ==0) {
				fprintf(stdout, "%d\n", i);
			}
			read(sock, &header, sizeof(header));
			n = read(sock, &buf, header.length * sizeof(char));
			
			fwrite(buf, header.length * sizeof(char), 1, fp);
			
			if (header.code == 0x00) {
				fputc('\0', fp);
fprintf(stderr, "break\n");
				break;
			}
		}
		fprintf(stdout, "get %s ", av[1]);
		if (path_num == 2) {
			fprintf(stdout, "as %s", av[2]);
		}
		fprintf(stdout, "\n");
		fclose(fp);
	} else if (header.type == 0x12) {
		//file erorr
		if (header.code == 0x01) {
			fprintf(stderr, "no access authorization\n");
			return 1;
		} else if (header.code == 0x00) {
			fprintf(stderr, "no such a file\n");
			return 1;
		}
	}

	return 0;
}

int put(int sock, char *av[], int path_num)
{
	int i, n;
	char path[PATH_SIZE];
	char buf[BUF_SIZE];
	struct myftph header;
	FILE *fp;

	if (path_num == 2) {
		strcpy(path, av[2]);
	} else if (path_num == 1) {
		strcpy(path, av[1]);
	} else {
		fprintf(stderr, "Usage: put <input file> [output file]\n");
		return 1;
	}

	if (access(av[1], F_OK) == 0) {
		if (access(av[1], R_OK) == 0) {
			header.type = 0x06;
			header.code = 0x00;
			header.length = strlen(path);;
			write(sock, &header, sizeof(header));


			write(sock, &path, header.length * sizeof(char));
			read(sock, &header, sizeof(header));
			if (header.type == 0x10) {
				if ((fp = fopen(av[1], "r")) == NULL) {
					fprintf(stderr, "cannot open file\n");
					return 1;
				}
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
			} else if (header.type == 0x12) {
				//erorr
				if (header.code == 0x00) {
					fprintf(stderr, "File does not exist\n");
					return 1;
				} else if (header.code == 0x01) {
					fprintf(stderr, "No access authorization\n");
					return 1;
				}
			}
		} else {
			// no access authorization
			fprintf(stderr, "No file access authorization\n");
			return 1;
		}
	} else {
		// no such a file
		fprintf(stderr, "No such a file\n");
		return 1;
	}	
	return 0;
}

