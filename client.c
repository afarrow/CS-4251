#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

/* 
Client requirements:
1. Interactive (command line input)
2. Responds to time and weather queries
3. Connects to the server to get queries
4. Can request log of interactions
5. Prints out responses from server
*/

/*Macros
prints out the error and exits the program */
#define handle_error(msg) \
  do { perror(msg); exit(EXIT_FAILURE); } while(0)
#define DEF_PORT 30000
#define DEF_IP "127.0.0.1"
#define MAXLEN 5000


/*Function prototypes*/
int setup_socket();
void send_request(int sockfd, char* msg);
void send_message(int sockfd, char* msg);


int main(int argc, char *argv[]) {

    char quit[] = "q\n";
    char str[100];
    char temp[100];
    char req[110];

    printf("Welcome to the Internet Time and Weather Service software package. ");
    int sockfd = setup_socket();

    while(strcmp(str, quit) != 0) {
    
        printf("Please enter a command: \n");
        printf("Press 1 to request the current time in a location\n");
        printf("Press 2 to request the current weather in a location\n");
        printf("Press 3 to request the current time and weather in a location\n");
        printf("Type \"log\" to request a log of all the requests sent to the server\n");
        printf("Press q to exit the program\n");

        memset(str, 0, sizeof(str));
        fgets(str, 100, stdin);
        memset(req, 0, sizeof(req));

        if(strcmp(str, "1\n") == 0) {
            printf("Please enter the location:");
            fgets(str, 100, stdin);
            strcpy(req, "time ");
            strcat(req, str);
            printf("Requesting current time in %s", str);
            send_request(sockfd, req);

        } else if(strcmp(str, "2\n") == 0) {
            printf("Please enter the location:");
            fgets(str, 100, stdin);
            strcpy(req, "weather ");
            strcat(req, str);
            printf("Requesting the current weather in %s", str);
            send_request(sockfd, req);
            
        } else if(strcmp(str, "3\n") == 0) {
            printf("Please enter the location:");
            fgets(str, 100, stdin);
            strcpy(req, "timeweather ");
            strcat(req, str);
            printf("Requesting the current time and weather in %s", str);
            send_request(sockfd, req);
            
        } else if(strcmp(str, "log\n") == 0) {
            printf("Requesting the server log...\n");
            strcpy(req, "log");
            send_request(sockfd, req);

        } else if(strcmp(str, quit) == 0){
            /*do nothing because they are trying to quit the program*/
        } else {
            printf("Invalid input, try again\n");
        }
    }
    printf("Exiting program\n");

    /*Sending a message to the server letting it
        know that the client is done */
    strcpy(req, "close");
    send_message(sockfd, req);

    if(close(sockfd)!=0)
        handle_error("close()");
    
    return 0;
}

int setup_socket() {

    /*create struct to hold address info*/
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DEF_PORT);
    if(inet_aton(DEF_IP, &addr.sin_addr) == 0) 
        handle_error("inet_aton()");

    /*create the socket*/
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    /*error if socket() returns less than zero*/
    if(sockfd<0)
        handle_error("socket()");

    if(connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) <0)
        handle_error("connect()");


    return sockfd;
}

/* Sends a request to the server and then
    waits and prints out the response */
void send_request(int sockfd, char* msg) {
    int i;
    int count, total = 0;
    char buffer[MAXLEN];

    /*send the request to the server*/
    if(send(sockfd, &msg[0], strlen(msg), 0) <0)
        handle_error("send() in send_request()");

    /*receive the request from the server and print it out
    do while server continues to send*/
    i = recv(sockfd, &buffer[0], MAXLEN,0);

    if(i==-1) {
        handle_error("recv()");
    } else {
        /* printing out what was received from the server */
        fwrite((void*)buffer, sizeof(char),i, stdout);
    }
    printf("\n");
}

/* Sends a message to the server without expecting
    any response */
void send_message(int sockfd, char* msg) {
    if(send(sockfd, &msg[0], strlen(msg), 0) < 0) 
        handle_error("send() in send_message()");
}