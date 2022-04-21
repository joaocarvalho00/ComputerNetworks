#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "linklayer.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define FLAG 0x7E
#define ESC 0x7D

#define A_3 0x03
#define A_1 0x01
#define SET 0x03
#define DISC 0x0B
#define UA 0x07
#define RR0 0x01
#define RR1 0x21
#define REJ1 0x25
#define REJ0 0x05
#define C0 0x00
#define C1 0x02


// Opens a conection using the "port" parameters defined in struct linkLayer, returns "-1" on error and "1" on sucess

volatile int STOP=FALSE;
int timeoutCounter = 0, flag = 0, fd, R = 1, S=0,FrameSize;
int MaxTimeout, TimeoutTime;
unsigned char *frame;
int destuffedbits=0;
struct termios oldtio,newtio;
int role;

void atende()                   // atende alarme
{
    if (timeoutCounter == MaxTimeout)
    {
        //printf("Vou Fechar\n");
        if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) 
        {
            perror("tcsetattr");
            exit(-1);
        }
        flag = 0;
        timeoutCounter =0;
        close(fd);
    } else
    if(flag == 0)
    {
      timeoutCounter++;
	    printf("TIMEOUT # %d\n", timeoutCounter);
	    alarm(TimeoutTime);
	    int res2 = write(fd,frame,5);   
        //printf("%d bytes written\n", res2);
	}
}

