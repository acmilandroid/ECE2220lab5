/* lab5.c
 * Basil Lin
 * basill
 * ECE 2220, Fall 2016
 *
 * Purpose: Program to perform edits to .bmp files including edge detection
 *          and rotation, and by doing so, practice using structures,
 * 			dynamic memory allocation, and frees
 *
 * Assumptions: 
 *       The file must be 24-bit color, without compression, and without
 *       a color map.
 *
 *       Some bmp files do not set the ImageSize field.  This code
 *       prints a warning but does not consider this an error since the
 *       size of the image can be calculated from other values.
 *
 * Command line argument
 *   editing command, name of bit mapped image file (bmp file) to read,
 *	 name of edited output file
 *
 * Bugs: None known
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* WARNING: the header is 14 bytes, however on most systems
 * you will find that sizeof(struct Header) is 16 due to alignment
 * Thus trying to read the header with one fread may fail.  So,
 * read each member separately
 */
struct Header
{  
    unsigned short int Type;                 /* Magic identifier            */
    unsigned int Size;                       /* File size in bytes          */
    unsigned short int Reserved1, Reserved2;
    unsigned int Offset;                     /* Offset to image data, bytes */
};

struct InfoHeader
{  
    unsigned int Size;               /* header size in bytes      */
    int Width, Height;               /* Width and height of image */
    unsigned short int Planes;       /* Number of colour planes   */
    unsigned short int Bits;         /* Bits per pixel            */
    unsigned int Compression;        /* Compression type          */
    unsigned int ImageSize;          /* Image size in bytes       */
    int xResolution,yResolution;     /* Pixels per meter          */
    unsigned int Colors;             /* Number of colors         */
    unsigned int ImportantColors;    /* Important colors         */
};

const char Matrix[3][3] = 
{ 
    {  0, -1,  0 },
    { -1,  4, -1 },
    {  0, -1,  0 }
};

#define LINE 256

struct Pixel
{ 
    unsigned char Red, Green, Blue;
}; 

void edgedetect(struct Pixel **raw, struct Pixel **detected, int rows, int cols, int arg);
void rotright(struct Pixel **inputr, struct Pixel **outputr, int rowsr, int colsr);
void rotleft(struct Pixel **inputl, struct Pixel **outputl, int rowsl, int colsl);


/*----------------------------------------------------------*/

