/**
  * mysmartctl -- controller and terminal for the mySmartUSB
  *
  * Copyright (C) 2011  Oliver Mader <b52@reaktor42.de>
  *
  * Permission is hereby granted, free of charge, to any person obtaining a copy
  * of this software and associated documentation files (the "Software"), to deal
  * in the Software without restriction, including without limitation the rights
  * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  * copies of the Software, and to permit persons to whom the Software is
  * furnished to do so, subject to the following conditions:
  * 
  * The above copyright notice and this permission notice shall be included in
  * all copies or substantial portions of the Software.
  * 
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  * THE SOFTWARE.
  */

#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <getopt.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/select.h>
#include <unistd.h>
#include <ncurses.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>

enum {
    DATA_MODE,
    PROGRAMMER_MODE,
    QUIET_MODE,
    RESET_BOARD,
    RESET_PROGRAMMER,
    BOARD_ON,
    BOARD_OFF,
    TERMINAL
};

enum {
    NO_PARITY = 'N',
    EVEN_PARITY = 'E',
    ODD_PARITY = 'O',
};

static char *interface;
static int mode = -1;
static char *baud_raw = "9600";
static speed_t baud = B9600;
static int two_stopbits = 0;
static int parity = NO_PARITY;

static void parse_options(int argc, char *argv[]);
static int terminal();
static int mysmartusb_ctl(char command);
static int open_tty(const char *path, speed_t baud, int two_stop_bits,
    int parity);
static int write_tty(int device, const char *buffer, int length);
static int close_tty(int device);

#define mysmartusb(a,b,c) \
    if (mysmartusb_ctl((a)) == -1) { \
        printf(b); \
        return 1; \
    } else { \
        printf(c); \
        return 0; \
    }

int main(int argc, char *argv[])
{
    parse_options(argc, argv);

    switch (mode) {
        case DATA_MODE:
            mysmartusb('d', "Unable to switch into data mode\n",
                "Successfully switched to data mode\n");
        case PROGRAMMER_MODE:
            mysmartusb('p', "Unable to switch into programming mode\n",
                "Successfully switched to programming mode\n");
        case QUIET_MODE:
            mysmartusb('q', "Unable to switch into quiet mode\n",
                "Successfully switched to quiet mode\n");
        case RESET_BOARD:
            mysmartusb('r', "Unable to reset the board\n",
                "Successfully reset the board\n");
        case RESET_PROGRAMMER:
            mysmartusb('R', "Unable to reset the programmer\n",
                "Successfully reset the programmer\n");
        case BOARD_ON:
            mysmartusb('+', "Unable to turn the board power on\n",
                "Successfully turned the board power on\n");
        case BOARD_OFF:
            mysmartusb('-', "Unable to turn the board power off\n",
                "Successfully turned the board power off\n");
        case TERMINAL:
            return terminal();
    }

    return 0;
}

