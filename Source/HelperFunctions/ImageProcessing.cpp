#include "ImageProcessing.h"

#define PI 3.14159265

double 
ImageProcessing::ColorDistance( QColor color1, QColor color2)
///
/// A simple rgb distance comparison to compare color distance.
///
/// @todo [crystal] Research alternative color comparisons.
///  http://www.compuphase.com/cmetric.htm
///
/// @param color1
///  The first color to be compared.
///
/// @param color2
///  The second color to be compared.
///
/// @return
///  Nothing.
///
{
	return abs(	(color1.red() - color2.red())*(color1.red() - color2.red()) +
						(color1.green() - color2.green())*(color1.green() - color2.green()) +
						(color1.blue() - color2.blue())*(color1.blue() - color2.blue())	);
}

std::vector<QPoint> 
ImageProcessing::GetPoissonDisks(int width, int height, int min_dist) 
///
/// Takes a poisson sampling (a set of randomized points over an area that
/// are a given minimum distance appart) of a given width and height.
///
/// @param width
///  The width of the poisson sampling area.
///
/// @param height
///  The height of the poisson sampling area.
///
/// @param min_dist
///  The minimum distance between sampled points.
///
/// @return
///  A vector of points where each point is part of the sample.
///
{

	const double root2 = 1.414214;
	const int k = 30;

	// Initialize data structures
	int cell_size = (int)(min_dist/root2);
	if(cell_size < 1) cell_size = 1;

	int grid_width = (int)width/cell_size;
	int grid_height = (int)height/cell_size;

	if( width%grid_width != 0 ) grid_width++;
	if( height%grid_height != 0 ) grid_height++;

	std::vector<QPoint> grid;
	grid.resize(grid_width*grid_height);
	for( size_t i = 0; i < grid.size(); i++)
	{
		grid[i] = QPoint(-1, -1);
	} 

	std::vector<QPoint> processing;
	std::vector<QPoint> output;
	processing.reserve(grid_width*grid_height);
	output.reserve(grid_width*grid_height);
	
	// Find random start point
	// Add to the output list, processing list, and grid
	QPoint start = QPoint(rand()%width, rand()%height);
	grid[start.y()/cell_size*grid_width + start.x()/cell_size] = start;
	processing.push_back(start);
	output.push_back(start);

	// Poisson sampling loop
	while( !processing.empty() ) 
	{
		// Choose a random point from the processing list
		int get_at = rand()%processing.size();
		QPoint next_point = processing[get_at];
		for( size_t i = get_at; i < processing.size() - 1; i++ )
		{
			processing[i] = processing[i + 1];
		} 
		processing.pop_back();

		// For this point, generate k points around this point
		for( int i = 0; i < k; i++ ) 
		{
			// Generate a new point randomly chosen
			// with a random angle between 0 and 2*PI
			// and a random radius between minDist and 2*minDist
			int radius = rand()%min_dist + min_dist;
			int temp = rand()%360;
			double angle = temp/360.0*2*PI;
		
			int new_x = (int)(next_point.x() + radius * cos(angle));
			int new_y = (int)(next_point.y() + radius * sin(angle));

			// Check that point is not too close to an existing point
			// and that the point is within the image boundaries
			bool valid = true;
			if( new_x < 0 || new_y < 0 || new_x >= width || new_y >= height ) 
			{
				valid = false;
			} 
			else 
			{
				int grid_x = new_x/cell_size;
				int grid_y = new_y/cell_size;

				// Get surrounding cells
				for( int y = -2; y < 3; y++ ) 
				{
					for( int x = -2; x < 3; x++ ) 
					{
						// Check valid grid point
						if( x >= 0 && x < grid_width && y >= 0 && y < grid_height ) 
						{ 
							// Calculate point
							QPoint temp = grid[grid_y*grid_width + grid_x];
							int temp_x = temp.x() + x;
							int temp_y = temp.y() + y;
							if(!(temp_x == new_x && temp_y == new_y) ) {
								// Get the distance between this point and the point in the cell
								int dist_sqrd = (new_x - temp_x)*(new_x - temp_x) + (new_y - temp_y)*(new_y - temp_y);
								if( dist_sqrd < min_dist*min_dist )
								{
									valid = false;
								} 
							}
						}
					}
				}
			}

			// If point is valid, add it as a new point
			if( valid ) 
			{
				grid[new_y/cell_size*grid_width + new_x/cell_size] = QPoint( new_x, new_y );
				output.push_back( QPoint( new_x, new_y ) );
				processing.push_back( QPoint(new_x, new_y) );
			}
		}
	}

	processing.clear();
	grid.clear();
	std::vector<QPoint>().swap( processing );
	std::vector<QPoint>().swap( grid );
	return output;
}

