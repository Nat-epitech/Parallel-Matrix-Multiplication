// Distributed Systems - 2021
// Assignment 3 Part 1 - template
// Nathan Di Palma - 30774242

//includes
#include <iostream>
#include "mpi.h"

//set global variable : world size, node rank, Matrix size
int M_size, rank, size;
//set a tab which allows to interpret the error codes returned by the mediator
const char Tab[3][29] = {{"Error : Not enough arguments"}, {"Error : Wrong argument(s)"}, {"Error : Incorrect matrice(s)"}};

//Method used to print a matrix
void printMatrix(int *matrix)
{
	std::cout << std::endl << "Result:";
	//Print each value of the matrix:
    for (int i = 0; i < M_size * M_size; i++){
		//If the value is the end of a row, print a new line
		if (i % M_size == 0)
			std::cout << std::endl << " " << matrix[i];
		else
        	std::cout << "\t" << matrix[i];
	}
	std::cout << std::endl;
}

//Method which takes in a stripe of A, the matrix B and computes a stripe of C
int *multiplyStripe(int *aa, int *matrix_b){
	//Initiate counter
	int stripe = 0, f = 0, j = 0, sum = 0;
	//Allocate the necessary memory for the new stripe
	int *cc = (int *)malloc(sizeof(int) * M_size*M_size/size);
	//Compute each value of the new stripe
	while (f < M_size*M_size/size){
    	for (int i = 0; i < M_size; i++){
        	for (int n = 0; n < M_size; j += M_size, n++)
            	sum += aa[n + stripe] * matrix_b[j + i]; //Compute the value
    		cc[f++] = sum; //Stock the computed value in the stripe
    		sum = 0;
			j = 0;
    	}
		stripe += M_size;
	}
	//Return the stripe
	return (cc);
}

//Method called by participant
void participant(){
	//Create var to stock the error value returned by mediator
	int status;
	//Get the error code from mediator
	MPI_Bcast(&status, 1, MPI_INT, 0, MPI_COMM_WORLD);
	//If the error code is different of 0, print the error message linked and exit 
	if (status != 0){
		std::cout << "Node [" << rank << "] Interrupted -> " << Tab[status - 1] << std::endl;
		return;
	}
	//Else continue the computation
	std::cout << "Node [" << rank << "] : Matrix and size are correct -> proceed to computation" << std::endl;
	//Get the matrix size sent by the mediator
	MPI_Bcast(&M_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
	//Initiate an empty Matrix B and an empty stripe of A thanks to the matrix size recieved 
	int matrix_b[M_size * M_size];
	int aa[M_size*M_size/size];
	//scatter rows of matrix A (stock it in the empty stripe initialize before)
	MPI_Scatter(nullptr, 0, MPI_INT, aa, M_size*M_size/size, MPI_INT,0,MPI_COMM_WORLD);
	//broadcast matrix B (stock it in the empty matrix initialize before)
	MPI_Bcast(matrix_b, M_size*M_size, MPI_INT, 0, MPI_COMM_WORLD);
	//Compute stripes of C thanks to the fonction multiplyStripe
	int *cc = multiplyStripe(aa, matrix_b);
	//Gather stripes of C
	MPI_Gather(cc, M_size*M_size/size, MPI_INT, nullptr, 0, MPI_INT, 0, MPI_COMM_WORLD);
}

//Method called by Mediator
void mediator(int argc, char *argv[]){
	// Print name and student number
    std::cout << "Nathan Di Palma 3077442" << std::endl;
	//Create var to stock the error value (default = 0)
	int status = 0;
	//If there are less than 4 arguments, returns the error code 1 to all processes
	if (argc < 4){
		status = 1;
		//Print the error message
		std::cout << "Node [0] Error founded -> " << Tab[status - 1] << " -> throwed to all nodes" << std::endl;
		MPI_Bcast(&status, 1, MPI_INT, 0, MPI_COMM_WORLD);
		return;
	}
	//Try to open the files received as arguments
	FILE *input_a = fopen(argv[1], "r");
	FILE *input_b = fopen(argv[2], "r");
	//Try to get the size of the matrix received as an argument
	M_size = atoi(argv[3]);
	//If we cant open files or get the size of the matrix, returns the error code 2 to all processes
	if (!input_a || !input_b || M_size <= 0){
		status = 2;
		//Print the error message
		std::cout << "Node [0] Error founded -> " << Tab[status - 1] << " -> throwed to all nodes" << std::endl;
		MPI_Bcast(&status, 1, MPI_INT, 0, MPI_COMM_WORLD);
		return;
	}
	//Initiate empty matrix A and empty matrix B
	int matrix_a[M_size * M_size];
	int matrix_b[M_size * M_size];
	//Read the opened files and stock the read value into the empty matrix
	int read_A = fread(matrix_a, sizeof(int), M_size * M_size, input_a);
	int read_B = fread(matrix_b, sizeof(int), M_size * M_size, input_b);
	//Close the opened files
	fclose(input_a);
	fclose(input_b);
	//If matrix dont have a good size, returns the error code 3 to all processes
	if (read_A != M_size * M_size || read_B != M_size * M_size || read_A % size != 0)
	{
		status = 3;
		//Print the error message
		std::cout << "Node [0] Error founded -> " << Tab[status - 1] << " -> throwed to all nodes" << std::endl;
		MPI_Bcast(&status, 1, MPI_INT, 0, MPI_COMM_WORLD);
		return;
	}
	//Send the error code 0 to  all processes (means that there is no error)
	MPI_Bcast(&status, 1, MPI_INT, 0, MPI_COMM_WORLD);
	std::cout << "Node [0] : Matrix and size are correct -> proceed to computation" << std::endl;
	//Send the matrix size to all processes
	MPI_Bcast(&M_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
	//Initiate an empty stripe of A thanks to the matrix size
	int aa[M_size*M_size/size];
	//scatter rows of first matrix to different processes     
	MPI_Scatter(matrix_a, M_size*M_size/size, MPI_INT, aa, M_size*M_size/size, MPI_INT,0,MPI_COMM_WORLD);
	//broadcast second matrix to all processes
	MPI_Bcast(matrix_b, M_size*M_size, MPI_INT, 0, MPI_COMM_WORLD);
	//Compute stripes of C thanks to the fonction multiplyStripe
  	int *cc = multiplyStripe(aa, matrix_b);
	//Initiate an empty Matrix C thanks to the matrix size 
	int c[M_size * M_size];
	//Gather stripes of C and stock them into the empty matrix initialize before
	MPI_Gather(cc, M_size*M_size/size, MPI_INT, c, M_size*M_size/size, MPI_INT, 0, MPI_COMM_WORLD);
	//Print the matrix C thanks to fonction printMatrix
	printMatrix(c);
}

//Main method
int main(int argc, char *argv[])
{
	//MPI Initialisation
    MPI_Init(&argc, &argv);
	//Each process gets it's own rank and the world size
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	//The process with the rank 0 become the mediator, others become participants
	if (rank == 0)
		mediator(argc, argv);     
	else
		participant();
	//MPI Finalization
	MPI_Finalize();
	return (0);
}