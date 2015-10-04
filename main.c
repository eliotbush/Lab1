//  Emma Bryce - Eliot Bush - Ygor Jean
//  thermalShit.c
//  ECE 353 Lab 1
//  9/25/15
//test - push

//I don't think he wants us to call the file "thermalShit" but I didn't see a name specified in the assignment.
//test
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>

//Runge-Kutta Algorithm
// 1. we need to define the differential of the effective age with respect to temperature before we can use this.
// 2. we need to define the differential of the effective temperature with respect to time before we can use that...
//defined h as step size, t as time and y as the ith order output...
double rk(double(*f)(double, double), double h, double t, double y){
    double	k1 = h * f(t, y),
    k2 = h * f(t + h / 2, y + k1 / 2),
    k3 = h * f(t + h / 2, y + k2 / 2),
    k4 = h * f(t + h, y + k3);
    return y + (k1 + 2 * k2 + 2 * k3 + k4) / 6;
}
///////////////////////////////////////////////////////////////////////////////////
int row_count(FILE *file){
       
        int count = 1;
        char test;
    
        while(!feof(file))
    {
        //get the current character
        test = fgetc(file);
        //if it's a line break, count it as a new row
        if(test == '\n'){count++;}
    
    }
	fseek(file, 0L, SEEK_SET);
        return count;
    }

///////////////////////////////////////////////////////////////////////////////////

double** fileArray(FILE *file, int rows, int columns){
	double **array;
	int i;
	int j;
	array = (double **) malloc(rows*sizeof(double *));
	for(i=0; i<rows; i++){
		array[i] = (double *) malloc(columns*sizeof(double));
	}

	//beep is the "string" we'll end up reading in. not actually a string (I didn't even import the lib)
	char beep[256];
	//boop is the double we'll convert beep into
	double boop;
	//iterate over rows
	for(i=0; i<rows; i++){
		//iterate over columns
		for(j=0; j<columns; j++){
			//fscanf gets the next string (really a char array) in the file, where spaces are the delimiters.
			fscanf(file, "%s", beep);
			//this turns the string we read into a double
			sscanf(beep, "%lf", &boop);
			array[i][j] = boop;
		}
	}
	return array;
}

///////////////////////////////////////////////////////////////////////////////

double* initializeT(double ambient){
	double *temps;
	temps = (double *) malloc(5*sizeof(double));
	int i;
	for(i=0; i<5; i++){temps[i] = ambient;}
	return temps;
}

//////////////////////////////////////////////////////////////////////////

double* updateT(double *temps, double **param, double *trace, double h, double ambientR){
	double *dTdt;
	dTdt = (double *) malloc(4*sizeof(double));
	int i;
	for(i=0; i<4; i++){dTdt[0]=0.0;}
	int j;
	for(i=0; i<4; i++){
		for(j=0; j<5; j++){
			if(i!=j){
				if(j!=4){dTdt[i] -= (temps[i]-temps[j])/(param[i][j]*param[0][i]);}
				else{dTdt[i] -= (temps[i]-temps[j])/(ambientR*param[0][i]);}
			}
		}
		dTdt[i] += trace[i+1]/param[0][i];
	}
	for(i=0; i<4; i++){temps[i] += h * dTdt[i];}
	return temps;
}

///////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]){

	double h= 0.05; //step size parameter to be passed into rk
	double x0 = 0; //core 0
	double x1 = 3; //core 3

	//thermal paramaters file
	FILE *tpfp;
	//power trace file
	FILE *ptfp;
	//output file
	FILE *ofp;

	double ambientTemp=300;
	if(argc=5){
	sscanf(argv[3], "%lf", &ambientTemp);
	ofp = fopen(argv[4], "w");}

	//open the files
	tpfp = fopen(argv[1], "rb");
	ptfp = fopen(argv[2], "rb");
	if(ofp==NULL){ofp = fopen(argv[3], "w");}

	//make sure the files are there
	assert(tpfp != NULL);
	assert(ptfp != NULL);
	assert(ofp != NULL);

	//number of rows in each input file. initialized as 1 because otherwise it won't count the last row
	int thermalParamLength= row_count(tpfp);
	int powerTraceLength=row_count(ptfp);

	//declare pointer array thingies
	//note that they're 2d. indexing is done by array[row][column]
	//so for example if you want the power of node 2 at timestep 441, it's powerTrace[2][441]
	double **thermalParam = fileArray(tpfp, thermalParamLength, 4);
	double **powerTrace = fileArray(ptfp, powerTraceLength, 5);

	double *T;
	T = initializeT(ambientTemp);

	//print and test stuff for temperature function
	printf("\nAmbient Temp:\n\n%lf\n\n", ambientTemp);
	int i;
	for(i=0; i<4; i++){
		printf("T%i: %lf\n", i, T[i]); 
	}
	int j;
	for(j=0; j<100; j++){
		printf("\n\nstep %i:\n", j);
		T = updateT(T, thermalParam, powerTrace[j], h, 10.0);
		for(i=0; i<4; i++){
			printf("T%i: %lf\n", i, T[i]); 
		}
	}

/*
	//more print stuff
	printf("Thermal parameters:\n\n");
	for(i=0; i<thermalParamLength; i++){
		for(j=0; j<4; j++){
			printf("%lf ", thermalParam[i][j]);
		}
		printf("\n");
	}
	printf("\nPower Trace:\n\n");
	for(i=0; i<powerTraceLength; i++){
		for(j=0; j<5; j++){
			printf("%lf ", powerTrace[i][j]);
		}
		printf("\n");
	}
	printf("\nAmbient Temp:\n\n%lf\n\n", ambientTemp);
	for(i=0; i<4; i++){
		printf("T%i: %lf\n", i, T[i]); 
	}
*/
}
