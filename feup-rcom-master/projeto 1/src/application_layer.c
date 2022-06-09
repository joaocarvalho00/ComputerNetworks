#include "application_layer.h"


int is_start = FALSE;
static file_info file;
static application_layer app_info;


int send_message(unsigned char* msg, int length){
	int res;
	if(is_start == FALSE){
		unsigned char* data_package = data_package_constructor(msg, &length);
		res= LLWRITE(app_info.file_descriptor, data_package, &length);

	}
	else{
		is_start= FALSE;
		res = LLWRITE(app_info.file_descriptor, msg, &length);
	}

	if (res == FALSE){

		return FALSE;
	}


	return TRUE;

}


unsigned char* get_message(){
	int length;
	unsigned char* readed_msg;
	unsigned char* only_data;
	static int file_received_size = 0;
		readed_msg = LLREAD(app_info.file_descriptor, &length);
		if(readed_msg == NULL || readed_msg[0] == DISC){
			return readed_msg;
		}
	switch(readed_msg[0]){
		case 0x02:
			fprintf(fp_log, "[BEGIN FILE]\n");
			get_file_params(readed_msg);
			break;
		case 0x01:
			utils_n_package++;
			only_data = get_only_data(readed_msg, &length);
			handle_writefile(only_data,length);
			file_received_size += length;
			progress_bar(file.filesize, file_received_size, file.filename, 'r');
			break;
		case 0x03:
			fprintf(fp_log, "[END FILE]\n");
			verify_end(readed_msg);
			break;

	}


	return readed_msg;

}

unsigned char* get_only_data(unsigned char* readed_msg, int* length){
	int j=0;
	unsigned int size = readed_msg[2]*256 + readed_msg[3];
	unsigned char* only_data = (unsigned char*) malloc(size);
	for(; j<size; j++){
			only_data[j] = readed_msg[j+4];
	}
	*length = size;
	free(readed_msg);
	return only_data;
}


int verify_end(unsigned char* msg){
	int i=0;
	unsigned char file_size[4];
	int file_size_size = msg[2];
	int file_size_total;

	for(; i<file_size_size; i++){
		file_size[i] = msg[i+3];
	}
	file_size_total = (file_size[0] <<24) | (file_size[1] << 16) | (file_size[2] << 8) | (file_size[3]);

	if(file_size_total == file.filesize && file_size_total == get_file_size()){
				fprintf(fp_log,"[END FILE] Received file size is correct\n");
				return TRUE;
	}
	else{
				fprintf(fp_log,"[END FILE] Received file is probably corrupted\n");
				return FALSE;
	}
	return FALSE;
}


void get_file_params(unsigned char* msg){

	int i=0;
	int j=0;
	unsigned char filesize[4];
	int filename_size;
	if(msg[1] == 0x00){
		int filesize_size = msg[2];
		for(; i<filesize_size; i++){
			filesize[i] = msg[i+3];
		}
		file.filesize = (filesize[0] <<24) | (filesize[1] << 16) | (filesize[2] << 8) | (filesize[3]);
	}
	i += 3;
	if(msg[i] == 0x01){
		i++;
		filename_size = msg[i];
		i++;
		file.filename = (char*) malloc (filename_size+1);
		for(; j<filename_size; j++,i++){
			file.filename[j] = msg[i];
		}
		file.filename[filename_size] = '\0';
	}

	file.fp = fopen((char*)file.filename,"wb");
	start_counting_time();

}


unsigned char* data_package_constructor(unsigned char* msg, int* length){

		unsigned char* data_package = (unsigned char*) malloc(*length+4);

		unsigned char c = 0x01;
		static unsigned int n = 0;
		int l2 = *length/256;
		int l1 = *length%256;

		data_package[0] = c;
		data_package[1] = (char) n;
		data_package[2] = l2;
		data_package[3] = l1;

		utils_n_package++;
		n++;
		n = (n % 256);
		int i=0;
		for(; i<*length; i++){
			data_package[i+4] = msg[i];
		}

		*length = *length+4;
		/*free(msg);*/

		return data_package;
}