void
ImageProcessing::HorizontalConvo( uchar* source, uchar* destination, int width, int height, int channels, double* kernel, int kernel_size )
///
/// Performs a horizontal convolution using a given 1D kernel.
///
/// @param source
///  The source image data.
///
/// @param destination
///  The image data where the result is to be stored (must be the same dimensions as source).
///
/// @param width
///  The width of the image.
///
/// @param height
///  The height of the image.
///
/// @param channels
///  The number of color channels in the image.
///
/// @param kernel
///  A pointer to the 1D kernel to be used for convolution.
///
/// @param kernel_size
///  The size of the given kernel.
///
/// @return
///  Nothing.
///
{
	for( int j = 0; j < height; j++ )
	{
		for( int i = 0; i < width; i++ )
		{
			for( int c = 0; c < channels; c++ )
			{
				double total = 0.0;
				for( int kx = 0; kx < kernel_size; kx++ )
				{
                    int x_pos = i + kx - kernel_size/2;
                    if( x_pos < 0 ) x_pos = 0;
                    if( x_pos >= width ) x_pos = width - 1;

                    total += source[j*width*channels + x_pos*channels + c]*kernel[kx];
				}
                if( total > 255 ) total = 255;
                if( total < 0 ) total = 0;
				destination[(j)*width*channels + (i)*channels + c] = (uchar)total;
			}
		}
	}
}

void
ImageProcessing::VerticalConvo( uchar* source, uchar* destination, int width, int height, int channels, double* kernel, int kernel_size )
///
/// Performs a vertical convolution using a given 1D kernel.
///
/// @param source
///  The source image data.
///
/// @param destination
///  The image data where the result is to be stored (must be the same dimensions as source).
///
/// @param width
///  The width of the image.
///
/// @param height
///  The height of the image.
///
/// @param channels
///  The number of color channels in the image.
///
/// @param kernel
///  A pointer to the 1D kernel to be used for convolution.
///
/// @param kernel_size
///  The size of the given kernel.
///
/// @return
///  Nothing.
///
{
	for( int j = 0; j < height; j++ )
	{
		for( int i = 0; i < width; i++ )
		{
			for( int c = 0; c < channels; c++ )
			{
				double total = 0.0;
				for( int ky = 0; ky < kernel_size; ky++ )
				{
                    int y_pos = j + ky - kernel_size/2;
                    if( y_pos < 0 ) y_pos = 0;
                    if( y_pos >= height ) y_pos = height - 1;

                    total += source[y_pos*width*channels + i*channels + c]*kernel[ky];
				}
                if( total > 255 ) total = 255;
                if( total < 0 ) total = 0;
				destination[(j)*width*channels + (i)*channels + c] = (uchar)total;
			}
		}
	}
}

void
ImageProcessing::TwoDConvo( uchar* source, uchar* destination, int width, int height, int channels, double* kernel, int kernel_size )
///
/// Performs a 2D convolution using a given 2D kernel.
///
/// @param source
///  The source image data.
///
/// @param destination
///  The image data where the result is to be stored (must be the same dimensions as source).
///
/// @param width
///  The width of the image.
///
/// @param height
///  The height of the image.
///
/// @param channels
///  The number of color channels in the image.
///
/// @param kernel
///  A pointer to the 2D kernel to be used for convolution.
///
/// @param kernel_size
///  The width and height of the given kernel i.e the kernel is an kernel_size * kernel_size array.
///
/// @return
///  Nothing.
///
{
	for( int j = 0; j < height; j++ )
	{
		for( int i = 0; i < width; i++ )
		{
			for( int c = 0; c < channels; c++ )
			{
				double total = 0.0;
				for( int ky = 0; ky < kernel_size; ky++ )
				{
					for( int kx = 0; kx < kernel_size; kx++ )
					{
                    	int y_pos = j + ky - kernel_size/2;
                    	if( y_pos < 0 ) y_pos = 0;
                    	if( y_pos >= height ) y_pos = height - 1;

                    	int x_pos = i + kx - kernel_size/2;
                    	if( x_pos < 0 ) x_pos = 0;
                    	if( x_pos >= width ) x_pos = width - 1;

                    	total += source[y_pos*width*channels + x_pos*channels + c]*kernel[ky*kernel_size + kx];
					}
				}
                if( total > 255 ) total = 255;
                if( total < 0 ) total = 0;
				destination[(j)*width*channels + (i)*channels + c] = (uchar)total;
			}
		}
	}
}

