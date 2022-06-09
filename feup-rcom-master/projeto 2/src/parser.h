//
//  parser.h
//

#ifndef parser_h
#define parser_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define MAX_STRING_LENGTH 256

typedef struct{
    char* user;
    char* password;
    char* hostname;
    char* file_path;
    char* ip;
    int data_port;
    int data_socket;
    long size;
    
} connection_info;

/**
 * Parse user input and
 * create a new connection_info
 * structure with the information gotten
*/
connection_info* parseArgs(char* input);
/**
* Parse message gotten as response for PASV
* command and get the port to the data socket
*/
int parsePasvPort(char* msgToParse);
/**
 * Parse size message response
 * to get the file size
*/
int parseSize(char* response);

#endif /* parser_h */
