#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "predefined_dictionaries.hpp"
#include "predefined_dictionaries_apriltag.hpp"

struct db_t {
	const uint8_t *base;
	const char *name;
	int bytes;
	int bits;
	int size, check;
};

#define array_count(a)			((int)(sizeof(a) / (sizeof(a[0]))))

static db_t databases[10] = {
	{ DICT_ARUCO_BYTES[0][0],		"ARUCO_DB_ORIGINAL",		4, 5,	1024,	array_count(DICT_ARUCO_BYTES)		},
	{ DICT_4X4_1000_BYTES[0][0],		"ARUCO_DB_4X4_1000",		2, 4,	1000,	array_count(DICT_4X4_1000_BYTES)	},
	{ DICT_5X5_1000_BYTES[0][0],		"ARUCO_DB_5X5_1000",		4, 5,	1000,	array_count(DICT_5X5_1000_BYTES)	},
	{ DICT_6X6_1000_BYTES[0][0],		"ARUCO_DB_6X6_1000",		5, 6,	1000,	array_count(DICT_6X6_1000_BYTES)	},
	{ DICT_7X7_1000_BYTES[0][0],		"ARUCO_DB_7X7_1000",		7, 7,	1000,	array_count(DICT_7X7_1000_BYTES)	},
	{ DICT_ARUCO_MIP_36h12_BYTES[0][0],	"ARUCO_DB_ARUCO_MIP_36h12",	5, 6,	250,	array_count(DICT_ARUCO_MIP_36h12_BYTES)	},
	{ DICT_APRILTAG_16h5_BYTES[0][0],	"ARUCO_DB_APRILTAG_16h5",	2, 4,	30,	array_count(DICT_APRILTAG_16h5_BYTES)	},
	{ DICT_APRILTAG_25h9_BYTES[0][0],	"ARUCO_DB_APRILTAG_25h9",	4, 5,	35,	array_count(DICT_APRILTAG_25h9_BYTES)	},
	{ DICT_APRILTAG_36h10_BYTES[0][0],	"ARUCO_DB_APRILTAG_36h10",	5, 6,	2320,	array_count(DICT_APRILTAG_36h10_BYTES)	},
	{ DICT_APRILTAG_36h11_BYTES[0][0],	"ARUCO_DB_APRILTAG_36h11",	5, 6,	587,	array_count(DICT_APRILTAG_36h11_BYTES)	},
};


static void export_db(db_t &db)
{
	const uint8_t *ptr = db.base;
	int i, j, k;

	if (db.size != db.check) {
		printf("ERROR: mismatch %d %d\n", db.size, db.check);
		exit(1);
	}

	printf("#ifndef ARUCO_DB_SIZE\n");
	printf("#define ARUCO_DB_SIZE %d\n", db.size);
	printf("#endif\n\n");

	printf("#define ARUCO_BITS	%d\n\n", db.bits);

	printf("static unsigned char database[ARUCO_DB_SIZE][4][%d] = {\n", db.bytes);

	for (i = 0; i < db.size; i++) {
		printf("\t#if (ARUCO_DB_SIZE > %d)\n", i);
		printf("\t\t{ ");
		for (j = 0; j < 4; j++) {
			printf("{ ");
			for (k = 0; k < db.bytes; k++) {
				printf("%d%c", *ptr, (k == db.bytes - 1) ? ' ' : ',');
				ptr++;
			}
			printf("}, ");
		}
		printf("},\n");
		printf("\t#endif\n");
	}
	printf("\t#if (ARUCO_DB_SIZE > %d)\n", i);
	printf("\t#error invalid ARUCO_DB_SIZE, max database size for this aruco type is %d\n", db.size);
	printf("\t#endif\n\n");

	printf("};\n");
}


int main(void)
{
	int db;

	printf("// This file is based on code from the OpenCV project, converted using the \n"
		"// code in https://github.com/pmarques-dev/aruco-database-converter.\n"
		"// It is subject to the license terms in the LICENSE file found on this folder\n"
		"// and at http://opencv.org/license.html.\n\n"
	);

	for (db = 0; db < 10; db++)
		printf("#define %s	%d\n", databases[db].name, db);
	printf("\n");

	printf("#if !defined(ARUCO_DB)\n\n");
	printf("#error you need to select one aruco database by defining ARUCO_DB to be one of:");
	for (db = 0; db < 10; db++)
		printf(" %s", databases[db].name);
	printf("\n");

	for (db = 0; db < 10; db++) {
		printf("\n#elif (ARUCO_DB == %s)\n\n", databases[db].name);
		//printf("\n#elif (ARUCO_DB == %d)\n\n", db);
		export_db(databases[db]);
	}
	printf("\n#endif\n\n");

	for (db = 0; db < 10; db++)
		printf("#undef %s\n", databases[db].name);
	printf("\n");

	return 0;
}