int main(int argc, char** argv){


	if ( (argc < 3) ||
  	   ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	    (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

    if(strcmp("r",argv[2]) != 0 && strcmp("w",argv[2]) != 0){
    	printf("Usage:\tinvalid read/write mode. a correct mode (r / w).\n\tex: nserial /dev/ttyS1 r\n");
      exit(1);
    }
	srand(time(NULL));

	app_info.status = argv[2];



	if(strcmp("w", app_info.status)==0){
		if(argv[3] == NULL || argv[4] == NULL){
			printf("You need to specify the file to send and the size to read\n");
			exit(1);
		}
		if(argv[5] == NULL || argv[6] == NULL){
			printf("You need to specify the timeout value and the maximum number of transmissions\n");
			exit(1);
		}
		app_info.file_descriptor = LLOPEN(argv[1], app_info.status, argv[5], argv[6]);
			if(app_info.file_descriptor>0){
				file.filename = (char*) argv[3];
				file.size_to_read = atoi(argv[4]);
				int start_end_max_size;
				unsigned char* start_packet;
				unsigned char* end_packet;

				file.fp = fopen((char*)file.filename,"rb");
				if(file.fp == NULL)
				{
					printf("invalid file!\n");
					exit(-1);
				}
				if((file.filesize = get_file_size()) == -1)
				{
					return FALSE;
				}
				start_end_max_size = 2*(strlen(file.filename) + 9 ) + MAXSIZELINK; //max size for start/end package;
				start_packet = (unsigned char*) malloc(start_end_max_size);

				int start_created_size = create_STARTEND_packet(start_packet, START_PACKET_TYPE);

				if(start_created_size == -1){
					printf("Error creating start packet\n");
					exit(-1);
				}

				int i=0;
				for(;i < start_created_size;i++){
				}

				is_start = TRUE;
				if (send_message(start_packet,start_created_size) == FALSE){
					LLCLOSE(app_info.file_descriptor, -1);
				}

				handle_readfile();

				is_start = TRUE;
				end_packet = (unsigned char*) malloc(start_end_max_size);
				int endpacket_size = create_STARTEND_packet(end_packet,END_PACKET_TYPE);

				is_start=TRUE;
				if(send_message(end_packet,endpacket_size) == FALSE){
					LLCLOSE(app_info.file_descriptor, -1);
				}

				LLCLOSE(app_info.file_descriptor,WRITER);


			}
		}
		else if(strcmp("r", app_info.status)==0){
			if(argv[3] == NULL || argv[4] == NULL){
					printf("You need to specify the timeout value and the maximum number of transmissions\n");
					exit(1);
			}
			app_info.file_descriptor = LLOPEN(argv[1], app_info.status, argv[3], argv[4]);
			if(app_info.file_descriptor>0){
				unsigned char* msg;
				unsigned char null_val[] = {0xAA};


				do{
					msg = get_message();

					if(msg == NULL)
					{
						msg = null_val;
					}

				}while(msg[0] != DISC);

			}
		}else
		{
			printf("Error opening serial port\n");
			return 1;
		}
	return 0;

}


int get_file_size(){

	fseek(file.fp, 0L, SEEK_END);
	int filesize = (int) ftell(file.fp);
	if(filesize == -1)
		return -1;
	fseek(file.fp, 0L, SEEK_SET);
	return filesize;

}

int create_STARTEND_packet(unsigned char* packet, int type){
	int i = 0;
	int j = 3;
	unsigned char filesize_char[4];
	unsigned int filename_length = (unsigned int) strlen(file.filename);

	//convert filesize to and unsigned char array
	filesize_char[0] = (file.filesize >> 24) & 0xFF;
	filesize_char[1] = (file.filesize >> 16) & 0xFF;
	filesize_char[2] = (file.filesize >> 8) & 0xFF;
	filesize_char[3] = file.filesize & 0xFF;

	//size of filesize unsigned char array, normally is 4
	int length_filesize = sizeof(filesize_char)/sizeof(filesize_char[0]);


	if(type == START_PACKET_TYPE)
		packet[0] = 0x02;
	else if(type == END_PACKET_TYPE) {
		packet[0] = 0x03;
	}
	else{
		return -1;
	}
	packet[1] = 0x00;
	packet[2] = length_filesize;


	//put filesize unsigned char array in packet array
	for(; i < length_filesize; i++,j++){
		packet[j] = filesize_char[i];
	}

	packet[j] = 0x01;
	j++;
	packet[j] =  filename_length;

	j++;
	i=0;
	for(;i < filename_length; i++,j++)
	{
		packet[j] = file.filename[i];
	}


	return j;

}



void handle_readfile()
{
	unsigned char* data = malloc(file.size_to_read);
	int file_sent_size = 0;
	start_counting_time();
	utils_n_package = 0;
	fprintf(fp_log, "[BEGIN FILE]\n");
	fseek(file.fp,0,SEEK_SET);
	while(TRUE)
	{
		int res = 0;
		res = fread(data,sizeof(unsigned char),file.size_to_read,file.fp);
		if(res > 0)
		{

			if(send_message(data,res) == FALSE){
				LLCLOSE(app_info.file_descriptor, -1);
				exit(-1);
			}
			fprintf(fp_log, "[PACKAGE %d] RR%d\n", utils_n_package, dl_layer.control_value);
			file_sent_size += res;
			progress_bar(file.filesize, file_sent_size, file.filename, 'w');
		}
		if(feof(file.fp))
			break;

	}
	fprintf(fp_log, "[END FILE]\n");

}

void handle_writefile(unsigned char* data,int sizetowrite){

	fseek(file.fp,0,SEEK_END);
	fwrite(data,sizeof(unsigned char),sizetowrite,file.fp);
}
