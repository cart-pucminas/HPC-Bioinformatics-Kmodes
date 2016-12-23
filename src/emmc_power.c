/*
 * Copyright(C) 2016 Matheus Alcântara Souza <matheusalcantarasouza@gmail.com>
 *
 * DMI-T50 Electric Multimeter Crawler
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h> /* getprotobyname */
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "power.h"
#include <pthread.h>


#define MAX_REQUEST_LEN 1024

typedef struct {
	double voltage;
	double current;
	double apparentPower;
	double realPower;
	double powerFactor;
	double frequency;
	double reactivePower;
} emmc_data_t;

typedef struct {
	double current;
	double realPower;
	double powerFactor;
	double none; /* To be discovered... */
	double averageVoltage;
} emmc_totals_t;

/* HTTP requests functions
void buildSocket(int *socket_file_descriptor);
void buildAddress(struct sockaddr_in *myaddr);
void connectAndRead(int *socket_file_descriptor, struct sockaddr_in *myaddr,
                      char request[MAX_REQUEST_LEN], int request_len, char *data);
*/
/* Energy data reading functions */
extern void emmc_power_init(int);

extern double emmc_power_end(void);

/* The authentication method is basic. So a encoded base64 string is required.
   This string contains user:password. So if any of them was changed, you'll need
    to re-encode the string, using 'echo -n "user:password" | base64'
*/
static const char *hostname = "10.111.0.20";
static const char *path = "/sistema.cgi?lermc=0,26";
static const char *authToken = "YWRtaW46Q0FSVEBwaWVzITI4NDg3OQ==";
static char *data;

static pthread_t tid;
static const long time_stamp = 100000000L;
static int sensor = 1;
static volatile double avg = 0;
static unsigned count = 0;
static unsigned live;

void buildSocket(int *socket_file_descriptor) {
    struct protoent *protoent;
    protoent = getprotobyname("tcp");
    if (protoent == NULL) {
        perror("getprotobyname");
        exit(EXIT_FAILURE);
    }
    *socket_file_descriptor = socket(AF_INET, SOCK_STREAM, protoent->p_proto);
    if (*socket_file_descriptor == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
}

void buildAddress(struct sockaddr_in *myaddr) {
    struct hostent *hostent;
    in_addr_t in_addr;

    hostent = gethostbyname(hostname);
    if (hostent == NULL) {
        fprintf(stderr, "error: gethostbyname(\"%s\")\n", hostname);
        exit(EXIT_FAILURE);
    }
    in_addr = inet_addr(inet_ntoa(*(struct in_addr*)*(hostent->h_addr_list)));
    if (in_addr == (in_addr_t)-1) {
        fprintf(stderr, "error: inet_addr(\"%s\")\n", *(hostent->h_addr_list));
        exit(EXIT_FAILURE);
    }
    (*myaddr).sin_addr.s_addr = in_addr;
    (*myaddr).sin_family = AF_INET;
    (*myaddr).sin_port = htons(80);
}

void connectAndRead(int *socket_file_descriptor, struct sockaddr_in *myaddr,
                      char request[MAX_REQUEST_LEN], int request_len) {
    int i;
    ssize_t nbytes_total, nbytes_last;
    char buffer[BUFSIZ];
    char response[5 * BUFSIZ] = ""; /* The response should be at most 5 times the buffer */
    char *readData;

    if (connect(*socket_file_descriptor, (struct sockaddr*)myaddr, sizeof(*myaddr)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    /* Send HTTP request. */
    nbytes_total = 0;
    while (nbytes_total < request_len) {
        nbytes_last = write(*socket_file_descriptor, request + nbytes_total, request_len - nbytes_total);
        if (nbytes_last == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }
        nbytes_total += nbytes_last;
    }

    /* Read data */
    i=0;
    while ((nbytes_total = read(*socket_file_descriptor, buffer, BUFSIZ)) > 0) {
        i+=1;
        if(i > 5) {
            perror("Response size is greater than 5 times the buffer. The return must be wrong.");
            exit(EXIT_FAILURE);
        }
        strcat(response, buffer);
        // write(STDOUT_FILENO, buffer, nbytes_total);
    }
    if (nbytes_total == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    } else {
        readData = strstr(response, "[");
        data = malloc(strlen(readData));
        strncpy(data, readData+1, strlen(readData) - 1);
    }
}


static double emmc_power_get(void) {
    int socket_file_descriptor, request_len;

    struct sockaddr_in myaddr;
    emmc_data_t ed[3];

    char request[MAX_REQUEST_LEN];

    request_len = snprintf(request, MAX_REQUEST_LEN,"GET %s HTTP/1.1\r\n"
                                    "Host: %s\r\n"
                                    "Connection: close\r\n"
                                    "Authorization: Basic %s \r\n\r\n",
                                    path, hostname, authToken);
    if (request_len >= MAX_REQUEST_LEN) {
        fprintf(stderr, "request length large: %d\n", request_len);
        exit(EXIT_FAILURE);
    }

    buildSocket(&socket_file_descriptor);
    buildAddress(&myaddr);
    connectAndRead(&socket_file_descriptor, &myaddr, request, request_len);
    close(socket_file_descriptor);

    sscanf(data, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
       &ed[0].voltage,     &ed[0].current,   &ed[0].apparentPower, &ed[0].realPower,
       &ed[0].powerFactor, &ed[0].frequency, &ed[0].reactivePower,

       &ed[1].voltage,     &ed[1].current,   &ed[1].apparentPower, &ed[1].realPower,
       &ed[1].powerFactor, &ed[1].frequency, &ed[1].reactivePower,

       &ed[2].voltage,     &ed[2].current,   &ed[2].apparentPower, &ed[2].realPower,
       &ed[2].powerFactor, &ed[2].frequency, &ed[2].reactivePower);

    return (ed[sensor].realPower);
}

static void *emmc_power_listen(void *unused)
{
    struct timespec ts;
    ((void)unused);
    ts.tv_sec = 0;
    ts.tv_nsec = time_stamp;

    do
    {
        avg += emmc_power_get();
        count++;

        nanosleep(&ts, NULL);
    } while (live);

    avg /= count;

    return (NULL);
}

void power_init()
{
    live = 1;
    sensor = 0;
    pthread_create(&tid, NULL, emmc_power_listen, NULL);
}

/*
 * Terminates power measurement utility.
 */
double power_end(void)
{
    live = 0;
    avg = 0;
    count = 0;
    pthread_join(tid, NULL);
    return (avg);
}
