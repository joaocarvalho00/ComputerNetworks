#include "utils.h"

struct timespec initial_time, final_time;
int n_number = -256;

void start_counting_time(){
    clock_gettime(CLOCK_REALTIME, &initial_time);
}

double calculate_time_elapsed(){
  clock_gettime(CLOCK_REALTIME, &final_time);
  double time_val = (final_time.tv_sec-initial_time.tv_sec)+
					(final_time.tv_nsec- initial_time.tv_nsec)/1E9;
  return time_val;
}

void progress_bar(int filesize, int file_sent_size, char* filename, char type){
  system("clear");
  int perc_real =  file_sent_size * 100 / filesize;
  int size_bar = 50;
  int perc = perc_real * size_bar / 100;
  int i;

  /* SHOW TYPE */
  if (type == 'r')
    printf("\n\t\t\t\t     RECEIVING\n");
  else
    printf("\n\t\t\t\t      SENDING\n");

  /*SHOW FILENAME */
  unsigned int filename_length = (unsigned int) strlen(filename);
  printf("\n\t| FILE: ");
  for (i = 0; i < filename_length; i++) {
    printf("%c", filename[i]);
  }
  printf("\n\t|");

  /*SHOW FILE SIZE */
  printf("\n\t| SIZE: %d bytes", filesize);
  printf("\n\t|");

  /*SHOW FILE SIZE */
  double timeSpent = calculate_time_elapsed();
  printf("\n\t| TIME ELAPSED: %.4lf s", timeSpent);
  printf("\n\t|");

  /* SHOW TRANSMISSION SPEED */
  float speed = (float) (file_sent_size / timeSpent);
  printf("\n\t| SPEED: %.2lf bytes/s", speed);
  printf("\n\t|");

  /* SHOW RESPONSE (ONLY READER) */
  if (type == 'r')
  {
	switch(utils_response_value[0]){

		case 0:
			printf("\n\t| RESPONSE: REJ%d", utils_response_value[1]);
  			printf("\n\t|");
			break;

		case 1:
			printf("\n\t| RESPONSE: RR%d", utils_response_value[1]);
			printf("\n\t|");
			break;

	}
  }

  /* SHOW PACKAGE NUMBER */
  printf("\n\t| PACKAGE NUMBER: %d", utils_n_package);
  printf("\n\t|");

  /* SHOW PROGRESS BAR */
  printf("\n\t| STATUS [");
  for (i = 0; i < perc; i++) {
      printf("*");
  }

  int rest = size_bar - perc;
  for (i = 0; i < rest ; i++) {
      printf(" ");
  }
  printf("]");

  /* convert % to char */
  int tmp = 37;
  unsigned char ch = tmp;

  printf(" %d%c\n\n", perc_real, ch);

}

void open_log_file(char* mode){

	struct tm * timeinfo;
	time_t stime;

	time(&stime);
	timeinfo = localtime(&stime);

	int d = timeinfo->tm_mday;
	int m = timeinfo->tm_mon + 1;
	int y = timeinfo->tm_year + 1900;

	int h = timeinfo->tm_hour;
	int mi = timeinfo->tm_min;
	int s = timeinfo->tm_sec;

	char hour[2];
	hour[0] = (h/10);
	hour[1] = (h%10);

	char min[2];
	min[0] = (mi/10);
	min[1] = (mi%10);

	char sec[2];
	sec[0] = (s/10);
	sec[1] = (s%10);

	char day[2];
	day[0] = (d/10);
	day[1] = (d%10);

	char month[3];

	switch(m){
		case 1:
			month[0] = (char) 'J';
			month[1] = 'A';
			month[2] = 'N';
			break;

		case 2:
			month[0] = 'F';
			month[1] = 'E';
			month[2] = 'B';
			break;

		case 3:
			month[0] = 'M';
			month[1] = 'A';
			month[2] = 'R';
			break;

		case 4:
			month[0] = 'A';
			month[1] = 'P';
			month[2] = 'R';
			break;

		case 5:
			month[0] = 'M';
			month[1] = 'A';
			month[2] = 'Y';
			break;

		case 6:
			month[0] = 'J';
			month[1] = 'U';
			month[2] = 'N';
			break;

		case 7:
			month[0] = 'J';
			month[1] = 'U';
			month[2] = 'L';
			break;

		case 8:
			month[0] = 'A';
			month[1] = 'U';
			month[2] = 'G';
			break;

		case 9:
			month[0] = 'S';
			month[1] = 'E';
			month[2] = 'T';
			break;

		case 10:
			month[0] = 'O';
			month[1] = 'C';
			month[2] = 'T';
			break;

		case 11:
			month[0] = 'N';
			month[1] = 'O';
			month[2] = 'V';
			break;

		case 12:
			month[0] = 'D';
			month[1] = 'E';
			month[2] = 'C';
			break;

	}

	fp_log = fopen("log.txt", "a");
  if(strcmp(mode,"r") == 0){
    fprintf(fp_log, "READER ");
  }
  else {
    fprintf(fp_log, "WRITER ");
  }
	fprintf(fp_log, "LOG: %x%x:", hour[0], hour[1]);
	fprintf(fp_log, "%x%x:", min[0], min[1]);
	fprintf(fp_log, "%x%x, ", sec[0], sec[1]);
	fprintf(fp_log, "%x%x ", day[0], day[1]);
	fprintf(fp_log, "%c%c%c ",  month[0], month[1], month[2]);
	fprintf(fp_log, "%d\n\n", y);

}
