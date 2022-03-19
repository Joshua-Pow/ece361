#ifndef conferencing_h
#define conferencing_h

#define MAX_NAME 100
#define MAX_DATA 1000

#define LOGIN 1
#define LO_ACK 2
#define LO_NAK 3
#define EXIT 4
#define JOIN 5
#define JN_ACK 6
#define JN_NAK 7
#define LEAVE_SESS 8
#define NEW_SESS 9
#define NS_ACK 10
#define MESSAGE 11
#define QUERY 12
#define QU_ACK 13


struct message {
	unsigned int type;
	unsigned int size;
	unsigned char source[MAX_NAME];
	unsigned char data[MAX_DATA];
};

void packet_fill(struct packet* message, char* buf, int numbytes){

	//Buf is going to be "3:2:10:foobar.txt:lo World!\n"
	//Buf is going to be type:size:source:message
	//ie: "1:64:tashan:HelloWorld!"

	//struct packet recieve;
	//printf("Correct: %s", buf);
	char* string = (char*)malloc(numbytes*sizeof(char));
	memcpy(string, buf, numbytes);

	//printf("str: %s\n", string);
	//printf("temp2\n");

	char* token = strtok(string,":");
	printf("type: %s\n", token);
	message->type = strtoul(token, NULL, 10);

	token = strtok(NULL, ":");
	printf("frag_no: %s\n", token);
	message->size = strtoul(token, NULL, 10);

	token = strtok(NULL, ":");
	printf("size: %s\n", token);
	message->source = strtoul(token, NULL, 10);

	// token = strtok(NULL, ":");
	// printf("file data: %s\n", token);
	int count=0;
	for(int i=0; i<numbytes; i++){
		if (buf[i]==':'){
			count++;
			if (count==3){
				//printf("bytes: %d\n", numbytes-i);
				memcpy(recieve->data, buf+i+1, message->size);
				printf("Data: %x\n", message->data);
				break;
			}
		}
	}
	// memcpy(recieve->filedata, token, sizeof(token));
}

#endif