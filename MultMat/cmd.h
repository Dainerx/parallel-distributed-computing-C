#include <stdbool.h>
#define MAX_DIMENSION 10e6
struct CmdInput
{
    int num_threads;
    int lines_a;
    int columns_a;
    int lines_b;
    int columns_b;
};

int print_colored(int color, const char *format, ...);

int toint(char *s);

bool check_input(struct CmdInput input);

struct CmdInput read_input(int argc, char *argv[]);