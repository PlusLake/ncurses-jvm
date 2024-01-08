#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <ncurses.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "protocol.c"

#define PATH "/tmp/kncurses-socket"

int main();
int prepare_socket();
void loop_key(int socket);
void *loop_event(void* args);
void init_thread(int socket);
void init_ncurses();

int main() {
    int socket = prepare_socket();
    init_ncurses();
    init_thread(socket);
    loop_key(socket);
    return 0;
}

void loop_key(int socket) {
    for (short key = 0, index = 0, width, height;; key = getch()) {
        getmaxyx(stdscr, height, width);
        short send[3] = { key, width, height };
        write(socket, &send, sizeof(send));
    }
}

void *loop_event(void *args) {
    long socket = (long) args;
    char buffer[1];
    while (1) {
        read(socket, buffer, 1);
        int instruction = buffer[0];
        if (jncurses_protocol_execute(socket, instruction)) {
            endwin();
            exit(0);
            break;
        };
    }
}

void init_thread(int socket) {
    pthread_t thread;
    pthread_create(&thread, NULL, loop_event, (void *)((long) socket));
}

int prepare_socket() {
    int file_descriptor = socket(AF_UNIX, SOCK_STREAM, 0);
    if (file_descriptor == -1) {
        fprintf(stderr, "Failed to create socket. (%d)\n", file_descriptor);
        exit(-1);
    }

    struct sockaddr_un address;
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, PATH, sizeof(address.sun_path) - 1);

    if (connect(file_descriptor, (struct sockaddr*) &address, sizeof(address)) == -1) {
        fprintf(stderr, "Failed to connect to socket");
        exit(-1);
    }
    return file_descriptor;
}

void init_ncurses() {
    initscr();
    curs_set(0);
    noecho();
    keypad(stdscr, TRUE);
}