void 
ImageProcessing::BoxBlur( uchar* source, uchar* destination, int width, int height, int channels, int kernel_size )
///
/// Blurs a given image using a simple box blur.
///
/// @param source
///  The source image data.
///
/// @param destination
///  The destination image data.
///
/// @param width
///  The width of the image.
///
/// @param height
///  The height of the image.
///
/// @param channels
///  The number of color channels.
///
/// @param kernel_size
///  The size of the blur kernel to be used. 5 by default.
///
/// @return
///  Nothing
///
{
	double* kernel = new double[kernel_size];
	for( int k = 0; k < kernel_size; k++ )
	{
		kernel[k] = 1.0/kernel_size;
	}

	ImageProcessing::HorizontalConvo(source, destination, width, height, channels, kernel, kernel_size);
	ImageProcessing::VerticalConvo(destination, destination, width, height, channels, kernel, kernel_size);

	delete [] kernel;
}

void 
ImageProcessing::GaussianBlur( uchar* source, uchar* destination, int width, int height, int channels, int kernel_size, double sigma )
///
/// Blurs an image using convolution with a gaussian kernel.
///
/// @param source
///  The image to be blurred.
///
/// @param destination
///  The destination image data.
///
/// @param width
///  The width of the image.
///
/// @param height
///  The height of the image.
///
/// @param channels
///  The number of color channels that the image contains.
///
/// @param kernel_size
///  The size of the gaussian kernel to be used for blurring. 5 by default.
///
/// @param sigma
///  The sigma parameter. Effects how blurry the image is. 1.5 by default.
///
/// @return
///  Nothing.
///
{
	// Create a gaussian kernel of size=kernel_size and strength=sigma
	double* kernel = new double[kernel_size];

	double sum = 0.0;
	double s_const = 1.0/(2*PI*sigma*sigma);
	for( int i = 0; i < kernel_size; i++ )
	{
		int dist = i - kernel_size/2;
		double neg_dist = -1.0*dist*dist;
		kernel[i] = s_const*exp( neg_dist/(2*sigma*sigma) );
		sum += kernel[i];
	}

	for( int i = 0; i < kernel_size; i++ )
	{
		kernel[i] /= sum;
	}

	ImageProcessing::HorizontalConvo(source, destination, width, height, channels, kernel, kernel_size);
	ImageProcessing::VerticalConvo(destination, destination, width, height, channels, kernel, kernel_size);
	delete [] kernel;
}

void 
ImageProcessing::SobelEdgeDetection( uchar* source, uchar* gradient_magnitude, int width, int height, int channels )
///
/// Performs the sobel operator on a given image, which gives an approximation of the image
/// gradients which is useful for edge detection.
///
/// @param source
///  The source image.
///
/// @param gradient_magnitude
///  A one channel image that stores the gradient strength returned by the sobel operator at each point.
///
/// @param gradient_direction
///  A one channel image that stores the gradient direction returned by the sobel operator at each point.
///
/// @param width
///  The width of the image.
///
/// @param height
///  The height of the image.
///
/// @param channels
///  The number of channels in the source image.
///
/// @return
///  Nothing.
{
	ImageProcessing::ConvertToOneChannel( source, gradient_magnitude, width, height, channels);

	uchar* gx = new uchar[width*height];
	uchar* gy = new uchar[width*height];
	double sobel_x[9] = {-1.0, 0.0, 1.0, -2.0, 0.0, 2.0, -1.0, 0.0, 1.0};
	double sobel_y[9] = {1.0, 2.0, 1.0, 0.0, 0.0, 0.0, -1.0, -2.0, -1.0};

	ImageProcessing::TwoDConvo(gradient_magnitude, gx, width, height, 1, sobel_x, 3);
	ImageProcessing::TwoDConvo(gradient_magnitude, gy, width, height, 1, sobel_y, 3);

	// Added together the gradient images in the x and y direction
	ImageProcessing::AddImages(gx, gy, gradient_magnitude, width, height, 1);

	delete [] gx;
	delete [] gy;
}

