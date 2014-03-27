#include "GlassPatternsFilter.h"
#include "HelperFunctions/ImageProcessing.h"

const double GlassPatternsFilter::FILTER_STRENGTH_DEFAULT = 1.0;

void ApplyGlassPatterns(QImage * source, QImage * destination, int a, double sd, double theta, int n, double h, double strength);
void GetCrossCGP( uchar* image, uchar* noise, double* v_x, double* v_y, int width, int height, int n, double h );
void Evaluate(uchar* image, uchar* canvas, double* v_x, double* v_y, int width, int height, int channels, double h);
void Evaluate(double* image, double* canvas, double* v_x, double* v_y, int width, int height, int channels, double h);
void Gauss(QImage * image, double* x_sigma, double* y_sigma, double sd);
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
	ApplyGlassPatterns( source, canvas, 8, 14.0, M_PI/2.0, 4, 0.1, filter_strength );
	return canvas;
}

void 
ApplyGlassPatterns(QImage * img, QImage * canvas, int a, double sd, double theta, int n, double h, double strength) 
///
/// Use glass patterns to give an impressionist look to the image
///
/// @param img
///  The image to apply the filter to
///
/// @param canvas
///  A blank canvas to paint the filtered image onto
///
/// @param a
///
/// @param sd
///
/// @param theta
///
/// @param n
///
/// @param h
///
/// @param strength
///  The strength of the filter
///
/// @return
///  Nothing
///
{
	int new_a = a*strength;
	if(new_a < a*strength) new_a++;
	a = new_a;

	// need to smooth the image first, but meh, do that later
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
	
	uchar* random_noise = new uchar[img->width()*img->height()];
	GetRandomNoise( random_noise, img->width(), img->height() );

	double* v_x = new double[ img->width()*img->height()*3 ];
	double* v_y = new double[ img->width()*img->height()*3 ];
	GetVectorField( img->bits(), v_x, v_y, img->width(), img->height(), a, theta, sd);

	// Add noise to the image
	for( int y = 0; y < canvas->height(); y++ )
    {
		for( int x = 0; x < canvas->width(); x++ )
        {
			double noise = (WhiteNoise() - 0.5)/8.0*strength;
			for(int c = 0; c < 3; c++)
            {
				smoothed[y*canvas->width()*4 + x*4 + c] += noise;
			}
		}
	}
	GetCrossCGP(smoothed, random_noise, v_x, v_y, canvas->width(), canvas->height(), n, h);
	//cvScale(src,src,255.f/1.f);
	//cvConvert(src,canvas);

	/*uchar* canvas_bits = new uchar[img->width()*img->height()*4];
	for( int j = 0; j < img->height(); j ++ )
	{
		for( int i = 0; i < img->width(); i++ )
		{
			canvas_bits[j*img->width()*4 + i*4] = (uchar)v_y[j*img->width()*3 + i*3];
			canvas_bits[j*img->width()*4 + i*4 + 1] = (uchar)v_y[j*img->width()*3 + i*3 + 1];
			canvas_bits[j*img->width()*4 + i*4 + 2] = (uchar)v_y[j*img->width()*3 + i*3 + 2];
			canvas_bits[j*img->width()*4 + i*4 + 3] = 255;
		}
	}
	*canvas = QImage(canvas_bits, img->width(), img->height(), img->format() );
	delete [] canvas_bits;*/
    *canvas = QImage(smoothed, img->width(), img->height(), img->format() );
	delete [] v_x;
	delete [] v_y;
    delete [] smoothed;
    delete [] random_noise;
}

void 
GetCrossCGP(uchar* image, uchar* noise, double* v_x, double* v_y, int width, int height, int n, double h) 
///
/// Use the Euler algorithm to determine the GP defined by z(r) and v(r).
/// Apply the GP to the image I(r)
///
/// @param iR
///  The image to apply the GP to
///
/// @param z
///  An image containing white noise
///
/// @param vX
///
/// @param vY
///
/// @param n
///
/// @param h
///
/// @return
///  Nothing
{
	uchar* i1 = new uchar[width*height*4];
	for( int j = 0; j < height; j++ )
	{
		for( int i = 0; i < width; i++ )
		{
			for( int c = 0; c < 4; c++ )
			{
				i1[j*width*4 + i*4 + c] = image[j*width*4 + i*4 + c];
			}
		}
	}

	uchar* z1 = new uchar[width*height];
	double* w_x = new double[width*height];
	double* w_y = new double[width*height];
	for( int i = 0; i < n; i++ ) 
	{
		Evaluate(noise, z1, v_x, v_y, width, height, 1, h);
		Evaluate(image, i1, v_x, v_y, width, height, 4, h);
		for( int y = 0; y < height; y++ ) 
		{
			for( int x = 0; x < width; x++ ) 
			{
				if( noise[y*width + x] <= z1[y*width + x] ) 
				{
					noise[y*width + x] = z1[y*width + x];
					for( int c = 0; c < 4; c++ )
					{
						image[y*width*4 + x*4 + c] = i1[y*width*4 + x*4 + c];
					}
				}
			}
		}
		Evaluate(v_x, w_x, v_x, v_y, width, height, 1, h);
		Evaluate(v_y, w_y, v_x, v_y, width, height, 1, h);
		ImageProcessing::AddImages(v_x, w_x, v_x, width, height, 1);
		ImageProcessing::AddImages(v_y, w_y, v_y, width, height, 1);
	}
	delete [] i1;
	delete [] z1;
	delete [] w_x;
	delete [] w_y;
}

