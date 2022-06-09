//
//  parser.c
//

#include "parser.h"

static connection_info connection;



int verifyInputRE(const char *input)
{
    int    status;
    regex_t    reg;
    char* RE = "ftp://.*:.*@.*/.*";
    
    if (regcomp(&reg, RE, REG_EXTENDED|REG_NOSUB) != 0) {
        return(0);      /* Report error. */
    }

    status = regexec(&reg, input, (size_t) 0, NULL, 0);
    regfree(&reg);
    if (status != 0) {
        return(0);      /* Report error. */
    }
    return(1);
}

connection_info* parseArgs(char* input) {
    
    if(!verifyInputRE(input))
        return NULL;
    
    connection.user = (char*) malloc(MAX_STRING_LENGTH);
    memset(connection.user, 0, MAX_STRING_LENGTH);
    connection.password = (char*) malloc(MAX_STRING_LENGTH);
    memset(connection.password, 0, MAX_STRING_LENGTH);
    connection.hostname = (char*) malloc(MAX_STRING_LENGTH);
    memset(connection.hostname, 0, MAX_STRING_LENGTH);
    connection.file_path = (char*) malloc(MAX_STRING_LENGTH);
    memset(connection.file_path, 0, MAX_STRING_LENGTH);
    
    unsigned int i=6;
    unsigned int word_index = 0;
    unsigned int state=0;
    unsigned int input_length = strlen(input);
    char elem;

    while(i < input_length){
        
        elem = input[i];
        switch(state){
            case 0:
                if(elem == ':') {
                    word_index = 0;
                    state = 1;
                    
                } else {
                    connection.user[word_index] = elem;
                    word_index++;
                }
                break;
            case 1:
                if(elem == '@'){
                    word_index = 0;
                    state = 2;
                    
                } else {
                    connection.password[word_index] = elem;
                    word_index++;
                }
                break;
            case 2:
                if(elem == '/'){
                    word_index = 0;
                    state = 3;
                } else {
                    connection.hostname[word_index] = elem;
                    word_index++;
                }
                break;
            case 3:
                connection.file_path[word_index] = elem;
                word_index++;
                break;
        }
        i++;
    }
    printf("[Username: %s]\n", connection.user);
    printf("[Password: %s]\n", connection.password);
    return &connection;
}


int parsePasvPort(char* msgToParse){
    
    // get only numbers
    char pasvCodes[24];
    unsigned int length = strlen(msgToParse)-29;
    int i=0;
    for(; i < length; i++){
        pasvCodes[i] = msgToParse[i+26];
    }
    // parse to get the last two numbers
    int num1, num2, escp;
    sscanf(pasvCodes, "(%d,%d,%d,%d,%d,%d)", &escp, &escp, &escp, &escp, &num1, &num2);
    connection.data_port = (num1*256+num2);
    return 0;
}

int parseSize(char* response) {
    int escp;
    sscanf(response, "%d %ld", &escp, &connection.size);
    printf(" > File size: %ld\n", connection.size);
    return 0;
}
