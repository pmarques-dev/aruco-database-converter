#include <stdio.h>

// simple code that can be used to test the database.h file to see if it
// compiles as expected

#define ARUCO_DB	ARUCO_DB_6X6_1000
#define ARUCO_DB_SIZE	50

#include "database.h"

int main(void)
{
	printf("%d\n", database[0][0][0]);
	return 0;
}
