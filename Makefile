
all: myftpd myftpc 
TARGET1: myftpd.c
	     gcc -o myftpd -Wall myftpd.c
TARGET2: myftpc.c
	     gcc -o myftpd -Wall myftpc.c