void 
Evaluate(uchar* image, uchar* canvas, double* v_x, double* v_y, int width, int height, int channels, double step_size)
///
/// Ummm..... @todo[crystal 5.11.2012] finish documentation
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

					canvas[y*width*channels + x*channels + c] = color11*(x2 - new_x)*(y2 - new_y) + color21*(new_x - x1)*(y2 - new_y)
						+ color12*(x2 - new_x)*(new_y - y1) + color22*(new_x - x1)*(new_y - y1);
				}
			}
		}
	}
}

void 
Evaluate(double* image, double* canvas, double* v_x, double* v_y, int width, int height, int channels, double step_size)
///
/// Ummm..... @todo[crystal 5.11.2012] finish documentation
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
Gauss(uchar* source, double* x_sigma, double* y_sigma, int width, int height, double sd) 
///
/// Gets the convolution of the gradient of the Gaussian function with the image
///
/// @param image
///  The image to use in the convolution
///
/// @param x_sigma
///
/// @param y_sigma
///
/// @param standard_deviation
///  The standard deviation of the Gauss function.
///
/// @return
///  Nothing
///
{
	// k, the size of the kernel for convolution
	int k = 31;
	
	// Create the kernel for convolution
	double* kernel_x = new double[k*k];
	double* kernel_y = new double[k*k];
	for(int j = -1*k/2; j < k/2 + 1; j+=5) 
	{
		for(int i = -1*k/2; i < k/2 + 1; i+=5) 
		{
			double c1 = 1.0/(2*PI*sd*sd);
			double c2 = 2*sd*sd;
			double g = c1*exp(-1.0*((i*i + j*j)/c2));
			double g_x = g*((-1.0*i)/(sd*sd));
			double g_y = g*((-1.0*j)/(sd*sd));
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
						x_sum[c] += source[y_pos*width*4 + x_pos*4 + c]*gauss_x;
						y_sum[c] += source[y_pos*width*4 + x_pos*4 + c]*gauss_y;
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
GetVectorField( uchar* source, double* v_x, double* v_y, int width, int height, int a, double th0, double standard_deviation ) 
///
/// Get the vector field v(r) based on the image I(r) given the parameters a and theta0
///
/// @param source
///  The image to calculate the vector field for
///
/// @param v_v
///  The image used to store the x component of the vector field
///
/// @param v_y
///  The image used to store the y component of the vector field
///
/// @param a
///
/// @param th0
///
/// @param sd
///
/// @return
///  Nothing
{
	
	// Convolve the smoothed image with the gradient of the gaussian function
	double* x_sigma = new double[ width*height*3 ];
	double* y_sigma = new double[ width*height*3 ];
	Gauss( source, x_sigma, y_sigma, width, height, standard_deviation);
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
				double theta2 = theta + M_PI/2.0;
				double f_th1 = 0.5*((e + g) + cos(2.0*theta)*(e - g) + 2.0*f*sin(2.0*theta));
				double f_th2 = 0.5*((e + g) + cos(2.0*theta2)*(e - g) + 2.0*f*sin(2.0*theta2));
				if(f_th2 > f_th1) {
					theta = theta2;
				}
				for( int c = 0; c < 3; c++ )
				{
					v_x[y*width*3 + x*3 + c] = (double)a*cos(theta + th0);
					v_y[y*width*3 + x*3 + c] = (double)a*sin(theta + th0);
				}
			} else {
				for( int c = 0; c < 3; c++ )
				{
					v_x[y*width*3 + x*3 + c] = 0.0;
					v_y[y*width*3 + x*3 + c] = 0.0;
				}
			}
		}
	}
}

double 
WhiteNoise() 
///
/// Returns Gaussian white noise between 0 and 1
///
/// @return
///  The white noise value calculated
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
/// Fills the image z(r) with Gaussian white noise
///
/// @param z
///  The image z(r)
///
/// @return
///  Nothing
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