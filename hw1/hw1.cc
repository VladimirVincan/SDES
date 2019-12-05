#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define WIDTH 640
#define HEIGHT 480
#define MAX_PKT_SIZE (640*480*4)

//comment to send pixels as commands via regular write function of char driver
//leave uncommented to write directly to memory (faster)
#define MMAP

unsigned int img[HEIGHT][WIDTH];

std::string black = "BLACK";
std::string red = "RED";
std::string yellow = "YELLOW";
std::string green = "GREEN";
std::string blue = "BLUE";

unsigned int get_color(std::string color_s)
{
	unsigned int black_color = 0;
	unsigned int red_color   = (1<<15) | (1<<14) | (1<<13) | (1<<12) | (1<<11);
	unsigned int green_color = (1<<10) | (1<<9)  | (1<<8)  | (1<<7)  | (1<<6)  | (1<<5);
	unsigned int blue_color  = (1<<4)  | (1<3)   | (1<<2)  | (1<<1)  | (1<<0);
	unsigned int yellow_color = red_color | green_color; 

	std::cout << "input color = " << color_s << std::endl;

	if (!color_s.compare(black))
		return black_color;
	else if (!color_s.compare(red))
		return red_color;
	else if (!color_s.compare(yellow))
		return yellow_color;
	else if (!color_s.compare(green))
		return green_color;
	else if (!color_s.compare(blue))
		return blue_color;

	printf("Not valid color.\n");
	exit(3);
}

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
		int cmd = 0, x1 = 0, x2 = 0, y1 = 0, y2 = 0;
		unsigned int color = 0;
		delimiter = ":";	
		pos = line.find(delimiter);
		token = line.substr(0, pos);
    		line.erase(0, pos + delimiter.length() + 1);
		
		if (token[0] == 'B')
			cmd = 1; // BCKG
		else if (token[0] == 'R')
			cmd = 2; // RECT
		else if (token[5] == 'H')
			cmd = 3; // LINE_H
		else if (token[5] == 'V')
			cmd = 4; // LINE_V

		if (!cmd) 
		{
			printf("Error reading command.\n");
			exit(2);
		}

		if (cmd == 1) // BCKG
		{
			color = get_color(line);
			for (int i = 0; i < HEIGHT; ++i)
				for (int j = 0; j < WIDTH; ++j)
					img[i][j] = color;		

			std::cout << "color = " << color << std::endl << std::endl;
		}
		else if (cmd == 2) // RECT
		{
			delimiter = ",";	

			pos = line.find(delimiter);
			token = line.substr(0, pos);
    			line.erase(0, pos + delimiter.length());
			sscanf(token.c_str(), "%d", &x1);

			pos = line.find(delimiter);
			token = line.substr(0, pos);
    			line.erase(0, pos + delimiter.length());
			sscanf(token.c_str(), "%d", &x2);

			pos = line.find(delimiter);
			token = line.substr(0, pos);
    			line.erase(0, pos + delimiter.length());
			sscanf(token.c_str(), "%d", &y1);

			delimiter = ";";	

			pos = line.find(delimiter);
			token = line.substr(0, pos);
    			line.erase(0, pos + delimiter.length() + 1);
			sscanf(token.c_str(), "%d", &y2);

			std::cout << "x1, x2, y1, y2 = " << x1 << " " << x2 << " " << y1 << " " << y2 << std::endl;

			color = get_color(line);
			for (int i = y1; i < y2; ++i)
				for (int j = x1; j < x2; ++j)
					img[i][j] = color;

			std::cout << "color = " << color << std::endl << std::endl;
		}
		else if (cmd == 3) // LINE_H
		{
			delimiter = ",";	

			pos = line.find(delimiter);
			token = line.substr(0, pos);
    			line.erase(0, pos + delimiter.length());
			sscanf(token.c_str(), "%d", &x1);

			pos = line.find(delimiter);
			token = line.substr(0, pos);
    			line.erase(0, pos + delimiter.length());
			sscanf(token.c_str(), "%d", &x2);

			delimiter = ";";	

			pos = line.find(delimiter);
			token = line.substr(0, pos);
    			line.erase(0, pos + delimiter.length() + 1);
			sscanf(token.c_str(), "%d", &y1);

			std::cout << "x1, x2, y1 = " << x1 << " " << x2 << " " << y1 << std::endl;

			color = get_color(line);
			for (int j = x1; j < x2; ++j)
				img[y1][j] = color;

			std::cout << "color = " << color << std::endl << std::endl;
		}
		else if (cmd == 4) // LINE_V
		{
			delimiter = ",";	

			pos = line.find(delimiter);
			token = line.substr(0, pos);
    			line.erase(0, pos + delimiter.length());
			sscanf(token.c_str(), "%d", &x1);

			pos = line.find(delimiter);
			token = line.substr(0, pos);
    			line.erase(0, pos + delimiter.length());
			sscanf(token.c_str(), "%d", &y1);

			delimiter = ";";	

			pos = line.find(delimiter);
			token = line.substr(0, pos);
    			line.erase(0, pos + delimiter.length() + 1);
			sscanf(token.c_str(), "%d", &y2);

			std::cout << "x1, y1, y2 = " << x1 << " " << y1 << " " << y2 << std::endl;
			
			color = get_color(line);
			for (int i = y1; i < y2; ++i)
				img[i][x1] = color;

			std::cout << "color = " << color << std::endl << std::endl;
		}
	}
	infile.close();

	#ifdef MMAP
	// If memory map is defined send img directly via mmap
	int fd;
	int *p;
	// fd = open("/dev/vga_dma", O_RDWR|O_NDELAY);
  // fd = shm_open("/home/bici/vga_ldd_emulator/vga_buffer", O_RDWR, 0666);
  fd = shm_open("vga_buffer", O_RDWR, 0666);
	if (fd < 0)
	{
		// printf("Cannot open /dev/vga for write\n");
		printf("Cannot open vga_buffer for write\n");
		return -1;
	}
	p=(int*)mmap(0,640*480*4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	memcpy(p, img, MAX_PKT_SIZE);
	munmap(p, MAX_PKT_SIZE);
	close(fd);
	if (fd < 0)
	{
		// printf("Cannot close /dev/vga for write\n");
    printf("Cannot close vga_buffer\n");
		return -1;
	}

	#else
	int x,y;
	// Send via regualar driver interface
	FILE* fp;
	for(y=0; y<480; y++)
	{
		for(x=0; x<640; x++)
		{
			fp = fopen("/vga_buffer", "w");
			if(fp == NULL)
			{
				// printf("Cannot open /dev/vga for write\n");
        printf("Cannot open /vga_buffer for write\n");
				return -1;
			}
			fprintf(fp,"%d,%d,%#04x\n",x,y,img[y*640+x]);
			fclose(fp);
			if(fp == NULL)
			{
				// printf("Cannot close /dev/vga\n");
				printf("Cannot close /vga_buffer\n");
				return -1;
			}
		}
	}

	#endif
	return 0;
}