int llopen(linkLayer connectionParameters)
{
    
    int c, res;
    
    int i = 0, sum = 0, speed = 0;
    


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */


    fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(connectionParameters.serialPort); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 chars received */



  /* 
    VTIME e VMIN devem ser alterados de forma char_read proteger com um temporizador char_read 
    leitura do(s) pr�ximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
    MaxTimeout = connectionParameters.numTries;
    TimeoutTime = connectionParameters.timeOut;
    printf("New termios structure set\n");
    role = connectionParameters.role;
    if(connectionParameters.role == TRANSMITTER)
    {
        (void) signal(SIGALRM, atende);
        frame = (unsigned char*)calloc(5,sizeof(unsigned char));
        frame[0] = FLAG;
        frame[1] = A_3;
        frame[2] = SET;
        frame[3] = A_3^SET;
        frame[4] = FLAG;
        FrameSize = 5;
        res = write(fd,frame,FrameSize);   
        printf("%d bytes written\n", res);
        
     

      /* 
        O ciclo FOR e as instru��es seguintes devem ser alterados de modo char_read respeitar 
        o indicado no gui�o 
      */
        unsigned char char_read;
        int state = 0;
        alarm(TimeoutTime);
        while (1) 
        {                 
                  
          if(state==5){
            printf("STOP\n");
            break;
          }
          if((res = read(fd,&char_read,1)) == -1)
          {
            printf("CONNECTION FAILED\n");
            return -1;
          }
          
          flag = 1;
          if(res == 0)
          {
            continue;
          }
            
          /*---------------------------------------------------------------------*/
          /*-----------------------------STATE MACHINE---------------------------*/
          /*---------------------------------------------------------------------*/
          
          printf("char_read= %d \n",char_read);
          if(state==0 && char_read==FLAG){
            state=1;
            printf("State 0-1\n");
          }
          else if(state==0 && char_read!=FLAG){
            state=0;
            printf("State 0-0\n");
            flag = 0;
            alarm(TimeoutTime);
          }
          else if(state==1 && char_read==A_1){
            state=2;
            printf("State 1-2\n");
          }
          else if(state==1 && char_read==FLAG){
            state=1;
            printf("State 1-1\n");
            
          }
          else if(state==1 && char_read!=FLAG && char_read!=A_1){
            state=0;
            printf("State 1-0\n");
            flag = 0;
            alarm(TimeoutTime);
          }
          else if(state==2 && char_read==FLAG){
            state=1;
            printf("State 2-1\n");
          }
          else if(state==2 && char_read==UA){
            state=3;
            printf("State 2-3\n");
          }
          else if(state==2 && char_read!=FLAG && char_read!=UA){
            state=0;
            printf("State 2-0\n");
            flag = 0;
            alarm(TimeoutTime);
          }
          else if(state==3 && char_read==FLAG){
            state=1;
            printf("State 3-1\n");
          }
          else if(state==3 && char_read==A_1^UA){
            state=4;
            printf("State 3-4\n");
          }
          else if(state==3 && char_read!=FLAG && char_read!=A_1^UA){
            state=0;
            printf("State 3-0\n");
            flag = 0;
            alarm(TimeoutTime);
          }
          else if(state==4 && char_read==FLAG){
            state=5;
            printf("State 4-5\n");
            
          }
          else if(state==4 && char_read!=FLAG){
            state=0;
            printf("State 4-0\n");
            flag = 0;
            alarm(TimeoutTime);
          }
          
          /*---------------------------------------------------------------------*/
          /*-----------------------------STATE MACHINE---------------------------*/
          /*---------------------------------------------------------------------*/
        }
        free(frame);
    } else if(connectionParameters.role == RECEIVER)
    {
        int i=0;
        int state=0;
        unsigned char char_read;
        while (1) 
        {                 
          if(state==5)
          {
            printf("STOP\n");
            break;
          }                    
             
          res = read(fd,&char_read,1);
          printf("char_read= %d \n",char_read);
          
          


          /* ----------------------------------------------------------- */ 
          /* ---------------------- STATE MACHINE ---------------------- */
          /* ----------------------------------------------------------- */        

          if(state==0 && char_read==FLAG){
            state=1;
            printf("State 0-1\n");
          }
          else if(state==0 && char_read!=FLAG){
            state=0;
            printf("State 0-0\n");
          }
          else if(state==1 && char_read==A_3){
            state=2;
            printf("State 1-2\n");
          }
          else if(state==1 && char_read==FLAG){
            state=1;
            printf("State 1-1\n");
          }
          else if(state==1 && char_read!=FLAG && char_read!=A_3){
            state=0;
            printf("State 1-0\n");
          }
          else if(state==2 && char_read==FLAG){
            state=1;
            printf("State 2-1\n");
          }
          else if(state==2 && char_read==SET){
            state=3;
            printf("State 2-3\n");
          }
          else if(state==2 && char_read!=FLAG && char_read!=SET){
            state=0;
            printf("State 2-0\n");
          }
          else if(state==3 && char_read==FLAG){
            state=1;
            printf("State 3-1\n");
          }
          else if(state==3 && char_read==A_3^SET){
            state=4;
            printf("State 3-4\n");
          }
          else if(state==3 && char_read!=FLAG && char_read!=A_3^SET){
            state=0;
            printf("State 3-0\n");
          }
          else if(state==4 && char_read==FLAG){
            state=5;
            printf("State 4-5\n");
          }
          else if(state==4 && char_read!=FLAG){
            state=0;
            printf("State 4-0\n");
          }
        }




        /* ----------------------------------------------------------- */ 
        /* ---------------------- STATE MACHINE ---------------------- */
        /* ----------------------------------------------------------- */         

          printf("Recebi, vou enviar!\n");
          frame = (unsigned char*)calloc(5,sizeof(unsigned char));
          frame[0]= FLAG;
          frame[1]= A_1;
          frame[2]= UA; 
          frame[3]= A_1^UA;
          frame[4] = FLAG;
          FrameSize = 5;
        /*testing*/ 
        
        res = write(fd,frame,FrameSize);   
        printf("%d bytes written\n", res);
        free(frame);
    }
    

   
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
    timeoutCounter =0;
    flag = 0;
    return fd;
}

