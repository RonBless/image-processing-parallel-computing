
#ifndef UTILS_H_
#define UTILS_H_

void readFromFile(double* matchingValue, int* pictureAmount, int** pictureIDs, int** pictureDims, int** pictures,
		int* objectAmount, int** objectIDs, int** objectDims, int** objects);

int findObjectInPicture(double matchingValue, int pictureDims, int* pictures,
		int objectAmount, int* objectIDs, int* objectDims, int* objects, int* x, int* y, int* object_found);

int findObjectInPictureSequential(double matchingValue, int pictureDims, int *picture,
		int objectAmount, int *objectIDs, int *objectDims, int *objects, int *x,
		int *y, int *object_found);

int checkMatching(int objectDim, int* picture, int* object, double matchingValue,int row,int col,int pictureDim);

void writeToFile(char* buffer);

void cleanOutputFile();

#endif /* UTILS_H_ */
