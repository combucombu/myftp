
all: myftpd myftpc 
TARGET1: myftpd.c
	     gcc -Wall -o myftpd myftpd.c
TARGET2: myftpc.c
	     gcc -Wall -o myftpd myftpc.c
