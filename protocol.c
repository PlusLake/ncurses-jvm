#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ncurses.h>

#define JNCURSES_EXIT 1
#define JNCURSES_NEXT 2

/** Exit ncurses gracefully. */
void jncurses_protocol_error(const char* format, ...) {
    endwin();
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(-1);
}

/** Exit ncurses. */
int jncurses_protocol_exit(int socket) {
    return JNCURSES_EXIT;
}

/** End the turn and wait for next key */
int jncurses_protocol_next(int socket) {
    return JNCURSES_NEXT;
}

/** Read bytes from the unix socket and print with ncurses. */
int jncurses_protocol_print(int socket) {
    char size[4];
    int length = read(socket, size, sizeof(size));
    if (length != 4) jncurses_protocol_error("Illegal length of print (%d)\n", length);

    char buffer[32768];
    int bytes_to_read = *((int *) size);
    length = read(socket, buffer, bytes_to_read);
    printw("%.*s", length, buffer);
    return 0;
}

/** Read bytes from the unix socket and move the cursor. */
int jncurses_protocol_move(int socket) {
    char location[4];
    int length = read(socket, location, sizeof(location));
    if (length != 4) jncurses_protocol_error("Illegal length of move (%d)\n", length);

    int x = *((short *) location);
    int y = *((short *) &location[2]);
    move(y, x);
    return 0;
}

/** Clear the screen */
int jncurses_protocol_clear(int socket) {
    erase();
    return 0;
}

/** Execute instructions. */
int jncurses_protocol_execute(int socket, int instruction) {
    int (*protocols[]) (int) = {
        jncurses_protocol_exit,
        jncurses_protocol_next,
        jncurses_protocol_print,
        jncurses_protocol_move,
        jncurses_protocol_clear,
    };
    int protocol_count = sizeof(protocols) / sizeof(protocols[0]);
    if (instruction < 0 || instruction > protocol_count)
        jncurses_protocol_error("Illegal instruction (%d)\n", instruction);
    return protocols[instruction](socket);
}
