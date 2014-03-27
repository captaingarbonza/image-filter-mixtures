#ifndef _IMAGE_PROCESSING_FUNCTIONS_H_
#define _IMAGE_PROCESSING_FUNCTIONS_H_

#include <QtWidgets>
#include <vector>
#include <math.h>

class ImageProcessing
{
	public:
		static double ColorDistance( QColor color1, QColor color2);

		static std::vector<QPoint> GetPoissonDisks(int width, int height, int minDist);

		static void HorizontalConvo( uchar* source, uchar* destination, int width, int height, int channels, double* kernel, int kernel_size );
		static void VerticalConvo( uchar* source, uchar* destination, int width, int height, int channels, double* kernel, int kernel_size );
		static void TwoDConvo( uchar* source, uchar* destination, int width, int height, int channels, double* kernel, int kernel_size );

		static void BoxBlur( uchar* source, uchar* destination, int width, int height, int channels, int kernel_size = 5 );
		static void GaussianBlur( uchar* source, uchar* destination, int width, int height, int channels, int kernel_size = 5, double sigma = 1.5 );

		static void SobelEdgeDetection( uchar* source, uchar* gradient_magnitude, int width, int height, int channels );
		static void SobelEdgeDetection( uchar* source, uchar* gradient_magnitude, uchar* gradient_direction, int width, int height, int channels );
		static void CannyEdgeDetection( uchar* source, uchar* edges, int width, int height, int channels, int gaussian_kernel_size = 5, double sigma = 1.5, int max_threshold = 80, int min_threshold = 20 );

		static void ConvertToOneChannel( uchar* source, uchar* destination, int width, int height, int channels = 4, int alpha_channel = 3);
		static void ConvertFromOneChannel( uchar* source, uchar* destination, int width, int height, int channels = 4, int alpha_channel = 3);
		//static void RGBToHSV( double r, double g, double b, double* hue, double* sat, double* val );
		//static void HSVToRGB( double hue, double sat, double val, double* r, double* g, double* b );

		static void AddImages(uchar* image1, uchar* image2, uchar* result, int width, int height, int channels = 4);
		static void AddImages(double* image1, double* image2, double* result, int width, int height, int channels = 4);
};

#endif