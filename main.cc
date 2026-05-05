#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "predefined_dictionaries.hpp"
#include "predefined_dictionaries_apriltag.hpp"


// -----------------------------------------------------------------------------
//     aruco database handling
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

	int last_byte_rot = ((db.bits * db.bits) % 8);
	if (last_byte_rot)
		last_byte_rot = 8- last_byte_rot;

	for (i = 0; i < db.size; i++) {
		printf("\t#if (ARUCO_DB_SIZE > %d)\n", i);
		printf("\t\t{ ");
		for (j = 0; j < 4; j++) {
			printf("{ ");
			for (k = 0; k < db.bytes; k++) {
				if (k == db.bytes - 1)
					printf("%d ", *ptr << last_byte_rot);
				else
					printf("%d,", *ptr);
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


static void export_database(void)
{
	int db;

	printf("// This file is based on code from the OpenCV project, converted using the \n"
		"// code in https://github.com/pmarques-dev/aruco-database-converter.\n"
		"// It is subject to the license terms in the LICENSE file found on this folder\n"
		"// and at http://opencv.org/license.html.\n\n"
	);

	for (db = 0; db < 10; db++)
		printf("#define %s	%d\n", databases[db].name, db + 1);
	printf("#define ARUCO_6x6_CODE32	99\n");
	printf("\n");

	printf("#if !defined(ARUCO_DB)\n\n");
	printf("#error you need to select one aruco database by defining ARUCO_DB to be one of:");
	for (db = 0; db < array_count(databases); db++)
		printf(" %s", databases[db].name);
	printf("\n");

	for (db = 0; db < array_count(databases); db++) {
		printf("\n#elif (ARUCO_DB == %s)\n\n", databases[db].name);
		//printf("\n#elif (ARUCO_DB == %d)\n\n", db);
		export_db(databases[db]);
	}

	printf("\n#elif (ARUCO_DB == ARUCO_6x6_CODE32)\n\n");

	printf("#define ARUCO_DB_SIZE	0\n\n");
	printf("#define ARUCO_BITS	6\n\n");
	printf("#define ARUCO_CODE32\n\n");

	printf("\n#else\n\n"
		"#error ARUCO_DB is defined to an invalid value\n\n"
		"#endif\n\n");

	for (db = 0; db < 10; db++)
		printf("#undef %s\n", databases[db].name);
	printf("\n");
}

// -----------------------------------------------------------------------------
//     Aruco6x6_Code32 image generation code

// this code takes an SVG file with one aruco per line with the following format:
//    centerX centerY width code
// all dimensions are in mm. The first line contains the document width and
// height. An example file is something like:
//    110.0 60.0
//    30.0 30.0 40.0 0x12345678
//    80.0 30.0 40.0 0x90abcdef
// coordinates are in SVG space, so 0,0 is the lower left corner

struct aruco_gen_t {
	double x, y, size;
	uint32_t code;

	double bit_size(int bits) {
		return size * (bits * (1.0 / 8));
	}

	double bit_x(int bit) {
		return x + size * (bit * (1.0 / 8) - 0.5);
	}
	double bit_y(int bit) {
		return y + size * (bit * (1.0 / 8) - 0.5);
	}
};

static void output_aruco(FILE *out, aruco_gen_t &g)
{
	// output the black background square
	fprintf(out, "<rect width=\"%g\" height=\"%g\" x=\"%g\" y=\"%g\" fill=\"black\"></rect>",
		g.size, g.size, g.bit_x(0), g.bit_y(0));

	// output the white bits
	uint8_t bits[6][6];
	memset(bits, 0, sizeof(bits));

	// mark the top left corner in white
	bits[0][0] = 1;

	uint32_t bit = 0x80000000;
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			// ignore corners
			if ((i == 0 || i == 5) && (j == 0 || j == 5))
				continue;

			bits[i][j] = ((g.code & bit) != 0);
			bit >>= 1;
		}
	}

	// print the white bits. Print horizontal and vertical bars to make sure
	// the SVG renderer won't produce black lines between bits
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			if (bits[i][j] == 0)
				continue;

			int l = 0;
			while (j < 6 && bits[i][j])
				j++, l++;

			// we don't need to "print" horizontal blocks of size 1
			// if in the other direction we are going to have bigger
			// blocks
			if (l == 1 && ((i > 0 && bits[i-1][j-1]) || (i < 5 && bits[i+1][j-1])))
				continue;

			fprintf(out, "<rect width=\"%g\" height=\"%g\" x=\"%g\" y=\"%g\" fill=\"white\"></rect>",
				g.bit_size(l), g.bit_size(1), g.bit_x(j - l + 1), g.bit_y(i + 1));
		}

		for (int j = 0; j < 6; j++) {
			if (bits[j][i] == 0)
				continue;

			int l = 0;
			while (j < 6 && bits[j][i])
				j++, l++;

			if (l != 1)
				fprintf(out, "<rect width=\"%g\" height=\"%g\" x=\"%g\" y=\"%g\" fill=\"white\"></rect>",
					g.bit_size(1), g.bit_size(l), g.bit_x(i + 1), g.bit_y(j - l + 1));
		}
	}
}


static void generate_file(const char *in_file, const char *out_file)
{
	double doc_width, doc_height;
	char buf[1024];
	aruco_gen_t g;
	std::vector<aruco_gen_t> arucos;

	FILE *in = fopen(in_file, "r");
	if (in == NULL) {
		perror("fopen");
		exit(1);
	}
	FILE *out = fopen(out_file, "w+");
	if (out == NULL) {
		perror("fopen");
		exit(2);
	}

	if (fgets(buf, sizeof(buf), in) == NULL || strlen(buf) < 3) {
		printf("error: empty input file\n");
		exit(3);
	}
	if (sscanf(buf, "%lf %lf", &doc_width, &doc_height) != 2) {
		printf("invalid document dimensions\n");
		exit(4);
	}

	// read the file into a vector in memory
	double min_x = 1e9, max_x = -1e9, min_y = 1e9, max_y = -1e9;
	while (fgets(buf, sizeof(buf), in) != NULL) {
		if (strlen(buf) < 3)
			break;
		if (sscanf(buf, "%lf %lf %lf %x", &g.x, &g.y, &g.size, &g.code) != 4) {
			printf("invalid line: %s\n", buf);
			exit(2);
		}
		arucos.push_back(g);

		if (g.x - g.size * 0.5 < min_x)
			min_x = g.x - g.size * 0.5;
		if (g.y - g.size * 0.5 < min_y)
			min_y = g.y - g.size * 0.5;
		if (g.x + g.size * 0.5 > max_x)
			max_x = g.x + g.size * 0.5;
		if (g.y + g.size * 0.5 > max_y)
			max_y = g.y + g.size * 0.5;
	}

	// output all the arucos
	fprintf(out, "<svg viewBox=\"0 0 %g %g\" xmlns=\"http://www.w3.org/2000/svg\" shape-rendering=\"crispEdges\" width=\"%gmm\" height=\"%gmm\">",
		doc_width, doc_height, doc_width, doc_height);

	for (aruco_gen_t &g: arucos)
		output_aruco(out, g);

	fprintf(out, "</svg>");

	fclose(in);
	fclose(out);
}



int main(int argc, char *argv[])
{
	if (argc == 1)
		export_database();
	else if (argc == 3)
		generate_file(argv[1], argv[2]);
	else
		printf("invalid parameters\n");

	return 0;
}
