#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
// #include <string>

int selection = 0;
int filesize;
int sent = 0;
FILE * file;
int *p;

#define WIDTH 640
#define HEIGHT 480
#define MAX_PKT_SIZE (640*480*4)
// unsigned int img[HEIGHT][WIDTH];

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

int x_pos[4] = {WIDTH/4-20, WIDTH*3/4-20, WIDTH/4-20, WIDTH*3/4-20};
int y_pos[4] = {HEIGHT/4-20, HEIGHT/4-20, HEIGHT*3/4-20, HEIGHT*3/4-20};
bool set_rect[4] = {false, false, false, false};
int x_min[4] = {0,           WIDTH/2+2,  0,          WIDTH/2+2};
int x_max[4] = {WIDTH/2-40,  WIDTH-40,   WIDTH/2-40, WIDTH-40};
int y_min[4] = {0,           0,          HEIGHT/2+2, HEIGHT/2+2};
int y_max[4] = {HEIGHT/2-40, HEIGHT/2-40,HEIGHT-40,  HEIGHT-40};

void line_v(int x, int y1, int y2, std::string color)
{
  unsigned int rgb;
  rgb = get_color(color);
  for (int i = y1; i < y2; ++i)
    // img[i][x] = rgb;
    *(p+WIDTH*i+x) = rgb;
}

void line_h(int x1, int x2, int y, std::string color)
{
  unsigned int rgb;
  rgb = get_color(color);
  for (int i = x1; i < x2; ++i)
    // img[y][i] = rgb;
    *(p+WIDTH*y+i) = rgb;
}

void rect(int x1, int x2, int y1, int y2, std::string color)
{
  unsigned int rgb;
  rgb = get_color(color);
  for (int i = y1; i < y2; ++i)
    for (int j = x1; j < x2; ++j)
      // img[i][j] = rgb;
      *(p+WIDTH*i+j) = rgb;
}

void rect40(int x, int y, std::string color)
{
  rect(x,x+40,y, y+40, color);
}

void flush_img(int id)
{
  unsigned int rgb;
  rgb = get_color("BLACK");
  for (int i = y_min[id]; i < y_max[id]+40; ++i)
    for (int j = x_min[id]; j < x_max[id]+40; ++j)
      // img[i][j] = rgb;
      *(p+i*WIDTH+j) = rgb;
}

