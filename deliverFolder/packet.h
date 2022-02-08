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
	//printf("Correct: %s", buf);
	char* string = (char*)malloc(numbytes*sizeof(char));
	memcpy(string, buf, numbytes);

	//printf("str: %s\n", string);
	//printf("temp2\n");

	char* token = strtok(string,":");
	printf("total frag: %s\n", token);
	recieve->total_frag = strtoul(token, NULL, 10);

	token = strtok(NULL, ":");
	printf("frag_no: %s\n", token);
	recieve->frag_no = strtoul(token, NULL, 10);

	token = strtok(NULL, ":");
	printf("size: %s\n", token);
	recieve->size = strtoul(token, NULL, 10);

	token = strtok(NULL, ":");
	printf("filename: %s\n", token);
	recieve->filename = token;

	// token = strtok(NULL, ":");
	// printf("file data: %s\n", token);
	int count=0;
	for(int i=0; i<numbytes; i++){
		if (buf[i]==':'){
			count++;
			if (count==4){
				//printf("bytes: %d\n", numbytes-i);
				memcpy(recieve->filedata, buf+i+1, recieve->size);
				printf("Data: %x\n", recieve->filedata);
				break;
			}
		}
	}
	// memcpy(recieve->filedata, token, sizeof(token));
}

#endif