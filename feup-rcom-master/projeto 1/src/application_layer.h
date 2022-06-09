#ifndef APPLICATION_LAYER_H
#define APPLICATION_LAYER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "datalink_layer.h"

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define DATA_CONTROL 1


#define MAXSIZELINK 7 //2*1(bcc2) + 5 other element trama datalink

#define START_PACKET_TYPE 1
#define END_PACKET_TYPE 0


typedef struct {
  int filesize;
  char* filename;
  FILE* fp;
  int size_to_read;
}file_info;

typedef struct{
  int file_descriptor;
  char* status;
}application_layer;


/**
 * @brief Create package to send and send it to the datalink layer using LLWRITE
 *
 * If is not a strat/end frame the package header
 * is added and after that the message is sent
 * To be used by the Writer
 *
 * @param msg array with the data that is to send
 * @param length length of the message that is to send
 * @return returns TRUE if message was sent and false otherwise
 */
int send_message(unsigned char* msg, int length);

/**
 * @brief Removes the package header from the received trama
 * To be used by the Reader
 * 
 *
 * @param readed_msg array with the trama received
 * @param length Pointer to the length of the trama received. This param is updated to the length of the returned array
 * @return returns an array with the real data from the readed_msg
 */
unsigned char* get_only_data(unsigned char* readed_msg, int* length);

/**
 * @brief Create package to send and send it to the datalink layer using LLWRITE
 *
 * If is not a start/end frame the package header is added and after that the message is sent
 * To be used by the Reader
 *
 * @param msg array with the data that is to send
 * @param length length of the message that is to send
 * @return returns TRUE if message was sent and false otherwise
 */
unsigned char* get_message();

/**
 * @brief Create package to send and send it to the datalink layer using LLWRITE
 *
 * If is not a start/end frame the package header is added and after that the message is sent
 * To be used by the Writer 
 *
 * @param msg array with the data that is to send
 * @param length length of the message that is to send
 * @return returns TRUE if message was sent and false otherwise
 */
unsigned char* data_package_constructor(unsigned char* msg, int* length);
/**
 * @brief Get the size of the file set in the global variable file
 * To be used by the both, Reader and Writer
 *
 * @return returns the size of the file
 */
int get_file_size();
/**
 * @brief Creates a start/end packet array with the name and the size set in the global variable file
 *  To be used by the Writer
 * 
 * @param packet array to be filled
 * @param type Type of the packet to be created (START_PACKET_TYPE / END_PACKET_TYPE)
 * @return Returns the size of the packet array created
 */
int create_STARTEND_packet(unsigned char* packet, int type);
/**
 * @brief Read file parameters from the msg and fill the global variable file with that information.
 *  To be used by the Reader
 */
void get_file_params(unsigned char* msg);
/**
 * @brief Read the file by packets of the size specified in the global variable file
 * After that this function calls send_message to send the readed packet
 * To be used by the Writer
 */
void handle_readfile();
/**
 * @brief Write the data to the created file
 * To be used by the Reader
 *
 * @param data Array with the data to be written
 * @param sizetowrite Size of data array
 */
void handle_writefile(unsigned char * data,int sizetowrite);
/**
 * @brief Verify if the end frame has the same parameters from the start frame and from the created file, mainly the file size
 * @param msg End frame to be checked
 * @return Returns TRUE if is everything ok, and FALSE otherwise
 */
int verify_end(unsigned char* msg);
/**
 * @brief Main function where the program starts
 * Creates the connection calling LLOPEN and starts the execution for reader and for writer
 * @return Returns TRUE if everything is ok and FALSE if can't connect to the serial port
 */
int main(int argc, char** argv);

#endif
