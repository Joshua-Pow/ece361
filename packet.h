#ifndef packet_h
#define packet_h

struct packet {
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char* filename;
    char filedata[1000];
};

void packet_fill (struct packet* recieve, char* buf, int numbytes){

	//Buf is going to be "3:2:10:foobar.txt:lo World!\n"
	//struct packet recieve;
	char string[numbytes];
	memcpy(string, buf, numbytes+1);
	printf("str: %s\n", string);
	printf("temp2\n");

	char* token = strtok(string,":");
	printf("%s\n", token);
	recieve->total_frag = strtoul(token, NULL, 10);

	token = strtok(NULL, ":");
	printf("%s\n", token);
	recieve->frag_no = strtoul(token, NULL, 10);

	token = strtok(NULL, ":");
	printf("%s\n", token);
	recieve->size = strtoul(token, NULL, 10);

	token = strtok(NULL, ":");
	printf("%s\n", token);
	recieve->filename = token;

	token = strtok(NULL, ":");
	printf("%s\n", token);
	memcpy(recieve->filedata, token, strlen(token)+1);
}

#endif