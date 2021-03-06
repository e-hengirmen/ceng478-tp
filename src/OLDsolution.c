#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define master 0

int rank,size;

#define N 1200  //1250 max

int A[N][N],B[N][N];    // NxN input matrices
int C[N][N];            // NxN result matrix

void create_matrices(){
    for(int i=0;i<N;i++)for(int k=0;k<N;k++)A[i][k]=rand()%10;        //0 to 9
    for(int i=0;i<N;i++)for(int k=0;k<N;k++)B[i][k]=rand()%10;        //0 to 9
    /*for(int i=0;i<16;i++){
        A[i/N][i%N]=i;
        B[i/N][i%N]=1+i%N;
    }*/
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








/*                 CONTENT TABLE IN ORDER
---------CANNON's MATRIX MULTIPLICATION ALGORITHM SEQUENTIAL----------
Note: In this implementation we are assuming that only process
    0 has access to initial A B matrices
1-Initialization and checks
2-Sending matrix blocks from process 0 to every other process
3-Initializing values of C block to 0 on every process
---------------------------PART 2-PARALLEL-----------------------------
first       --->    Block receival from process 0 and partial block calculation
    second* --->    Sending new A and B blocks from previous_partner process and 
                        sending the old ones to the next_partner process
    third   --->    calculating from new A and B blocks and adding results to block C 
fourth      --->    Sending Block back to process 0 for presentation

* second and third will be repeated sqrt(process_count)-1 times
** after sqrt(process_count) block calculations that process's C_block will be ready

*/








//we are assuming only process 0 has access to the input matrices in this problem
    //using a square number of processes is neccessary (for example:4 9 16)
    //also dimensions of the matrix must be divisible to the square root of the process count
int main(){


    //----------------------------1-------------------------
    MPI_Init(NULL,NULL);

    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    
    int size_1=sqrt(size);
    if(!rank&&size_1*size_1!=size){
        printf("number of processes is not a square number\nAborted\n");
        MPI_Abort(MPI_COMM_WORLD,2);
    }
    if(N%size_1!=0 && !rank){         //checking really high process count
        printf("dimensions of the matrix must be divisible by square root of the process count.\n change N into a multiple of %d.\nAborted\n",size_1);
        MPI_Abort(MPI_COMM_WORLD,2);
    }



    if(!rank){
        FILE* file=fopen("input","w");
        srand(time(NULL));
        create_matrices();
        print_matrix((int**)A,N,file);
        fprintf(file,"\n");
        print_matrix((int**)B,N,file);
        fclose(file);
    }

    MPI_Barrier(MPI_COMM_WORLD);        //The algorithm will start here
        //In here we are just checking for requirements amd creating the initial A B matrices
    //------------------------------------------------------
    double TIMER;
    if(!rank)TIMER=MPI_Wtime();     //for checking how many seconds the program take



    
    



    //--------------------------2---------------------------
    int block_size=N/size_1;     //dimensions of the block that we are gonna send 

    if(!rank){          //sending row and column groups from process 0 to others
        MPI_Datatype BLOCK;
        MPI_Type_vector(block_size,block_size,N,MPI_INT,&BLOCK);    //Block type
        MPI_Type_commit(&BLOCK);

        MPI_Request REQ_A[size],REQ_B[size];
        MPI_Status stat_A[size],stat_B[size];
        
        for(int i=0;i<size;i++){        //sending row and column groups to every process
            int a=i/size_1,b=i%size_1; 
            int A_first_index=block_size*(a);
            int A_second_index=block_size*((b+a+size_1)%size_1);

            int B_first_index=block_size*((a+b+size_1)%size_1);
            int B_second_index=block_size*(b);



            //int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
            MPI_Isend(&A[A_first_index][A_second_index],1,BLOCK,i,0,MPI_COMM_WORLD,REQ_A+i);   //tag 0 is for A
            MPI_Isend(&B[B_first_index][B_second_index],1,BLOCK,i,1,MPI_COMM_WORLD,REQ_B+i);   //tag 1 is for B
        }
        //will only be used in process 0 but put here to make sure they dont get deallocated
    }
    //------------------------------------------------------



    int A_Block[block_size][block_size],B_Block[block_size][block_size],C_Block[block_size][block_size];
    //this are the blocks of size N/sqrt(p) x N/sqrt(p) 
        //block A and B are blocks that will come from process 0 block C is the result matrix
    
    //------------------------third-------------------------
    for(int i=0;i<block_size;i++){
        for(int k=0;k<block_size;k++){
            C_Block[i][k]=0;
        }
    }
    //------------------------------------------------------





    //------------------------first-------------------------
    MPI_Status ABStatus[2];
    MPI_Recv(&A_Block[0][0],block_size*block_size,MPI_INT,0,0,MPI_COMM_WORLD,ABStatus);
    MPI_Recv(&B_Block[0][0],block_size*block_size,MPI_INT,0,1,MPI_COMM_WORLD,ABStatus+1);

    for(int i=0;i<block_size;i++){
        for(int k=0;k<block_size;k++){
            for(int l=0;l<block_size;l++){
                C_Block[i][k]+=A_Block[i][l]*B_Block[l][k];
            }
        }
    }
    // printf("%d rank (%d %d %d %d %d %d %d %d)(%d %d %d %d)\n",rank,A_Block[0][0],A_Block[0][1],A_Block[1][0],A_Block[1][1]
        // ,B_Block[0][0],B_Block[0][1],B_Block[1][0],B_Block[1][1]
        // ,C_Block[0][0],C_Block[0][1],C_Block[1][0],C_Block[1][1]);

    //------------------------------------------------------






    int previous_partner_A=rank+1,next_partner_A=rank-1;
    if(rank%size_1==0)next_partner_A=rank+(size_1-1);
    if(rank%size_1==size_1-1)previous_partner_A=rank-(size_1-1);

    int previous_partner_B=rank+size_1,next_partner_B=rank-size_1;
    if(rank/size_1==0)next_partner_B=rank+(size_1-1)*size_1;
    if(rank/size_1==size_1-1)previous_partner_B=rank-(size_1-1)*size_1;

    //partners that we will send and receive blocks from has been chosen


    int A_Block1[block_size][block_size],B_Block1[block_size][block_size];
    //these blocks will be mutually used to with A/B_Blocks enhance send receive operations .

        int which=0;
    int (*NEXT_A)[block_size];
    int (*NEXT_B)[block_size];
    int (*CURRENT_A)[block_size];
    int (*CURRENT_B)[block_size];
    //These array pointers will be used to send and receive info beetween processes
        //we are using it like this to increase our efficiency
            //they will either represent A/B_Block or A/B_Block1



    //tags 0 and 1
    
    for(int i=1;i<size_1;i++){      //since first operation is already done there are size_1-1 ops left
        MPI_Status recv_ST[2];
        MPI_Request send_RQ[2];
        //------------------------second-------------------------
        switch(which){
            case 0:             //initial
            NEXT_A=A_Block1;    //info will always come to nexts    
            NEXT_B=B_Block1;
            CURRENT_A=A_Block;
            CURRENT_B=B_Block;
            break;

            case 1:
            NEXT_A=A_Block;
            NEXT_B=B_Block;
            CURRENT_A=A_Block1;
            CURRENT_B=B_Block1;
            break;
        }
        which=!which;

        MPI_Isend(&CURRENT_A[0][0],block_size*block_size,MPI_INT,next_partner_A,2*i,MPI_COMM_WORLD,send_RQ);        //A tags are even
        MPI_Isend(&CURRENT_B[0][0],block_size*block_size,MPI_INT,next_partner_B,2*i+1,MPI_COMM_WORLD,send_RQ+1);      //B tags are odd

        MPI_Recv(&NEXT_A[0][0],block_size*block_size,MPI_INT,previous_partner_A,2*i,MPI_COMM_WORLD,recv_ST);
        MPI_Recv(&NEXT_B[0][0],block_size*block_size,MPI_INT,previous_partner_B,2*i+1,MPI_COMM_WORLD,recv_ST+1);

        // printf("%d 222rank222 (%d %d %d %d %d %d %d %d)\n",rank,NEXT_A[0][0],NEXT_A[0][1],NEXT_A[1][0],NEXT_A[1][1],NEXT_B[0][0],NEXT_B[0][1],NEXT_B[1][0],NEXT_B[1][1]);


            //We are using Isends because we can do other things while waiting for the other process
                //to receive this info
            //But we are not using non-blocking receive because 
                //we will need to use newly acquired A/B blocks right after this point
        //------------------------------------------------------



        //----------------------third---------------------------
        for(int i=0;i<block_size;i++){
            for(int k=0;k<block_size;k++){
                for(int l=0;l<block_size;l++){
                    C_Block[i][k]+=NEXT_A[i][l]*NEXT_B[l][k];
                }
            }
        }
        //------------------------------------------------------
        MPI_Status statusWait[2];
        MPI_Waitall(2,send_RQ,statusWait);
        // MPI_Waitall(1,send_RQ,statusWait);

    }
    // 




    //----------------------------fourth----------------------
    int TAG=size_1*2+rank*block_size;


    for(int i=0;i<block_size;i++){
        MPI_Request req;
        MPI_Isend(&C_Block[i][0],block_size,MPI_INT,0,TAG+i,MPI_COMM_WORLD,&req);
    }

    //first exhaust that process
    if(!rank){      //Receiving for process 0
        for(int i=0;i<size;i++){        //i is process
            for(int k=0;k<block_size;k++){      //k is row from that process's C block

                MPI_Status status;
                MPI_Recv(&C[i/size_1*block_size+k][(i%size_1)*block_size],
                            block_size,MPI_INT,i,
                            size_1*2+i*block_size+k,MPI_COMM_WORLD,&status);
            }
        }
        //algorithm is over now we can write the time
        printf("Parallel time is %f seconds\n",MPI_Wtime()-TIMER);       //writing time

        FILE* file=fopen("output","w");
        print_matrix((int**)C,N,file);
        fclose(file);
    }





   






    







    MPI_Finalize();

}