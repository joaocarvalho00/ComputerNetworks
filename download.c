#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>


#define FTP_PORT  21
#define MAX_STRING_SIZE 256


int isValidURL(char *url)
{
    regex_t regex;
    int aux;
  // Regex to check valid URL
    
 
    if(regcomp(&regex, "^ftp://([a-z0-9]+:[a-z0-9]+@)?([.a-z0-9-]+)/([./a-z0-9-]+)$", REG_EXTENDED|REG_ICASE) == 0)
    {
      printf("Regex Compiled! \n");
    }
    else
    {
        printf("Regex not Compiled! :C \n");
        exit(1);
    }
    aux = regexec(&regex, url , 0, NULL, 0);
    if(aux == 0)
    {
        regfree(&regex);
        return 0;
    }
    else if(aux == REG_NOMATCH)
    {
        regfree(&regex);
        return -1;
    }
    regfree(&regex);
    return -2;
}

int get_reply(int sockfd)
{
    char recv_char;
    int state = 0;
    int index = 0;
    char replycode[3];
    while (state != 10)
    {
        read(sockfd,&recv_char,1);
        printf("%c",recv_char);
        switch (state)
        {
        case 0:
            if(isdigit(recv_char))
            {
                replycode[state] = recv_char;
                state=1;
            }
            break;
        case 1:
            if(isdigit(recv_char))
            {
                replycode[state] = recv_char;
                state=2;
            }
            break;
        case 2:
            if(isdigit(recv_char))
            {
                replycode[state] = recv_char;
                state=3;
            }
            break;
        case 3:
            if(recv_char == '-')
            {
                state = 4;
            }
            if(recv_char == ' ')
            {
                state = 9;
            }
            break;
        case 4:
            if(isdigit(recv_char) && recv_char == replycode[state-4])
            {
                state = 5;
            }
            else
            {
                state = 4;
            }
            break;
        case 5:
            if(isdigit(recv_char) && recv_char == replycode[state-4])
            {
                state = 6;
            }
            else
            {
                state = 4;
            }
            break;
        case 6:
            if(isdigit(recv_char) && recv_char == replycode[state-4])
            {
                state = 3;
            }
            else
            {
                state = 4;
            }
            break;
        case 9:
            if(recv_char == '\n')
            {
                //printf("\n Should exit here \n");
                state = 10;
            }
            break;
        
        }
    }
    return replycode[0];
}


int main(int argc, char *argv[])
{
	struct hostent *h;
	int sockfd,sockfd_client;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;


	if(isValidURL(argv[1])>0)
    {
        printf("USAGE: ftp://<user>:<password>@<host>/<url-path>\n");
		return -1;
    }
    else
    {
        printf("Valid url, proceding\n");
    }
	
    char* user = calloc(MAX_STRING_SIZE,sizeof(char));
	char* pass = calloc(MAX_STRING_SIZE,sizeof(char));
	char* host = calloc(MAX_STRING_SIZE,sizeof(char));
	char* urlpath = calloc(MAX_STRING_SIZE,sizeof(char));

    int state = 0;
    int urlindex = 6, index = 0;

    while (urlindex < strlen(argv[1]))
    {
        if(state ==0)
        {
            if(argv[1][urlindex] != ':')
            {
                user[index] = argv[1][urlindex];
                index++;
            }
            else
            {
                user[index] = '\n';
                user[index+1] = '\0';
                index = 0;
                state = 1;
                urlindex++;
            }
        }
        if(state ==1)
        {
            if(argv[1][urlindex] != '@')
            {
                pass[index] = argv[1][urlindex];
                index++;
            }
            else
            {
                pass[index] = '\n';
                pass[index+1] = '\0';
                index = 0;
                state = 2;
                urlindex++;
            }
        }
        if(state ==2)
        {
            if(argv[1][urlindex] != '/')
            {
                host[index] = argv[1][urlindex];
                index++;
            }
            else
            {
                host[index] = '\0';
                index = 0;
                state = 3;
                urlindex++;
            }
        }
        if(state ==3)
        {
            urlpath[index] = argv[1][urlindex];
            index++;
        }
        urlindex++;
    }
    urlpath[index] = '\n';
    urlpath[index+1] = '\0';

    if ((h=gethostbyname(host)) == NULL) 
    {  
        herror("gethostbyname");
        exit(1);
    }

    printf("Host name  : %s\n", h->h_name);
    printf("IP Address : %s\n",inet_ntoa(*((struct in_addr *)h->h_addr)));

    bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)h->h_addr)));	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(FTP_PORT);		/*server TCP port must be network byte ordered */
    
	/*open an TCP socket*/
	if ((sockfd =socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        perror("socket()");
        exit(0);
    }
	/*connect to the server*/
    if(connect(sockfd,(struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect()");
		exit(0);
	}

    get_reply(sockfd);

    printf("user %s", user);
    write(sockfd,"user ",5);
    write(sockfd, user, strlen(user));

    int replycode = get_reply(sockfd);
    if (replycode == '4' || replycode == '5')
    {
		close(sockfd);
		return -1;
	}

    printf("pass %s", user);
    write(sockfd,"pass ",5);
    write(sockfd, pass, strlen(pass));

    replycode = get_reply(sockfd);
    if (replycode == '4' || replycode == '5')
    {
		close(sockfd);
		return -1;
	}

    printf("pasv\n");
    write(sockfd,"pasv\n",5);

    char recv_char = 0;
	int counter = 0;
	char buf[7];
	index = 0;
	while(read(sockfd,&recv_char,1))
    {
		printf("%c", recv_char);
		if(recv_char==')')
		{
			printf("\n");
			break;
		}
		if (counter >= 4)
        {
			buf[index]=recv_char;
			index++;
		}
		if(recv_char == ',')
        {
			counter++;
        }
	}
    int first=0, second=0, flag =0;
    for(int i = 0; i < strlen(buf); i++)
    {
        if(buf[i] == ',')
        {
            flag = 1;
        }else
        if(!flag)
        {
            first= first*10;
            first = first + (int)(buf[i] - '0');
            
        }else
        {
            second = second*10;
            second = second + (int)(buf[i] - '0');
        }
    }

    int newport = first*256 + second;
    
    bzero((char*)&client_addr,sizeof(client_addr));

	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)h->h_addr)));	/*32 bit Internet address network byte ordered*/
	client_addr.sin_port = htons(newport);		/*server TCP port must be network byte ordered */
    
	/*open an TCP socket*/
	if ((sockfd_client = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        perror("socket()");
        exit(0);
    }
	/*connect to the server*/
    if(connect(sockfd_client,(struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
    {
		perror("connect()");
		exit(0);
	}

    printf("retr %s", urlpath);
    write(sockfd,"retr ",5);
    write(sockfd, urlpath, strlen(urlpath));

    replycode = get_reply(sockfd);
    if (replycode == '4' || replycode == '5')
    {
        close(sockfd_client);
		close(sockfd);
		return -1;
	}
    char* filename = calloc(MAX_STRING_SIZE,sizeof(char));
	index = 0;
	for(int i = 0; i < strlen(urlpath); i++) {
		if(urlpath[i] == '/') {
			filename = calloc(MAX_STRING_SIZE,sizeof(char));
			index = 0;
		}
		else{
			filename[index] = urlpath[i];
			index++;
		}
	}
    //printf("%s\n", filename);
    FILE *f;
    f = fopen(filename,"w");
    index = 0;
    while((index=read(sockfd_client,&recv_char,1))){
		fprintf(f,"%c", recv_char);
	}

	fclose(f);
	close(sockfd);
	close(sockfd_client);

	return 0;
}