void 
ImageProcessing::SobelEdgeDetection( uchar* source, uchar* gradient_magnitude, uchar* gradient_direction, int width, int height, int channels )
///
/// Performs the sobel operator on a given image, which gives an approximation of the image
/// gradients which is useful for edge detection.
///
/// @param source
///  The source image.
///
/// @param gradient_magnitude
///  A one channel image that stores the gradient strength returned by the sobel operator at each point.
///
/// @param gradient_direction
///  A one channel image that stores the gradient direction returned by the sobel operator at each point.
///
/// @param width
///  The width of the image.
///
/// @param height
///  The height of the image.
///
/// @param channels
///  The number of channels in the source image.
///
/// @return
///  Nothing.
{
	ImageProcessing::ConvertToOneChannel( source, gradient_magnitude, width, height, channels);

	uchar* gx = new uchar[width*height];
	uchar* gy = new uchar[width*height];
	double sobel_x[9] = {-1.0, 0.0, 1.0, -2.0, 0.0, 2.0, -1.0, 0.0, 1.0};
	double sobel_y[9] = {1.0, 2.0, 1.0, 0.0, 0.0, 0.0, -1.0, -2.0, -1.0};

	ImageProcessing::TwoDConvo(gradient_magnitude, gx, width, height, 1, sobel_x, 3);
	ImageProcessing::TwoDConvo(gradient_magnitude, gy, width, height, 1, sobel_y, 3);

	// Added together the gradient images in the x and y direction
	ImageProcessing::AddImages(gx, gy, gradient_magnitude, width, height, 1);

	for( int j = 0; j < height; j++ )
	{
		for( int i = 0; i < width; i++ )
		{
			if( gx[j*width + i] != 0)
			{
				int direction = atan(gy[j*width + i]/gx[j*width + i]) * 180 / PI;
				gradient_direction[j*width + i] = direction;
			}
			else if( gy[j*width + i] == 0)
			{
				gradient_direction[j*width + i] = 0;
			}
			else
			{
				gradient_direction[j*width + i] = 90;
			}
		}
	}

	delete [] gx;
	delete [] gy;
}

void 
NonmaximumSupression( uchar* gradient_magnitude, uchar* gradient_direction, uchar* edges, int width, int height )
///
/// Supresses the gradient magnitude if it is not the local maximum in it's search direction.
///
/// @param gradient_magnitude
///  The intensity of the gradient at each point in the image.
///
/// @param gradient_direction
///  The direction of the gradient at each point in the image.
///
/// @param edges
///  An empty image that stores the resulting edge information.
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
	for( int j = 0; j < height; j++ )
	{
		for( int i = 0; i < width; i++ )
		{
			// Set gradient strength to zero if edges are not the maximum in their search direction.
			int search_direction = gradient_direction[j*width + i];
			if( search_direction > 180 ) search_direction = search_direction % 180;
			int x_pos = 0;
			int y_pos = 0;
			if( search_direction > 22.5 && search_direction <= 67.5 )
			{
				// 45 degrees, north-west/south-east
				x_pos = 1;
				y_pos = 1;
			}
			else if( search_direction > 67.5 && search_direction <= 112.5 )
			{
				// 90 degrees, east/west
				x_pos = 0;
				y_pos = 1;
			}
			else if( search_direction > 112.5 && search_direction <= 157.5)
			{
				// 135 degrees, north-east/south-west
				x_pos = -1;
				y_pos = 1;
			}
			else
			{
				// 0 degrees, north/south
				x_pos = 1;
				y_pos = 0;
			}
			edges[j*width + i] = gradient_magnitude[j*width + i];
			if( j - y_pos >= 0 && i - x_pos >= 0 && j - y_pos < height && i - x_pos < width && !(x_pos == 0 && y_pos == 0) )
			{
				if( gradient_magnitude[(j - y_pos)*width + i - x_pos] > gradient_magnitude[j*width + i])
				{
					edges[j*width + i] =  0;
				}
			}
			if( j + y_pos >= 0 && i + x_pos >= 0 && j + y_pos < height && i + x_pos < width && !(x_pos == 0 && y_pos == 0) )
			{
				if( gradient_magnitude[(j + y_pos)*width + i + x_pos] > gradient_magnitude[j*width + i])
				{
					edges[j*width + i] = 0;
				}
			}
		}
	}
}

