#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include "mpi.h"
#include "utils.h"
#include <math.h>

#define INPUT_FILE "../input.txt"
#define OUTPUT_FILE "../output.txt"

void readFromFile(double *matchingValue, int *pictureAmount, int **pictureIDs,
		int **pictureDims, int **pictures, int *objectAmount, int **objectIDs,
		int **objectDims, int **objects) {
	FILE *filePointer;

	if ((filePointer = fopen(INPUT_FILE, "r")) == 0) {
		printf("cannot open file %s\n", INPUT_FILE);
		exit(0);
	}

	fscanf(filePointer, "%lf", matchingValue);

	fscanf(filePointer, "%d", pictureAmount);
	//After getting the Amount of pictures we can allocate memory for the IDS and Dims we need to read from the file
	*pictureIDs = (int*) malloc((*pictureAmount) * sizeof(int));
	if (*pictureIDs == NULL) {
		printf("Problem to allocate memory\n");
		exit(0);
	}
	*pictureDims = (int*) malloc((*pictureAmount) * sizeof(int));
	if (*pictureDims == NULL) {
		printf("Problem to allocate memory\n");
		exit(0);
	}

	// Reading Pictures
	int size = 0;
	for (int i = 0; i < *pictureAmount; i++) {
		fscanf(filePointer, "%d", *pictureIDs + i);
		fscanf(filePointer, "%d", *pictureDims + i);
		size += *(*pictureDims + i) * (*(*pictureDims + i));
		if (i == 0)
			*pictures = (int*) malloc(size * sizeof(int));
		else
			*pictures = (int*) realloc(*pictures, size * sizeof(int));
		//Reallocate Pictures memory
		if (*pictures == NULL) {
			printf("Problem to reallocate Pictures memory\n");
			exit(0);
		}
		//Reading the Picture
		int startIndex = 0;
		for (int j = 0; j < i; j++) {
			startIndex += *(*pictureDims + j) * (*(*pictureDims + j));
		}
		for (int k = startIndex; k < size; k++) {
			fscanf(filePointer, "%d", *pictures + k);
		}

	}
	//************

	// Reading Objects
	fscanf(filePointer, "%d", objectAmount);
	*objectIDs = (int*) malloc(*objectAmount * sizeof(int));
	if (*objectIDs == NULL) {
		printf("Problem to allocate memory\n");
		exit(0);
	}
	*objectDims = (int*) malloc(*objectAmount * sizeof(int));
	if (*objectDims == NULL) {
		printf("Problem to allocate memory\n");
		exit(0);
	}
	size = 0; // reset size
	for (int i = 0; i < *objectAmount; i++) {
		fscanf(filePointer, "%d", *objectIDs + i);
		fscanf(filePointer, "%d", *objectDims + i);
		size += *(*objectDims + i) * (*(*objectDims + i));
		if (i == 0)
			*objects = (int*) malloc(size * sizeof(int));
		else
			*objects = (int*) realloc(*objects, size * sizeof(int));
		//Reallocate Objects memory
		if (*objects == NULL) {
			printf("Problem to reallocate Objects memory\n");
			exit(0);
		}
		//Reading the Object
		int startIndex = 0;
		for (int j = 0; j < i; j++) {
			startIndex += *(*objectDims + j) * (*(*objectDims + j));
		}
		for (int k = startIndex; k < size; k++) {
			fscanf(filePointer, "%d", *objects + k);
		}
	}
	// ************

	fclose(filePointer);
}

void writeToFile(char* buffer) {
	FILE *filePointer;
	if ((filePointer = fopen(OUTPUT_FILE, "a")) == 0) {
		printf("cannot open file %s\n", OUTPUT_FILE);
		exit(0);
	}
	fprintf(filePointer, buffer);
	fclose(filePointer);
}

void cleanOutputFile(){
	FILE *filePointer;
	if ((filePointer = fopen(OUTPUT_FILE, "w")) == 0) {
		printf("cannot open file %s\n", OUTPUT_FILE);
		exit(0);
	}
	fclose(filePointer);
}

int findObjectInPicture(double matchingValue, int pictureDims, int *picture,
		int objectAmount, int *objectIDs, int *objectDims, int *objects, int *x,
		int *y, int *object_found) {
	int foundFlag = 1; // 1 = not found yet
	int startIndex;
	int matching;
	int i, j, k;

#pragma omp parallel for private(startIndex, matching, i, j, k)
	for (i = 0; i < objectAmount; i++) {
		startIndex = 0;
		matching = 0;
		if (foundFlag == 1) {
			for (j = 0; j < i; j++) { // Calculate starting index for objects
				startIndex += objectDims[j] * objectDims[j];
			}
			for (j = 0; j < pictureDims - objectDims[i] + 1 && foundFlag; j++) // rows in Picture
			{
				for (k = 0; k < pictureDims - objectDims[i] + 1 && foundFlag;
						k++) // Columns in Picture
				{
					matching = checkMatching(objectDims[i], picture,
							objects + startIndex, matchingValue, j, k,
							pictureDims);
					if (matching) {
						*x = j;
						*y = k;
						*object_found = objectIDs[i];
						foundFlag = 0;
					}
				}
			}
		}

	}
	if (foundFlag){
		*object_found = -1;
		*x = -1;
		*y = -1;
	}
	return -1;
}

int findObjectInPictureSequential(double matchingValue, int pictureDims, int *picture,
		int objectAmount, int *objectIDs, int *objectDims, int *objects, int *x,
		int *y, int *object_found) {
	int startIndex=0;
	int matching;
	int i, j, k;
	for (i = 0; i < objectAmount; i++) {
		for (j = 0; j < pictureDims - objectDims[i] + 1; j++) // rows in Picture
		{
			for (k = 0; k < pictureDims - objectDims[i] + 1;k++) // Columns in Picture
			{
				matching = checkMatching(objectDims[i], picture,
						objects + startIndex, matchingValue, j, k,
						pictureDims);
				if (matching) {
					*x = j;
					*y = k;
					*object_found = objectIDs[i];
					return -1;
				}
			}
		}
		startIndex += objectDims[i] * objectDims[i];
	}
	*object_found = -1;
	*x = -1;
	*y = -1;
	return -1;
}

int checkMatching(int objectDim, int *picture, int *object,
		double matchingValue, int row, int col, int pictureDim) {
	double matching = 0;
	for (int i = 0; i < objectDim; i++, row++) {
		int column = col;
		for (int j = 0; j < objectDim; j++, column++) {
			double p = (double) picture[row * pictureDim + column];
			double o = (double) object[i * objectDim + j];
			double result = (p - o) / p;
			result = fabs(result);
			matching += result;
		}
	}
	//printf("matching: %lf \n" ,matching);
	if (matching <= matchingValue)
		return 1;
	else
		return 0;
}
