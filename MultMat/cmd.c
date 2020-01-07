#include "cmd.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>

int print_colored(int color, const char *format, ...)
{
    switch (color)
    {
    case 1:                   //red
        printf("\033[0;31m"); // sets color for red
        break;
    case 2:
        printf("\033[0;31m"); // sets color for green
        break;
    default:
        printf("\033[0m"); // resets the text to default color
        break;
    }
    va_list arg;
    int done;

    va_start(arg, format);
    done = vfprintf(stdout, format, arg);
    va_end(arg);
    printf("\033[0m"); // resets the text to default color
    return done;
}

void usage()
{
    const char *USAGE = "Multmat C project for matrix multiplication benchmarking of different solvers.\nUsage :\n-a <lines_a>\t\tlines for matrix A\n-b <columns_a>\t\tcolumns for matrix A\n-c <lines_b>\t\tlines for matrix B\n-d <columns_b>\t\tcolumns for matrix\n-n <number_threads>\tnumber of threads to launch parallel solvers with\n-h\t\t\tprint usage\n";
    printf("%s", USAGE);
}

int toint(char *s)
{
    char *p;
    errno = 0; // not 'int errno', because the '#include' already defined it
    int a = strtol(s, &p, 10);
    if (*p != '\0' || errno != 0)
    {
        return -1;
    }
    return a;
}

int check_flag_value(char *flag, char *s)
{
    const char *ERROR_FLAG_VALUE = "ERROR: flag %s is required.\n";
    const char *ERROR_FLAG_VALUE_TYPE = "ERROR: flag %s needs to be a stricly positive integer.\n";
    if (s == NULL)
    {
        print_colored(1, ERROR_FLAG_VALUE, flag);
        return -1;
    }
    int x = toint(s);
    if (x <= 0)
    {
        print_colored(1, ERROR_FLAG_VALUE_TYPE, flag);
        return -1;
    }
    return x;
}

bool check_input(struct CmdInput input)
{
    const char *ERROR_DIMENSION_INTEGRITY = "ERROR: LINES_B # COLUMNS_A. A: %d x %d, B: %d x %d.\n";
    const char *ERROR_DIMENSION_MAX = "ERROR: LINES * COLUMNS > 10e6. A: %d x %d, B: %d x %d.\n";

    if (input.lines_a == -1 || input.lines_b == -1 || input.columns_a == -1 || input.columns_b == -1 || input.num_threads == -1)
    {
        return false;
    }
    if (input.columns_a != input.lines_b)
    {
        print_colored(1, ERROR_DIMENSION_INTEGRITY, input.lines_a, input.columns_a, input.lines_b, input.columns_b);
        return false;
    }
    if ((input.lines_a) * (input.columns_a) > MAX_DIMENSION || (input.lines_b) * (input.columns_b) > MAX_DIMENSION)
    {
        print_colored(1, ERROR_DIMENSION_MAX, input.lines_a, input.columns_a, input.lines_b, input.columns_b);
        return false;
    }

    return true;
}

struct CmdInput read_input(int argc, char *argv[])
{
    struct CmdInput i = {-1, -1, -1, -1, -1};
    char *avalue = NULL;
    char *bvalue = NULL;
    char *cvalue = NULL;
    char *dvalue = NULL;
    char *nvalue = NULL;

    int c;
    opterr = 0;
    while ((c = getopt(argc, argv, "a:b:c:d:n:h")) != -1)
    {
        switch (c)
        {
        case 'a':
            avalue = optarg;
            break;
        case 'b':
            bvalue = optarg;
            break;
        case 'c':
            cvalue = optarg;
            break;
        case 'd':
            dvalue = optarg;
            break;
        case 'n':
            nvalue = optarg;
            break;
        case 'h':
            usage();
            return i;
        }
    }

    i.lines_a = check_flag_value("a", avalue);
    i.columns_a = check_flag_value("b", bvalue);
    i.lines_b = check_flag_value("c", cvalue);
    i.columns_b = check_flag_value("d", dvalue);
    i.num_threads = check_flag_value("n", nvalue);
    return i;
}