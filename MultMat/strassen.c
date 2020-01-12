#include "strassen.h"
#include "matrix_util.h"

void strassen(int n, int **a, int **b, int **c)
{
    int **A11 = malloc_mat(n, n);
    int **A12 = malloc_mat(n, n);
    int **A21 = malloc_mat(n, n);
    int **A22 = malloc_mat(n, n);

    int **B11 = malloc_mat(n, n);
    int **B12 = malloc_mat(n, n);
    int **B21 = malloc_mat(n, n);
    int **B22 = malloc_mat(n, n);

    int **C11 = malloc_mat(n, n);
    int **C12 = malloc_mat(n, n);
    int **C21 = malloc_mat(n, n);
    int **C22 = malloc_mat(n, n);

    int **M1 = malloc_mat(n, n);
    int **M2 = malloc_mat(n, n);
    int **M3 = malloc_mat(n, n);
    int **M4 = malloc_mat(n, n);
    int **M5 = malloc_mat(n, n);
    int **M6 = malloc_mat(n, n);
    int **M7 = malloc_mat(n, n);

    int **AA = malloc_mat(n, n);
    int **BB = malloc_mat(n, n);

    if (n == 1)
    {
        c[0][0] = a[0][0] * b[0][0];
    }
    else if (n == 2)
    {
        // case de base
        multiply_mats(n, a, b, c);
    }
    else
    {
        // Divisez les matrices A et B en quatre sous-matrices du même ordre,
        // C'est-à-dire l'idée de diviser pour régner
        for (int i = 0; i < n / 2; i++)
        {
            for (int j = 0; j < n / 2; j++)
            {
                A11[i][j] = a[i][j];
                A12[i][j] = a[i][j + n / 2];
                A21[i][j] = a[i + n / 2][j];
                A22[i][j] = a[i + n / 2][j + n / 2];

                B11[i][j] = b[i][j];
                B12[i][j] = b[i][j + n / 2];
                B21[i][j] = b[i + n / 2][j];
                B22[i][j] = b[i + n / 2][j + n / 2];
            }
        }

        add_mats(n / 2, A11, A22, AA);
        add_mats(n / 2, B11, B22, BB);
        strassen(n / 2, AA, BB, M1);

        // Calculer M2 = (A2 + A3) × B0
        add_mats(n / 2, A21, A22, AA);
        strassen(n / 2, AA, B11, M2);

        // Calculer M3 = A0 × (B1 - B3)
        substract_mats(n / 2, B12, B22, BB);
        strassen(n / 2, A11, BB, M3);

        // Calculer M4 = A3 × (B2 - B0)
        substract_mats(n / 2, B21, B11, BB);
        strassen(n / 2, A22, BB, M4);

        // Calculer M5 = (A0 + A1) × B3
        add_mats(n / 2, A11, A12, AA);
        strassen(n / 2, AA, B22, M5);

        // Calculer M6 = (A2 - A0) × (B0 + B1)
        substract_mats(n / 2, A21, A11, AA);
        add_mats(n / 2, B11, B12, BB);
        strassen(n / 2, AA, BB, M6);

        // Calculer M7 = (A1 - A3) × (B2 + B3)
        substract_mats(n / 2, A12, A22, AA);
        add_mats(n / 2, B21, B22, BB);
        strassen(n / 2, AA, BB, M7);

        // Calculer C0 = M1 + M4 - M5 + M7
        add_mats(n / 2, M1, M4, AA);
        substract_mats(n / 2, M7, M5, BB);
        add_mats(n / 2, AA, BB, C11);

        // Calculer C1 = M3 + M5
        add_mats(n / 2, M3, M5, C12);

        // Calculer C2 = M2 + M4
        add_mats(n / 2, M2, M4, C21);

        // Calculer C3 = M1 - M2 + M3 + M6
        substract_mats(n / 2, M1, M2, AA);
        add_mats(n / 2, M3, M6, BB);
        add_mats(n / 2, AA, BB, C22);

        // Mettre le résultat dans c
        for (int i = 0; i < n / 2; i++)
        {
            for (int j = 0; j < n / 2; j++)
            {
                c[i][j] = C11[i][j];
                c[i][j + n / 2] = C12[i][j];
                c[i + n / 2][j] = C21[i][j];
                c[i + n / 2][j + n / 2] = C22[i][j];
            }
        }
    }
}
