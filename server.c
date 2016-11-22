#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h> /*close*/

/* 
Client requirements:
1. Connects to the client to get queries
2. Keep a log of all the queries made (and who made them)
3. Randomly generate time (using system time) and weather
4. New thread each time someone tries to connect
*/

/*Macros*/
#define handle_error(msg) \
  do { perror(msg); exit(EXIT_FAILURE); } while(0)
#define MAXLEN 5000
#define DEF_PORT 30000
#define LISTEN_BACKLOG 100
#define NUM_THREADS 100
#define BUFFER_LENGTH 110
#define TIME_LEN 20
#define WEATHER_LEN 100

typedef struct Connection{
    int acc_sockfd;
    int conn_num;
} conn_info;

/*Function Prototypes*/
int setup_socket();
void* new_connection(void *conn_attrib);
void add_to_log(char* insert);
void get_weather(char* weather);
void get_time(char* time);


/*Globals*/
char request_log[MAXLEN];
pthread_mutex_t insert_log_mutex;
conn_info threads_data_array[NUM_THREADS];



int main(int argc, char *argv[]) {
    struct sockaddr_in cli_addr;
    socklen_t cli_len;
    int sockfd = setup_socket();
    int acc_sockfd, rc;
    int conn_num = 0;
    pthread_t threads[NUM_THREADS];
    /*Initialzing random number generator*/
    srand(time(NULL));

    pthread_mutex_init(&insert_log_mutex, NULL);

    /*Loop to accept new connections*/
    while(1 && conn_num<NUM_THREADS) {
        acc_sockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &cli_len);

        /* Loading info into struct that will be passed to thread */
        threads_data_array[conn_num].acc_sockfd = acc_sockfd;
        threads_data_array[conn_num].conn_num = conn_num;

        /* Creating the new thread that will handle the connection */
        rc = pthread_create(&threads[conn_num], NULL, new_connection, (void*)&threads_data_array[conn_num]);
        if(rc)
            handle_error("pthread_create()");

        conn_num++;
    }
}

/*Sets up the socket to be ready to accept incoming connections*/
int setup_socket() {
    int sockfd;

    struct sockaddr_in svr_addr;
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_port = htons(DEF_PORT);


    /*Create the socket for receiving connections */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd<0)
        handle_error("socket()");

    /* Bind the socket that was just created */
    if(bind(sockfd, (struct sockaddr *) &svr_addr, sizeof(svr_addr)) < 0)
        handle_error("bind()");

    /*Mark the socket as a passive socket*/
    if(listen(sockfd, LISTEN_BACKLOG)<0)
        handle_error("listen()");

    return sockfd;
}

