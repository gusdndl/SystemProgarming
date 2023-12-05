#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdint.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <pthread.h>
#include <errno.h>
#include <wiringPi.h>

#define IN 0
#define OUT 1
#define LOW 0
#define HIGH 1
#define SUPDO 7
#define MUNJI 20
#define GAS 21
#define VALUE_MAX 40
#define ARRAY_SIZE(array) sizeof(array)/sizeof(array[0])
#define MAX_TIMINGS    85

static const char *DEVICE = "/dev/spidev0.0";
static uint8_t MODE = SPI_MODE_0;
static uint8_t BITS = 8;
static uint32_t CLOCK = 1000000;
static uint16_t DELAY = 5;
char buffer[10]={0};
int gasgap, munjigap, supdogap;

void error_handling(char *message){
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}

static int GPIOExport(int pin);
static int GPIODirection(int pin, int dir);
static int GPIOWrite(int pin, int value);
static int GPIOUnexport(int pin);
static int GPIORead(int pin);
static int prepare(int fd);
uint8_t control_bits_differential(uint8_t channel);
uint8_t control_bits(uint8_t channel);
int readadc(int fd, uint8_t channel);

void* gas(){
	int state;
	while(1){
		state = GPIORead(GAS);
    if(state == 0)
        printf("gas bad\n");
    if(state == 1)
        printf("gas good\n");
	gasgap = state;
	sleep(1);
	}
}

void* munji(){
	int light;
	float state, density;
	
    int fd = open(DEVICE, O_RDWR);
    prepare(fd);
    
	while(1){
		GPIOWrite(MUNJI,0);
		usleep(280);
		light = readadc(fd, 0);
		usleep(40);
		GPIOWrite(MUNJI,1);
		usleep(9680);
		state = light * 5.0 / 1023.0;
		density = (state-0.3)/0.05;
		munjigap = light/10;
		printf("munji : %d density : %.2f\n",light, density);
		sleep(1);
	}
}

void* temp(){
	
	wiringPiSetup();
	int data[5] = { 0, 0, 0, 0, 0 };
	
	while(1){
	uint8_t laststate    = HIGH;
    uint8_t counter        = 0;
    uint8_t j            = 0, i;
 
    data[0] = data[1] = data[2] = data[3] = data[4] = 0;
 
    /* pull pin down for 18 milliseconds */
    pinMode(SUPDO, OUTPUT );
    digitalWrite( SUPDO, LOW );
    delay( 18 );
 
    /* prepare to read the pin */
    pinMode( SUPDO, INPUT );
 
    /* detect change and read data */
    for ( i = 0; i < MAX_TIMINGS; i++ )
    {
        counter = 0;
        while ( digitalRead( SUPDO ) == laststate )
        {
            counter++;
            delayMicroseconds( 1 );
            if ( counter == 255 )
            {
                break;
            }
        }
        laststate = digitalRead( SUPDO );
 
        if ( counter == 255 )
            break;
 
        /* ignore first 3 transitions */
        if ( (i >= 4) && (i % 2 == 0) )
        {
            /* shove each bit into the storage bytes */
            data[j / 8] <<= 1;
            if ( counter > 26 )
                data[j / 8] |= 1;
            j++;
            //printf("%d %d\n",counter,j);
        }
    }
    printf("state : %d\n",laststate);
 
    /*
     * check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
     * print it out if data is good
     */
    //printf("%d %d %d %d %d",data[0],data[1],data[2],data[3],data[4]);
    if ( (j >= 40) &&
         (data[4] == ( (data[0] + data[1] + data[2] + data[3]) & 0xFF) ) )
    {
        float h = (float)((data[0] << 8) + data[1]) / 10;
        if ( h > 100 )
        {
            h = data[0];    // for DHT11
        }
        float c = (float)(((data[2] & 0x7F) << 8) + data[3]) / 10;
        if ( c > 125 )
        {
            c = data[2];    // for DHT11
        }
        if ( data[2] & 0x80 )
        {
            c = -c;
        }
        float f = c * 1.8f + 32;
        printf( "Humidity = %.1f %% Temperature = %.1f *C (%.1f *F)\n", h, c, f );
    }else  {
        printf( "Data not good, skip\n" );
    }
    
    supdogap = data[0];
    sleep(1);
}
}

