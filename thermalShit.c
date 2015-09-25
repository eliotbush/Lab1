//  Emma Bryce - Eliot Bush - Ygor Jean
//  thermalShit.c
//  ECE 353 Lab 1
//  9/25/15


//We need to split this into functions. Just posting something that works so you guys can take a look at the general idea
//I don't think he wants us to call the file "thermalShit" but I didn't see a name specified in the assignment.

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

main(){
	//thermal paramaters file
	FILE *tpfp;
	//power trace file
	FILE *ptfp;
	//output file
	FILE *ofp;

	//open the files
	tpfp = fopen("./thermalParam.txt", "r");
	ptfp = fopen("./powerTrace.txt", "r");
	ofp = fopen("./tempOutput.txt", "w");

	//make sure the files are there
	assert(tpfp != NULL);
	assert(ptfp != NULL);
	assert(ofp != NULL);

	//number of rows in each input file.
	//have to figure out a way to read this automatically
	int thermalParamLength=5;
	int powerTraceLength=500;

	//declare pointer array thingies
	double **thermalParam;
	int i;
	int j;
	thermalParam = (double **) malloc(thermalParamLength*sizeof(double *));
	for(i=0; i<thermalParamLength; i++){
		thermalParam[i] = (double *) malloc(4*sizeof(double));
	}
	double **powerTrace;
	powerTrace = (double **) malloc(powerTraceLength*sizeof(double *));
	for(i=0; i<powerTraceLength; i++){
		powerTrace[i] = (double *) malloc(4*sizeof(double));
	}


	//read shit from file into arrays
	char buff[256];
	double boop;
	for(i=0; i<thermalParamLength; i++){
		for(j=0; j<4; j++){
			fscanf(tpfp, "%s", buff);
			sscanf(buff, "%lf", &boop);
			thermalParam[i][j] = boop;
		}
	}
	for(i=0; i<powerTraceLength; i++){
		for(j=0; j<4; j++){
			fscanf(ptfp, "%s", buff);
			sscanf(buff, "%lf", &boop);
			powerTrace[i][j] = boop;
		}
	}	


	//print print print
	printf("Thermal parameters:\n\n");
	for(i=0; i<thermalParamLength; i++){
		for(j=0; j<4; j++){
			printf("%lf ", thermalParam[i][j]);
		}
		printf("\n");
	}
	printf("\nPower Trace:\n\n");
	for(i=0; i<powerTraceLength; i++){
		for(j=0; j<4; j++){
			printf("%lf ", powerTrace[i][j]);
		}
		printf("\n");
	}

}
