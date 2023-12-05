#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_MAX 3
#define DIRECTION_MAX 35

#define IN 0
#define OUT 1
#define PWM 0

#define LOW 0
#define HIGH 1
#define VALUE_MAX 256

#define POUT 4
#define PIN 24

//실내모터
#define motor1_POUT1 6
#define motor1_POUT2 13
#define motor1_POUT3 19
#define motor1_POUT4 26

//실외모터
#define motor2_POUT1 4
#define motor2_POUT2 17
#define motor2_POUT3 27
#define motor2_POUT4 22

//실내LED
#define led1_good_POUT 5
#define led1_normal_POUT 18
#define led1_bad_POUT 23
#define led1_terrible_POUT 24

//실외LED
#define led2_good_POUT 12
#define led2_normal_POUT 16
#define led2_bad_POUT 20
#define led2_terrible_POUT 21

#define led_gas_POUT 25

#define INNER_SERVER 1
#define OUTTER_SERVER 2
#define ACTUATOR 3


/**
 * 실내 센서 데이터
 * 미세먼지 int(0~)
 * 습도 int(0~)
 * 가스 int(0 or 1)
*/
typedef struct {
    int dust_data;
    int humidity;
    int gas;
}inside_data;

/**
 * 실외 센서 데이터
 * 미세먼지 int(0~)
 * 우천여부 int(0 or 1)
 * 
*/
typedef struct {
    int dust_data;
    int rain;
}outside_data;

void error_handling(char *message){
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}

