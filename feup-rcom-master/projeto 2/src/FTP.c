#include "parser.h"
#include "FTP.h"

static connection_info* connection;


int sendMessage(int sockfd, char* message, char* param){
  int bytes;
  char* total_message = (char*) malloc(MAX_STRING_LENGTH);
  memset(total_message, 0, MAX_STRING_LENGTH);
  strcat(total_message,message);
  if(param != NULL)
    strcat(total_message, param);
  strcat(total_message, "\r\n");
  /*send a string to the server*/
  bytes = write(sockfd, total_message, strlen(total_message));
  return bytes;
}


int readResponse(int sockfd,char* code){
  int bytes=0;
  memset(code, 0, 3);
  char maybecode[3];
  char buf;
  int i=0, state=0, finish = 0; 
  do {
	bytes += read(sockfd, &buf, 1);
	printf("%c", buf);
	switch(state) {
	   case 0:
		code[i] = buf;
		i++;
		if(i > 3) {
		   if(buf != ' '){
		   	state = 1;
		   	i=0;
		   } else
			state = 2;
		}
		break;
           case 1:
		if(isdigit(buf)) {
		  maybecode[i] = buf;
		  i++;
		  if(i==3) {
		      state = 3;
		      i=0;
		  }				
		}	
		break;
	   case 2:
		if(buf == '\n')
		   finish = 1; 
		break;
	   case 3:
		if((maybecode[0] == code[0]) &&
           (maybecode[1] == code[1]) &&
           (maybecode[2] == code[2])){
		  if(buf == '-')
			state = 1;
		   else
			state = 2;
		
		} else {
		   state = 1;
		}
		break;		   		   
	};

   }while( finish != 1);	 
  return bytes;
}

int readData(int sockfd, char* response) {
  int bytes = 0;
  memset(response, 0, MAX_STRING_LENGTH);
    bytes = read(sockfd, response, MAX_STRING_LENGTH);
  return bytes;
}


int getCodeResponse(int sockfd,char* response){
  int responseCode;
  responseCode = (int) response[0]-'0';
  if(responseCode == 5) {
  	close(sockfd);
    exit(1);
  }

  return responseCode;
}


int readOtherResponse(int sockfd, char* response, char* message) {
    int bytes;
    char* all_resp = (char*) malloc(MAX_STRING_LENGTH);
    memset(all_resp, 0, MAX_STRING_LENGTH);
    bytes = read(sockfd, all_resp, MAX_STRING_LENGTH);
    printf("%s", all_resp);
    if(bytes > 0) {
        memset(response, 0, 3);
        response[0] = all_resp[0];
        response[1] = all_resp[1];
        response[2] = all_resp[2];
        if(strcmp(message, "pasv") == 0)
            parsePasvPort(all_resp);
        else
            parseSize(all_resp);
    }
    return bytes;
}


int communication(int sockfd,char* message,char* param){
  char* response = (char*) malloc(3);

  int finalcode;
  int bytes;

  do{

      sendMessage(sockfd, message, param);
      
      do{
          if((strcmp(message,"pasv") == 0) ||
             (strcmp(message, "SIZE ") == 0)) {
              bytes = readOtherResponse(sockfd, response, message);
          } else {
        
              bytes = readResponse(sockfd, response);
          }
          if(bytes > 0) {
	     
              finalcode = getCodeResponse(sockfd, response);

              if(finalcode == 1 && strcmp("retr ", message) == 0) {
	       	
                  //get data and create file
                  getFile();
                  close(connection->data_socket);
              }
          }

      }while(finalcode == 1);

  }while(finalcode == 4);

  return finalcode;
}



int logInServer(int sockfd){
    printf(" > Username will be sent\n");
  int response = communication(sockfd, "user ", connection->user);
  if(response != 3){
      return 1;
  }
  printf(" > Username correct. Password will be sent\n");
  response = communication(sockfd, "pass ", connection->password);
  if(response != 2){
      fprintf(stderr, "%s", "User or password incorrect\n");
      return 1;
  }
  printf(" > User logged in\n");
  return 0;
}


char* get_ip_addr(){
    struct hostent *h;
    char* ip = (char*) malloc(MAX_IP_LENGTH);
    memset(ip, 0, MAX_IP_LENGTH);
    printf("[HOST: %s]\n", connection->hostname);
    if ((h=gethostbyname(connection->hostname)) == NULL) {
        herror("gethostbyname");
        exit(1);
    }
    ip = inet_ntoa(*((struct in_addr *)h->h_addr));
    return ip;
}


