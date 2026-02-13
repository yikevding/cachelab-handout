/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if (M == 32 && N == 32)
    {
        int i, ii, jj;
        int temp0, temp1, temp2, temp3, temp4, temp5, temp6, temp7;

        // Process matrix in 8×8 blocks
        for (ii = 0; ii < 32; ii += 8)
        {
            for (jj = 0; jj < 32; jj += 8)
            {
                // Transpose the 8×8 block
                for (i = ii; i < ii + 8; i++)
                {
                    // Read 8 elements from A into local variables
                    // one miss, seven hit
                    temp0 = A[i][jj];
                    temp1 = A[i][jj + 1];
                    temp2 = A[i][jj + 2];
                    temp3 = A[i][jj + 3];
                    temp4 = A[i][jj + 4];
                    temp5 = A[i][jj + 5];
                    temp6 = A[i][jj + 6];
                    temp7 = A[i][jj + 7];

                    // Write transposed elements to B
                    // 8 separate miss
                    B[jj][i] = temp0;
                    B[jj + 1][i] = temp1;
                    B[jj + 2][i] = temp2;
                    B[jj + 3][i] = temp3;
                    B[jj + 4][i] = temp4;
                    B[jj + 5][i] = temp5;
                    B[jj + 6][i] = temp6;
                    B[jj + 7][i] = temp7;
                }
            }
        }
    }
    else if (M == 64 && N == 64)
    {
        int i, j, i1, j1, t1, t2, t3, t4, t5, t6, t7, t8;

        for (i = 0; i < N; i += 8)
            for (j = 0; j < M; j += 8)
            {
                // each iteration handles one 8x8 block
                for (i1 = i; i1 < i + 4; i1++)
                {
                    // load one cache block
                    t1 = A[i1][j];
                    t2 = A[i1][j + 1];
                    t3 = A[i1][j + 2];
                    t4 = A[i1][j + 3];
                    t5 = A[i1][j + 4];
                    t6 = A[i1][j + 5];
                    t7 = A[i1][j + 6];
                    t8 = A[i1][j + 7];

                    // write directly to its final place, top left
                    B[j][i1] = t1;
                    B[j + 1][i1] = t2;
                    B[j + 2][i1] = t3;
                    B[j + 3][i1] = t4;

                    // park at the wrong place for now to avoid thrashing
                    // note we have already loaded B[j][i1+4] when we load B[j][i1]
                    // top right
                    B[j][i1 + 4] = t5;
                    B[j + 1][i1 + 4] = t6;
                    B[j + 2][i1 + 4] = t7;
                    B[j + 3][i1 + 4] = t8;
                }

                for (j1 = j + 4; j1 < j + 8; j1++)
                {
                    // bottom left
                    t5 = A[i + 4][j1 - 4];
                    t6 = A[i + 5][j1 - 4];
                    t7 = A[i + 6][j1 - 4];
                    t8 = A[i + 7][j1 - 4];

                    // extract top right value that was temporarily placed here
                    t1 = B[j1 - 4][i + 4];
                    t2 = B[j1 - 4][i + 5];
                    t3 = B[j1 - 4][i + 6];
                    t4 = B[j1 - 4][i + 7];

                    // write bottom left value to its final place
                    B[j1 - 4][i + 4] = t5;
                    B[j1 - 4][i + 5] = t6;
                    B[j1 - 4][i + 6] = t7;
                    B[j1 - 4][i + 7] = t8;

                    // move top right to its correct place
                    B[j1][i] = t1;
                    B[j1][i + 1] = t2;
                    B[j1][i + 2] = t3;
                    B[j1][i + 3] = t4;

                    // bottom right block to its correct place
                    B[j1][i + 4] = A[i + 4][j1];
                    B[j1][i + 5] = A[i + 5][j1];
                    B[j1][i + 6] = A[i + 6][j1];
                    B[j1][i + 7] = A[i + 7][j1];
                }
            }
    }
    else
    {
        int i, j, i1, j1, d, tmp, b = 16;

        for (i = 0; i < N; i += b)
            for (j = 0; j < M; j += b)
                for (i1 = i; i1 < i + b && i1 < N; i1++)
                {
                    for (j1 = j; j1 < j + b && j1 < M; j1++)
                    {
                        if (i1 != j1)
                            B[j1][i1] = A[i1][j1];
                        else
                        {
                            d = i1;
                            tmp = A[d][d];
                        }
                    }
                    // delay the write to avoid direct evict each other
                    if (i == j)
                        B[d][d] = tmp;
                }
    }
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; j++)
        {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc);
}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; ++j)
        {
            if (A[i][j] != B[j][i])
            {
                return 0;
            }
        }
    }
    return 1;
}
