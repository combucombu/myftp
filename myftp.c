#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#define BUFSIZE 1024*1000

struct myftph {
	uint8_t		type;
	uint8_t		code;
	uint16_t	length;
};

int main(int argc, char *argv[])
{
	int i;
	int n;
	char buf[BUFSIZE];
	uint8_t *data;
	struct myftph* head_p;
	uint8_t *pay_p;

	FILE *fpi;
	FILE *fpo;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
		exit(1);
	}
	//file open
	if ((fpi = fopen(argv[1], "r")) == NULL) {
		fprintf(stderr, "Cannot open file %s\n", argv[1]);
		exit(2);
	}
	if ((fpo = fopen(argv[2], "w")) == NULL) {
		fprintf(stderr, "Cannot open file %s\n", argv[2]);
		exit(3);
	}
	

	//read input file
	for (;;) {
		n = fread(buf, sizeof(char), BUFSIZE, fpi);
		fprintf(stderr, "finished reading files %d bytes\n", n);
		if (n < BUFSIZE) {
			break;
		}

		fprintf(stderr, "did not break\n");

		//memory allocate
		if ((data = (uint8_t *)malloc(BUFSIZE* sizeof(char) + sizeof (struct myftph))) == NULL) {
			fprintf(stderr, "Cannot alloocate %d Byte\n", head_p->length);
			exit(4);
		}
		//set head_p
		head_p = (struct myftph *)data;
		head_p->type = (uint8_t)0x00;
		head_p->code = (uint8_t)0x01;
		head_p->length = (uint16_t)(sizeof(char) * n);
		fprintf(stderr, "%d, %d, %d\n", head_p->type, head_p->code, head_p->length);

		//set pay_p
		pay_p = (uint8_t *)(data + sizeof(struct myftph));

		//memoru copy
		memcpy(pay_p, buf, BUFSIZE);

		//write data
		fwrite(pay_p, BUFSIZE, 1, fpo);
	}

	fprintf(stderr, "did break\n");

	//memory allocate
	if ((data = (uint8_t *)malloc(n * sizeof(char) + sizeof (struct myftph))) == NULL) {
		fprintf(stderr, "Cannot alloocate %d Byte\n", head_p->length);
		exit(5);
	}
	fprintf(stderr, "finished allocating memories\n");
	
	//set head_p
	head_p = (struct myftph *)data;
	head_p->type = (uint8_t)0x00;
	head_p->code = (uint8_t)0x00;
	head_p->length = (uint16_t)(sizeof(char) * n);
	fprintf(stderr, "%d, %d, %d\n", head_p->type, head_p->code, head_p->length);

	//set pay_p
	pay_p = (uint8_t *)(data + sizeof(struct myftph));

	//memory copy
	memcpy(pay_p, buf, n);

	fprintf(stderr, "payload = %s\n", pay_p);

	fwrite(pay_p, n * sizeof(char), 1, fpo);

	//free memory
	free(data);
	fclose(fpi);
	fclose(fpo);
	return 0;
}