static int GPIOExport(int pin){
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;

    fd = open("/sys/class/gpio/export",O_WRONLY);
    if(-1 == fd){
        fprintf(stderr, "Failed to open export for writing!\n");
        return(-1);
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written);
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

static int GPIODirection(int pin, int dir){
    static const char s_directions_str[] = "in\0out";
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

/**
 * 실내 모터
*/
void motor1bang(int step){
    if(step == 0) {
        GPIOWrite(motor1_POUT1,HIGH);
        GPIOWrite(motor1_POUT2,LOW);
        GPIOWrite(motor1_POUT3,LOW);
        GPIOWrite(motor1_POUT4,LOW);
    }
    if(step == 1) {
        GPIOWrite(motor1_POUT1,LOW);
        GPIOWrite(motor1_POUT2,HIGH);
        GPIOWrite(motor1_POUT3,LOW);
        GPIOWrite(motor1_POUT4,LOW);
    }
    if(step == 2) {
        GPIOWrite(motor1_POUT1,LOW);
        GPIOWrite(motor1_POUT2,LOW);
        GPIOWrite(motor1_POUT3,HIGH);
        GPIOWrite(motor1_POUT4,LOW);
    }
    if(step == 3) {
        GPIOWrite(motor1_POUT1,LOW);
        GPIOWrite(motor1_POUT2,LOW);
        GPIOWrite(motor1_POUT3,LOW);
        GPIOWrite(motor1_POUT4,HIGH);
    }
}

void motor2bang(int step){
    if(step == 0) {
        GPIOWrite(motor2_POUT1,HIGH);
        GPIOWrite(motor2_POUT2,LOW);
        GPIOWrite(motor2_POUT3,LOW);
        GPIOWrite(motor2_POUT4,LOW);
    }
    if(step == 1) {
        GPIOWrite(motor2_POUT1,LOW);
        GPIOWrite(motor2_POUT2,HIGH);
        GPIOWrite(motor2_POUT3,LOW);
        GPIOWrite(motor2_POUT4,LOW);
    }
    if(step == 2) {
        GPIOWrite(motor2_POUT1,LOW);
        GPIOWrite(motor2_POUT2,LOW);
        GPIOWrite(motor2_POUT3,HIGH);
        GPIOWrite(motor2_POUT4,LOW);
    }
    if(step == 3) {
        GPIOWrite(motor2_POUT1,LOW);
        GPIOWrite(motor2_POUT2,LOW);
        GPIOWrite(motor2_POUT3,LOW);
        GPIOWrite(motor2_POUT4,HIGH);
    }
}

/**
 * inside, outside data 전역변수로 선언하여
 * 각 쓰레드간에 공유할 수 있도록d
*/

int sock1, sock2;
struct sockaddr_in serv_addr1, serv_addr2;


inside_data inner_data;
outside_data outter_data;

//쓰레드 함수
void *t_function(void *data) {

    int id = *((int *)data);

    if(id == INNER_SERVER) {
        if(connect(sock1, (struct sockaddr*)&serv_addr1, sizeof(serv_addr1))==-1)
            error_handling("error occurred when connecting INNER server\n");

        char msg[10];
        int str_len;

        while(1) {
            str_len = read(sock1, msg, sizeof(msg));
            printf("inside server msg: %s\n", msg);

            /**
             * 버퍼 문자열 파싱
            */
            char *ptr = strtok(msg, " ");
            int cnt = 0;
            while(ptr != NULL) {
                if(cnt == 0) {
                    inner_data.gas = atoi(ptr);

                }
                if(cnt == 1) {
                    inner_data.dust_data = atoi(ptr);

                }
                if(cnt == 2) {
                    inner_data.humidity = atoi(ptr);

                }

                ptr = strtok(NULL, " ");
                cnt++;
            }

            printf("inner_gas: %d, inner_dust: %d, inner_humidity: %d\n", inner_data.gas, inner_data.dust_data, inner_data.humidity);

            if(str_len == -1)
                error_handling("read() error");

            sleep(1);
        }

        //update inner data
    }

    if(id == OUTTER_SERVER) {
        if(connect(sock2, (struct sockaddr*)&serv_addr2, sizeof(serv_addr2))==-1)
            error_handling("error occurred when connecting OUTTER server\n");

        char msg[10] = "1 200";
        int str_len;

        while(1) {
            str_len = read(sock2, msg, sizeof(msg));
            
            printf("outside server msg: %s\n", msg);

            /**
             * 버퍼 문자열 파싱
            */
            char *ptr = strtok(msg, " ");
            int cnt = 0;
            while(ptr != NULL) {
                if(cnt == 0) {
                    outter_data.rain = atoi(ptr);

                }
                if(cnt == 1) {
                    outter_data.dust_data = atoi(ptr);

                }
                ptr = strtok(NULL, " ");
                cnt++;
            }

            printf("out_rain: %d, out_dust : %d\n", outter_data.rain, outter_data.dust_data);

            if(str_len == -1)
                error_handling("read() error");

            sleep(1);
        }
        
        //update outter data
    }

    if(id == ACTUATOR) {

        //5초에 한번씩 inner, outter data 확인하여 조건에 맞게 actuator 작동
        while(1) {
            int outter_dust = outter_data.dust_data;
            int outter_rain = outter_data.rain;

            int inner_dust = inner_data.dust_data;
            int inner_gas = inner_data.gas;
            int inner_humidity = inner_data.humidity;


            /**
             * 실내 LED
            */
            if(inner_gas == 0) {
                GPIOWrite(led_gas_POUT, 1);
            } else{
                GPIOWrite(led_gas_POUT, 0);
            }
            if(inner_dust >= 0 && inner_dust < 51) {
                //청색
                GPIOWrite(led1_good_POUT, 1);
                GPIOWrite(led1_normal_POUT, 0);
                GPIOWrite(led1_bad_POUT, 0);
                GPIOWrite(led1_terrible_POUT, 0);
                
            }
            else if(inner_dust > 50 && inner_dust < 101) {
                //녹색
                GPIOWrite(led1_good_POUT, 0);
                GPIOWrite(led1_normal_POUT, 1);
                GPIOWrite(led1_bad_POUT, 0);
                GPIOWrite(led1_terrible_POUT, 0);
            }
            else if(inner_dust > 100 && inner_dust < 151) {
                //황색
                GPIOWrite(led1_good_POUT, 0);
                GPIOWrite(led1_normal_POUT, 0);
                GPIOWrite(led1_bad_POUT, 1);
                GPIOWrite(led1_terrible_POUT, 0);
            }
            else if(inner_dust > 150) {
                //적색
                GPIOWrite(led1_good_POUT, 0);
                GPIOWrite(led1_normal_POUT, 0);
                GPIOWrite(led1_bad_POUT, 0);
                GPIOWrite(led1_terrible_POUT, 1);
            }


            /**
             * 실외 LED
            */
            if(outter_dust >= 0 && outter_dust < 51) {
                //청색
                GPIOWrite(led2_good_POUT, 1);
                GPIOWrite(led2_normal_POUT, 0);
                GPIOWrite(led2_bad_POUT, 0);
                GPIOWrite(led2_terrible_POUT, 0);
                
            }
            else if(outter_dust > 50 && outter_dust < 101) {
                //녹색
                GPIOWrite(led2_good_POUT, 0);
                GPIOWrite(led2_normal_POUT, 1);
                GPIOWrite(led2_bad_POUT, 0);
                GPIOWrite(led2_terrible_POUT, 0);
                
            }
            else if(outter_dust > 100 && outter_dust < 151) {
                //황색
                GPIOWrite(led2_good_POUT, 0);
                GPIOWrite(led2_normal_POUT, 0);
                GPIOWrite(led2_bad_POUT, 1);
                GPIOWrite(led2_terrible_POUT, 0);

            }
            else if(outter_dust > 150) {
                //적색
                GPIOWrite(led2_good_POUT, 0);
                GPIOWrite(led2_normal_POUT, 0);
                GPIOWrite(led2_bad_POUT, 0);
                GPIOWrite(led2_terrible_POUT, 1);

            }


            /**
             * 모터 작동
            */
            if(inner_gas == 0) {
                printf("gas detected -> outside circulation\n");
                    GPIOWrite(motor1_POUT1,LOW);
                    GPIOWrite(motor1_POUT2,LOW);
                    GPIOWrite(motor1_POUT3,LOW);
                    GPIOWrite(motor1_POUT4,LOW);

                    for(int i = 0; i < 2500; i++) {
                        motor2bang(i % 4);
                        usleep(2000);
                    }
            }
            else{
                if(outter_dust > inner_dust) {
                printf("outer_dust > inner_dust\n");
                if(inner_humidity > 60) {
                    //외기순환
                    printf("humidity high -> outside circulation\n");
                    GPIOWrite(motor1_POUT1,LOW);
                    GPIOWrite(motor1_POUT2,LOW);
                    GPIOWrite(motor1_POUT3,LOW);
                    GPIOWrite(motor1_POUT4,LOW);

                    for(int i = 0; i < 2500; i++) {
                        motor2bang(i % 4);
                        usleep(2000);
                    }
                }
                else {
                    //내기순환
                    printf("humidity low -> inside circulation\n");

                    
                    //motor1: 실내, motor2: 실외

                    GPIOWrite(motor2_POUT1,LOW);
                    GPIOWrite(motor2_POUT2,LOW);
                    GPIOWrite(motor2_POUT3,LOW);
                    GPIOWrite(motor2_POUT4,LOW);

                    for(int i = 0; i < 2500; i++) {
                        motor1bang(i % 4);
                        usleep(2000);
                    }
                        

                }
            }
            else {
                printf("outer_dust < inner_dust\n");
                if(inner_humidity > 60) {
                    //외기순환
                    printf("humidity high -> inside circulation\n");
                    
                    
                    GPIOWrite(motor1_POUT1,LOW);
                    GPIOWrite(motor1_POUT2,LOW);
                    GPIOWrite(motor1_POUT3,LOW);
                    GPIOWrite(motor1_POUT4,LOW);

                    for(int i = 0; i < 2500; i++) {
                        motor2bang(i % 4);
                        usleep(2000);
                    }
                }
                else {
                    //외기순환
                    printf("humidity low-> outside circulation\n");
                    GPIOWrite(motor2_POUT1,LOW);
                    GPIOWrite(motor2_POUT2,LOW);
                    GPIOWrite(motor2_POUT3,LOW);
                    GPIOWrite(motor2_POUT4,LOW);

                    for(int i = 0; i < 2500; i++) {
                        motor1bang(i % 4);
                        usleep(2000);
                    }
                }

            }
            printf("==========================\n");
            }
            printf("==========================\n");
        }

    }
}

int main(int argc, char *argv[]) {
    
    /**
     * 서버에 연결 요청
     * 쓰레드 1: 실내 서버 데이터 수신
     * 쓰레드 2: 실외 서버 데이터 수신
     * 쓰레드 3: 데이터 조건 확인하여 액추에이터 가동
     *          데이터는 전역 변수로 관리하여 각 쓰레드로부터 접근 가능하도록 처리.
     * 
    
     * 
     * 부모 프로세스에서는 실내 서버와 연결, 
     * 자식 프로세스에서는 실외 서버와 연결,
     * 각 프로세스별로 서버와 연결 수립 후, 데이터 받아오기.
     * 데이터 전처리 후, 
    */
    if(-1 == GPIOExport(motor1_POUT1) || -1 == GPIOExport(motor1_POUT2) || -1 == GPIOExport(motor1_POUT3) || -1 == GPIOExport(motor1_POUT4) ||
    -1 == GPIOExport(motor2_POUT1) || -1 == GPIOExport(motor2_POUT2) || -1 == GPIOExport(motor2_POUT3) || -1 == GPIOExport(motor2_POUT4) ||
    -1 == GPIOExport(led1_good_POUT) || -1 == GPIOExport(led1_normal_POUT) || -1 == GPIOExport(led1_bad_POUT) || -1 == GPIOExport(led1_terrible_POUT) ||
    -1 == GPIOExport(led2_good_POUT) || -1 == GPIOExport(led2_normal_POUT) || -1 == GPIOExport(led2_bad_POUT) || -1 == GPIOExport(led2_terrible_POUT) || -1 == GPIOExport(led_gas_POUT)
    )
        return -1;

    if(-1 == GPIODirection(motor1_POUT1, OUT) || -1 == GPIODirection(motor1_POUT2, OUT) || -1 == GPIODirection(motor1_POUT3, OUT) || -1 == GPIODirection(motor1_POUT4, OUT) ||
    -1 == GPIODirection(motor2_POUT1, OUT) || -1 == GPIODirection(motor2_POUT2, OUT) || -1 == GPIODirection(motor2_POUT3, OUT) || -1 == GPIODirection(motor2_POUT4, OUT) ||
    -1 == GPIODirection(led1_good_POUT, OUT) || -1 == GPIODirection(led1_normal_POUT, OUT) || -1 == GPIODirection(led1_bad_POUT, OUT) || -1 == GPIODirection(led1_terrible_POUT, OUT) ||
    -1 == GPIODirection(led2_good_POUT, OUT) || -1 == GPIODirection(led2_normal_POUT, OUT) || -1 == GPIODirection(led2_bad_POUT, OUT) || -1 == GPIODirection(led2_terrible_POUT, OUT) || -1 == GPIODirection(led_gas_POUT, OUT)
    )
        return -1;

    /**
     * for thread
    */
    pthread_t p_thread[3];
    int thr_id;
    int a = INNER_SERVER;
    int b = OUTTER_SERVER;
    int c = ACTUATOR;

    /**
     * for server
    */

    if(argc!=5){
        printf("Usage : %s <INNER_SERVER_IP> <PORT> <OUTTER_SRVER_IP> <PORT>\n",argv[0]);
        exit(1);
    }

    sock1 = socket(PF_INET, SOCK_STREAM, 0);
    sock2 = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_addr1, 0, sizeof(serv_addr1));
    serv_addr1.sin_family = AF_INET;
    serv_addr1.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr1.sin_port = htons(atoi(argv[2]));

    memset(&serv_addr2, 0, sizeof(serv_addr2));
    serv_addr2.sin_family = AF_INET;
    serv_addr2.sin_addr.s_addr = inet_addr(argv[3]);
    serv_addr2.sin_port = htons(atoi(argv[4]));

    //실내 서버 쓰레드
    thr_id = pthread_create(&p_thread[0], NULL, t_function, (void *)&a);
    if (thr_id < 0) {
        perror("thread1 create error : ");
        exit(0);
    }

    //실외 서버 쓰레드
    thr_id = pthread_create(&p_thread[1], NULL, t_function, (void *)&b);
    if (thr_id < 0) {
        perror("thread2 create error : ");
        exit(0);
    }

    //액추에이터 쓰레드
    thr_id = pthread_create(&p_thread[2], NULL, t_function, (void *)&c);
    if (thr_id < 0) {
        perror("thread3 create error : ");
        exit(0);
    }
    
    while(1) {
        sleep(5);
    }

    if(-1==GPIOUnexport(motor1_POUT1) || -1 == GPIOUnexport(motor1_POUT2) || -1 == GPIOUnexport(motor1_POUT3) || -1 == GPIOUnexport(motor1_POUT4) ||
    -1==GPIOUnexport(motor2_POUT1)|| -1 == GPIOUnexport(motor2_POUT2) || -1 == GPIOUnexport(motor2_POUT3) || -1 == GPIOUnexport(motor2_POUT4) ||
    -1==GPIOUnexport(led1_good_POUT) || -1==GPIOUnexport(led1_normal_POUT) || -1==GPIOUnexport(led1_bad_POUT) || -1==GPIOUnexport(led1_terrible_POUT) || 
    -1==GPIOUnexport(led2_good_POUT) || -1==GPIOUnexport(led2_normal_POUT) || -1==GPIOUnexport(led2_bad_POUT) || -1==GPIOUnexport(led2_terrible_POUT) || -1 == GPIOUnexport(led_gas_POUT)
    )
        return -1;

    return 0;
}
