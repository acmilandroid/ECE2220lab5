/* parsebmp.c
 * Prof. Russell
 * harlanr 
 * ECE 2220, Fall 2016
 *
 * Purpose: Example program to read in a bmp file and check for 
 *          valid structures
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
 *   name of bit mapped image file (bmp file) to read
 *
 * Bugs:
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


/*----------------------------------------------------------*/

int main(int argc, char *argv[])
{ 
    char filein[LINE];
    FILE *fpin;
    struct InfoHeader infoheader;
    struct Header header;
    int expected_bytes;
    int error_code = 0;
    int row, column;
    int pixel_cols, pixel_rows, pixel_count;
    int items_found;
    struct Pixel one_pixel;

    if (argc != 2)
    {
        printf("Usage: parsebmp filename\n");
        exit(1);
    }
    strcpy(filein, argv[1]);

    if ((fpin = fopen(filein, "rb")) == NULL)
    { 
        printf("Cannot Open File. %s\n", filein);
        exit (1);
    }

    /* Read header */
    fread(&header.Type, sizeof(short int), 1, fpin);
    fread(&header.Size, sizeof(int), 1, fpin);
    fread(&header.Reserved1, sizeof(short int), 1, fpin);
    fread(&header.Reserved2, sizeof(short int), 1, fpin);
    fread(&header.Offset, sizeof(int), 1, fpin);

    printf("header.Type = %x\n", header.Type);
    printf("header.Size = %d\n", header.Size);
    printf("header.Offset = %d\n", header.Offset);

    if (header.Type != 0x4D42)
    {
        printf("This does not appear to be a bmp file: %s\n", filein);
        exit(1);
    }
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

    printf("infoheader.Size = %d\n", infoheader.Size);
    printf("infoheader.Width = %d\n", infoheader.Width);
    printf("infoheader.Height = %d\n", infoheader.Height);
    printf("infoheader.Planes = %d\n", infoheader.Planes);
    printf("infoheader.Bits = %d\n", infoheader.Bits);
    printf("infoheader.Compression = %d\n", infoheader.Compression);
    printf("infoheader.ImageSize = %d\n", infoheader.ImageSize);
    printf("infoheader.xResolution = %d\n", infoheader.xResolution);
    printf("infoheader.yResolution = %d\n", infoheader.yResolution);
    printf("infoheader.Colors = %d\n", infoheader.Colors);
    printf("infoheader.ImportantColors = %d\n", infoheader.ImportantColors);

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

    for (row = 0; row < pixel_rows; row++)
    { 
        for (column = 0; column < pixel_cols; column++)
        { 
	    items_found = fread(&one_pixel, 3, 1, fpin);
            if (items_found != 1)
            {
                printf("failed to read pixel %d at [%d][%d]\n", 
                        pixel_count, row, column);
                exit(1);
            }
            // comment out the following print after you 
            // confirm the file is being read correctly.
            printf("%d: R=%u, G=%u, B=%u\n", pixel_count,
                   one_pixel.Red, one_pixel.Green, one_pixel.Blue);
            pixel_count++;
        }
    }
    fclose(fpin);

    return 0;
}