void doprocessing (int sock, int id)
{
  /*lokalne promenljive*/
  int n;
  char buffer[256];
  char sendBuf[256];
  char tempstr[256];
  bzero(buffer,256);
  int done = 0;
  int i = 0;
  int w = 30;
  std::cout << "Socket no = " << id << std::endl;
  set_rect[id] = true;
  // call_mmap(id);
  flush_img(id);
  rect40(x_pos[id],y_pos[id],"RED");

  while (!done)
    {
      printf("[IDLE] Waiting for WASD command..\n");
      n = -1;
      while(n < 1)
        n = read(sock,buffer,255);
      buffer[n] = 0;//terminiraj string primljen od strane klijenta
      printf("Received command: %s\n",buffer);
      if (strcmp(buffer,"q") == 0){
        printf("Client disconnected.\n");
        set_rect[id] = false;
        flush_img(id);
        // call_mmap(-1);
        return;
      }
      else if (strcmp(buffer,"w") == 0)
        {
          printf("Received W.\n");
          if (y_pos[id] > y_min[id] && y_pos[id]-10 > y_min[id]) y_pos[id] -= 10;
          else y_pos[id] = y_min[id];
        }
      else if (strcmp(buffer,"a") == 0)
        {
          printf("Received A.\n");
          if (x_pos[id] > x_min[id] && x_pos[id]-10 > x_min[id]) x_pos[id] -= 10;
          else x_pos[id] = x_min[id];
        }
      else if (strcmp(buffer,"s") == 0)
        {
          printf("Received S.\n");
          if (y_pos[id] < y_max[id] && y_pos[id]+10 < y_max[id]) y_pos[id] += 10;
          else y_pos[id] = y_max[id];
        }
      else if (strcmp(buffer,"d") == 0)
        {
          printf("Received D.\n");
          if (x_pos[id] < x_max[id] && x_pos[id]+10 < x_max[id]) x_pos[id] += 10;
          else x_pos[id] = x_max[id];
        }
      else {
        printf("Could not recognize command string...\n");
        printf("%s\n", buffer);
      }
      // call_mmap(id);
      flush_img(id);
      rect40(x_pos[id],y_pos[id],"RED");
    }
}
/* glavni program serverske aplikacije */
#define SHMEM_SIZE (4 * sizeof(int))
int main( int argc, char *argv[] )
{
  int fd;
  // fd = open("/dev/vga_dma", O_RDWR|O_NDELAY);
  fd = shm_open("vga_buffer", O_RDWR, 0666);
  if (fd < 0)
    {
      // printf("Cannot open /dev/vga for write\n");
      printf("Cannot open vga_buffer for write\n");
      exit(2);
    }
  p=(int*)mmap(0,640*480*4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

//==========

  int *arr_cli;
  int shmem_fd = shm_open("cli_arr", O_RDWR | O_CREAT, 0666);
  if (shmem_fd < 0) {
    std::cout << "ERROR creating shared memory vga_buffer\n";
    return EXIT_FAILURE;
  }
  ftruncate(shmem_fd, SHMEM_SIZE);

  arr_cli = (int *)mmap(NULL, SHMEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
                       shmem_fd, 0);
  arr_cli[0] = 0;
  arr_cli[1] = 0;
  arr_cli[2] = 0;
  arr_cli[3] = 0;

  int sockfd, newsockfd, portno, clilen;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;
  int n;
  /* najpre se poziva uvek socket() funkcija da se registruje socket:
     AF_INET je neophodan kada se zahteva komunikacija bilo koja
     dva host-a na Internetu;
     Drugi argument definise tip socket-a i moze biti SOCK_STREAM ili SOCK_DGRAM:
     SOCK_STREAM odgovara npr. TCP komunikaciji, dok SOCK_DGRAM kreira npr. UDP kanal
     Treci argument je zapravo protokol koji se koristi: najcesce se stavlja 0 sto znaci da
     OS sam odabere podrazumevane protokole za dati tip socket-a (TCP za SOCK_STREAM
     ili UDP za SOCK_DGRAM) */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    {
      perror("ERROR opening socket");
      exit(1);
    }
  /* Inicijalizacija strukture socket-a */
  bzero((char *) &serv_addr, sizeof(serv_addr));
  rect(0,WIDTH,0,HEIGHT,"BLACK");
  line_v(WIDTH/2,0,HEIGHT-1,"BLUE");
  line_h(0,WIDTH-1,HEIGHT/2,"BLUE");
  portno = 5001;
  serv_addr.sin_family = AF_INET; //mora biti AF_INET
  /* ip adresa host-a. INADDR_ANY vraca ip adresu masine na kojoj se startovao server */
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  /* broj porta-ne sme se staviti kao broj vec se mora konvertovati u
     tzv. network byte order funkcijom htons*/
  serv_addr.sin_port = htons(portno); 
  /* Sada bind-ujemo adresu sa prethodno kreiranim socket-om */
  if (bind(sockfd, (struct sockaddr *) &serv_addr,
           sizeof(serv_addr)) < 0)
    {
      perror("ERROR on binding");
      exit(1);
    }
  printf("FTP server started.. waiting for clients ...\n");
  /* postavi prethodno kreirani socket kao pasivan socket
     koji ce prihvatati zahteve za konekcijom od klijenata
     koriscenjem accept funkcije */
  // call_mmap(-1);
  listen(sockfd,5); //maksimalno 5 klijenata moze da koristi moje usluge
  clilen = sizeof(cli_addr);
  while (1)
    {
      unsigned int clilen2 = (unsigned int) clilen;
      /*ovde ce cekati sve dok ne stigne zahtev za konekcijom od prvog klijenta*/
      newsockfd = accept(sockfd,
                         (struct sockaddr *) &cli_addr, &clilen2);
      printf("FTP client connected...\n");
      if (newsockfd < 0)
        {
          perror("ERROR on accept");
          exit(1);
        }
      /* Kreiraj child proces sa ciljem da mozes istovremeno da
         komuniciras sa vise klijenata */
      int cli_num = -1;
      if (arr_cli[0] == 0)
        {
          arr_cli[0] = 1; cli_num = 0;
        }
      else if (arr_cli[1] == 0)
        {
          arr_cli[1] = 1; cli_num = 1;
        }
      else if (arr_cli[2] == 0)
        {
          arr_cli[2] = 1; cli_num = 2;
        }
      else if (arr_cli[3] == 0)
        {
          arr_cli[3] = 1; cli_num = 3;
        }
      std::cout << "CLI NUM = " <<  cli_num << std::endl;
      int pid = fork();


      if (pid < 0)
        {
          perror("ERROR on fork");
          exit(1);
        }
      if (pid == 0)
        {
          /* child proces ima pid 0 te tako mozemo znati da li
             se ovaj deo koda izvrsava u child ili parent procesu */
          close(sockfd);
          doprocessing(newsockfd, cli_num);
          arr_cli[cli_num] = 0;
          for (int i = 0;i<4;++i)
            std::cout << "arr cli[" << i << "]" << arr_cli[i] << std::endl;
          exit(0);
        }
      else
        {
          /*ovo je parent proces koji je samo zaduzen da
            delegira poslove child procesima-stoga ne moras
            da radis nista vec samo nastavi da osluskujes
            nove klijente koji salju zahtev za konekcijom*/
          close(newsockfd);
        }
      // call_mmap();
    } /* end of while */

  munmap(arr_cli, SHMEM_SIZE);
  close(shmem_fd);
  shm_unlink("arr_cli");

  munmap(p, MAX_PKT_SIZE);
  close(fd);
  if (fd < 0)
    {
      // printf("Cannot close /dev/vga for write\n");
      printf("Cannot close vga_buffer\n");
      exit(3);
    }
}
