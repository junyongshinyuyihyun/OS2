#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>




#define master_IOCTL_CREATESOCK 0x12345677
#define master_IOCTL_MMAP 0x12345678
#define master_IOCTL_EXIT 0x12345679
#define master_IOCTL_PAGE 0 //anything except 0x12345677/8/9
#define FCNTL 1
#define MMAP 2

#define PAGE_SIZE 4096

#define BUF_SIZE 512*16
#define MMAP_SIZE 100*4096


size_t get_filesize(const char* filename);//get the size of the input file


int main (int argc, char* argv[])
{
	char buf[BUF_SIZE];
	int i, dev_fd;//, file_fd;// the fd for the device and the fd for the input file
	size_t ret, file_size, tmp, total_file_size = 0;
	char dir_name[50], name_tmp[50], file_name[100], method[20];
	int fmethod;
	char *kernel_address = NULL, *file_address = NULL;
	struct timeval start;
	struct timeval end;
	double trans_time; //calulate the time between the device is opened and it is closed

	strcpy(dir_name, argv[1]);
	strcpy(method, argv[2]);
	if(strcmp(method, "fcntl") == 0)
		fmethod = FCNTL;
	else if(strcmp(method, "mmap") == 0)
		fmethod = MMAP;
	else
	{
		perror("method should be \"fcntl\"\"mmap\"\n");
		return 1;
	}


	if( (dev_fd = open("/dev/master_device", O_RDWR)) < 0)
	{
		perror("failed to open /dev/master_device\n");
		return 1;
	}
	gettimeofday(&start ,NULL);

	while(scanf("%s", &name_tmp) != EOF)
	{
		strcpy(file_name, dir_name);
		strcat(file_name, name_tmp);
		#ifdef DEBUG
			printf("%s\n", file_name);
		#endif

		int file_fd;
		if( (file_fd = open (file_name, O_RDWR)) < 0 )
		{
			perror("failed to open input file\n");
			return 1;
		}

		if( (file_size = get_filesize(file_name)) < 0)
		{
			perror("failed to get filesize\n");
			return 1;
		}

		total_file_size += file_size;

		if(ioctl(dev_fd, master_IOCTL_CREATESOCK) == -1) //0x12345677 : create socket and accept the connection from the slave
		{
			perror("ioclt server create socket error\n");
			return 1;
		}

		if(fmethod == FCNTL) //fcntl : read()/write()
		{
			do
			{
				ret = read(file_fd, buf, sizeof(buf)); // read from the input file(which will copy data from kernel space(input file) to user space(buf))
				write(dev_fd, buf, ret);//write to the the device by socket (which will copy data from user space(buf) to kernel space(master device))
			}while(ret > 0);
		}
		else if(fmethod == MMAP)
		{
			char *file_mmap, *dev_mmap;
			size_t offset = 0, left_size = file_size, send_size;
			
			if((file_mmap = mmap(NULL, file_size, PROT_READ, MAP_SHARED, file_fd, 0)) < 0)
			{
					perror("failed to mmap input file\n");
					return 1;
			}
			if((dev_mmap = mmap(NULL, MMAP_SIZE, PROT_WRITE, MAP_SHARED, dev_fd, 0)) < 0)
			{
					perror("failed to mmap master device\n");
					return 1;
			}
			while(offset < file_size) // copy and send data of 'PAGE_SIZE' bits each time until there is the end of file
			{
				send_size = left_size < MMAP_SIZE ? left_size : MMAP_SIZE; // if 'left_size' is less than 'PAGE_SIZE', just send 'send_size' to avoid accessing memory out the range of mmap size
				memcpy(dev_mmap, file_mmap + offset, send_size); // copy data from user space to user space, faster than transmission between user space and kernel space 
				ret = ioctl(dev_fd, master_IOCTL_MMAP, send_size); // use socket to send data 
				if(ret < 0)
				{
					perror("failed to send data by mmap\n");
					return -1;
				}
				left_size -= ret;
				offset += ret;
			}
			ioctl(dev_fd, master_IOCTL_PAGE, dev_mmap); // show page descriptor
			munmap(dev_mmap, PAGE_SIZE);
			munmap(file_mmap, file_size);

		}

		if(ioctl(dev_fd, master_IOCTL_EXIT) == -1) // end sending data, close the connection
		{
			perror("ioclt server exits error\n");
			return 1;
		}

		close(file_fd);
			
	}
	
	gettimeofday(&end, NULL);
	trans_time = (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)*0.0001;
	printf("Transmission time(Master): %lf ms, File size: %d bytes, Method: %s\n", trans_time, total_file_size, method);

	close(dev_fd);

	return 0;
}

size_t get_filesize(const char* filename)
{
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}