void 
Hysteresis( uchar* edges, int width, int height, int max_threshold, int min_threshold )
///
/// Traces connected edges to minimize noise. An edge begins if it strength is greater than
/// or equal to the maximum threshold and continues until it drops below the minimum threshold.
///
/// @param edges
///  The gradient magnitude at each point in the image.
///
/// @param width
///  The width of the image.
///
/// @param height
///  The height of the image.
///
/// @param max_threshold
///  The maximum threshold. Edge tracing begins if an edge is at least this intensity.
///
/// @param min_threshold
///  The minimum threshold. Edge tracing ends if an edge drops below this intensity.
///
/// @return
///  Nothing.
{
	for( int j = 0; j < height; j++ )
	{
		for( int i = 0; i < width; i++ )
		{
			if( edges[j*width + i] >= max_threshold )
			{
				// Set pixels that definitely edges to 255
				edges[j*width + i] = 255;
			} 
			else if( edges[j*width + i] >= min_threshold )
			{
				// Set pixels that might be edges to 100
				edges[j*width + i] = 100;
			}
			else
			{
				// Set pixels that are not edges to 0
				edges[j*width + i] = 0;
			}

		}
	}

	// Perform hysteresis on the image until the whole image is traversed with no new edges
	bool done = false;
	while( !done )
	{
		done = true;
		for( int j = 1; j < height - 1; j++ )
		{
			for( int i = 1; i < width - 1; i++ )
			{
				if( edges[j*width + i] > 0 && edges[j*width + i] < 255 )
				{
					// If a pixel is undecided, set it to be an edge if it has any neighbours that are edges
					if( edges[(j-1)*width + (i-1)] == 255 || edges[(j-1)*width + i] == 255 || edges[(j-1)*width + (i+1)] == 255 ||
						edges[j*width + (i-1)] == 255 || edges[j*width + (i+1)] == 255 ||
						edges[(j+1)*width + (i-1)] == 255 || edges[(j+1)*width + i] == 255 || edges[(j+1)*width + (i+1)] == 255 )
					{
						edges[j*width + i] = 255;
						done = false;
					}
						
				}
			}
		}
	}

	// We have found all the edges, so set any remaining undecided pixels to 0
	for( int j = 0; j < height; j ++ )
	{
		for( int i = 0; i < width; i++ )
		{
			if( edges[j*width + i] < 255 && edges[j*width + i] > 0)
			{
				edges[j*width + i] = 0;
			}
		}
	}
}

void 
ImageProcessing::CannyEdgeDetection( uchar* source, uchar* edges, int width, int height, int channels, int gaussian_kernel_size, double sigma, int max_threshold, int min_threshold )
///
/// Runs Canny edge detection on a given image and returns the result.
///
/// @param source
///  The source image to perform the edge detection on.
///
/// @param edges
///  The edges discovered by the canny operator
/// @param width
///  The width of the image.
///
/// @param height
///  The height of the image.
///
/// @param channels
///  The number of color channels that the image contains.
///
/// @param gaussian_kernel_size
///  The size of the gaussian kernel used to smooth the image. 5 by default.
///
/// @param sigma
///  The sigma value of the gaussian kernel used to smooth the image. 1.5 by default.
///
/// @param max_threshold
///  The maximum hysteresis threshold. Edge tracing begins if an edge is at least this intensity. 80 by default.
///
/// @param min_threshold
///  The minimum hysteresis threshold. Edge tracing ends if an edge drops below this intensity. 20 by default.
///
/// @return
///  Nothing.
///
{
	uchar* smoothed = new uchar[width*height*channels];

	// Apply a Gaussian blur with kernel size 5 to get rid of any noise
	ImageProcessing::GaussianBlur( source, smoothed, width, height, channels, gaussian_kernel_size, sigma );

	// Apply the sobel operator to the smoothed image to approximate the image gradients
	uchar* gradient_magnitude = new uchar[width*height];
	uchar* gradient_direction = new uchar[width*height];
	ImageProcessing::SobelEdgeDetection( smoothed, gradient_magnitude, gradient_direction, width, height, channels );
	delete [] smoothed;

	// Apply nonmaximum supression to thin edges
	NonmaximumSupression( gradient_magnitude, gradient_direction, edges, width, height );
	delete [] gradient_magnitude;
	delete [] gradient_direction;

	// Apply hysteresis to minimize streaking
	Hysteresis( edges, width, height, max_threshold, min_threshold );
}

