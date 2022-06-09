/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define FLAG 0x7E
#define ADDR 0x03
#define CTRL 0x03
#define S0 0
#define S1 1
#define S2 2
#define S3 3
#define S4 4

volatile int STOP=FALSE;


int stateMachine(unsigned char c, int* state, char* trama){
	
	switch(*state){
		case S0:
			if(c == FLAG){
				*state = S1;
			}
			break;
		case S1:
			if(c != FLAG){
				*state = S2;
				trama[0] = c;
			}
			break;
		case S2:
			if(c != FLAG){
				*state = S3;
				trama[1] = c;				
			}
			break;
		case S3:
			if(c != FLAG){
				trama[2] = c;
				if(trama[0]^trama[1] != trama[2]){
					*state = S0;			
				}
				else{
					*state=S4;
				}
								
			}
			break;
		case S4:
			if(c == FLAG){
				*state = S5;
			}
			else{
				*state = S0;
			}
			break;
		case S5:
			break;
			
	}
}




int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  
    
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) próximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

	int i=0;
    while (STOP==FALSE) {       /* loop for input */
      res = read(fd,&buf[i],1);   /* returns after 5 chars have been input */
	if(res>0){
			
		if (buf[i]==FLAG) STOP=TRUE;
		i++;
      } 
     
    }
	printf("%s\n",buf);



  /* 
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no guião 
  */
	
 
	res = write(fd,buf,strlen(buf)+1);


    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
