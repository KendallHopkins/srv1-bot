/* laplaceBlob.c
 * See http://en.wikipedia.org/Blob_detection
 * Created by Eric "Siggy" Scott, 02 February, 2010
 */

#include <jpeglib.h>
#include <stdio.h>
#include <math.h>

float dx2_Gaussian (int sd2, float x, float y)
{ // 2nd partial derivative with respect to x of the 2-Dimensional Gaussian function with standard deviation sqrt(sd2).
	return ((pow(x,2)-sd2)/(2*3.14159*pow(sd2,3))*exp(-(pow(x,2) + pow(y,2))/(2*sd2)));
}

float dy2_Gaussian (int sd2, float x, float y)
{ // 2nd partial derivative with respect to x of the 2-Dimensional Gaussian function with standard deviation sqrt(sd2).
	return ((pow(y,2)-sd2)/(2*3.14159*pow(sd2,3))*exp(-(pow(x,2) + pow(y,2))/(2*sd2)));
}

int main ( void )
{
	int sd2 = pow(4,2);

	//Load JPEG image
	struct jpeg_decompress_struct cinfo; //Set up jpeglib structures
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	FILE* infile = fopen("sample.jpg","rb"); // Open the file
	jpeg_stdio_src(, infile);
	jpeg_read_header(&cinfo, TRUE); // Read image dimensions, etc.
	jpeg_start_decompress(); // Decompress the image
	JSAMPARRAY img* = (JSAMPARRAY) malloc(cinfo.output_width*cinfo.output_height*3); // 3 components per pixel (RGB)
	jpeg_read_scanlines(&cinfo, img, cinfo.output_height);  // Put whole decompressed image in memory

	/* The blob detector works by computing the Laplacian of the convolution of a Gaussian with the image.
	 * If #: convolution operator, _xx: second partial with respect to x, and _yy: second partial with respect to y,
	 * then the Laplacian is:
	 * 	L(g#f) = (g#f)_xx + (g#F)_yy = (g_xx # f) + (g_yy # f)
	 * We choose four standard deviations as the range of the convolution, since anything outside of that distance
	 * from the Gaussian is neglibibly small.
	 */
	convolution_diameter = 4; // Only convolve within 4 standard deviations
	float L = malloc(cinfo.output_width*cinfo.output_height);
	// Iterate through every pixel
	int x, y;
	for (x = 0; x < cinfo.output_width; x++)
	{
		for (y = 0; y < cinfo.output_height; y++)
		{ //Perform the convolution
			L[x][y] = 0;
			int cx, cy;
			for (cx = -convolution_diameter; cx <= convolution_diameter; cx++)
			{
				for (cy = -convolution_diameter; cy <= convolution_diameter; cy++)
				{
					L[x][y] += (dx2_Gaussian(cx,cy,sd2) + dy2_Gaussian(cx,cy,sd2))*img[x+cx][y+cy][0];
				}
			}
			
		}
	}

	// Find the min of L (corresponds to bright blob) and print its location
	float min = L[0][0];
	int min_x = 0;
	int min_y = 0;
	for (x = 0; x < cinfo.output_width; x++)
	{
		for (y = 0; y < cinfo.output_height; y++)
		{
			if (L[x][y] < min)
			{
				min = L[x][y];
				min_x = x;
				min_y = y;
			}
		}
	}

	printf("Best spot: %f at (%d,%d)", min, min_x, min_y);

}