void
ImageProcessing::ConvertToOneChannel(uchar *source, uchar *destination, int width, int height, int channels, int alpha_channel)
///
/// Converts a multichannel image to a one channel image by averaging each color component excluding the alpha channel.
///
/// @param source
///  The image to be converted.
///
/// @param destination
///  The image that will store the resulting one channel image.
///
/// @param width
///  The width of the image.
///
/// @param height
///  The height of the image.
///
/// @param channels
///  The number of color channels that the source image contains. 4 by default.
///
/// @param alpha_channel
///  The index of the source image's alpha channel. -1 if it has no alpha. 3 by default.
///
/// @return
///  Nothing.
///
{
	for( int j = 0; j < height; j++ )
	{
		for( int i = 0; i < width; i++ )
		{
			double average = 0.0;
			for( int c = 0; c < channels; c++ )
			{
				if( c%channels != alpha_channel || alpha_channel == -1) // don't include the alpha channel
				{
					average += source[j*width*channels + i*channels + c];
				}
			}
			average /= alpha_channel == -1 ? channels : channels - 1;
			if( average < 0 ) average = 0;
			if( average > 255 ) average = 255;
			destination[j*width + i] = average;
		}
	}
}

void
ImageProcessing::ConvertFromOneChannel(uchar *source, uchar *destination, int width, int height, int channels, int alpha_channel)
///
/// Converts a one channel image to a multichannel image by setting each color component to the pixel value of the one channel image,
/// (except the alpha channel if there is one, which is set to 255).
///
/// @param source
///  The image to be converted.
///
/// @param destination
///  The image that will store the resulting multichannel image.
///
/// @param width
///  The width of the image.
///
/// @param height
///  The height of the image.
///
/// @param channels
///  The number of color channels that the destination image will contain. 4 by default.
///
/// @param alpha_channel
///  The index of the destination image's alpha channel. -1 if it has no alpha. 3 by default.
///
/// @return
///  Nothing.
///
{
	for( int j = 0; j < height; j++ )
	{
		for( int i = 0; i < width; i++ )
		{
			for( int c = 0; c < channels; c++ )
			{
				if( c%channels != alpha_channel || alpha_channel == -1 ) // don't include the alpha channel
				{
					destination[j*width*channels + i*channels + c] = source[j*width + i];
				}
				else
				{
					destination[j*width*channels + i*channels + c] = 255;
				}
			}
		}
	}
}

void
ImageProcessing::AddImages(uchar *image1, uchar *image2, uchar *result, int width, int height, int channels)
///
/// Adds two images by adding their color components at every pixel. Values are clipped at 0 and 255.
///
/// @param image1
///  The first image to be added.
///
/// @param image2
///  The second image to be added.
///
/// @param result
///  The image that results from adding the two images.
///
/// @param width
///  The width of the images.
///
/// @param height
///  The height of the images.
///
/// @param channels
///  The number of color channels that the images contain. 4 by default.
///
/// @return
///  Nothing.
///
{
	for( int j = 0; j < height; j++ )
	{
		for( int i = 0; i < width; i++ )
		{
			for( int c = 0; c < channels; c++ )
			{
				int pixel = j*width*channels + i*channels + c;
				int total = image1[pixel] + image2[pixel];
				if( total < 0 ) total = 0;
				if( total > 255 ) total = 255;
				result[pixel] = total;
			}
		}
	}
}

void
ImageProcessing::AddImages(double *image1, double *image2, double *result, int width, int height, int channels)
///
/// Adds two images by adding their color components at every pixel. Values are clipped at 0 and 255.
///
/// @param image1
///  The first image to be added.
///
/// @param image2
///  The second image to be added.
///
/// @param result
///  The image that results from adding the two images.
///
/// @param width
///  The width of the images.
///
/// @param height
///  The height of the images.
///
/// @param channels
///  The number of color channels that the images contain. 4 by default.
///
/// @return
///  Nothing.
///
{
	for( int j = 0; j < height; j++ )
	{
		for( int i = 0; i < width; i++ )
		{
			for( int c = 0; c < channels; c++ )
			{
				int pixel = j*width*channels + i*channels + c;
				int total = image1[pixel] + image2[pixel];
				if( total < 0 ) total = 0;
				if( total > 255 ) total = 255;
				result[pixel] = total;
			}
		}
	}
}