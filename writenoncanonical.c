/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define FLAG 0x7E
#define A_3 0x03
#define A_1 0x01
#define C 0x03
#define BCC_3 A_3^C
#define BCC_1 A_1^C


volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    unsigned char buf[5];
    int i = 0, sum = 0, speed = 0;
    
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

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

    
    buf[0] = FLAG;
    buf[1] = A_3;
    buf[2] = C;
    buf[3] = BCC_3;
    buf[4] = FLAG;
  
    res = write(fd,buf,5);   
    printf("%d bytes written\n", res);
    
 

  /* 
    O ciclo FOR e as instru��es seguintes devem ser alterados de modo a respeitar 
    o indicado no gui�o 
  */
    unsigned char a;
    int state = 0;
    while (1) 
    {                 
              
      if(state==5){
        printf("STOP\n");
        break;
      }
      res = read(fd,&a,1);
      printf("a= %d \n",a);
      if(state==0 && a==FLAG){
        state=1;
        printf("State 0-1\n");
      }
      else if(state==0 && a!=FLAG){
        state=0;
        printf("State 0-0\n");
      }
      else if(state==1 && a==A_1){
        state=2;
        printf("State 1-2\n");
      }
      else if(state==1 && a==FLAG){
        state=1;
        printf("State 1-1\n");
      }
      else if(state==1 && a!=FLAG && a!=A_1){
        state=0;
        printf("State 1-0\n");
      }
      else if(state==2 && a==FLAG){
        state=1;
        printf("State 2-1\n");
      }
      else if(state==2 && a==C){
        state=3;
        printf("State 2-3\n");
      }
      else if(state==2 && a!=FLAG && a!=C){
        state=0;
        printf("State 2-0\n");
      }
      else if(state==3 && a==FLAG){
        state=1;
        printf("State 3-1\n");
      }
      else if(state==3 && a==BCC_1){
        state=4;
        printf("State 3-4\n");
      }
      else if(state==3 && a!=FLAG && a!=BCC_1){
        state=0;
        printf("State 3-0\n");
      }
      else if(state==4 && a==FLAG){
        state=5;
        printf("State 4-5\n");
        
      }
      else if(state==4 && a!=FLAG){
        state=0;
        printf("State 4-0\n");
      }
      
    }


   
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }




    close(fd);
    return 0;
}
