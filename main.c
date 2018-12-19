#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

int MatARowsCount,MatAColumnsCount;
int MatBRowsCount,MatBColumnsCount;
int **MatA, **MatB, **outputMatrixProcedure1, **outputMatrixProcedure2;
sem_t mutex;
double timeSpentOfProcedure1, timeSpentOfProcedure2;

struct entryCordinates
{
    int rowNumber;
    int columnNumber;
};

void readInputFile()
{
    FILE* inputFile = fopen("input.txt","r"); //open the input file to be read

    /****************** READ FIRST MATRIX ELEMENTS ***************/

    fscanf(inputFile,"%d",&MatARowsCount); // scan 1st matrix rows count
    fscanf(inputFile,"%d",&MatAColumnsCount); // scan 1st matrix columns count

    /**allocating memory space for 1st matrix and scaning its elements**/

    MatA = malloc(MatARowsCount* sizeof(int *));

    for(int i = 0; i < MatARowsCount ; i++)
    {
        MatA[i] = malloc(MatAColumnsCount*sizeof(int));
        for(int j = 0; j < MatAColumnsCount ; j++)
            fscanf(inputFile,"%d",&MatA[i][j]);
    }

    /****************** READ SECOND MATRIX ELEMENTS ***************/

    fscanf(inputFile,"%d",&MatBRowsCount); // scan 2nd matrix rows count
    fscanf(inputFile,"%d",&MatBColumnsCount); // scan 2nd matrix columns count

    /**allocating memory space for 2nd matrix and scaning its elements**/

    MatB = malloc(MatBRowsCount* sizeof(int *));

    for(int i = 0; i < MatBRowsCount ; i++)
    {
        MatB[i] = malloc(MatBColumnsCount*sizeof(int));
        for(int j = 0; j < MatBColumnsCount ; j++)
            fscanf(inputFile,"%d",&MatB[i][j]);
    }

    fclose(inputFile);
}


/** allocating memory space for 1st procedure output matrix & 2nd procedure output matrix
    and set their elements by zero**/

void initializeOutputMatrices()
{
    outputMatrixProcedure1 = malloc(MatARowsCount* sizeof(int *));
    for(int i = 0; i < MatARowsCount ; i++)
        outputMatrixProcedure1[i] = (int *)calloc(MatBColumnsCount,sizeof(int));

    outputMatrixProcedure2 = malloc(MatARowsCount* sizeof(int *));
    for(int i = 0; i < MatARowsCount ; i++)
        outputMatrixProcedure2[i] = (int *)calloc(MatBColumnsCount,sizeof(int));
}

/** function will be executed by each thread of 1st procedure to calculate the element that the thread
    must calculate for the output matrix **/

void *evaluateOutputMatrixElement(void *threadArgument)
{
    struct entryCordinates *cordinates = (struct entryCordinates *)threadArgument;
    sem_wait(&mutex); // the comming codes will be executed only by one thread  until making post for the semaphore

    //calculate the output matrix element at the specific row & column number
    for(int i = 0 ; i < MatAColumnsCount; i++)
        outputMatrixProcedure1[cordinates->rowNumber][cordinates->columnNumber] +=
            MatA[cordinates->rowNumber][i]*MatB[i][cordinates->columnNumber];

    sem_post(&mutex);// let any thread wants to execute the previous codes to execute them
}
/** procedure one require to the computation of each element of the output matrix happens in a thread**/

void procedureOne()
{
    // define array of thread with size = 1st matrix rows count * 2nd matrix columns number
    pthread_t threadsArray[MatARowsCount * MatBColumnsCount];
    int counter = 0;

    //create each thread and set thread start routine and its arguments (row and column number)
    for(int rowNumber = 0 ; rowNumber< MatARowsCount; rowNumber++)
    {
        for(int columnNumber = 0; columnNumber < MatBColumnsCount; columnNumber++)
        {
            //define struct to set the row and column number in it and set theam to the create function as one argument
            struct entryCordinates *cordinates = malloc(sizeof(struct entryCordinates));
            cordinates->rowNumber = rowNumber;
            cordinates->columnNumber = columnNumber;
            pthread_create(&threadsArray[counter++], NULL, evaluateOutputMatrixElement, (void*) cordinates);

        }
    }

    //wait for all the created thread in array threadsArray to finish their jobs to make the main thread to continue its job
    for(int i = 0; i< MatARowsCount*MatBColumnsCount; i++)
        pthread_join(threadsArray[i], NULL);
}

