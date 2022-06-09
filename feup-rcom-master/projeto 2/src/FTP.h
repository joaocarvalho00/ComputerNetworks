//
//  FTP.h
//

#ifndef FTP_h
#define FTP_h

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <strings.h>
#include <ctype.h>

#define SERVER_PORT 21
#define MAX_IP_LENGTH 16

/**
 * Send a message to the file descriptor sokfd 
 * with the following structure: message+param+"\r\n". 
 * The 2 final characters represent the end of message
*/
int sendMessage(int sockfd, char* message, char* param);
/**
 * Get first number from the response gotten
*/
int getCodeResponse(int sockfd, char* response);
/**
 * Send a message to the server
 * and wait a response from it
*/
int communication(int sockfd,char* message,char* param);
/*
 * Send log in information to the server
*/
int logInServer(int sockfd);
/**
 * get the ip address from the hostname given
*/
char* get_ip_addr();
/**
 * open a new connection to the specified port
*/
int openConnection(int port,int isCommandOpen);
/**
 * get the filename from the given path
*/
char* getFilename();
/**
 * create a local file and
 * get file data from the data socket
*/
int getFile();
/**
 * main function
*/
int main(int argc, char** argv);

#endif /* FTP_h */