/* Handles a connection with the client */
void* new_connection(void *conn_attrib) {

    /* Passing the connection info into the method */
    conn_info *info;

    int n;
    char buffer[BUFFER_LENGTH];

    info = (conn_info *)conn_attrib;
    

    while(1) {
        /*Zero out the buffer from previous transmissions */
        memset(&buffer, 0, BUFFER_LENGTH);

        /* Receive the message from the client*/
        n = recv(info->acc_sockfd, (void *)&buffer, BUFFER_LENGTH-1, 0);
        if(n<0)
            handle_error("recv()");
        
        /* Parse request to see what to do next: Since in each messsage
        sent from the client to the server there is whitespace or a terminating 
        character in a unique location between the request type and location, 
        I can just check where it is to discern the request type */
        if(buffer[3]=='\0') { /*log request*/
            /*Send the log to the client*/
            if(send(info->acc_sockfd, &request_log[0], strlen(request_log), 0)<0)
                handle_error("send() log");

        } else if(buffer[4]==' ') { /*time request*/
            /*Get the location from the request*/
            int i = 5;
            while(buffer[i]!='\0') {
                i++;
            }
            char location[i-4];
            /*Copying the location from the buffer*/
            memcpy(location, &buffer[5], i-4);

            /*Generate and send the response */
            char curr_time[TIME_LEN];
            get_time(curr_time);
            char response[200];
            snprintf(response, sizeof(response), "The current time in %s is %s\n", location, curr_time);

            /*Send response to client*/
            if(send(info->acc_sockfd, &response[0], strlen(response), 0)<0)
                handle_error("send() time response");

            /*Write the request to the log*/
            char new_entry[200];
            snprintf(new_entry, sizeof(new_entry), "User %d requested time in %s at %s\n", info->conn_num, location, curr_time);

            add_to_log(new_entry);

        } else if(buffer[5]=='\0') { /*close connection request*/
            /*Break loop to close connection & thread*/
            break;
        } else if(buffer[7]==' ') { /*weather request*/
            /*Get the location of the request*/
            int i = 8;
            while(buffer[i]!='\0') {
                i++;
            }
            char location[i-7];
            /*Copying the location from the buffer*/
            memcpy(location, &buffer[8], i-7);

            /*Creating response*/
            char curr_weather[WEATHER_LEN];
            get_weather(curr_weather);
            char response[200];
            snprintf(response, sizeof(response), "The curent weather in %s is %s\n", location, curr_weather);

            /*Send response to client*/
            if(send(info->acc_sockfd, &response[0], strlen(response), 0)<0)
                handle_error("send() weather response");

            /*Write the new request to the log*/
            char new_entry[200];
            char curr_time[TIME_LEN];
            get_time(curr_time);
            snprintf(new_entry, sizeof(new_entry), "User %d requested weather in %s at %s\n", info->conn_num, location, curr_time);

            add_to_log(new_entry);

        } else if(buffer[11]==' ') { /*time & weather request*/
            /*Get the location of the request*/
            int i = 12;
            while(buffer[i]!='\0') {
                i++;
            }
            char location[i-11];
            /*Copying the location from the buffer*/
            memcpy(location, &buffer[12], i-11);

            /*Creating response*/
            char curr_weather[WEATHER_LEN];
            get_weather(curr_weather);
            char response[200];
            char curr_time[TIME_LEN];
            get_time(curr_time);
            snprintf(response, sizeof(response), "The curent time in %s is %s and the weather is %s\n", location, curr_time,curr_weather);

            /*Send resposne to client*/
            if(send(info->acc_sockfd, &response[0], strlen(response), 0)<0)
                handle_error("send() weather response");

            /*Write the new request to the log*/
            char new_entry[200];
            snprintf(new_entry, sizeof(new_entry), "User %d requested time & weather in %s at %s\n", info->conn_num, location, curr_time);

            add_to_log(new_entry);
        } else { /*Something went wrong*/
            printf("There was an error parsing the request\n");
            exit(EXIT_FAILURE);
        }
    }
    
    close(info->acc_sockfd);
    pthread_exit(NULL);
}

/*Adds the string passed in to the log of requests*/
void add_to_log(char* insert) {
    pthread_mutex_lock(&insert_log_mutex);

    strcat(request_log, insert);

    pthread_mutex_unlock(&insert_log_mutex);
}

/*Randomly generates a weather report*/
void get_weather(char* weather) {

    /*Generating the temperature*/
    int degrees = rand() % 100;

    /*Generating weather conditions*/
    int i = rand() % 8;
    char* rand_weather;
    switch(i) {
        case 0:
            rand_weather = "cloudy";
            break;
        case 1:
            rand_weather = "sunny";
            break;
        case 2:
            rand_weather = "overcast";
            break;
        case 3:
            rand_weather = "partly cloudy";
            break;
        case 4:
            if(degrees<32)
                rand_weather = "snowing";
            else
                rand_weather = "raining";
            break;
        case 5:
            rand_weather = "windy and clear";
            break;
        case 6:
            rand_weather = "smoggy";
            break;
        default:
            if(degrees<32)
                rand_weather = "blizzard-like conditions";
            else
                rand_weather = "heavy rain";
            break;
    }
    snprintf(weather, WEATHER_LEN, "%d°F and %s",degrees, rand_weather);
}

/* Gets the current system time in GMT */
void get_time(char* curr_time) {
  time_t t = time(NULL);
  struct tm tm = *gmtime(&t);

  snprintf(curr_time, TIME_LEN, "%d:%d:%d GMT", tm.tm_hour, tm.tm_min, tm.tm_sec);
}