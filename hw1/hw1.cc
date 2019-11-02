#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#define MAX_PKT_SIZE (640*480*4)

//comment to send pixels as commands via regular write function of char driver
//leave uncommented to write directly to memory (faster)
#define MMAP

int img[480][640];

int main(int argc, char *argv[])
{
	if (!argc){
		printf("Error parsing input.\n");
		exit(1);
	}

	std::fstream infile(argv[1], std::ios::in);
	if(!infile.is_open())
		std::cout << "Could not enter file " << argv[1] << std::endl;

	int pos = 0;
	std::string line;
	std::string delimiter;
	std::string token;
	while (std::getline(infile, line))
	{
		int cmd = 0, color = 0, x1 = 0, x2 = 0, y1 = 0, y2 = 0;
		delimiter = ":";	
		while ((pos = line.find(delimiter)) != std::string::npos) 
		{
			token = line.substr(0, pos);
    			std::cout << token << std::endl;
    			line.erase(0, pos + delimiter.length());
		}

		delimiter = ":";	
		while ((pos = line.find(delimiter)) != std::string::npos) 
		{
			token = line.substr(0, pos);
    			std::cout << token << std::endl;
    			line.erase(0, pos + delimiter.length());
		}

	}
	infile.close();

#ifdef ASD
	int x,y;
	#ifdef MMAP
	// If memory map is defined send image directly via mmap
	int fd;
	int *p;
	fd = open("/dev/vga_dma", O_RDWR|O_NDELAY);
	if (fd < 0)
	{
		printf("Cannot open /dev/vga for write\n");
		return -1;
	}
	p=(int*)mmap(0,640*480*4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	memcpy(p, image, MAX_PKT_SIZE);
	munmap(p, MAX_PKT_SIZE);
	close(fd);
	if (fd < 0)
	{
		printf("Cannot close /dev/vga for write\n");
		return -1;
	}

	#else
	// Send via regualar driver interface
	FILE* fp;
	for(y=0; y<480; y++)
	{
		for(x=0; x<640; x++)
		{
			fp = fopen("/dev/vga_dma", "w");
			if(fp == NULL)
			{
				printf("Cannot open /dev/vga for write\n");
				return -1;
			}
			fprintf(fp,"%d,%d,%#04x\n",x,y,image[y*640+x]);
			fclose(fp);
			if(fp == NULL)
			{
				printf("Cannot close /dev/vga\n");
				return -1;
			}
		}
	}

	#endif
#endif //ASD
	return 0;
}
