#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mpi.h"
#include "utils.h"
#include <math.h>

int main(int argc, char *argv[]) {
	int my_rank; /* rank of process */
	int p; /* number of processes */
	int tag = 0; /* tag for messages */
	MPI_Status status; /* return status for receive */
	char buffer[100];
	// Object and pictures parameters
	double matchingValue;
	int pictureAmount;
	int *pictureIDs;
	int *pictureDims;
	int *pictures;
	int objectAmount;
	int *objectIDs;
	int *objectDims;
	int *objects;
	int size; // size to send and receive from master
	int source; // Sender
	int objectsSize = 0;
	int picturesSent = 0; // will be our index to if we finish going through all the pictures
	int slaveFlag = 1;
	int object_found= -1; // -1 = not found
	int x, y; // cords where object was found
	int pictureId; // Current picture
	double startTime = 0;
	// end of parameters
	/* start up MPI */

	MPI_Init(&argc, &argv);

	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &p);


	if (my_rank == 0) { // Master does not take part in the work Only Manage the slaves
		cleanOutputFile();
		readFromFile(&matchingValue, &pictureAmount, &pictureIDs, &pictureDims,
				&pictures, &objectAmount, &objectIDs, &objectDims, &objects);
		int startIndex = 0;
		startTime=MPI_Wtime();
		if( p == 1)
		{
			for (int i = 0; i < pictureAmount; i++) {
				if(i > 0)
					startIndex = pictureDims[i-1] * pictureDims[i-1];
				pictures = pictures + startIndex;
				findObjectInPictureSequential(matchingValue, pictureDims[i], pictures,
						objectAmount, objectIDs, objectDims, objects, &x, &y,
						&object_found);
				pictureId = pictureIDs[i];
				if(object_found >= 0){
					snprintf(buffer, sizeof(buffer),"Picture %d: found object %d at (%d, %d)\n" ,pictureId, object_found, x , y);
				}else{
					snprintf(buffer, sizeof(buffer),"Picture %d: No Objects were found\n" ,pictureId);
				}
				writeToFile(buffer);
			}
		}else{
			MPI_Bcast(&matchingValue, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
			MPI_Bcast(&objectAmount, 1, MPI_INT, 0, MPI_COMM_WORLD);
			MPI_Bcast(objectIDs, objectAmount, MPI_INT, 0, MPI_COMM_WORLD);
			MPI_Bcast(objectDims, objectAmount, MPI_INT, 0, MPI_COMM_WORLD);
			for (int i = 0; i < objectAmount; i++)
				objectsSize += objectDims[i] * objectDims[i];
			MPI_Bcast(&objectsSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
			MPI_Bcast(objects, objectsSize, MPI_INT, 0, MPI_COMM_WORLD);

			for (int i = 1; i < p && i <= pictureAmount; i++, picturesSent++) {
				size = pictureDims[picturesSent] * pictureDims[picturesSent];
				MPI_Send(&size, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
				MPI_Send(pictureIDs + (i - 1), 1, MPI_INT, i, tag, MPI_COMM_WORLD);
				MPI_Send(pictures + startIndex, size, MPI_INT, i, tag,
						MPI_COMM_WORLD);
				startIndex += pictureDims[i-1] * pictureDims[i-1];
			}
			int pictrueRecieved = 0;
			while(pictrueRecieved++ < pictureAmount){ // Run and send the next picture when a pc becomes available
				MPI_Recv(&source, 1, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status); // receive sender ID
				MPI_Recv(&object_found, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &status);
				MPI_Recv(&pictureId, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &status);
				if(object_found >= 0){
					MPI_Recv(&x, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &status);
					MPI_Recv(&y, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &status);
					snprintf(buffer, sizeof(buffer),"Picture %d: found object %d at (%d, %d)\n" ,pictureId, object_found, x , y);
				}else{
					snprintf(buffer, sizeof(buffer),"Picture %d: No Objects were found\n" ,pictureId);
				}
				writeToFile(buffer);
				if(picturesSent >= pictureAmount){
					slaveFlag = 0;
				}
				MPI_Send(&slaveFlag, 1, MPI_INT, source, tag, MPI_COMM_WORLD);
				if(pictureAmount > picturesSent){
					size = pictureDims[picturesSent] * pictureDims[picturesSent];
					MPI_Send(&size, 1, MPI_INT, source, tag, MPI_COMM_WORLD);
					MPI_Send(pictureIDs+(picturesSent), 1, MPI_INT, source, tag, MPI_COMM_WORLD);
					MPI_Send(pictures+startIndex, size, MPI_INT, source, tag, MPI_COMM_WORLD);
					startIndex += pictureDims[picturesSent]*pictureDims[picturesSent];
					picturesSent++;
				}
			}
		}
		double endTime=MPI_Wtime();
		printf("runTime=%f Seconds\n", endTime-startTime);

	} else {
		MPI_Bcast(&matchingValue, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		MPI_Bcast(&objectAmount, 1, MPI_INT, 0, MPI_COMM_WORLD);

		objectIDs = (int*) malloc(sizeof(int) * objectAmount);
		if (objectIDs == NULL) {
			printf("objectIDs Malloc Failed");
			exit(0);
		}
		MPI_Bcast(objectIDs, objectAmount, MPI_INT, 0, MPI_COMM_WORLD);

		objectDims = (int*) malloc(sizeof(int) * objectAmount);
		if (objectDims == NULL) {
			printf("objectDIms Malloc Failed");
			exit(0);
		}
		MPI_Bcast(objectDims, objectAmount, MPI_INT, 0, MPI_COMM_WORLD);

		MPI_Bcast(&objectsSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
		objects = (int*) malloc(sizeof(int) * objectsSize);
		if (objects == NULL) {
			printf("objects Malloc Failed");
			exit(0);
		}
		MPI_Bcast(objects, objectsSize, MPI_INT, 0, MPI_COMM_WORLD);

		while (slaveFlag) {
			MPI_Recv(&size, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
			MPI_Recv(&pictureId, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
			pictures = (int*) realloc(pictures, size * sizeof(int));
			if (pictures == NULL) {
				printf("pictures realloc Failed");
				exit(0);
			}
			MPI_Recv(pictures, size, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);

			findObjectInPicture(matchingValue, sqrt(size), pictures,
					objectAmount, objectIDs, objectDims, objects, &x, &y,
					&object_found);

			MPI_Send(&my_rank, 1, MPI_INT, 0, tag, MPI_COMM_WORLD); // send sender ID
			MPI_Send(&object_found, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
			MPI_Send(&pictureId, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
			if (object_found >= 0) {
				MPI_Send(&x, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
				MPI_Send(&y, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
			}
			MPI_Recv(&slaveFlag, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
		}
	}

	MPI_Finalize();
	return 0;
}
