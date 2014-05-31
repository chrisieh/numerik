﻿#include "matrix.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

MATRIX matrix_alloc(int n, int m) {
  MATRIX ret;
  int i;
  
  /* Fehlercheck fehlt */
  ret.data = malloc(n * m * sizeof(double));
  ret.n = n;
  ret.m = m;
  
  /* GET Pointerstruktur */
  ret.elem = malloc(n * sizeof(double *));
  for (i = 0; i < n; i++) {
    ret.elem[i] = ret.data + i * m;
  }
  
  return ret;  
}

void matrix_init(MATRIX *A, double value) {
  int i, j;
  for (i = 0; i < A->n; i++) {
    for (j = 0; j < A->m; j++) {
      A->elem[i][j] = value;
    }
  }
}

void matrix_free(MATRIX *A) {
  /* Fehlercheck */
  free(A->elem);
  free(A->data);
}

void matrix_print(MATRIX *A) {
  int i, j;
  for (i = 0; i < A->n; i++) {
    for (j = 0; j < A->m; j++) {
      printf("%f ", A->elem[i][j]);
    }
    printf("\n");
  }
}

/* Operationen */
MATRIX matrix_mult(MATRIX *A, MATRIX *B) {
  /* (L x M) * (M x N) = (L x n) */
  MATRIX ret;
  int i, j, k;
  double sum;
  
  /* Kompatibilitaet */
  if (A->m != B->n) {
    printf("Matrizen nicht kompatibel\n");
  }
  
  ret = matrix_alloc(A->n, B->m);
  
  for (i = 0; i < A->n; i++) {
    for (j = 0; j < B->m; j++) {
      sum = 0;
      for (k = 0; k < A-> m; k++) {
        sum += A->elem[i][k] * B->elem[k][j];
      }
      ret.elem[i][j] = sum;
    }
  }
  
  return ret;
}

void matrix_swap_row(MATRIX *A, int i, int j) {
  double *temp;
  
  temp = A->elem[i];
  A->elem[i] = A->elem[j];
  A->elem[j] = temp;
}

int LU_decomp(MATRIX *A, int *permutation) {
  int i, j, k;
  int n = A->n;
  int piv, temp;
  
  for (i = 0; i < n; i++) {
    /* Hier Pivotisierung */
    piv = pivot(A, i);
    if (piv != i) {
      /* Tauschen der Zeilen */
      matrix_swap_row(A, i, piv);
      /* Merken der Vertauschung */
      temp = permutation[i];
      permutation[i] = permutation[piv];
      permutation[piv] = temp;
    }
    /* U */
    for (j = i; j < n; j++) {
      for (k = 0; k < i; k++) {
        A->elem[i][j] -= A->elem[i][k] * A->elem[k][j];
      }
    }
    /* check ob singulär (Warum hier?) */
    if (fabs(A->elem[i][i]) < 1E-10) {
      return -1;
    }
    /* L */
    for (j = i + 1; j < n; j++) {
      for (k = 0; k < i; k++) {
        A->elem[j][i] -= A->elem[j][k] * A->elem[k][i];
      }
      A->elem[j][i] /= A->elem[i][i];
    }
  }
  
  return 0;
}

int pivot(MATRIX *A, int k) {
  int i, j;
  int piv = k;
  double max = 0;
  double temp, sum;
  
  if (k < A->n - 1) {
    for (i = k; i < A->n; i++) {
      sum = 0;
      for (j = k; j < A->n; j++) {
        sum += fabs(A->elem[i][j]);
      }
      temp = A->elem[i][k] / sum;
    
      if (temp > max) {
        max = temp;
        piv = i;
      }
    }
  }
  
  return piv;
}

int LU_solve(MATRIX *LU, VECTOR *Pb, VECTOR *sol) {
  VECTOR y = vector_alloc(Pb->n);
  
  LU_forward_sub(LU, Pb, &y);
  LU_back_sub(LU, &y, sol);
  
  vector_free(&y);
  
  return 0;
}

int LU_forward_sub(MATRIX *LU, VECTOR *b, VECTOR *sol) {
  int i, j;
  int n = LU->n;
  
  if (n != LU->m || n != b->n) {
    return -1;
  }
  
  for (i = 0; i < n; i++) {
    sol->elem[i] = b->elem[i];
    for (j = 0; j < i; j++) {
      sol->elem[i] -= LU->elem[i][j] * sol->elem[j];
    }
    /* Es muss nicht durch L_i_i geteilt werden, da die diagonalelemente der 
     * L Matrix 1 sind; sonst gibt es keinen zugriff auf das Diag.elem. */
  }
  
  return 0;
}

int LU_back_sub(MATRIX *LU, VECTOR *b, VECTOR *sol) {
  int i, j;
  int n = LU->n;
  
  if (n != LU->m || n != b->n) {
    return -1;
  }
  
  for (i = n - 1; i >= 0; i--) {
    sol->elem[i] = b->elem[i];
    for (j = i + 1; j < n; j++) {
      sol->elem[i] -= LU->elem[i][j] * sol->elem[j];
    }
    sol->elem[i] /= LU->elem[i][i];
  }
  
  return 0;
}

int linear_solve(MATRIX *A, VECTOR *b, VECTOR *sol) {
  int i;
  int n = A->n;
  int *permutation = malloc(n * sizeof(int));
  VECTOR Pb = vector_alloc(n);
  
  /* setup permutation */
  for (i = 0; i < n; i++) {
    permutation[i] = i;
  }
  
  /* LUP-Decomp */
  LU_decomp(A, permutation);
  
  printf("{");
  for (i = 0; i < n - 1; i++) {
    printf("%i, ", permutation[i]);
  }
  printf("%i}\n", permutation[n-1]);
  
  /* Permutiere b */
  for (i = 0; i < n; i++) {
    Pb.elem[i] = b->elem[permutation[i]];
  }
  
  LU_solve(A, &Pb, sol);
  
  return 0;
}

VECTOR vector_alloc(int n) {
  VECTOR ret;
  
  /* Fehlercheck */
  ret.elem = malloc(n * sizeof(double));
  ret.n = n;
  
  return ret;
}

void vector_init(VECTOR *v, double value) {
  int i;
  for (i = 0; i < v->n; i++) {
    v->elem[i] = value;
  }
}

void vector_free(VECTOR *v) {
  free(v->elem);
}

void vector_print(VECTOR *v) {
  int i;
  for (i = 0; i < v->n; i++) {
    printf("%f\n", v->elem[i]);
  }
}