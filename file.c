#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#define BUFSIZE 256

int main(int argc, char *argv[])
{
	int i, j, k;
	int file_size;
	FILE *fp;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
		exit(1);
	}
	//file open
	if ((fp = fopen(argv[1], "w")) == NULL) {
		fprintf(stderr, "Cannot open file %s\n", argv[2]);
		exit(3);
	}
	//size
	file_size = atoi(argv[2]);
	fprintf(stdout, "sizeof(int) = %d\n", (int)sizeof(int));

	for (i = 0; i < file_size; i++) {
		for (j = 0; j < BUFSIZE; j++) {
			fprintf(fp, "%d\n", j);
		}
		fprintf(fp, "%d\n", i);
		fprintf(stdout, "%d\n", i);
	}

	fclose(fp);


	return 0;

}