static void parse_options(int argc, char *argv[])
{
    const char usage[] =
        "Usage: mysmartctl <ACTION> [OPTIONS] INTERFACE\n\n"
        "Actions\n"
        "  -d, --data-mode        Switch into data mode\n"
        "  -p, --programmer-mode  Switch into programming mode\n"
        "  -q, --quiet-mode       Switch into quiet mode\n"
        "  -r, --reset-board      Reset the board\n"
        "  -R, --reset-programmer Reset the programmer\n"
        "  -o, --board-on         Turn board power on\n"
        "  -O, --board-off        Turn board power off\n"
        "  -t, --terminal         Open a terminal session\n\n"
        "Options (for terminal mode only)\n"
        "  -b, --baud=BAUD        Defines the baud rate (default: 9600)\n"
        "  -c, --parity=MODE      Either none,even or odd (default: none)\n"
        "  -e, --two-stopbits     Two stop bits intead of one\n"
        "\n"
        "  -v, --version          Print the current version\n"
        "  -h, --help             Print out this help\n";

    struct option long_opts[] = {
        {"data-mode", no_argument, NULL, 'd'},
        {"programmer-mode", no_argument, NULL, 'p'},
        {"quiet-mode", no_argument, NULL, 'q'},
        {"reset-board", no_argument, NULL, 'r'},
        {"reset-programmer", no_argument, NULL, 'R'},
        {"board-on", no_argument, NULL, 'o'},
        {"board-off", no_argument, NULL, 'O'},
        {"terminal", no_argument, NULL, 't'},
        {"baud", required_argument, NULL, 'b'},
        {"two-stopbits", no_argument, NULL, 'e'},
        {"parity", required_argument, NULL, 'c'},
        {"help", no_argument, NULL, 'h'},
        {"version", no_argument, NULL, 'v'},
        {0, 0, 0, 0}
    };
    const char *short_opts = "dpqrRoOtb:ec:hv";
    int c;

    while ((c = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1)
        switch (c) {
            case 'd':
                mode = DATA_MODE;
                break;
            case 'p':
                mode = PROGRAMMER_MODE;
                break;
            case 'q':
                mode = QUIET_MODE;
                break;
            case 'r':
                mode = RESET_BOARD;
                break;
            case 'R':
                mode = RESET_PROGRAMMER;
                break;
            case 'o':
                mode = BOARD_ON;
                break;
            case 'O':
                mode = BOARD_OFF;
                break;
            case 't':
                mode = TERMINAL;
                break;
            case 'b':
                switch (atoi(optarg)) {
                    case 50: baud = B50; break;
                    case 75: baud = B75; break;
                    case 110: baud = B110; break;
                    case 134: baud = B134; break;
                    case 150: baud = B150; break;
                    case 200: baud = B200; break;
                    case 300: baud = B300; break;
                    case 600: baud = B600; break;
                    case 1200: baud = B1200; break;
                    case 1800: baud = B1800; break;
                    case 2400: baud = B2400; break;
                    case 4800: baud = B4800; break;
                    case 9600: baud = B9600; break;
                    case 19200: baud = B19200; break;
                    case 38400: baud = B38400; break;
                    case 57600: baud = B57600; break;
                    case 115200: baud = B115200; break;
                    case 230400: baud = B230400; break;
                    default:
                        fprintf(stderr, "%s is not a supported baud rate\n",
                            optarg);
                        exit(EXIT_FAILURE);
                }
                baud_raw = optarg;
                break;
            case 'e':
                two_stopbits = 1;
                break;
            case 'c':
                if (strcasecmp(optarg, "none") == 0)
                    parity = NO_PARITY;
                else if (strcasecmp(optarg, "even") == 0)
                    parity = EVEN_PARITY;
                else if (strcasecmp(optarg, "odd") == 0)
                    parity = ODD_PARITY;
                else {
                    fprintf(stderr, "%s is not a valid parity\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'v':
                printf(PACKAGE_STRING "\n");
                exit(EXIT_SUCCESS);
            default:
                printf(usage);
                exit(EXIT_SUCCESS);
        }

    if (argc - optind != 1 || mode == -1) {
        printf(usage);
        exit(EXIT_FAILURE);
    }

    interface = argv[optind];
}

volatile sig_atomic_t running = 1;

static void sig_handler(int signal)
{
    running = 0;
}

static int terminal()
{
    signal(SIGTERM, sig_handler);
    signal(SIGINT, sig_handler);
    signal(SIGQUIT, sig_handler);

    initscr();
    noecho();
    cbreak();
    curs_set(0);

    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);

    WINDOW *info = newwin(2, 0, 0, 0);
    WINDOW *log = newwin(0, 0, 2, 0);
    mvwhline(info, 1, 0, ACS_HLINE, COLS);
    scrollok(log, TRUE);
    nodelay(log, TRUE);

    int tty = open_tty(interface, baud, two_stopbits, parity);

    if (tty == -1)
        return tty;

    unsigned rx = 0;
    unsigned tx = 0;
    time_t start = time(NULL);

    fd_set fds;
    int stdin_fd = STDIN_FILENO;
    int max_fd = (tty > stdin_fd) ? tty : stdin_fd;
    int bytes, c;
    char buffer[128];

    while (running) {
        FD_ZERO(&fds);
        FD_SET(tty, &fds);
        FD_SET(stdin_fd, &fds);

        struct timeval timeout = {
            .tv_sec = 1,
            .tv_usec = 0
        };

        int ret = select(max_fd + 1, &fds, NULL, NULL, &timeout);

        if (ret > 0) {
            if (FD_ISSET(tty, &fds)) {
                if ((bytes = read(tty, buffer, 128)) > 0) {
                    rx += bytes;
                    wcolor_set(log, 1, 0);
                    waddnstr(log, buffer, bytes);
                }
            }
            if (FD_ISSET(stdin_fd, &fds)) {
                bytes = 0;
                while ((c = wgetch(log)) != ERR)
                    buffer[bytes++] = c;

                tx += bytes;
                write_tty(tty, buffer, bytes);
                wcolor_set(log, 2, 0);
                waddnstr(log, buffer, bytes);
            }
            wrefresh(log);
        }
        if (ret < 0 && errno != EINTR) {
            perror("select()");
            close_tty(tty);
            return -1;
        }

        time_t diff = time(NULL) - start;
        int hours = diff / 3600;
        int minutes = (diff - hours * 3600) / 60;
        int seconds = diff - hours * 3600 - minutes * 60;

        mvwaddstr(info, 0, 0, "Connected: ");
        snprintf(buffer, 128, "%02u:%02u:%02u", hours, minutes, seconds);
        mvwaddstr(info, 0, 11, buffer);
        mvwaddstr(info, 0, 24, "RX/TX: ");
        int space = snprintf(buffer, 128, "%uB/%uB   ", rx, tx);
        mvwaddstr(info, 0, 31, buffer);
        mvwaddstr(info, 0, 31 + space, "Mode: ");
        snprintf(buffer, 128, "%s,%c,%i", baud_raw, parity, two_stopbits + 1);
        mvwaddstr(info, 0, 37 + space, buffer);

        wrefresh(info);
    }

    delwin(info);
    delwin(log);
    endwin();

    close_tty(tty);

    return 0;
}

static int mysmartusb_ctl(char command)
{
    int tty = open_tty(interface, B19200, 0, NO_PARITY);

    if (tty == -1)
        return -1;

    char buffer[16] = "\xE6\xB5\xBA\xB9\xB2\xB3\xA9";
    char response[4] = "\xF7\xB1 \0";
    buffer[7] = response[2] = command;
    
    write_tty(tty, buffer, 8);

    int bytes, length = 0;
    while ((bytes = read(tty, buffer + length, 15 - length)) > 0 && length < 16) {
        buffer[length += bytes] = 0;

        if (length > 1 && strstr(buffer, "\r\n") != NULL)
            break;
    }

    close_tty(tty);

    if (strstr(buffer, response) == NULL)
        return -1;

    return 0;
}

static int open_tty(const char *path, speed_t baud, int two_stop_bits,
    int parity)
{
    int tty = open(path, O_RDWR | O_NOCTTY | O_NDELAY);

    if (tty == -1) {
        perror("Unable to open tty");
        return -1;
    }

    fcntl(tty, F_SETFL, 0);

    struct termios termios;

    if (tcgetattr(tty, &termios) == -1) {
        perror("tcgetattr()");
        return -1;
    }

    if (cfsetispeed(&termios, baud) == -1 ||
        cfsetospeed(&termios, baud) == -1)
    {
        perror("Unable to set baud rate");
        return -1;
    }
    
    termios.c_iflag = 0;
    termios.c_oflag = 0;
    termios.c_cflag &= ~(PARENB | HUPCL | CSIZE | CSTOPB);
    termios.c_cflag |= CREAD | CLOCAL | (two_stop_bits ? CSTOPB : 0) | CS8;
    termios.c_lflag &= ~(ISIG | ICANON | ECHO);
    termios.c_cc[VMIN] = 1;
    termios.c_cc[VTIME] = 0;

    if (two_stop_bits)
        termios.c_cflag |= CSTOPB;

    if (parity != NO_PARITY)
        termios.c_cflag |= PARENB | ((parity == ODD_PARITY) ? PARODD : 0);

    if (tcsetattr(tty, TCSANOW, &termios) == -1) {
        perror("Changing device options failed");
        return -1;
    }

    tcflush(tty, TCIFLUSH);

    return tty;
}

static int write_tty(int device, const char *buffer, int length)
{
    return write(device, buffer, (length > 0) ? length : strlen(buffer));
}

static int close_tty(int device)
{
    return close(device);
}