int main(int argc, char *argv[])
{ 
	if (argc != 4) { //Checks if there are 4 input arguments
        printf("Not enough input arguments.\n");
        exit(1);
    } else if (strcmp(argv[2], argv[3]) == 0) { //Checks that output is different from input
		printf("Error: Output file name is same as input file name.\n");
		exit(1);
	}

    char filein[LINE];
    FILE *fpin, *fpout;
    struct InfoHeader infoheader;
    struct Header header;
    int expected_bytes;
    int error_code = 0;
    int row, column;
    int pixel_cols, pixel_rows, pixel_count;
    int items_found;
    strcpy(filein, argv[2]);

	/* checks if file can be opened */
    if ((fpin = fopen(filein, "rb")) == NULL) {
        printf("Cannot Open File. %s\n", filein);
        exit (1);
    }

    /* Read header */
    fread(&header.Type, sizeof(short int), 1, fpin);
    fread(&header.Size, sizeof(int), 1, fpin);
    fread(&header.Reserved1, sizeof(short int), 1, fpin);
    fread(&header.Reserved2, sizeof(short int), 1, fpin);
    fread(&header.Offset, sizeof(int), 1, fpin);

    if (header.Type != 0x4D42) { //Checks for bmp file
        printf("This does not appear to be a bmp file: %s\n", filein);
        exit(1);
    }

	/* read infoheader */
    fread(&infoheader.Size, sizeof(int), 1, fpin);
    fread(&infoheader.Width, sizeof(int), 1, fpin);
    fread(&infoheader.Height, sizeof(int), 1, fpin);
    fread(&infoheader.Planes, sizeof(short int), 1, fpin);
    fread(&infoheader.Bits, sizeof(short int), 1, fpin);
    fread(&infoheader.Compression, sizeof(int), 1, fpin);
    fread(&infoheader.ImageSize, sizeof(int), 1, fpin);
    fread(&infoheader.xResolution, sizeof(int), 1, fpin);
    fread(&infoheader.yResolution, sizeof(int), 1, fpin);
    fread(&infoheader.Colors, sizeof(int), 1, fpin);
    fread(&infoheader.ImportantColors, sizeof(int), 1, fpin);

	/* checks for errors */
    if (header.Offset != 54)
    {
        printf("problem with offset.  Cannot handle color table\n");
	error_code +=1;
    }
    if (infoheader.Size != 40)
    {
        printf("Size is not 40, perhaps a bmp format not handled\n");
	error_code +=2;
    }
    if (infoheader.Planes != 1 || infoheader.Compression != 0)
    {
        printf("Planes or Compression format not handled\n");
	error_code +=4;
    }
    if (infoheader.Bits != 24)
    {
        printf("Only 24 bit color handled\n");
	error_code +=8;
    }
    expected_bytes = (infoheader.Width * infoheader.Height * infoheader.Bits)/8;
    if (expected_bytes != infoheader.ImageSize)
    {
        printf("Problem with image size.  Sometimes this field is not set so we will ignore the error.\n");
	error_code +=16;
    }
    if (expected_bytes + 14 + 40 != header.Size)
    {
        printf("Problem with size in header\n");
	error_code +=32;
    }
    if (infoheader.Colors != 0 || infoheader.ImportantColors != 0)
    {
        printf("Cannot handle color map\n");
	error_code +=64;
    }
    if (error_code != 0 && error_code != 16)
    {
	printf("exit with code %x\n", error_code);
	exit(EXIT_FAILURE);
    }

    printf("Reading pixels\n");

    pixel_rows = infoheader.Height;
    pixel_cols = infoheader.Width;
    pixel_count = 0;

	int i;
	char *fileout = (char *)malloc(sizeof(char) * LINE);
	struct Pixel **rgb;
	struct Pixel **output;

	/* Dynamically allocates two-dimensional array of pixels */
	rgb = (struct Pixel **)malloc(pixel_rows * sizeof(struct Pixel *));
	for (i = 0; i < pixel_rows; i++) {
		rgb[i] = (struct Pixel *)malloc(pixel_cols * sizeof(struct Pixel));
	}

	/* allocates array of output pixels depending on command */
	if ((strcmp(argv[1], "rotr") == 0) || (strcmp(argv[1], "rotl") == 0)) {
		output = (struct Pixel **)malloc(pixel_cols * sizeof(struct Pixel *));
		for (i = 0; i < pixel_cols; i++) {
			output[i] = (struct Pixel *)malloc(pixel_rows * sizeof(struct Pixel));
		}
	} else {
		output = (struct Pixel **)malloc(pixel_rows * sizeof(struct Pixel *));
		for (i = 0; i < pixel_rows; i++) {
			output[i] = (struct Pixel *)malloc(pixel_cols * sizeof(struct Pixel));
		}
	}

	/* Reads in pixels */
    for (row = 0; row < pixel_rows; row++) { 
        for (column = 0; column < pixel_cols; column++) { 
	    	items_found = fread(&rgb[row][column], sizeof(struct Pixel), 1, fpin);
            if (items_found != 1)
            {
                printf("failed to read pixel %d at [%d][%d]\n", 
                        pixel_count, row, column);
                exit(1);
            }
        }
    }
	fclose(fpin);

	/* calls function depending on command line argument */
	int argument = 0;
	if (strcmp(argv[1], "edtrunc") == 0) { //truncation algorithm
		argument = 1; //determines which algorithm to use in function
		edgedetect(rgb, output, pixel_rows, pixel_cols, argument);
	}
	if (strcmp(argv[1], "edmag") == 0) { //magnitude and clip algorithm
		edgedetect(rgb, output, pixel_rows, pixel_cols, argument);
	}
	if (strcmp(argv[1], "rotr") == 0) { //clockwise 90 degree rotation
		rotright(rgb, output, pixel_rows, pixel_cols);
	}
	if (strcmp(argv[1], "rotl") == 0) { //counterclockwise 90 degree rotation
		rotleft(rgb, output, pixel_rows, pixel_cols);
	}

	/* writes output header */
	strcpy(fileout, argv[3]);
	fpout = fopen(fileout, "wb");
	fwrite(&header.Type, sizeof(short int), 1, fpout);
	fwrite(&header.Size, sizeof(int), 1, fpout);
	fwrite(&header.Reserved1, sizeof(short int), 1, fpout);
	fwrite(&header.Reserved2, sizeof(short int), 1, fpout);
	fwrite(&header.Offset, sizeof(int), 1, fpout);
	/* write output infoheader */
	fwrite(&infoheader.Size, sizeof(int), 1, fpout);
	if ((strcmp(argv[1], "rotr") == 0) || (strcmp(argv[1], "rotl") == 0)) {
		fwrite(&infoheader.Height, sizeof(int), 1, fpout);
		fwrite(&infoheader.Width, sizeof(int), 1, fpout);
	} else {
		fwrite(&infoheader.Width, sizeof(int), 1, fpout);
		fwrite(&infoheader.Height, sizeof(int), 1, fpout);
	}
	fwrite(&infoheader.Planes, sizeof(short int), 1, fpout);
	fwrite(&infoheader.Bits, sizeof(short int), 1, fpout);
	fwrite(&infoheader.Compression, sizeof(int), 1, fpout);
	fwrite(&infoheader.ImageSize, sizeof(int), 1, fpout);
	fwrite(&infoheader.xResolution, sizeof(int), 1, fpout);
	fwrite(&infoheader.yResolution, sizeof(int), 1, fpout);
	fwrite(&infoheader.Colors, sizeof(int), 1, fpout);
	fwrite(&infoheader.ImportantColors, sizeof(int), 1, fpout);

	/* writes output bitmap */
	if ((strcmp(argv[1], "rotr") == 0) || (strcmp(argv[1], "rotl") == 0)) {
		for (i = 0; i < pixel_cols; i++) {
			fwrite(output[i], sizeof(struct Pixel), pixel_rows, fpout);
		}
	} else {
		for (i = 0; i < pixel_rows; i++) {
			fwrite(output[i], sizeof(struct Pixel), pixel_cols, fpout);
		}
	}
	fclose(fpout);

	/* freeing dynamically allocated memory */
	for (i = 0; i < pixel_rows; i++) {
		free(rgb[i]);
	}
	free(rgb);
	if ((strcmp(argv[1], "rotr") == 0) || (strcmp(argv[1], "rotl") == 0)) {
		for (i = 0; i < pixel_cols; i++) {
			free(output[i]);
		}
	} else {
		for (i = 0; i < pixel_rows; i++) {
			free(output[i]);
		}
	}
	free(output);
	free(fileout);
	
    return 0;
}

