//
// Created by Sinclair Liang on 9/23/20.
//


#include "httpserver.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ctype.h>
#include <stdbool.h>
#include <pthread.h>

#define BUFFER_SIZE 32768 // size for 32Kib;
#define HEADER_SIZE 4000  // size for 4Kib;
#define DEFAULT_PORT 8080

pthread_t threadpool[4];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex4 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;

struct node
{
    struct node *next;
    int *client_sockd;
};
typedef struct node node_t;

typedef struct args
{
    int reqs;
    int errors;
    uint64_t offset;
    bool logging;
    char *logName;
} inputs;

bool checkValidName(char *name, uint8_t *copy_buff, int index)
{
    bool valid = true;
    if (strlen(name) > 27)
    {
        fprintf(stdout, "%s", "File Name is too long\n");
        valid = false;
    }
    for (int i = 0; i < strlen(name); i++)
    {
        if ((isalnum(copy_buff[index]) == 0) && (copy_buff[index] != '-') && (copy_buff[index] != '_'))
        {
            fprintf(stdout, "%s", "Invalid String in Filename\n");
            valid = false;
        }
        index++;
    }
    return valid;
}

void parseRequest(int *pclient, inputs *globals)
{
    uint8_t buff[BUFFER_SIZE + 1];
    ssize_t bytes = recv(*pclient, buff, BUFFER_SIZE + 1, 0);
    buff[bytes] = 0;
    uint8_t copy_buff[BUFFER_SIZE + 1];
    memcpy(copy_buff, buff, sizeof(buff));
    copy_buff[bytes] = 0;

    char request[10];
    char name[300];
    char version[10];
    sscanf((char *)buff, "%s %s %s", request, name, version);
    // The funny value 0644 is the permission bits of a file and these bits are only used when the file is actually created (does not exist).
    // Using 0644 will create a file that is Read/Write for owner, and Read Only for everyone else...
    int log = open(globals->logName, O_CREAT | O_WRONLY, 0644);

    char get[4] = "GET ";
    char head[5] = "HEAD ";
    char put[4] = "PUT ";

    char *charBuff = (char *)(buff);
    int getReq = strncmp(charBuff, get, 4);
    int putReq = strncmp(charBuff, put, 4);
    int headReq = strncmp(charBuff, head, 5);

    if (getReq == 0)
    {
        fprintf(stdout, "%s", "Get Request\n");

        char *space = strstr((char *)(&copy_buff[5]), " ");
        space[0] = '\0';
        char *fileName;

        //the name of the file provided in the request (the endpoint)
        fileName = (char *)(&copy_buff[5]);

        if (strcmp(fileName, "healthcheck") == 0)
        {
            fprintf(stdout, "%s\n", "dealing with healthcheck in GET");
        }
    }
}

int receiveFile(int inputFileDescriptor, int outputFileDescriptor, int sizeofFile)
{
    unsigned char buffer[BUFFER_SIZE];
    int remainingSize = sizeofFile;
    while (remainingSize > 0)
    {
        memset(buffer, 0, BUFFER_SIZE);
        int readSize = read(inputFileDescriptor, buffer, BUFFER_SIZE);
        if (readSize < 0)
        {
            fprintf(stderr, "%s", "Error while reading file\n");
            return 1;
        }
        write(outputFileDescriptor, buffer, readSize);
        remainingSize -= BUFFER_SIZE;
    }
    close(inputFileDescriptor);
    close(outputFileDescriptor);
    return 0;
}

void *handlingReq(void *args)
{
    while (1)
    {
        inputs *globals = (inputs *)args;
        int *pclient;
        pthread_mutex_lock(&mutex);
    }
}

int main(int argc, char *argv[])
{
    int numThreads = 4;
    inputs args;
    args.errors = 0;
    args.logging = false;
    args.logName = "";
    args.offset = 0;
    args.reqs = 0;

    unsigned char buffer[BUFFER_SIZE];
    char *hostname;
    char *port;

    if (argc < 2)
    {
        fprintf(stderr, "%s", "Not enough args specified\n");
        abort();
    }

    int opt;
    while ((opt = getopt(argc, argv, "N: l:")) != -1)
    {
        switch (opt)
        {
        case 'N':
            numThreads = atoi(optarg);
            break;
        case 'l':
            args.logging = true;
            args.logName = optarg;
            break;
        default:
            fprintf(stderr, "Usage: ./httpserver -N {numberofThreads} -l {filename} {ip address} {port}\n");
            exit(EXIT_FAILURE);
        }
    }

    int logFile;
    if (args.logging == true)
    {
        if (open(args.logName, O_WRONLY) != -1)
        {
            int file = open(args.logName, O_TRUNC);
            close(file);
        }
        logFile = open(args.logName, O_CREAT | O_WRONLY, 0644);
        close(logFile);
    }

    hostname = argv[argc - 2];
    port = argv[argc - 1];
    struct addrinfo *addrs, hints = {};

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(hostname, port, &hints, &addrs);

    int main_socket = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
    int enable = 1;
    setsockopt(main_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

    if (main_socket <= 0)
    {
        perror("An Error has occurred while creating a socket");
        return 1;
    }

    if (bind(main_socket, addrs->ai_addr, addrs->ai_addrlen) < 0)
    {
        perror("Cannot bind socket");
    }
    if (listen(main_socket, 16) < 0)
    {
        perror("Cannot listen socket");
    }
    while (1)
    {
        fprintf(stdout, "%s", ":::: Waiting for new connection ::::\n");
        memset(buffer, 0, BUFFER_SIZE);
        int connection_fd = accept(main_socket, NULL, NULL);
        if (connection_fd < 0)
        {
            fprintf(stderr, "%s", "In accept");
        }
        fprintf(stdout, ":::: Connected ::::\n");
        int valRead = read(connection_fd, buffer, BUFFER_SIZE);

        if (valRead < 0)
        {
            perror("Error in reading socket data");
        }
    }
}
