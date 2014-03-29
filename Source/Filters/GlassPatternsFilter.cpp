#include "GlassPatternsFilter.h"
#include "HelperFunctions/ImageProcessing.h"

const double GlassPatternsFilter::FILTER_STRENGTH_DEFAULT = 1.0;

void ApplyGlassPatterns(QImage * source, QImage * destination, int a, double sd, double theta, int n, double h, double strength);
void TranslateImageAccordingToGlassPattern( uchar* image, uchar* noise, double* v_x, double* v_y, int width, int height, int n, double h );
void TranslatePixels(uchar* image, uchar* canvas, double* v_x, double* v_y, int width, int height, int channels, double h);
void TranslatePixels(double* image, double* canvas, double* v_x, double* v_y, int width, int height, int channels, double h);
void GetImageGradients(QImage * image, double* x_sigma, double* y_sigma, double sd);
void GetVectorField( uchar* source, double* v_x, double* v_y, int width, int height, int a, double th0, double sd);
double WhiteNoise();
void GetRandomNoise( uchar* destination, int width, int height );

#define PI 3.14159265

GlassPatternsFilter::GlassPatternsFilter()
{

}

QImage*
GlassPatternsFilter::RunFilter( QImage* source )
{
	const int filter_strength = FILTER_STRENGTH_DEFAULT;

	QImage* canvas = new QImage(source->size(), QImage::Format_ARGB32);
	ApplyGlassPatterns( source, canvas, 8, 8.0, PI/2.0, 4, 0.3, filter_strength );
	return canvas;
}

void 
ApplyGlassPatterns(QImage * img, QImage * canvas, int vector_length, double gauss_standard_deviation, double vector_angle, int translation_iteration, double euler_step_size, double strength) 
///
/// Use pixel translation in the form of Glass patterns to give an impressionist look to an image.
///
/// @param img
///  The image to apply the filter to.
///
/// @param canvas
///  A blank canvas to paint the filtered image onto.
///
/// @param vector_length
///  The length of the vectors in the vector field.
///  Affects the length of the 'brush strokes' in the finished image.
///
/// @param gauss_standard_deviation
///  The standard deviation of the Gaussian function that is used to determine the image gradient.
///  Affects the 'roughness' of the 'brush strokes' in the finished image.
///
/// @param vector_angle
///  The angle the vectors in the vector field make with the color gradient.
///
/// @param translation_iterations
///  The number of iterations for the translation of pixels along the arc defined by the vector fields.
///
/// @param euler_step_size
///  The step size of the Euler algorithm used to determine a continuous Glass pattern.
///
/// @param strength
///  The strength of the filter.
///
/// @return
///  Nothing.
///
{
	// Alter the vector length according to the strength of the filter.
	int new_vector_length = vector_length*strength;
	if(new_vector_length < vector_length*strength) new_vector_length++;
	vector_length = new_vector_length;

	// Smooth the image
	uchar* smoothed = new uchar[img->width()*img->height()*4];
	int kernel_size = 5;
	if(strength < 0.5) kernel_size = 3;
	if(strength > 0.2)
	{
		ImageProcessing::GaussianBlur( img->bits(), smoothed, img->width(), img->height(), 4, kernel_size );
	}
    else
    {
        for( int j = 0; j < img->height(); j++ )
        {
            for( int i = 0; i < img->width(); i++ )
            {
                for( int c = 0; c < 4; c++ )
                {
                    smoothed[j*img->width()*4 + i*4 + c] = img->bits()[j*img->width()*4 + i*4 + c];
                }
            }
        }
    }
	
	// Create the noise to be used to determine the continuous Glass pattern.
	uchar* random_noise = new uchar[img->width()*img->height()];
	GetRandomNoise( random_noise, img->width(), img->height() );

	// 
	double* v_x = new double[ img->width()*img->height() ];
	double* v_y = new double[ img->width()*img->height() ];
	GetVectorField( img->bits(), v_x, v_y, img->width(), img->height(), vector_length, vector_angle, gauss_standard_deviation);

	// Add noise to the original image to make strokes more visible
	for( int y = 0; y < canvas->height(); y++ )
    {
		for( int x = 0; x < canvas->width(); x++ )
        {
			double noise = (WhiteNoise() - 0.5)/8.0*strength;
			for(int c = 0; c < 3; c++)
            {
            	int new_val = smoothed[y*canvas->width()*4 + x*4 + c] + noise*255;
            	if( new_val > 255 )
            	{
            		new_val = 255;
            	}
            	else if( new_val < 0 )
            	{
					new_val = 0;
            	}
            	smoothed[y*canvas->width()*4 + x*4 + c] = new_val;
			}
		}
	}

	// Apply a continuous Glass pattern defined by the noise and vector field created.
	TranslateImageAccordingToGlassPattern(smoothed, random_noise, v_x, v_y, canvas->width(), canvas->height(), translation_iteration, euler_step_size);

	// Copy the results to our canvas.
    *canvas = QImage(smoothed, img->width(), img->height(), img->format() );
	delete [] v_x;
	delete [] v_y;
    delete [] smoothed;
    delete [] random_noise;
}

