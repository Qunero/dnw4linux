#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

const char* dev = "/dev/secbulk0";
static long DOWNLOAD_ADDR = 0x50008000;

void showHelp();

int main(int argc, char* argv[])
{
	unsigned char* file_buffer = NULL;
	char filename[128] = {0};
	int fd = -1;                                /* if no '-f file' specified, it remains -1*/
	int oc;
	long addr = 0;

	// handle options
	while ((oc = getopt(argc, argv, "ha:f:")) != -1){
		switch (oc){
			case 'h':
				showHelp();
				return 0;

			case 'a':
				// addr may be hex or dec 
				if (optarg[0]=='0' && (optarg[1]=='x'|| optarg[1]=='X')){
					sscanf(optarg, "%x", &addr);
					printf("get hex addr: 0x%x\n", addr);
				}
				else{
					addr = atoll(optarg);
				}

				// check addr 
				if (addr <=0 || addr>0xfffffffe){
					printf("Please check your addr(0x%x), it may be wrong!", addr);
					return -1;
				}
				else
				{
					DOWNLOAD_ADDR = addr;
				}
				break;

			case 'f':
				fd = open(optarg, O_RDONLY);
				if(-1 == fd) {
					printf("Can not open file - %s\n", optarg);
					return 1;
				}
				memcpy(filename, optarg, sizeof(filename));
				break;

			case '?':
			default:
				printf("Unknow options, please see help \n");
				showHelp();
				return -1;
		}
	}

	// no '-f file' specified
	if(-1 == fd) {
		printf("You must specify exactly one file with -f to open\n");
		showHelp();
		return 1;
	}

	// get file size 
	struct stat file_stat;
	if( -1 == fstat(fd, &file_stat) ) {
		perror("Get file size filed!\n");
		return 1;
	}	

	file_buffer = (unsigned char*)malloc(file_stat.st_size+10);
	if(NULL == file_buffer) {
		perror("malloc failed!\n");
		goto error;
	}
	//memset(file_buffer, '\0', sizeof(file_buffer)); // bad code ! corrected by Qulory
	memset(file_buffer, '\0', sizeof(char)*(file_stat.st_size+10));

	// the first 8 bytes in the file_buffer is reserved, the last 2 bytes also;
	if( file_stat.st_size !=  read(fd, file_buffer+8, file_stat.st_size))	{
		perror("Read file failed!\n");
		goto error;
	}

	printf("File name : %s\n", filename);
	printf("File size : %ld bytes\n", file_stat.st_size);// off_t is long int 

	int fd_dev = open(dev, O_WRONLY);
	if( -1 == fd_dev)	{
		printf("Can not open %s\n", dev);
		goto error;
	}

	/*
	 * Note: the first 4 bytes store the dest addr ;
	 * the following 4 bytes store the file size ;
	 * and the last 2 bytes store the sum of each bytes of the file ;
	 */
	*((unsigned long*)file_buffer) = DOWNLOAD_ADDR; 	//load address
	*((unsigned long*)file_buffer+1) = file_stat.st_size+10;	//file size
	unsigned short sum = 0;
	int i;
	for(i=8; i<file_stat.st_size+8; i++)	{
		sum += file_buffer[i];
	}

	*((unsigned short*)(file_buffer+8+file_stat.st_size)) = sum;

	printf("Start Sending data...\n");
	printf("Dest address: 0x%x\n", DOWNLOAD_ADDR);
	size_t remain_size = file_stat.st_size+10;
	size_t block_size = 512;
	size_t written = 0;
	while(remain_size > 0)	{
		size_t to_write = remain_size > block_size ? block_size:remain_size;
		size_t real_write = write(fd_dev, file_buffer+written, to_write);
		if( to_write != real_write)	{
			printf(" write  /dev/secbulk0 failed!  to_write = %u real_write = %u \n" , to_write ,real_write );
			printf("Error detail: errno=%d, info:%s\n", errno, strerror(errno));
			return 1;
		}
		remain_size -= to_write;
		written += to_write;
		printf("\rSent %lu%% \t %u bytes !", written*100/(file_stat.st_size+10),  written);
		fflush(stdout);
	}	

	printf("OK\n");
	return 0;

error:
	if(-1 != fd_dev) {
		close(fd_dev);
	}
	if(fd != -1)  {
		close(fd);
	}
	if( NULL != file_buffer ) {
		free(file_buffer);
	}
	return -1;
}

void showHelp(){
	printf("Usage: dwn [-a download_addr] <-f filename>\n");
}
