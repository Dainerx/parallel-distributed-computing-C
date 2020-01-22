#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <omp.h>
/*#define LINES_A 500
#define COLUMS_A 2000
#define LINES_B 2000
#define COLUMS_B 700*/
#define LINES_A 3
#define COLUMS_A 4
#define LINES_B 4
#define COLUMS_B 3
#define MAX_VAL 10
#define MIN_VAL 2

int flatA[LINES_A * COLUMS_A];
int flatB[LINES_B * COLUMS_B];

int ** malloc_mat(int lines, int colums)
{
  int** mat = (int**)malloc(lines * sizeof(int*));
  for(int i = 0; i < lines; i++)
    mat[i] = (int*)malloc(colums * sizeof(int));
  return mat;
}

void free_mat(int** mat, int lines)
{
  for(int i = 0; i < lines; i++)
  {
    free(mat[i]);
  }
  free(mat);
}

void fill_mat(int** mat, int lines, int colums)
{
  for(int i = 0; i < lines; i++)
  {
    for(int j = 0; j < colums; j++)
    {
      mat[i][j] = (rand() % MAX_VAL) + MIN_VAL;
    }
  }
}

void display_mat(int** mat, int lines, int colums)
{
  for(int i = 0; i < lines; i++)
  {
    for(int j = 0; j < colums; j++)
    {
      printf("%d \t", mat[i][j]);
    }
    printf("\n");
  }
}

double sequential_mult(int** mat_A, int** mat_B, int** mat_C )
{
  double start, end, cpu_time_used;
  int sum;
  start = omp_get_wtime();
  for(int i = 0; i < LINES_A; i++)
  {
    for(int j = 0; j < COLUMS_B; j++)
    {
      sum = 0;
      for(int k = 0; k < COLUMS_A; k++)
      {
        sum += (mat_A[i][k] * mat_B[k][j]);
      }
      mat_C[i][j] = sum;
    }
  }
  end = omp_get_wtime();
  cpu_time_used = (end - start);
  return cpu_time_used;
}

void add_mats(int n, int **a, int **b, int **c)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            c[i][j] = a[i][j] + b[i][j];
        }
    }
}

void substract_mats(int n, int **a, int **b, int **c)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            c[i][j] = a[i][j] - b[i][j];
        }
    }
}

void multiply_mats(int n, int **a, int **b, int **c)
{
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            c[i][j] = 0;
            for (int t = 0; t < 2; t++)
            {
                c[i][j] = c[i][j] + a[i][t] * b[t][j];
            }
        }
    }
}

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

double strassen_mult(int **mat_A, int **mat_B, int **mat_C, int** mat_C_final,int n)
{
    double start, end, cpu_time_used;
    start = omp_get_wtime();
    strassen(n, mat_A, mat_B, mat_C);
    for(int i=0; i<LINES_A; i++)
    {
      for(int j=0; j<COLUMS_B; j++)
      {
        mat_C_final[i][j] = mat_C[i][j];
      }
    }
    end = omp_get_wtime();
    cpu_time_used = (end - start);
    return cpu_time_used;
}
bool check_input(int la, int ca, int lb, int cb)
{
  const char *DIMENSION_INTEGRITY = "ERROR: LINES_B # COLUMNS_A. A: %d x %d, B: %d x %d.\n";
  const char *DIMENSION_MAX = "ERROR: LINES * COLUMNS > 10e6. A: %d x %d, B: %d x %d.\n";

  printf("\033[0;31m"); // sets color for red to display errors.
  if (ca!=lb)
  {
    printf(DIMENSION_INTEGRITY,la,ca,lb,cb);
    return false;
  }
  if (la*ca > 10e6 || lb*cb > 10e6)
  {
    printf(DIMENSION_MAX,la,ca,lb,cb);
    return false;
  }
  printf("\033[0m"); // resets the text to default color
  return true;
}

void make_square(int** mat_squared, int** mat,int line, int column,int n)
{
      for(int i =0; i<line; i++)
      {
          for(int j =0; j<column; j++)
          {
            mat_squared[i][j] = mat[i][j];
          }
      }

}
int get_max(int a, int b)
{
  if(a>b)
    return a;
  else
    return b;
}

int main(int argc, char *argv[])
{
  if(!check_input(LINES_A,COLUMS_A,LINES_B,COLUMS_B))
  {
      return -1;
  }

  int** mat_A = malloc_mat(LINES_A, COLUMS_A);
  int** mat_B = malloc_mat(LINES_B, COLUMS_B);
  int** mat_C = malloc_mat(LINES_A, COLUMS_B);

  srand(time(NULL)); // PEUT ETRE AJOUTER QLQ CHOSE ICI
  fill_mat(mat_A, LINES_A, COLUMS_A);
  fill_mat(mat_B, LINES_B, COLUMS_B);

  printf("la matrice A\n");
  //display_mat(mat_A, LINES_A, COLUMS_A);
  printf("\n\n La matrice B\n");
  //display_mat(mat_B, LINES_B, COLUMS_B);
    printf("\n\n");

  double cpu_time_used, cpu_time_used_seq;
  cpu_time_used = sequential_mult(mat_A, mat_B, mat_C);
  printf("la matrice C\n");
  //display_mat(mat_C, LINES_A, COLUMS_B);
  printf("\n\n");
  printf("Cpu time used in seq: %f\n",cpu_time_used);

  int m = get_max(LINES_A, COLUMS_A);
  int max = get_max(m, COLUMS_B);
  // SI LES MATRICES NE SONT PAS CARRÉES
 if(LINES_A != COLUMS_A && LINES_B != COLUMS_B )
  {
    int** mat_A_squared = malloc_mat(max, max);
    int** mat_B_squared = malloc_mat(max, max);
    int** mat_C_squared = malloc_mat(max, max);

    make_square(mat_A_squared, mat_A, LINES_A,COLUMS_A, max);
    make_square(mat_B_squared,mat_B,LINES_B,COLUMS_B,max);
    cpu_time_used_seq = strassen_mult(mat_A_squared, mat_B_squared, mat_C_squared,mat_C, max);
  }else{
    cpu_time_used_seq = strassen_mult(mat_A, mat_B, mat_C, mat_C, COLUMS_A);
  }
  printf("Cpu time used in strassen: %f\n",cpu_time_used_seq);

  free_mat(mat_A,LINES_A);
  free_mat(mat_B,LINES_B);
  free_mat(mat_C,LINES_A);
  return EXIT_SUCCESS;
}
