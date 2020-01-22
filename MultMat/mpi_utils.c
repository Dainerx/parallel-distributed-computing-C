#include "mpi_utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>

/*
  Colorer les messages affichés
*/
int print_colored(int rank, int color, const char *format, ...)
{
    if (rank == 0)
    {

        switch (color)
        {
        case 1:                   //rouge
            printf("\033[0;31m"); // Définir la couleur rouge
            break;
        case 2:                   //vert
            printf("\033[0;32m"); // Définir la couleur vert
            break;
        default:
            printf("\033[0m"); // Remettre le texte à la couleur par défaut
            break;
        }
        va_list arg;
        int done;

        va_start(arg, format);
        done = vfprintf(stdout, format, arg);
        va_end(arg);
        printf("\033[0m"); // Remettre le texte à la couleur par défaut
        return done;
    }
    return -1;
}

// Afficher un guide pour Multmat
void usage()
{
    const char *USAGE = "Multmat C project for matrix multiplication benchmarking of different solvers.\nUsage :\n-a <lines_a>\t\tlines for matrix A\n-b <columns_a>\t\tcolumns for matrix A\n-c <lines_b>\t\tlines for matrix B\n-d <columns_b>\t\tcolumns for matrix\n-n <number_threads>\tnumber of threads to launch parallel solvers with\n-h\t\t\tprint usage\n";
    printf("%s", USAGE);
}

// Convertir une chaine de caractères à un entier positif.
// Retourner -1 dans le cas d'une erreur.
int toint(char *s)
{
    char *p;
    errno = 0; // pas 'int errno', car le '#include' l'a déjà défini
    int a = strtol(s, &p, 10);
    if (*p != '\0' || errno != 0)
    {
        return -1;
    }
    return a;
}

// Retourner la valeur d'un flag, -1 dans le cas d'une erreur.
int check_flag_value(int rank, char *flag, char *s)
{
    const char *ERROR_FLAG_VALUE = "ERROR: flag %s is required.\n";
    const char *ERROR_FLAG_VALUE_TYPE = "ERROR: flag %s needs to be a stricly positive integer.\n";
    if (s == NULL)
    {
        print_colored(rank, 1, ERROR_FLAG_VALUE, flag);
        return -1;
    }
    int x = toint(s);
    if (x <= 0)
    {
        print_colored(rank, 1, ERROR_FLAG_VALUE_TYPE, flag);
        return -1;
    }
    return x;
}

// Retourner vrai si l'input est correct, faux sinon.
bool check_input(int rank, struct CmdInputDistributed input)
{
    const char *ERROR_DIMENSION_INTEGRITY = "ERROR: LINES_B # COLUMNS_A. A: %d x %d, B: %d x %d.\n";
    const char *ERROR_DIMENSION_MAX = "ERROR: LINES * COLUMNS > 10e6. A: %d x %d, B: %d x %d.\n";

    if (input.lines_a == -1 || input.lines_b == -1 || input.columns_a == -1 || input.columns_b == -1 || input.num_threads == -1)
    {
        return false;
    }

    if (input.columns_a != input.lines_b)
    {
        print_colored(rank, 1, ERROR_DIMENSION_INTEGRITY, input.lines_a, input.columns_a, input.lines_b, input.columns_b);
        return false;
    }
    if ((input.lines_a) * (input.columns_a) > MAX_DIMENSION || (input.lines_b) * (input.columns_b) > MAX_DIMENSION)
    {
        print_colored(rank, 1, ERROR_DIMENSION_MAX, input.lines_a, input.columns_a, input.lines_b, input.columns_b);
        return false;
    }

    return true;
}

// Lire tous les valeurs des arguments de la ligne de commande
// préparer l'input et retourner une instance de CmdInput.
struct CmdInputDistributed read_input(int rank, int world_size, int argc, char *argv[])
{
    struct CmdInputDistributed i = {-1, -1, -1, -1, -1, -1};
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

    i.num_machines = world_size;
    i.lines_a = check_flag_value(rank, "a", avalue);
    i.columns_a = check_flag_value(rank, "b", bvalue);
    i.lines_b = check_flag_value(rank, "c", cvalue);
    i.columns_b = check_flag_value(rank, "d", dvalue);
    i.num_threads = check_flag_value(rank, "n", nvalue);
    return i;
}

bool result_correct(int lines, int columns, int *c, int **matrixC)
{
    bool correct = true;
    for (int i = 0; i < lines; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            if (c[i * columns + j] != matrixC[i][j])
            {
                printf("Found no equal cell: (%d,%d) distribué(%d,%d)=%d ; sequentiel(%d,%d)=%d\n", i, j, i, j, c[i * lines + j], i, j, matrixC[i][j]);
                correct = false;
            }
        }
    }
    return correct;
}