void 
TranslateImageAccordingToGlassPattern(uchar* ref_image, uchar* ref_noise, double* v_x, double* v_y, int width, int height, int iterations, double euler_step_size) 
///
/// Translates noise according to the trajectories of a given vector field giving
/// a continuous Glass pattern. At the maximum points on each arc, translates the pixels of
/// the original image along the same trajectory. Continues for a number of iterations.
///
/// @param ref_image
///  The image to apply the Glass pattern to.
///
/// @param ref-noise
///  An image containing white noise.
///
/// @param v_x
///  The vector field in the x direction.
///
/// @param v_y
///  The vector field in the y direction.
///
/// @param width
///  The width of the image.
///
/// @param height
///  The height of the image.
///
/// @param iterations
///  The number of times to translate pixels along the Glass pattern.
///
/// @param euler_step_size
///  The step size of the euler algorithm.
///
/// @return
///  Nothing.
///
{
	uchar* glass_image = new uchar[width*height*4];
	for( int j = 0; j < height; j++ )
	{
		for( int i = 0; i < width; i++ )
		{
			for( int c = 0; c < 4; c++ )
			{
				glass_image[j*width*4 + i*4 + c] = ref_image[j*width*4 + i*4 + c];
			}
		}
	}

	uchar* glass_noise = new uchar[width*height];
	double* w_x = new double[width*height];
	double* w_y = new double[width*height];
	for( int i = 0; i < iterations; i++ ) 
	{
		TranslatePixels(ref_noise, glass_noise, v_x, v_y, width, height, 1, euler_step_size);
		TranslatePixels(ref_image, glass_image, v_x, v_y, width, height, 4, euler_step_size);
		for( int y = 0; y < height; y++ ) 
		{
			for( int x = 0; x < width; x++ ) 
			{
				if( ref_noise[y*width + x] <= glass_noise[y*width + x] ) 
				{
					ref_noise[y*width + x] = glass_noise[y*width + x];
					for( int c = 0; c < 4; c++ )
					{
						ref_image[y*width*4 + x*4 + c] = glass_image[y*width*4 + x*4 + c];
					}
				}
			}
		}
		TranslatePixels(v_x, w_x, v_x, v_y, width, height, 1, euler_step_size);
		TranslatePixels(v_y, w_y, v_x, v_y, width, height, 1, euler_step_size);
		ImageProcessing::AddImages(v_x, w_x, v_x, width, height, 1);
		ImageProcessing::AddImages(v_y, w_y, v_y, width, height, 1);
	}
	delete [] glass_image;
	delete [] glass_noise;
	delete [] w_x;
	delete [] w_y;
}

void 
TranslatePixels(uchar* image, uchar* canvas, double* v_x, double* v_y, int width, int height, int channels, double step_size)
///
/// Translates pixels along the trajectory described by a vector field.
///
/// @param image
///  The image to be evaluated.
///
/// @param canvas
///  The canvas where this evaluation is stored.
///
/// @param v_x
///  The x component of the vector field.
///
/// @param v_y
///  The y component of the vector field.
///
/// @param width
///  The width of the image.
///
/// @param height
///  The height of the image.
///
/// @param step_size
///  The step size of the Euler algorithm.
///
/// @return
///  Nothing.
///
{
	for( int y = 0; y < height; y++ ) 
	{
		for( int x = 0; x < width; x++ ) 
		{
			double new_x = x + step_size*( v_x[y*width + x] );
			double new_y = y + step_size*( v_y[y*width + x] );
			int x1 = (int)new_x;
			int x2 = x1 + 1;
			int y1 = (int)new_y;
			int y2 = y1 + 1;
			if( x1 >= 0 && y1 >= 0 && x2 < width && y2 < height ) 
			{
				for( int c = 0; c < channels; c++ )
				{
					uchar color11 = image[y1*width*channels + x1*channels + c];
					uchar color21 = image[y2*width*channels + x1*channels + c];
					uchar color12 = image[y1*width*channels + x2*channels + c];
					uchar color22 = image[y2*width*channels + x2*channels + c];

					int new_color = color11*(x2 - new_x)*(y2 - new_y) + color21*(new_x - x1)*(y2 - new_y)
						+ color12*(x2 - new_x)*(new_y - y1) + color22*(new_x - x1)*(new_y - y1);
					if( new_color > 255 ) new_color = 255;
					canvas[y*width*channels + x*channels + c] = (uchar)new_color;
				}
			}
		}
	}
}