int llwrite(char* buf, int bufSize)
{
  //CREATE FRAME
  unsigned char header[4];
  header[0]=FLAG;
  header[1]=A_3;
  if(S==0)
  {
    header[2]=C0;
  }
  if(S==1)
  {
    header[2]=C1;
  }
  header[3] = header[1]^header[2];
  
  //Criar o BB2
  
  unsigned char BCC2;

  for (int i = 0; i < bufSize; i++)
  {
    BCC2 = buf[i]^BCC2;
  }
  // Criar o trailer
  unsigned char *trailer = (unsigned char*)calloc(2,sizeof(unsigned char));
  int trailerSize = 2;
  //BYTE STUFFING
  if ( BCC2 == FLAG || BCC2 == ESC)
  {
    trailer = (unsigned char*)realloc(trailer,sizeof(unsigned char)*3);
    trailerSize = 3;
    trailer[0] = ESC;
    trailer[1] = BCC2^0x20;
    trailer[2] = FLAG; 
  }else
  {
    trailer[0] = BCC2;
    trailer[1] = FLAG; 
  }
  // Criar data 
  unsigned char *data = (unsigned char*)calloc(bufSize,sizeof(unsigned char));
  int dataSize = bufSize;
  int j=0;
  for (int i = 0; i < bufSize; i++)
  {
    if ( buf[i] == FLAG || buf[i] == ESC)
    {
      dataSize++;
      data = (unsigned char*)realloc(data,sizeof(unsigned char)*dataSize);
      data[j] = ESC;
      data[j+1] = buf[i]^0x20;
      j++; 
    }else
    {
      data[j] = buf[i];
    }
    j++;
  }
  frame = (unsigned char*)calloc(4+dataSize+trailerSize,sizeof(unsigned char));
  FrameSize = 4+dataSize+trailerSize;
  frame[0] = header[0];
  frame[1] = header[1];
  frame[2] = header[2];
  frame[3] = header[3];
  for (int i = 0; i < dataSize; i++)
  {
    frame[4+i] = data[i];
  }
  for (int i = 0; i < trailerSize; i++)
  {
    frame[4+dataSize+i] = trailer[i];
    
  }
  printf("data: %d, frame: %d, stuffedSize: %d,trailerSize: %d\n",bufSize,FrameSize,dataSize,trailerSize);
  /*for (int i = 0; i < FrameSize; i++)
  {
    printf("%d x ", frame[i]);
    if(i%8==0)
    {
      printf("\n");
    }
  }*/
  tcflush(fd, TCIOFLUSH);

  if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }
  
  int res = write(fd,frame,FrameSize);   
  printf("%d bytes written\n", res);
  free(data);
  free(trailer);
  int written = res;
  unsigned char char_read;
  //(void) signal(SIGALRM, atende);
  int state = 0;

  //alarm(TimeoutTime);
  while (1) 
  {                 
            
    if(state==5){
      printf("STOP\n");
      break;
    }
    if((res = read(fd,&char_read,1)) == -1)
    {
      printf("CONNECTION FAILED\n");
      return -1;
    }
    if(res == 0)
    {
      continue;
    }
    flag = 1;
    

      
    /*---------------------------------------------------------------------*/
    /*-----------------------------STATE MACHINE---------------------------*/
    /*---------------------------------------------------------------------*/
    
    //printf("char_read= %d \n",char_read);
    if(state==0 && char_read==FLAG){
      state=1;
      printf("State 0-1\n");
    }
    else if(state==0 && char_read!=FLAG){
      state=0;
      //printf("State 0-0\n");
      //flag = 0;
      //alarm(TimeoutTime);
    }
    else if(state==1 && char_read==A_1){
      state=2;
      printf("State 1-2\n");
    }
    else if(state==1 && char_read==FLAG){
      state=1;
      printf("State 1-1\n");
      
    }
    else if(state==1 && char_read!=FLAG && char_read!=A_1){
      state=0;
      printf("State 1-0\n");
      //flag = 0;
      //alarm(TimeoutTime);
    }
    else if(state==2 && char_read==FLAG){
      state=1;
      //printf("State 2-1\n");
    }
    else if(state==2 && ((char_read==RR1 && S==0)||(char_read==RR0 && S==1))) {
      state=3;
      printf("State 2-3\n");
    }
    else if(state==2 && ((char_read==REJ1 && S==0)||(char_read==REJ0 && S==1))) {
      state=0;
      printf("MESSAGE REJECTED, RESENDING\n");
      res = write(fd,frame,FrameSize);   
      printf("%d bytes written\n", res);
      flag = 0;
      alarm(TimeoutTime);
    }
    else if(state==2 && char_read!=FLAG && ((char_read!=RR1 && S==0)||(char_read!=RR0 && S==1)) && ((char_read!=REJ1 && S==0)||(char_read!=REJ0 && S==1))){
      state=0;
      printf("State 2-0\n");
      //flag = 0;
      //alarm(TimeoutTime);
    }
    else if(state==3 && char_read==FLAG){
      state=1;
      printf("State 3-1\n");
    }
    else if(state==3 && ((char_read==RR1^A_1 && S==0)||(char_read==RR0^A_1 && S==1)) ){
      state=4;
      printf("State 3-4\n");
    }
    else if(state==3 && char_read!=FLAG && ((char_read!=RR1^A_1 && S==0)||(char_read!=RR0^A_1 && S==1)) ){
      state=0;
      printf("State 3-0\n");
      //flag = 0;
      //alarm(TimeoutTime);
    }
    else if(state==4 && char_read==FLAG){
      state=5;
      printf("State 4-5\n");
      
    }
    else if(state==4 && char_read!=FLAG){
      state=0;
      printf("State 4-0\n");
      //flag = 0;
      //alarm(TimeoutTime);
    }
    /*---------------------------------------------------------------------*/
    /*-----------------------------STATE MACHINE---------------------------*/
  } /*---------------------------------------------------------------------*/
  
  if(S==0)
  {
    S=1;
  }else
  {
    S=0;
  }
  if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }
  free(frame);
  timeoutCounter =0;
  flag = 0;
  return written;
}
// Receive data in packet
int llread(char* packet)
{
  int state=0;
  int rej = 0;
  unsigned char char_read;
  int res;
  frame = (unsigned char*)calloc(0,sizeof(unsigned char));
  FrameSize =0;

  //tcflush(fd, TCIOFLUSH);

  if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  while (1) 
  {
    if(state==5){
      printf("STOP\n");
      break;
    }
    if((res = read(fd,&char_read,1)) == -1)
    {
      printf("CONNECTION FAILED\n");
      return -1;
    }
    if(res == 0)
    {
      continue;
    }
    FrameSize++;
    //printf("char_read= %d \n",char_read);
    frame = (unsigned char*)realloc(frame,sizeof(unsigned char)*FrameSize);
    frame[FrameSize-1] = char_read; 
    /*---------------------------------------------------------------------*/
    /*-----------------------------STATE MACHINE---------------------------*/
    /*---------------------------------------------------------------------*/
    
    //printf("char_read= %d \n",char_read);
    if(state==0 && char_read==FLAG){
      state=1;
      printf("State 0-1\n");
    }
    else if(state==0 && char_read!=FLAG){
      state=0;
      printf("State 0-0\n");
    }
    else if(state==1 && char_read==A_3){
      state=2;
      printf("State 1-2\n");
    }
    else if(state==1 && char_read==FLAG){
      state=1;
      printf("State 1-1\n");
      
    }
    else if(state==1 && char_read!=FLAG && char_read!=A_3){
      state=5;
      rej=1;
      printf("message with errors on 1\n");

    }
    else if(state==2 && char_read==FLAG){
      state=1;
      printf("State 2-1\n");
    }
    else if(state==2 && ((char_read==C1 && R==0)||(char_read==C0 && R==1))) {
      state=3;
      printf("State 2-3\n");
    }
    else if(state==2 && char_read!=FLAG && ((char_read!=C1 && R==0)||(char_read!=C0 && R==1))){
      state=5;
      rej=1;
      printf("message with errors on 2\n");

    }
    else if(state==3 && char_read==FLAG){
      state=1;
      printf("State 3-1\n");
    }
    else if(state==3 && ((char_read==C1^A_3 && R==0)||(char_read==C0^A_3 && R==1)) ){
      state=4;
      printf("State 3-4\n");
    }
    else if(state==3 && char_read!=FLAG && ((char_read!=C1^A_3 && R==0)||(char_read!=C0^A_3 && R==1)) ){
      state=5;
      rej=1;
      printf("message with errors on 3\n");
    }
    else if(state==4 && char_read==FLAG){
      state=5;
      printf("State 4-5\n");
      
    }
    /*---------------------------------------------------------------------*/
    /*-----------------------------STATE MACHINE---------------------------*/
  } /*---------------------------------------------------------------------*/
  if(rej==1)
  {
    unsigned char header[5]; 

    header[0]=FLAG;
    header[1]=A_1;
    if(R==0)
    {
      header[2]=REJ0;
    }
    if(R==1)
    {
      header[2]=REJ1;
    }
    header[3]=header[1]^header[2];
    header[4]=FLAG;
    tcflush(fd, TCIOFLUSH);
    res = write(fd,header,5);   
    printf("%d bytes written\n", res);

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    return 0;
  }
  printf("%d READ\n",FrameSize);
  unsigned char* destuffedframe = (unsigned char*)calloc(FrameSize,sizeof(unsigned char));
  unsigned int destuffedframeSZ = FrameSize;
	unsigned int i, j=0;

	for(i = 0; i< FrameSize; i++, j++){
		if(frame[i] == 0x7D){
			i++;
			if(frame[i]==0x5E){
				destuffedframe[j] = 0x7E;
				destuffedframeSZ--;
			}
			if(frame[i]==0x5D){
				destuffedframe[j]= 0x7D;
				destuffedframeSZ--;
			}
		}else{
			destuffedframe[j] = frame[i];
		}
	}

	destuffedframe[j] = frame[FrameSize];

  ///Deconstruct frame
  printf("destuffed size = %d\n", destuffedframeSZ);
  int dataSZ= destuffedframeSZ-4-2;
  //printf("teste 1\n");
  unsigned char *data = (unsigned char*)calloc(dataSZ,sizeof(unsigned char));
  //printf("teste 1\n");
  for (int i = 4; i < destuffedframeSZ-2; i++)
  {
    data[i-4] = destuffedframe[i];
  }
  //printf("teste 2\n");
  unsigned char BCC2;
  for (int i = 0; i < dataSZ; i++)
  {
    BCC2 = data[i]^BCC2;
  }

  /*if(BCC2 != destuffedframe[destuffedframeSZ-2])
  {
    printf("message with errors on BCC2\n");
    unsigned char header[5]; 

    header[0]=FLAG;
    header[1]=A_1;
    if(R==0)
    {
      header[2]=REJ0;
    }
    if(R==1)
    {
      header[2]=REJ1;
    }
    header[3]=header[1]^header[2];
    header[4]=FLAG;
    tcflush(fd, TCIOFLUSH);
    res = write(fd,header,5);   
    printf("%d bytes written\n", res);

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    return 0;
  }*/
  //printf("teste 3\n");
  unsigned char header[5]; 

  header[0]=FLAG;
  header[1]=A_1;
  if(R==0)
  {
    header[2]=RR0;
  }
  if(R==1)
  {
    header[2]=RR1;
  }
  header[3]=header[1]^header[2];
  header[4]=FLAG;
  tcflush(fd, TCIOFLUSH);
  res = write(fd,header,5);   
  printf("%d bytes written\n", res);
  for (int i = 0; i < dataSZ; i++)
  {
    packet[i]=data[i];
  }
  if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }
   if(R==0)
  {
    R=1;
  }else
  {
    R=0;
  }
  return dataSZ;

}