int main(int argc, char* argv[]){
	int state = 1;
    int prev_state = 1;
    int light = 0;
    char buf[256];

	pthread_t p_thread[3];
    int thr_id;
    void *thread_status[3];
    int join;
    thr_id = pthread_create(&p_thread[0], NULL, gas, (void*)buf);
    thr_id = pthread_create(&p_thread[1], NULL, munji, (void*)buf);
    thr_id = pthread_create(&p_thread[2], NULL, temp, (void*)buf);
    
    int serv_sock,clnt_sock=-1;
    struct sockaddr_in serv_addr,clnt_addr;
    socklen_t clnt_addr_size;
    char msg[2];
    
    //Enable GPIO pins
    if (-1 == GPIOExport(GAS) || -1 == GPIOExport(MUNJI) || -1 == GPIOExport(SUPDO))
        return(1);

    //Set GPIO directions
    if (-1 == GPIODirection(GAS, IN) || -1 == GPIODirection(MUNJI,OUT))
        return(2);
    
    if(argc!=2){
        printf("Usage : %s <port>\n",argv[0]);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1)
        error_handling("socket() error");
        
    memset(&serv_addr, 0 , sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
    
    if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1)
        error_handling("bind() error");

if(listen(serv_sock,5) == -1)
            error_handling("listen() error");

if(clnt_sock<0){           
            clnt_addr_size = sizeof(clnt_addr);
            clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, 	&clnt_addr_size);
            if(clnt_sock == -1)
                error_handling("accept() error");   
        }
while(1) {
		char changeint[20];
		sprintf(buffer, "%d ",gasgap);
		sprintf(changeint, "%d ",munjigap);
		strcat(buffer, changeint);
		sprintf(changeint, "%d", supdogap);
		strcat(buffer, changeint);
	           
		write(clnt_sock, buffer, sizeof(buffer));
		printf("buffer = %s\n",buffer);
		sleep(1);
	}
    join = pthread_join(p_thread[0], &thread_status[0]);
    join = pthread_join(p_thread[1], &thread_status[1]);
    join = pthread_join(p_thread[2], &thread_status[2]);

    close(clnt_sock);
    close(serv_sock);
    
    //Disable GPIO pins
    if (-1 == GPIOUnexport(MUNJI) || -1 == GPIOUnexport(GAS) || -1 == GPIOUnexport(SUPDO))
        return(4);

    return(0);
}

static int GPIOExport(int pin){
#define BUFFER_MAX 3
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;

    fd = open("/sys/class/gpio/export", O_WRONLY);
    if(-1 == fd){
        fprintf(stderr, "Failed to open export for writing!\n");
        return(-1);
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd);
    return(0);
}

static int GPIODirection(int pin, int dir){
    static const char s_directions_str[] = "in\0out";

#define DIRECTION_MAX 35
     char path[DIRECTION_MAX]="/sys/class/gpio/gpio%d/direction";
     int fd;

     snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);

     fd = open(path, O_WRONLY);
     if(-1 == fd){
        fprintf(stderr, "Failed to open gpio direction for writing!\n");
        return(-1);
     }

     if(-1 == write(fd, &s_directions_str[IN == dir?0:3], IN == dir ? 2:3)){
        fprintf(stderr, "Failed to set direction!\n");
        close(fd);
        return(-1);
     }
     close(fd);
     return(0);
}

static int GPIORead(int pin){
    char path[VALUE_MAX];
    char value_str[3];
    int fd;

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_RDONLY);
    if(-1 == fd){
        fprintf(stderr,"Failed to open gpio value for reading!\n");
        return(-1);
    }

    if(-1 == read(fd, value_str,3)){
        fprintf(stderr, "Failed to read value!\n");
        close(fd);
        return(-1);
    }
    close(fd);
    return(atoi(value_str));
}

static int GPIOWrite(int pin, int value){
    static const char s_values_str[] = "01";

    char path[VALUE_MAX];
    int fd;

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value",pin);
    fd = open(path, O_WRONLY);
    if(-1 == fd){
        fprintf(stderr, "Failed to open gpio value for writing!\n");
        return(-1);
    }

    if(1!=write(fd, &s_values_str[LOW ==value ? 0:1],1)){
        fprintf(stderr, "Failed to write value!\n");
        close(fd);
        return(-1);
    }
    close(fd);
    return(0);
}

static int GPIOUnexport(int pin){
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if(-1 == fd){
        fprintf(stderr, "Failed to open unexport for writing!\n");
        return(-1);
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd);
    return(0);
}

static int prepare(int fd){
    if(ioctl(fd, SPI_IOC_WR_MODE, &MODE) == -1){
        perror("Can't set MODE!");
        return -1;
    }
    
    if(ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &BITS)==-1){
        perror("Can't set number of BITS");
        return -1;
    }
    
    if(ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &CLOCK) == -1){
        perror("Can't set write CLOCK");
        return -1;
    }
    
    if(ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &CLOCK)==-1){
        perror("Can't set read CLOCK");
        return -1;
    }
    
    return 0;

}

uint8_t control_bits_differential(uint8_t channel){
    return (channel&7)<<4;
}

uint8_t control_bits(uint8_t channel){
    return 0x8 | control_bits_differential(channel);
}

int readadc(int fd, uint8_t channel){
    uint8_t tx[] = {1, control_bits(channel),0};
    uint8_t rx[3];
    
    struct spi_ioc_transfer tr ={
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = ARRAY_SIZE(tx),
        .delay_usecs = DELAY,
        .speed_hz = CLOCK,
        .bits_per_word = BITS,
    };
    
    if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr)==1){
        perror("IO Error");
        abort();
    }
    
    return ((rx[1] << 8 ) & 0x300) | (rx[2] & 0xFF);
}
