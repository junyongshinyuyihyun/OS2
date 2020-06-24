#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>


#define slave_IOCTL_CREATESOCK 0x12345677
#define slave_IOCTL_MMAP 0x12345678
#define slave_IOCTL_EXIT 0x12345679
#define slave_IOCTL_PAGE 0
#define FCNTL 1
#define MMAP 2

#define PAGE_SIZE 4096

#define BUF_SIZE 512*16
#define MMAP_SIZE 100*4096


int main (int argc, char* argv[])
{
	char buf[BUF_SIZE];
	int i, dev_fd;//, file_fd;// the fd for the device and the fd for the input file
	size_t ret, file_size = 0, total_file_size = 0, data_size = -1;
	char dir_name[50], name_tmp[50], file_name[100];
	char method[20];
	char ip[20];
	int n_file, fmethod;
	struct timeval start;
	struct timeval end;
	double trans_time; //calulate the time between the device is opened and it is closed
	char *kernel_address, *file_address;


	strcpy(dir_name, argv[1]);
	strcpy(method, argv[2]);
	strcpy(ip, argv[3]);
	if(strcmp(method, "fcntl") == 0)
		fmethod = FCNTL;
	else if(strcmp(method, "mmap") == 0)
		fmethod = MMAP;
	else
	{
		perror("method should be \"fcntl\"\"mmap\"\n");
		return 1;
	}

	if( (dev_fd = open("/dev/slave_device", O_RDWR)) < 0)//should be O_RDWR for PROT_WRITE when mmap()
	{
		perror("failed to open /dev/slave_device\n");
		return 1;
	}
	gettimeofday(&start ,NULL);

    while(scanf("%s", &name_tmp) != EOF)
	{
		strcpy(file_name, dir_name);
		strcat(file_name, "received");
		strcat(file_name, name_tmp + 6);
		
		#ifdef DEBUG
			printf("%s\n", file_name);
		#endif
		file_size = 0;
		
		int file_fd;
		if( (file_fd = open (file_name, O_RDWR | O_CREAT | O_TRUNC)) < 0)
		{
			perror("failed to open input file\n");
			return 1;
		}

		if(ioctl(dev_fd, slave_IOCTL_CREATESOCK, ip) == -1)	//0x12345677 : connect to master in the device
		{
			perror("ioclt create slave socket error\n");
			return 1;
		}
		#ifdef DEBUG
			write(1, "ioctl success\n", 14);
		#endif

		if(fmethod == FCNTL) //fcntl : read()/write()
		{
			do
			{
				ret = read(dev_fd, buf, sizeof(buf)); // read from the the device by socket(which will copy data from kernel space(slave device) to user space(buf))
				write(file_fd, buf, ret); //write to the input file (which will copy data from user space(buf) to kernel space(output file))
				file_size += ret;
			}while(ret > 0);
		}
		else if(fmethod == MMAP)
		{
			char *file_mmap, *dev_mmap;
			size_t offset = 0;
			
			if((dev_mmap = mmap(NULL, MMAP_SIZE, PROT_READ, MAP_SHARED, dev_fd, 0)) < 0)
			{
					perror("failed to mmap master device\n");
					return 1;
			}
			while(1)
			{

				ret  = ioctl(dev_fd, slave_IOCTL_MMAP, MMAP_SIZE);  // receive data of 'PAGE_SIZE' bits each time until slave device can't receive data any more 
				if(ret == 0)
					break;
				if(ret < 0)
				{
					perror("failed to recieve data by mmap\n");
					return -1;
				}
				offset = file_size;
				file_size += ret;
				ftruncate(file_fd, file_size); // largen the size of output file which should be greater or equal to output file's mmap size
				if((file_mmap = mmap(NULL, file_size, PROT_WRITE, MAP_SHARED, file_fd, 0)) < 0)
				{
						perror("failed to mmap input file\n");
						return 1;
				}
				memcpy(file_mmap + offset, dev_mmap, ret); // copy receviced data to the exact position of output file's mmap memory
				munmap(file_mmap, file_size);
			}

			ioctl(dev_fd, slave_IOCTL_PAGE, dev_mmap);
			munmap(dev_mmap, MMAP_SIZE);
			
		}

		if(ioctl(dev_fd, slave_IOCTL_EXIT) == -1)// end receiving data, close the connection
		{
			perror("ioclt client exits error\n");
			return 1;
		}

		total_file_size += file_size;
		close(file_fd);
		
	}


	gettimeofday(&end, NULL);
	trans_time = (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)*0.0001;
	printf("Transmission time(Slave): %lf ms, File size: %d bytes, Method: %s\n", trans_time, total_file_size, method);


	//close(file_fd);
	close(dev_fd);
	return 0;
}


