#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define master 0

int rank,size;

#define N 1250  //1250 max  
int A[N][N];
int B[N][N];    
// NxN input matrices
int C[N][N];            // NxN result matrix

void create_matrices(int **a,int **b){
    int (*A)[N]=(int(*)[N])a;
    int (*B)[N]=(int(*)[N])b;

    for(int i=0;i<N;i++)for(int k=0;k<N;k++)scanf("%d",&A[i][k]);
    for(int i=0;i<N;i++)for(int k=0;k<N;k++)scanf("%d",&B[i][k]);
}


void print_matrix(int **MAT,int n,FILE* file){
    int (*arr)[n]=(int(*)[n])MAT;
    for(int i=0;i<n;i++){
        for(int k=0;k<n;k++){
            fprintf(file,"%d ",arr[i][k]);
        }
        fprintf(file,"\n");
    }
}


//we are assuming only process 0 has access to the input matrices in this problem
    //In this problem we will divide result matrix of C into P blocks and calculate each one of them in different processes
    //Since 
int main(){

    srand(time(NULL));



    create_matrices((int**)A,(int**)B);


    // printf("INITIAL A\n");
    // print_matrix((int**)A,N,M);
    // printf("\nINITIAL B\n");
    // print_matrix((int**)B,M,K);

    clock_t TIMER = clock();

    for(int n=0;n<N;n++){
        for(int k=0;k<N;k++){
            C[n][k]=0;
            for(int m=0;m<N;m++){
                C[n][k]+=A[n][m]*B[m][k];
            }
        }
    }








    FILE *file=fopen("output2","w");
    print_matrix((int**)C,N,file);
    fclose(file);

    double TIME=(double)(clock()-TIMER)/CLOCKS_PER_SEC;
    printf("Sequential time is %f seconds\n",TIME);

}


/*
0 1 2 
3 4 5 
6 7 8 

1 2 3 
1 2 3 
1 2 3

Result C matrix
3 6 9 
12 24 36 
21 42 63 
*/