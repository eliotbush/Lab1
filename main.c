//  Emma Bryce - Eliot Bush - Ygor Jean
//  thermalSimulation.c
//  ECE 353 Lab 1
//  10/09/15

//merge attempt...
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>

/////////////////////////////////////////////////////////////////////////////////////

//globals we'll need
double **thermalParam;
double **powerTrace;
int powerTraceLength;
double ambientTemp;
//equivalent resistance between the cores and ambient. this may need to be an array, need to ask Krishna about it
double ambientR;
/////////////////////////////////////////////////////////////////////////////////////

//Runge-Kutta Algorithm
//This has been modified to solve a system of functions y1(t+h), y2(t+h), ... yn(t+h) given f(y,t)=y' as an input
//n denotes how many functions are in the system

double* rk(double* (*f)(double, double*), double h, double t, double *y, int n){
	//k is a 4xn array storying (k10, k11,...k1n),(k20,...k2n)...(k40...k4n) 
	double **k;
	k = (double **) malloc(4*sizeof(double *));
	int j;
	for(j=0; j<4; j++){k[j] = (double *) malloc(n*sizeof(double));}
	
	//placeholder arrays for intermediate calculations
	double *placeHolderY;
	placeHolderY = (double *) malloc(n*sizeof(double));
	double *placeHolderK;
	placeHolderK = (double *) malloc(n*sizeof(double));

	//placeholder for k1 calculation
	placeHolderK = (*f)(t, y);
	//do k1=h*f(t,y) term by term
	for(j=0; j<n; j++){k[0][j] = h*placeHolderK[j];}

	//do y+k1/2 term by term, store in placeholder
	for(j=0; j<n; j++){placeHolderY[j] = y[j]+k[0][j]/2;}
	//k2 placeholder
	placeHolderK = (*f)((t+h/2), placeHolderY);
	//k2=h*f(t+h/2,y+k1/2) term by term
	for(j=0; j<n; j++){k[1][j] = h*placeHolderK[j];}

	//same process as above for k3
	for(j=0; j<n; j++){placeHolderY[j] = y[j]+k[1][j]/2;}
	placeHolderK = (*f)((t+h/2), placeHolderY);
	for(j=0; j<n; j++){k[2][j] = h*placeHolderK[j];}

	//and k4
	for(j=0; j<n; j++){placeHolderY[j] = y[j]+k[2][j];}
	placeHolderK = (*f)((t+h), placeHolderY);
	for(j=0; j<n; j++){k[3][j] = h*placeHolderK[j];}

	//array to be returned
	double *output;
	output = (double *) malloc(n*sizeof(double));
	//do yi(t+h) = yi(t) + (k1i + 2k2i + 2k3i + k4i)/6 term by term
	for(j=0; j<n; j++){output[j] = y[j] + (k[0][j] + 2*k[1][j] + 2*k[2][j] + k[3][j])/6.0;}
	return output;
}
////////////////////////////////////////////////////////////////////////////////////
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

//take a file and turn it into an 2d array with the right number of rows and columns
double** fileArray(FILE *file, int rows, int columns){
	double **array;
	int i;
	int j;
	array = (double **) malloc(rows*sizeof(double *));
	for(i=0; i<rows; i++){
		array[i] = (double *) malloc(columns*sizeof(double));
	}

	//beep is the "string" we'll end up reading in. not actually a string
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

//initialize the temperatures. just sets them all to ambient
double* initializeT(){
	double *temps;
	temps = (double *) malloc(5*sizeof(double));
	int i;
	for(i=0; i<5; i++){temps[i] = ambientTemp;}
	return temps;
}

//////////////////////////////////////////////////////////////////////////

//calculate dT/dt at T,t
double* calculatedTdt(double t, double *temps){	
	//dTdt[i] = dT_i/dt
	double *dTdt;
	dTdt = (double *) malloc(5*sizeof(double));
	int i;
	//initialize dT/dt as 0 for all i
	for(i=0; i<5; i++){dTdt[0]=0.0;}

	//work out which entry in the power trace we're currently on
	//go through the power trace until the time entry in the trace exceeds the current time and save which step it's after
	int step = 0;
	i=0;
	while (i<powerTraceLength){
		if (powerTrace[i][0]>t){
			step = i-1;
			i = powerTraceLength;
		}
		else{i++;}
	}

	int j;
	//iterate over each core
	for(i=0; i<4; i++){
		//iterate over each other core (note i!=j) and the ambient (j=4)
		for(j=0; j<5; j++){
			if(i!=j){
				//dT_i/dt = -sum(j=0..3, i!=j) T_i-T_j/R_ijC_i
				if(j!=4){dTdt[i] -= (temps[i]-temps[j])/(thermalParam[i][j]*thermalParam[0][i]);}
				//and the ambient term T_i-T_amb/R_ambC_i
				else{dTdt[i] -= (temps[i]-temps[j])/(ambientR*thermalParam[0][i]);}
			}
		}
		//and add the term from the dissipated power, omega_i/C_i
		dTdt[i] += powerTrace[step][i+1]/thermalParam[0][i];
	}

	
	return dTdt;
}

///////////////////////////////////////////////////////////////////////////
double rate(double x, double y){
    return x * sqrt(y);
}

    
////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]){

	double h = 0.005; //step size parameter to be passed into rk
	double t = 0;

    double *y;
    double x;
    double y2;
    int b;
    int n = 1 + (x1 - x0)/h;
	
    printf("x\ty\n");
    
    
    y = malloc(sizeof(double) * n);
    
    for (y[0] = 1, b = 1; b < n; b++)
        y[b] = rk(rate, h, x0 + h * (b - 1), y[b-1]);
    
    printf("x\ty\t\n------------\n");
    for (b = 0; b < n; b += 10) {
        x = x0 + h * b;
        y2 = pow(x * x / 4 + 1, 2);
        printf("%g\t%g\t\n", x, y[b]);
    }
    
	//thermal paramaters file
	FILE *tpfp;
	//power trace file
	FILE *ptfp;
	//output file
	FILE *ofp;

	double ambientTemp=300;
	if(argc==5){
	sscanf(argv[3], "%lf", &ambientTemp);
	//if there was, the output file is argument 4
	ofp = fopen(argv[4], "w");}

	//open the files
	tpfp = fopen(argv[1], "rb");
	ptfp = fopen(argv[2], "rb");
	//if there wasn't an ambient temp argument, output file is argument 3
	if(ofp==NULL){ofp = fopen(argv[3], "w");}

	//make sure the files are there
	assert(tpfp != NULL);
	assert(ptfp != NULL);
	assert(ofp != NULL);

	//number of rows in each input file
	int thermalParamLength=row_count(tpfp);
	powerTraceLength=row_count(ptfp);

	//read the files into pointer arrays for thermal param and power trace
	thermalParam = fileArray(tpfp, thermalParamLength, 4);
	powerTrace = fileArray(ptfp, powerTraceLength, 5);

	//T holds the temperatures. cores 0-3 are T[0]-T[3], T[4] is ambient
	double *T;
	T = initializeT(ambientTemp);

	//step through updating the temperature
	int i;
	int j=0;
	double endTime = powerTrace[powerTraceLength-1][0];
	while (t<=endTime){
		printf("\n\nstep %i:\n", j);
		printf("\ntime: %lf\n", t);
		T = rk(&calculatedTdt, h, t, T, 5);
		for(i=0; i<4; i++){
			printf("T%i: %lf\n", i, T[i]); 
		}
		t+=h;
		j++;
	}
}
