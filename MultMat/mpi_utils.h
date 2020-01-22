#include <stdbool.h>
#define MAX_DIMENSION 10e6
struct CmdInputDistributed
{
    int num_machines;
    int num_threads;
    int lines_a;
    int columns_a;
    int lines_b;
    int columns_b;
};

/**
 * Acts just like printf just take in argument a color to print with.
 */
int print_colored(int rank, int color, const char *format, ...);

/**
 * Converts to int a sequence of char.
 */
int toint(char *s);

/**
 * Takes as argument CmdInput, checks the input againt all the necessary tests and returns a boolean.
 * Returns true if the input is correct, false else.
 */
bool check_input(int rank, struct CmdInputDistributed input);

/**
 * Read input from command line, read values of all flags
 */
struct CmdInputDistributed read_input(int rank, int world_size, int argc, char *argv[]);

bool result_correct(int lines, int columns, int *c, int **matrixC);