void 
TranslatePixels(double* image, double* canvas, double* v_x, double* v_y, int width, int height, int channels, double step_size)
///
/// Translates pixels along the trajectory described by a vector field.
///
/// @param image
///  The image to be evaluated.
///
/// @param canvas
///  The canvas where this evaluation is stored.
///
/// @param v_x
///  The x component of the vector field.
///
/// @param v_y
///  The y component of the vector field.
///
/// @param width
///  The width of the image.
///
/// @param height
///  The height of the image.
///
/// @param step_size
///  The step size of the Euler algorithm.
///
/// @return
///  Nothing.
///
{
	for( int y = 0; y < height; y++ ) 
	{
		for( int x = 0; x < width; x++ ) 
		{
			double new_x = x + step_size*( v_x[y*width + x] );
			double new_y = y + step_size*( v_y[y*width + x] );
			int x1 = (int)new_x;
			int x2 = x1 + 1;
			int y1 = (int)new_y;
			int y2 = y1 + 1;
			if( x1 >= 0 && y1 >= 0 && x2 < width && y2 < height ) 
			{
				for( int c = 0; c < channels; c++ )
				{
					double color11 = image[y1*width*channels + x1*channels + c];
					double color21 = image[y2*width*channels + x1*channels + c];
					double color12 = image[y1*width*channels + x2*channels + c];
					double color22 = image[y2*width*channels + x2*channels + c];
					canvas[y*width*channels + x*channels + c] = color11*(x2 - new_x)*(y2 - new_y) + color21*(new_x - x1)*(y2 - new_y)
						+ color12*(x2 - new_x)*(new_y - y1) + color22*(new_x - x1)*(new_y - y1);
				}
			}
		}
	}
}

void 
GetImageGradients(uchar* source, double* x_sigma, double* y_sigma, int width, int height, double standard_deviation) 
///
/// Gets the convolution of the gradient of the Gaussian function with the image.
/// The gives us the color gradient of the image in the x and y direction.
///
/// @param source
///  The reference image to use in the convolution.
///
/// @param x_sigma
///  The calculated gradient in the x direction at each pixel.
///
/// @param y_sigma
///  The calculated gradient in the y direction at each pixel.
///
/// @param width
///  The width of the image.
///
/// @param height
///  The height of the image.
///
/// @param standard_deviation
///  The standard deviation of the Gauss function.
///
/// @return
///  Nothing.
///
{	
	// Create the Gaussian kernel for convolution.
	int k = 31;
	double* kernel_x = new double[k*k];
	double* kernel_y = new double[k*k];
	for(int j = -1*k/2; j < k/2 + 1; j+=5) 
	{
		for(int i = -1*k/2; i < k/2 + 1; i+=5) 
		{
			double c1 = 1.0/(2*PI*standard_deviation*standard_deviation);
			double c2 = 2*standard_deviation*standard_deviation;
			double g = c1*exp(-1.0*((i*i + j*j)/c2));
			double g_x = g*((-1.0*i)/(standard_deviation*standard_deviation));
			double g_y = g*((-1.0*j)/(standard_deviation*standard_deviation));
			kernel_x[(j + k/2)*k + i + k/2] = g_x;
			kernel_y[(j + k/2)*k + i + k/2] = g_y;
		}
	}

	// Get the convolution of the image with the gaussian kernel
	for( int j = 0; j < height; j++ ) 
	{
		for( int i = 0; i < width; i++ ) 
		{
			double x_sum[3] = {0.0, 0.0, 0.0};
			double y_sum[3] = {0.0, 0.0, 0.0};
			for( int n = 0; n < k; n+=5 ) 
			{
				for( int m = 0; m < k; m+=5 ) 
				{
					int x_pos = i + m - k/2;
					int y_pos = j + n - k/2;
					if(x_pos < 0) x_pos = 0;
					if(x_pos >= width) x_pos = width - 1;
					if(y_pos < 0) y_pos = 0;
					if(y_pos >= height) y_pos = height - 1;
					
					double gauss_x = kernel_x[n*k + m];
					double gauss_y = kernel_y[n*k + m];
					for(int c = 0; c < 3; c++) 
					{
						x_sum[c] += source[y_pos*width*4 + x_pos*4 + c]*gauss_x/255.0;
						y_sum[c] += source[y_pos*width*4 + x_pos*4 + c]*gauss_y/255.0;
					}
				}
			}
			for( int c = 0; c < 3; c++ )
			{
				x_sigma[j*width*3 + i*3 + c] = x_sum[c];
				y_sigma[j*width*3 + i*3 + c] = y_sum[c];
			}
		}
	}
	delete [] kernel_x;
	delete [] kernel_y;
}