/*
 * edgedetect
 * Purpose: algorithm for edge detection, changes rgb values of input file for output
 * Inputs: original bitmap pointer, output bitmap pointer, number of rows in bitmap, number of columns in bitmap, algorithm type
 * Outputs: none, ouput is pointer to output Pixel structure
 */
void edgedetect(struct Pixel **raw, struct Pixel **detected, int rows, int cols, int arg) {
	int r, g, b, i, j, k, m, c1, c2, c3, c4;
	const char *charptr = 0;
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			r = 0;
			g = 0;
			b = 0;
			c1 = 0;
			c2 = 0;
			c3 = 0;
			c4 = 0;
			/* Corrects for edges and corners */
			if (i == 0 && j == 0) {
				c1 = 1;
				c3 = 1;
			} else if (i == 0 && j == cols - 1) {
				c1 = 1;
				c4 = 1;
			} else if (i == rows - 1 && j == cols -1) {
				c2 = 1;
				c4 = 1;
			} else if (i == rows - 1 && j == 0) {
				c2 = 1;
				c3 = 1;
			} else if (i == 0) {
				c1 = 1;
			} else if (i == rows - 1) {
				c2 = 1;
			} else if (j == 0) {
				c3 = 1;
			} else if (j == cols - 1) {
				c4 = 1;
			}
			/* Finds new values for r, g, and b */
			for (k = c1; k < sizeof(Matrix[0]) - c2; k++) {
				for (m = c3; m < sizeof(Matrix[0]) - c4; m++) {
					charptr = &Matrix[k][m];
					r += charptr[0] * (int)(raw[-1+i+k][-1+j+m].Red);
					if (arg == 1) { //First algorithm
						r &= 0xFF;
					} else { //Second algorithm
						r = abs(r);
						if (r > 255) {
							r = 255;
						}
					}
					g += charptr[0] * (int)(raw[-1+i+k][-1+j+m].Green);
					if (arg == 1) {
						g &= 0xFF;
					} else {
						g = abs(g);
						if (g > 255) {
							g = 255;
						}
					}
					b += charptr[0] * (int)(raw[-1+i+k][-1+j+m].Blue);
					if (arg == 1) {
						b &= 0xFF;
					} else {
						b = abs(b);
						if (b > 255) {
							b = 255;
						}
					}
				}
			}	
			/* sets final edited bitmap */
			detected[i][j].Red = r;
			detected[i][j].Green = g;
			detected[i][j].Blue = b;
		}
	}
}

/* 
 * rotright
 * Purpose: algorithm that creates an output rotated 90 degrees clockwise
 * Inputs: original bitmap pointer, output bitmap pointer, number of columns, number of rows
 * Outputs: none, output is a pointer to output Pixel structure
 */
void rotright(struct Pixel **inputr, struct Pixel **outputr, int colsr, int rowsr) {
	int i, j;
	for (i = 0; i < rowsr - 1; i++) {
		for (j = 0; j < colsr - 1; j++) {
			outputr[i][j].Red = inputr[j][rowsr - i - 1].Red;
			outputr[i][j].Green = inputr[j][rowsr - i - 1].Green;
			outputr[i][j].Blue = inputr[j][rowsr - i - 1].Blue;
		}
	}
}

/* 
 * rotleft
 * Purpose: algorithm that creates an output rotated 90 degrees counterclockwise
 * Inputs: original bitmap pointer, output bitmap pointer, number of columns, number of rows
 * Outputs: none, output is a pointer to output Pixel structure
 */
void rotleft(struct Pixel **inputl, struct Pixel **outputl, int colsl, int rowsl) {
	int i, j;
	for (i = 0; i < rowsl - 1; i++) {
		for (j = 0; j < colsl - 1; j++) {
			outputl[i][j].Red = inputl[colsl - j - 1][i].Red;
			outputl[i][j].Green = inputl[colsl - j - 1][i].Green;
			outputl[i][j].Blue = inputl[colsl - j - 1][i].Blue;
		}
	}
}