// Closes previously opened connection; if showStatistics==TRUE, link layer should print statistics in the console on close

int llclose(int showStatistics)
{

  int res;
  
  if(role== TRANSMITTER)
  {
      (void) signal(SIGALRM, atende);
      frame = (unsigned char*)calloc(5,sizeof(unsigned char));
      frame[0] = FLAG;
      frame[1] = A_3;
      frame[2] = DISC;
      frame[3] = A_3^DISC;
      frame[4] = FLAG;
      FrameSize = 5;
      tcflush(fd, TCIOFLUSH);
      if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
      }
      res = write(fd,frame,FrameSize);   
      printf("%d bytes written\n", res);
      unsigned char char_read;
      int state = 0;
      alarm(TimeoutTime);
      while (1) 
      {                 
                
        if(state==5){
          printf("STOP\n");
          break;
        }
        if((res = read(fd,&char_read,1)) == -1)
        {
          printf("CONNECTION FAILED\n");
          return -1;
        }
        
        flag = 1;
        if(res == 0)
        {
          continue;
        }
          
        /*---------------------------------------------------------------------*/
        /*-----------------------------STATE MACHINE---------------------------*/
        /*---------------------------------------------------------------------*/
        
        //printf("char_read= %d \n",char_read);
        if(state==0 && char_read==FLAG){
          state=1;
          printf("State 0-1\n");
        }
        else if(state==0 && char_read!=FLAG){
          state=0;
          printf("State 0-0\n");
          flag = 0;
          alarm(TimeoutTime);
        }
        else if(state==1 && char_read==A_1){
          state=2;
          printf("State 1-2\n");
        }
        else if(state==1 && char_read==FLAG){
          state=1;
          printf("State 1-1\n");
          
        }
        else if(state==1 && char_read!=FLAG && char_read!=A_1){
          state=0;
          printf("State 1-0\n");
          flag = 0;
          alarm(TimeoutTime);
        }
        else if(state==2 && char_read==FLAG){
          state=1;
          printf("State 2-1\n");
        }
        else if(state==2 && char_read==DISC){
          state=3;
          printf("State 2-3\n");
        }
        else if(state==2 && char_read!=FLAG && char_read!=DISC){
          state=0;
          printf("State 2-0\n");
          flag = 0;
          alarm(TimeoutTime);
        }
        else if(state==3 && char_read==FLAG){
          state=1;
          printf("State 3-1\n");
        }
        else if(state==3 && char_read==A_1^DISC){
          state=4;
          printf("State 3-4\n");
        }
        else if(state==3 && char_read!=FLAG && char_read!=A_1^DISC){
          state=0;
          printf("State 3-0\n");
          flag = 0;
          alarm(TimeoutTime);
        }
        else if(state==4 && char_read==FLAG){
          state=5;
          printf("State 4-5\n");
          
        }
        else if(state==4 && char_read!=FLAG){
          state=0;
          printf("State 4-0\n");
          flag = 0;
          alarm(TimeoutTime);
        }
        
        /*---------------------------------------------------------------------*/
        /*-----------------------------STATE MACHINE---------------------------*/
        /*---------------------------------------------------------------------*/
      }
      
      frame = (unsigned char*)calloc(5,sizeof(unsigned char));
      frame[0] = FLAG;
      frame[1] = A_3;
      frame[2] = UA;
      frame[3] = A_3^UA;
      frame[4] = FLAG;
      FrameSize = 5;
      res = write(fd,frame,FrameSize);   
      printf("%d bytes written\n", res);
  }
  if(role == RECEIVER)
  {
      unsigned char char_read;
      int state = 0;
      if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
      }
      while (1) 
      {                 
                
        if(state==5){
          printf("STOP\n");
          break;
        }
        if((res = read(fd,&char_read,1)) == -1)
        {
          printf("CONNECTION FAILED\n");
          return -1;
        }
        
        if(res == 0)
        {
          continue;
        }
          
        /*---------------------------------------------------------------------*/
        /*-----------------------------STATE MACHINE---------------------------*/
        /*---------------------------------------------------------------------*/
        
        //printf("char_read= %d \n",char_read);
        if(state==0 && char_read==FLAG){
          state=1;
          printf("State 0-1\n");
        }
        else if(state==0 && char_read!=FLAG){
          state=0;
          printf("State 0-0\n");
        }
        else if(state==1 && char_read==A_3){
          state=2;
          printf("State 1-2\n");
        }
        else if(state==1 && char_read==FLAG){
          state=1;
          printf("State 1-1\n");
          
        }
        else if(state==1 && char_read!=FLAG && char_read!=A_3){
          state=0;
          printf("State 1-0\n");

        }
        else if(state==2 && char_read==FLAG){
          state=1;
          printf("State 2-1\n");
        }
        else if(state==2 && char_read==DISC){
          state=3;
          printf("State 2-3\n");
        }
        else if(state==2 && char_read!=FLAG && char_read!=DISC){
          state=0;
          printf("State 2-0\n");
          flag = 0;
          alarm(TimeoutTime);
        }
        else if(state==3 && char_read==FLAG){
          state=1;
          printf("State 3-1\n");
        }
        else if(state==3 && char_read==A_3^DISC){
          state=4;
          printf("State 3-4\n");
        }
        else if(state==3 && char_read!=FLAG && char_read!=A_1^DISC){
          state=0;
          printf("State 3-0\n");

        }
        else if(state==4 && char_read==FLAG){
          state=5;
          printf("State 4-5\n");
          
        }
        else if(state==4 && char_read!=FLAG){
          state=0;
          printf("State 4-0\n");
        }
      }
      frame = (unsigned char*)calloc(5,sizeof(unsigned char));
      frame[0] = FLAG;
      frame[1] = A_1;
      frame[2] = DISC;
      frame[3] = A_1^DISC;
      frame[4] = FLAG;
      FrameSize = 5;
      res = write(fd,frame,FrameSize);   
      printf("%d bytes written\n", res);
  }

  if (tcsetattr(fd,TCSANOW,&oldtio) != 0){
    return -1;
  }
  if (close(fd) != 0)
  {
    return -1;
  }
  return 0;
}

