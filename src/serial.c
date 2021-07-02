#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define master 0

int rank,size;
int N;

void create_matrices(int *A,int *B){
    int x=N*N;
    for(int i=0;i<x;i++)scanf("%d",&A[i]);
    for(int i=0;i<x;i++)scanf("%d",&B[i]);
}


void print_matrix(int *arr,int n,FILE* file){
    for(int i=0;i<n;i++){
        for(int k=0;k<n;k++){
            fprintf(file,"%d ",arr[i*n+k]);
        }
        fprintf(file,"\n");
    }
}


//we are assuming only process 0 has access to the input matrices in this problem
    //In this problem we will divide result matrix of C into P blocks and calculate each one of them in different processes
    //Since 
int main(){
    scanf("%d",&N);
    int *A=(int*)malloc(N*N*sizeof(int)),
        *B=(int*)malloc(N*N*sizeof(int)),         // NxN input matrices
        *C=(int*)calloc(N*N,sizeof(int));         // NxN result matrix

    srand(time(NULL));



    create_matrices(A,B);


    clock_t TIMER = clock();

    for(int n=0;n<N;n++){
        for(int k=0;k<N;k++){
            for(int m=0;m<N;m++){
                C[n*N+k]+=A[n*N+m]*B[m*N+k];
            }
        }
    }






    double TIME=(double)(clock()-TIMER)/CLOCKS_PER_SEC;
    printf("Sequential time is %f seconds\n",TIME);

    free(A);
    free(B);

    FILE *file=fopen("output2","w");
    print_matrix(C,N,file);
    fclose(file);

    free(C);


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