/** function will be executed by each thread of 2nd procedure to calculate the elements of the row that the thread
    must calculate for the output matrix **/

void *evaluateOutputMatrixRow(void* threadArgument)
{
    int *rowNumber = (int*) threadArgument; // extract argument that was send when creating the thread
    int temp = rowNumber;
    sem_wait(&mutex); // the comming codes will be executed only by one thread  until making post for the semaphore

    //calculate the output matrix row elements at the specific row number
    for(int i = 0; i < MatAColumnsCount; i++)
    {
        for(int j = 0; j < MatBColumnsCount; j++)
        {
            outputMatrixProcedure2[temp][j] += MatA[temp][i] * MatB[i][j];
        }
    }
    sem_post(&mutex);// let any thread wants to execute the previous codes to execute them
}

/** procedure two require to the computation of each row of the output matrix happens in a thread**/

void procedureTwo()
{
    // define array of thread with size = 1st matrix rows count
    pthread_t threadsArray[MatARowsCount];
    int *rowNumber = 0;

    //create each thread and set thread start routine and its arguments (row number)
    for(int i = 0; i < MatARowsCount; i++)
    {
        pthread_create(&threadsArray[i], NULL, evaluateOutputMatrixRow, (void*) rowNumber);
        int temp = rowNumber;
        temp++;
        rowNumber = temp;
    }

     //wait for all the created thread in array threadsArray to finish their jobs to make the main thread continue its job
    for(int i = 0; i < MatARowsCount; i++)
        pthread_join(threadsArray[i], NULL);

}

/**write the output matrix of procedure one and two in output file**/
void writeOutputFile()
{
    FILE* outputFile = fopen("output.txt","w"); //open the input file to be written

    //write procedure one output matrix
    for(int i = 0; i < MatARowsCount; i++)
    {
        for(int j = 0; j < MatBColumnsCount; j++)
        {
            fprintf(outputFile,"%d ",outputMatrixProcedure1[i][j]);
        }
        fprintf(outputFile,"\n");
    }
    fprintf(outputFile,"\nEND1    %f\n\n",timeSpentOfProcedure1); //write procedure one spent time

    //write procedure two output matrix
    for(int i = 0; i < MatARowsCount; i++)
    {
        for(int j = 0; j < MatBColumnsCount; j++)
        {
            fprintf(outputFile,"%d ",outputMatrixProcedure2[i][j]);
        }
        fprintf(outputFile,"\n");
    }
    fprintf(outputFile,"\nEND2    %f",timeSpentOfProcedure2); //write procedure two spent time

    fclose(outputFile);
}

int main()
{

    sem_init(&mutex,0,1); //initialize the semaphore "murex"
    readInputFile(); // read input data
    initializeOutputMatrices(); // initialize output matrices and set their elements by zero

    //evaluate procedure one spent time
    clock_t start,end;
    start = clock();
    procedureOne(); //strat procedure ine
    end = clock();
    timeSpentOfProcedure1 = (double)(end - start) / CLOCKS_PER_SEC;

    //evaluate procedure two spent time
    start = clock();
    procedureTwo(); //start procedure two
    end = clock();
    timeSpentOfProcedure2 =(double)(end - start) / CLOCKS_PER_SEC;

    free(MatA);
    free(MatB);

    writeOutputFile(); //write output matrices

    free(outputMatrixProcedure1);
    free(outputMatrixProcedure2);

    return 0;
}