int openConnection(int port,int isCommandConnection){

  int    sockfd;
  struct    sockaddr_in server_addr;

  /*server address handling*/
  bzero((char*)&server_addr,sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(connection->ip);
  server_addr.sin_port = htons(port);

  /*open an TCP socket*/
  if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
      perror("Error open socket connection ");
      exit(0);
  }


  /*connect to the server*/
  if(connect(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0){
      perror("connect()");
      exit(0);
  }

  char* openResponse = (char*) malloc(3);
  int code;
  //TODO verify this loop
  if(isCommandConnection){
    do{
      readResponse(sockfd, openResponse);
      code = getCodeResponse(sockfd, openResponse);
    }while(code != 2);
  }
  return sockfd;
}

char* getFilename(){
    char* filename = (char*) malloc(MAX_STRING_LENGTH);
    memset(filename, 0, MAX_STRING_LENGTH);
    unsigned int i=0, j=0, state=0;
    int length = strlen(connection->file_path);
    while(i<length){
        switch (state) {
            case 0:
                if(connection->file_path[i] != '/'){
                    filename[j] = connection->file_path[i];
                    j++;
                } else {
                    state = 1;
                }
                i++;
                break;
            case 1:
                memset(filename, 0, MAX_STRING_LENGTH);
                state = 0;
                j=0;
                break;
        }
    }
    printf("[Filename %s]\n", filename);
    return filename;
}


int getFile(){
    char* filename = getFilename();
    
    char* message = (char*) malloc(MAX_STRING_LENGTH);
    unsigned int bytesReaded;
    unsigned int totalBytes=0;
    
    FILE* filefd = fopen(filename, "w");
    if (filefd == NULL)
    {
        fprintf(stderr, "%s", " > Error opening file to write!\n");
        exit(1);
    }
    printf(" > Reading file\n");
    while((bytesReaded = readData(connection->data_socket, message)) > 0){
        totalBytes += bytesReaded;
        fseek(filefd, 0, SEEK_END);
        fwrite(message, sizeof(unsigned char), bytesReaded, filefd);
    }
    fclose(filefd);
    if(totalBytes <= 0)
	fprintf(stderr, "%s", " > Error reading the file\n");
    return totalBytes;
}

int verifyFileSize() {
    char* filename = getFilename();
    FILE* filefd = fopen(filename, "r");
    fseek(filefd, 0L, SEEK_END);
    int size = ftell(filefd);
    fseek(filefd, 0L, SEEK_SET);
    fclose(filefd);
    if(size == connection->size)
        return 1;
    else
        return 0;
}


int main(int argc, char** argv){

    int commandSocket;
    if(argv[1] == NULL) {
        printf(" > Error.Use the structure: ftp://<username>:<password>@<host>/<file>\n");
        exit(1);
    }
    if((connection = parseArgs(argv[1])) == NULL){
       printf(" > Input values are not valid! Please try again\n");
       exit(1);
    }
    connection->ip = get_ip_addr();
    printf("[IP address: %s]\n", connection->ip);
       
    //open connection to the server to send commands
    commandSocket = openConnection(SERVER_PORT, 1);
    
    //error logging In
    if(logInServer(commandSocket) != 0){
      fprintf(stderr, "%s", " > Error logging in. Please try again!\n");
      close(commandSocket);
      exit(1);
    }
    //send size command
    printf(" > Size command will be sent\n");
    communication(commandSocket, "SIZE ", connection->file_path);
    
    //send pasv and get port to receive the file
    printf(" > Pasv command will be sent\n");
    communication(commandSocket, "pasv", NULL);
    
    printf(" > Data socket will be opened\n");
    //open the new socket to receive the file
    connection->data_socket = openConnection(connection->data_port, 0);
    
    printf(" > Retr message will be sent\n");
    //send retrieve command to receive the file
    int finalcommandResponse;
    finalcommandResponse = communication(commandSocket, "retr ", connection->file_path);
    close(commandSocket);
    if(finalcommandResponse != 2) {
        fprintf(stderr, "%s", " > Error getting file or sending retr\n");
        exit(1);
    } else {
        if(verifyFileSize()) {
            exit(0);
        } else {
            fprintf(stderr, "%s\n", " > The received file is probably damaged\n");
            exit(1);
        }
    }

}
