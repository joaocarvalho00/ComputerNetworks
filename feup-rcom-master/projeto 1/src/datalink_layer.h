#ifndef DATALINK_LAYER_H
#define DATALINK_LAYER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "utils.h"



#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define FLAG 0x7E
#define ADDR 0x03
#define CW 0x03
#define CR 0x07
#define BCCW 0x00
#define BCCR 0x04
#define ERROR_PERCENTAGE_BCC1 0
#define ERROR_PERCENTAGE_BCC2 0
#define S0 0
#define S1 1
#define S2 2
#define S3 3
#define S4 4
#define SESC 5

#define READER 0
#define WRITER 1

#define RR 1
#define REJ 0
#define DISC 0x0B

#define C_START 2
#define C_END 3
#define C_I 0x01


#define TRAMA_S 0
#define TRAMA_I 1

#define ERROR_BCC1 1
#define ERROR_BCC2 2

typedef struct{
	struct termios oldtio;
	struct termios newtio;
	unsigned char control_value;
	unsigned int timeout;
	unsigned int max_transmissions;
} link_layer;

link_layer dl_layer;

/**
 * @brief Handles the alarm from thefunctions that wait a response.
 */
void alarm_handler();
/**
 * @brief verify the char received and if is ok, put it in the frame(trama)
 * Also verify if BCC1 is incorrect
 *
 * @param c char to check
 * @param state Actual state from the state machine
 * @param trama Frame array to be filled with the char
 * @param actual length of the frame array
 * @param trama_type Type of the trama that is to be check. TRAMA_S or TRAMA_I
 */
void state_machine(unsigned char c, int* state, unsigned char* trama, int* length, int trama_type);.
/**
 * @brief try to establish connection by the writer
 *
 * send the SET command and wait for the UA response
 *
 * @param fd pointer for the serial port descriptor
 * @return returns TRUE if connection is successfully established, and FALSE otherwise
 */
int set_writer(int* fd);
/**
 * @brief try to establish connection by the reader
 *
 * wait a SET command and sned UA as response
 *
 * @param fd pointer for the serial port descriptor
 * @return returns TRUE if connection is successfully established, and FALSE otherwise
 */
int set_reader(int* fd);
/**
 * @brief set and open the serial port with the new settings (ex: BAUDRATE)
 *
 * @param port serial port name
 * @param fd the descriptor that represents the serial port after it has been opened
 */
void set_serial_port(char* port, int* fd);
/**
 * @brief reset serial port settings to the settings from the beggining of the program and close it
 * @param desciptor to the serial port that is to close
 * @returns TRUE if everythings went well
 */
int close_serial_port(int fd);
/**
 * @brief Opens the serial port, and given the role (receiver or sender) prepares to either send a SET and receive an UA (sender) or 
 * receive a SET and send an UA(receiver)
 * 
 * @param port name of the port to be opened
 * @param mode the role, either sender ('w') or receiver ('r')
 * @param timeout seconds the alarm activates 
 * @param max_transmissions maximum number of attempts
 * @return int the file descriptor of the port, or -1 in case of error
 */
int LLOPEN(char* port, char* mode, char* timeout, char* max_transmissions);
/**
 * @brief Receives the data layer D1..DN adds BCC2, stuffs it, adds the link layer and creates the complete packet to be sent
 * 
 * @param msg Data of the file 
 * @param length lenght of msg
 * @return unsigned char*  the complete packet stuffed to be sent to the receiver
 */
unsigned char* create_frame(unsigned char* msg, int* length);
/**
 * @brief destuffs the control_message and checks if the BCC2 of the control message is equal to the continuous xor operation of data D1^D2^D3^...DN
 * 
 * @param control_message the part stuffed part of a packet
 * @param length the size of control_message
 * @return unsigned char* if the BCC2 is as expected it returns the control_message values after destuffin, otherwise returns null
 */
unsigned char* verify_bcc2(unsigned char* control_message, int* length);
/**
 * @brief remove frame header and tail
 * @param msg frame array to be changed
 * @param length pointer to the length of the frame array, to be changed at the end
 * @return returns the new frame without header or tail
 */
unsigned char* remove_head_msg_connection(unsigned char* msg, int* length);
/**
 * @brief Adds the link layer to the packet 
 * 
 * @param stuffed_frame application layer of the packet 
 * @param length  size of the application layer of the packet
 * @return unsigned char* the complete packet ready to be sent to the receiver
 */
unsigned char* add_frame_header(unsigned char* stuffed_frame, int* length);
/**
 * @brief does the mechanism of stuffing from the values D1 to Dn and BCC2 of an information packet
 * everytime that the portion of the packet has a flag value 0x7E it is substituted by 0x7D 0x5e, if it finds 0x7D 
 * is is substituded by 0x7D 0x5D.
 * 
 * @param msg the part of the packet where the stuffing is going to be applied
 * @param length size of msg
 * @return unsigned char*  returns the portion of the packet stuffed
 */
unsigned char* byte_stuffing(unsigned char* msg, int* length);
/**
 * @brief destuff received frame, basically does the opposite of the function byte_stuffing
 * @param frame to destuff
 * @param length pointer to the length of the frame array, to be changed at the end
 * @return returns the new frame destuffed
 */
unsigned char* byte_destuffing(unsigned char* msg, int* length);
/**
 * @brief writes a new frame to the receiver and waits his response
 * 
 * @param fd filed descriptor of the serial port
 * @param msg file data(or other if it is a START or END frame) to be transform into a packet ready to be sent
 * @param length the size of the data
 * @return int TRUE if everything went well otherwise FALSE, for example when timeout occurs more than the maximum times allowed
 */
int LLWRITE(int fd, unsigned char* msg, int* length);
/**
 * @brief Creates and send the response of the receiver, either RR or REJ
 * 
 * @param fd descriptor of the serial port
 * @param type describes if the response is to be RR or REJ
 * @param c variable control to know the next type of packet to receive 1 or 0
 * @return int the control value
 */
int send_response(int fd, unsigned int type, unsigned char c);
/**
 * @brief By the ERROR_PERCENTAGE_BCC1 macro this function has a chance to randomly change the value of BCC1 used for testing purposes
 * 
 * @param packet the packet to be changed, if its changed at all
 * @param size_packet the size of the packet to change
 * @return unsigned char* the packet either without any changes to it or with a diferrent BCC1
 */
unsigned char* mess_up_bcc1(unsigned char* packet, int size_packet);
/**
 * @brief By ERROR_PERCENTAGE_BCC2 macro this function has a chance to randomly change the value of BCC2, used for testing purposes only
 * 
 * @param packet the packet to be changed
 * @param size_packet the size of the packet
 * @return unsigned char* the packet either without any changes or with a different, wrong BCC2
 */
unsigned char* mess_up_bcc2(unsigned char* packet, int size_packet);
/**
 * @brief reads the new frame from the serial port and return it
 * @param fd descriptor for the serial port
 * @ param length pointer to the length of the frame array, to be changed at the end
 * @return returns the readed frame packet
 */
unsigned char* LLREAD(int fd, int* length);
/**
 * @brief establish the finish set up (sending SET and UA)
 * @param fd descriptor for the serial port
 * @param type type of the mode (w - wirter or r - reader)
 */
void LLCLOSE(int fd, int type);
/**
 * @brief send a disc command and wait a response returning it
 * @param fd descriptor for serial port
 * @return returns the received frame
 */
unsigned char* send_disc(int fd);


#endif
