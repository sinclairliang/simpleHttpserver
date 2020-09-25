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

#define BUFFER_SIZE 32768 // size for 32Kib;
#define HEADER_SIZE 4000  // size for 4Kib;
#define DEFAULT_PORT 8080


bool checkValidName(char *name, uint8_t *copy_buff, int index) {
    bool valid = true;
    if (strlen(name) > 27) {
        fprintf(stdout, "%s", "File Name is too long\n");
        valid = false;
    }
    for (int i = 0; i < strlen(name); i++) {
        if ((isalnum(copy_buff[index]) == 0) && (copy_buff[index] != '-') && (copy_buff[index] != '_')) {
            fprintf(stdout, "%s", "Invalid String in Filename\n");
            valid = false;
        }
        index++;
    }
    return valid;
}

int receiveFile(int inputFileDescriptor, int outputFileDescriptor, int sizeofFile) {
    unsigned char buffer[BUFFER_SIZE];
    int remainingSize = sizeofFile;
    while (remainingSize > 0) {
        memset(buffer, 0, BUFFER_SIZE);
        int readSize = read(inputFileDescriptor, buffer, BUFFER_SIZE);
        if (readSize < 0) {
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

int main(int argc, char *argv[]) {
    unsigned char buffer[BUFFER_SIZE];
    char *hostname;
    char *port;

    if (argc < 2) {
        fprintf(stderr, "%s", "Not enough args specified\n");
        abort();
    }

    if (argc == 3) {
        hostname = argv[1];
        port = argv[2];
    }

    struct addrinfo *addrs, hints = {};

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(hostname, port, &hints, &addrs);

    int main_socket = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
    int enable = 1;
    setsockopt(main_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

    if (main_socket <= 0) {
        perror("An Error has occurred while creating a socket");
        return 1;
    }

    if (bind(main_socket, addrs->ai_addr, addrs->ai_addrlen) < 0) {
        perror("Cannot bind socket");
    }
    if (listen(main_socket, 16) < 0) {
        perror("Cannot listen socket");
    }
    while (1) {
        fprintf(stdout, "%s", ":::: Waiting for new connection ::::\n");
        memset(buffer, 0, BUFFER_SIZE);
        int connection_fd = accept(main_socket, NULL, NULL);
        if (connection_fd < 0) {
            perror("In accept");
        }
        fprintf(stdout, ":::: Connected ::::\n");
        int valRead = read(connection_fd, buffer, BUFFER_SIZE);

        if (valRead < 0) {
            perror("Error in reading socket data");
        }
    }
}