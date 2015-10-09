//  Emma Bryce - Eliot Bush - Ygor Jean
//  thermalSimulation.c
//  ECE 353 Lab 1
//  10/7/15
////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////////

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
//n denotes how many functions are in the system; precision of the rk function

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

///////////////////////////////////////////////////////////////////////////////////

//count the number of rows in the file
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
				else{dTdt[i] -= (temps[i]-temps[j])/(thermalParam[4][i]*thermalParam[0][i]);}
			}
		}
		//and add the term from the dissipated power, omega_i/C_i
		dTdt[i] += powerTrace[step][i+1]/thermalParam[0][i];
	}

	
	return dTdt;
}

///////////////////////////////////////////////////////////////////////////
double* ageRate(double t, double *temps){
    int i;
    double *ageDiff;
    ageDiff = (double *) malloc(4*sizeof(double));
    double E_a = 0.8; //Activation Energy
    double K_b = 8.617E-5; //Boltzmann's constant
    double *a;
    a = (double *) malloc(4*sizeof(double));
    double *b;
    b = (double *) malloc(4*sizeof(double));
    for(i=0; i<4; i++){
        a[i] = (double) -E_a/(K_b*temps[i]); //temperature as a dependent variable
        a[i] = exp(a[i]); //intermediate step defining the aging effect at device temperature
        b[i] = (double) -E_a/(K_b* ambientTemp); // Ambient temperature
        b[i] = exp(b[i]); //intermediate step defining the aging effect at ambient
        ageDiff[i] = a[i]/b[i]; //the age rate the device
    }
    return ageDiff;
}
///////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]){

	double h = 0.005; //step size parameter to be passed into rk
	double t = 0;
        


    
    //thermal paramaters file
    FILE *tpfp;
    //power trace file
    FILE *ptfp;
    //output file
    FILE *ofp;
    
    //initialize ambient temperature and resistance
    ambientTemp = 300;
    
    //check to see if an argument was passed for ambient temp
    if(argc==5){
        //if it was, set ambientTemp to it
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
    T = initializeT();
    
    int i;
    double* aP = (double *) malloc(4*sizeof(double)); //age pointer...
    for(i=0; i<4; i++){aP[i] = 1;}
    
    //step through updating the temperature
    int j=0;
    double endTime = powerTrace[powerTraceLength-1][0];
    while (t<=endTime){
        printf("\n\nStep %i:\n", j);
        printf("\nTime: %lf\n", t);
        T = rk(&calculatedTdt, h, t, T, 5);
        aP = ageRate(t, T);
        double *age = rk(&ageRate, h, t, aP, 4);
        for(i=0; i<4; i++){
            printf("T%i: %lf\nA%i: %lf\n", i, T[i], i, age[i]);
        }
        t+=h;
        j++;
    }
}