void 
GetVectorField( uchar* source, double* v_x, double* v_y, int width, int height, int vector_length, double vector_angle, double gauss_standard_deviation ) 
///
/// Get the vector field based on the image 'source', where each vector is relative to
/// the image gradient at this point.
///
/// @param source
///  The image to calculate the vector field for.
///
/// @param v_v
///  The image used to store the x component of the vector field.
///
/// @param v_y
///  The image used to store the y component of the vector field.
///
/// @param width
///  The width of the image.
///
/// @param height
///  The height of the image.
///
/// @param vector_length
///  The length of the vectors in the vector field.
///
/// @param vector_angle
///  The angle the vectors make with the image gradient.
///
/// @param gauss_standard_deviation
///  The standard deviation of the Gaussian function used to determine the image gradients.
///
/// @return
///  Nothing.
///
{
	
	// Convolve the smoothed image with the gradient of the gaussian function
	double* x_sigma = new double[ width*height*3 ];
	double* y_sigma = new double[ width*height*3 ];
	GetImageGradients( source, x_sigma, y_sigma, width, height, gauss_standard_deviation);

	for(int y = 0; y < height; y++) 
	{
		for(int x = 0; x < width; x++) 
		{
			// Find theta, the image gradient at this point
			double e = 0.0;
			double f = 0.0;
			double g = 0.0;
			for(int c = 0; c < 3; c++) {
				e += x_sigma[y*width*3 + x*3 + c]*x_sigma[y*width*3 + x*3 + c];
				f += x_sigma[y*width*3 + x*3 + c]*y_sigma[y*width*3 + x*3 + c];
				g += y_sigma[y*width*3 + x*3 + c]*y_sigma[y*width*3 + x*3 + c];
			}
			
			double lambda1 = (e + g + sqrt((e-g)*(e-g) + 4.0*f*f))/2.0;
			double lambda2 = (e + g - sqrt((e-g)*(e-g) + 4.0*f*f))/2.0;
			if(lambda1 != lambda2) {
				double theta = 0.5*atan2((2.0*f),(e-g));
				double theta2 = theta + PI/2.0;
				double f_th1 = 0.5*((e + g) + cos(2.0*theta)*(e - g) + 2.0*f*sin(2.0*theta));
				double f_th2 = 0.5*((e + g) + cos(2.0*theta2)*(e - g) + 2.0*f*sin(2.0*theta2));
				if(f_th2 > f_th1) {
					theta = theta2;
				}

				// Find the vectors defined by our vector length, vector angle, and image gradient theta.
				v_x[y*width + x] = (double)vector_length*cos(theta + vector_angle);
				v_y[y*width + x] = (double)vector_length*sin(theta + vector_angle);
			} else {
				v_x[y*width + x] = 0.0;
				v_y[y*width + x] = 0.0;
			}
		}
	}
}

double 
WhiteNoise() 
///
/// Returns Gaussian white noise between 0 and 1.
///
/// @return
///  The white noise value calculated.
///
{
	double r = 0.0;
	for(int i = 0; i < 50; i++) {
		r += rand()%100/100.0;
	}
	r = r - 25.0;
	r = r*sqrt(12.0/50.0);
	r = (r + 1)/2.0;
	if(r < 0) r = 0.0;
	if(r  > 1) r = 1.0;
	return r;
}

void 
GetRandomNoise( uchar* destination, int width, int height ) 
///
/// Fills an image with Gaussian white noise
///
/// @param destination
///  The image to store the white noise in.
///
/// @param width
///  The width of the image.
///
/// @param height
///  The height of the image.
///
/// @return
///  Nothing.
///
{
	uchar* noise = new uchar[width*height];
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			double r = WhiteNoise();
			
			noise[y*width + x] = (uchar)r*255;
		}
	}
	int k = 5;
	ImageProcessing::GaussianBlur( noise, destination, width, height, 1, k );
	delete [] noise;
}