#include "LayeredStrokesFilter.h"

const int LayeredStrokesFilter::MAX_BRUSH_SIZE_DEFAULT = 7;
const int LayeredStrokesFilter::MIN_BRUSH_SIZE_DEFAULT = 2;
const int LayeredStrokesFilter::FIDELITY_THRESHOLD_DEFAULT = 200;
const int LayeredStrokesFilter::MINIMUM_POSSIBLE_BRUSH_SIZE = 1;
const int LayeredStrokesFilter::MAXIMUM_POSSIBLE_BRUSH_SIZE = 100;
const int LayeredStrokesFilter::MINIMUM_FIDELITY_THRESHOLD = 0;
const int LayeredStrokesFilter::MAXIMUM_FIDELITY_THRESHOLD = 600;

void RunFilter(QImage* source, QImage* destination, int max_brush_size, int min_brush_size, int error_threshold);
void DrawBrushStroke(QImage* source, QImage* destination, QPoint position, QColor color, int radius, int z_depth, uchar* depth_buffer, int max_stroke_length);
void DrawCircle(QImage* canvas, QPoint position, QColor color, int radius, int z_depth, uchar* depth_buffer);
void DrawHorizontalLine(QImage* canvas, int x_left, int x_right, int y, QColor color, int z_depth, uchar* depth_buffer);

double ColorDistance( QColor color1, QColor color2);

QImage*
RunLayeredStrokesFilter( QImage* source )
///
/// Filter function that is externally visible. Translates parameters into a form
/// that the algorithm understands and kicks off the algorithm.
///
/// @param image
///  The reference image that will be filtered.
///
/// @param canvas
///  A blank canvas that the result of the filtering will be painted on to.
///
/// @param parameters
///  A map of parameters in double form. The relevant ones will be extracted and converted
///  to an appropriate format for use with the filter. Any that are missing will be set to
///  default values.
///
/// @return
///  Nothing
///
{

	///
	/// Extract relevant parameters from the parameter list and set any that aren't
	/// available to their defaults.
	///
	int max_brush_size = LayeredStrokesFilter::MAX_BRUSH_SIZE_DEFAULT;
	int min_brush_size = LayeredStrokesFilter::MIN_BRUSH_SIZE_DEFAULT;
	int fidelity_threshold = LayeredStrokesFilter::FIDELITY_THRESHOLD_DEFAULT;

	///
	/// Make sure the parameters are within the allowed range for their parameter type.
	///
	max_brush_size = max_brush_size > LayeredStrokesFilter::MAXIMUM_POSSIBLE_BRUSH_SIZE ? 
		max_brush_size = LayeredStrokesFilter::MAXIMUM_POSSIBLE_BRUSH_SIZE : 
		( max_brush_size < LayeredStrokesFilter::MINIMUM_POSSIBLE_BRUSH_SIZE ? 
		LayeredStrokesFilter::MAXIMUM_POSSIBLE_BRUSH_SIZE : max_brush_size );
	min_brush_size = min_brush_size > LayeredStrokesFilter::MAXIMUM_POSSIBLE_BRUSH_SIZE ? 
		min_brush_size = LayeredStrokesFilter::MAXIMUM_POSSIBLE_BRUSH_SIZE : 
		( min_brush_size < LayeredStrokesFilter::MINIMUM_POSSIBLE_BRUSH_SIZE ? 
		LayeredStrokesFilter::MAXIMUM_POSSIBLE_BRUSH_SIZE : min_brush_size );
	min_brush_size = min_brush_size > max_brush_size ? max_brush_size : min_brush_size;
	fidelity_threshold = fidelity_threshold > LayeredStrokesFilter::MAXIMUM_FIDELITY_THRESHOLD ?
		LayeredStrokesFilter::MAXIMUM_FIDELITY_THRESHOLD :
		( fidelity_threshold < LayeredStrokesFilter::MINIMUM_FIDELITY_THRESHOLD ?
		LayeredStrokesFilter::MINIMUM_FIDELITY_THRESHOLD : fidelity_threshold );

	///
	/// Run the filter
	///
    QImage* canvas = new QImage(source->size(), QImage::Format_ARGB32);
	RunFilter( source, canvas, max_brush_size, min_brush_size, fidelity_threshold );
    return canvas;
}

void 
RunFilter(QImage* source, QImage* destination, int max_brush_size, int min_brush_size, int error_threshold)
///
/// Runs a filter that creates a painted image by building up a series of curved brush strokes
/// that approximate the reference image. Use three different brush sizes, a minimum, a maximum,
/// and one halfway in between.
///
/// @param source
///  The reference image that the filter will be performed on.
///
/// @param destination
///  A blank canvas that the result of the filter will be painted onto.
///
/// @param max_brush_size
///  The maximum brush size to be used. Must be at least 1 pixel.
///
/// @param min_brush_size
///  The minimum brush size to be used. Must be at least 1 pixel and less than or equal to the maximum brush size.
///
/// @param error_threshold
///  The error threshold to determine if a new brush stroke should be painted. The canvas is compared to the
///  reference image at each point and if the total error exceeds this threshold then a new stroke will be painted.
///  Must be between 0 and 300.
///
/// @return
///  Nothing
///
{
	///
	/// @todo [crystal 30.12.2012] Add asserts for values.
	///

	///
	/// Set up the set of brushes to be used.
	///
	int brushes[3] = { max_brush_size, (max_brush_size+min_brush_size)/2, min_brush_size };

	///
	/// Initialize canvas to white 
	///
	destination->fill(Qt::white);

	///
	/// Create the reference image which will be a version of image that is blurred with
	/// a kernel size equal to the current brush at each step.
	///
	QImage* reference_image = new QImage(source->size(), QImage::Format_ARGB32);

	///
	/// Create the depth buffer. This is used to give the strokes the appearance of being
	/// painted in a random order without the overhead of actually randomizing the order
	/// they are painted in. We would need to change this if this was extended to include
	/// strokes with transparency, but as all our strokes are opaque this speeds up the
	/// process considerably as we can just randomize the depth value of each stroke.
	///
	uchar* depth_buffer = new uchar[source->width()*source->height()];

	// Do process for each brush size
	for(int brush_index = 0; brush_index < 3; brush_index++) 
	{
		int current_brush_size = brushes[ brush_index ];
		///
		/// Clear the depth buffer
		///
        for(int i = 0; i < source->width()*source->height(); i++) {
            depth_buffer[i] = 0;
        }

		///
		/// Blur image using gaussian blurring, relative to the brush size
		///
		int blur_kernel = current_brush_size%2 == 0 ? current_brush_size + 1 : current_brush_size;
		// @todo: add blur algorithm
		*reference_image = *source;

		///
		/// For each position on a grid with spacing relative to the current brush size
		/// find the error between this grid point in the reference image and the canvas 
		/// that has been painted so far. If this is greater than the error threshold
		/// then add a new brush stroke to the depth buffer.
		int grid_size = current_brush_size <= 1 ? 2 : current_brush_size;
		for(int y = (int)grid_size/2; y < source->height(); y += grid_size) 
		{
			for(int x = (int)grid_size/2; x < source->width(); x += grid_size) 
			{
				const int x_min = fmax(x - grid_size/2, 0 );
				const int x_max = fmin( x_min + grid_size + 1, source->width() );
				const int y_min = fmax(y - grid_size/2, 0 );
				const int y_max = fmin( y_min + grid_size + 1, source->height() );

				///
				/// Find total error of neighbouring region
				/// Also store max error to save processing later
				///
				double total_error = 0.0;

				double max_error = 0.0;
				QPoint max_error_point;
				for(int j = y_min; j < y_max; ++j) 
				{
					for(int i = x_min; i < x_max; ++i) 
					{
						QColor canvas_color = QColor(destination->pixel(i, j));
						QColor reference_color = QColor(reference_image->pixel(i, j));
						double new_diff = ColorDistance(canvas_color, reference_color);

						total_error += new_diff;
						if( new_diff > max_error ) 
						{
							max_error_point = QPoint(i, j);
							max_error = new_diff;
						}
					}
				}

				// If the error is above the threshold, add a stroke to the buffer
				if(current_brush_size == 1)
				{ 
					total_error = total_error/2;
				}
				if(total_error/grid_size > error_threshold) 
				{
					///
					/// Render a new stroke at the point of maximum error with the color
					/// defined by the reference image at this point where the length of the stroke
					/// is 4 times the length of the largest brush stroke. Give this stroke a random depth value.
					///
					/// @todo [crystal 30.12.2012] Do we want to set the maximum stroke length manually?
					///
                    //
                    // @todo [crystal 05.03.2014] Malloc error is in here somewhere.
                    //  reference_image or destination being freed?
					DrawBrushStroke(reference_image, destination, max_error_point, QColor(reference_image->pixel(max_error_point.x(), max_error_point.y())), current_brush_size, rand()%256, depth_buffer, brushes[0]*4);
				}

			}
		}
	}
	delete [] depth_buffer;
	delete reference_image;
}

void 
DrawBrushStroke(QImage* source, QImage* destination, QPoint position, QColor color, int radius, int z_depth, uchar* depth_buffer, int max_stroke_length)
///
/// Draws a brush stroke onto the given canvas with the specified parameters.
/// Brush stokes are circles drawn at a series of control points until the maximum
/// length is reached or the error in color with the reference image at the point becomes too great.
///
/// @param reference_image
///  The reference image used to determine the color error with the stroke being drawn.
///
/// @param canvas
///  The canvas to draw the brush stroke onto.
///
/// @param position
///  The position to draw the stroke at.
///
/// @param color
///  The color of the new brush stroke.
///
/// @param radius
///  The radius of the brush to draw the stroke with.
///
/// @param z_depth
///  The depth of the new stroke (i.e. whether or not other strokes will be drawn over top of it).
///
/// @param depth_buffer
///  The depth buffer to store the z_depth of each stroke in.
///
/// @param max_stroke_length
///  The maximum stroke length to draw.
///
/// @return
///  Nothing
///
{
	///
	/// Set the distance between the control points. More space means less processing
	/// but you can see the spaces between the circles if its too small.
	///
	float control_point_distance = radius/4.0 < 1 ? 1.0 : radius/4.0;
	
	int minimum_stroke_length = 1; /// Brush strokes must be at least this long, regardless of color error

	///
	/// Draw a circle at the first point.
	DrawCircle(destination, position, color, radius, z_depth, depth_buffer);

	float x = position.x();
	float y = position.y();
	float d_x = 0.0;
	float d_y = 0.0;

	///
	/// Place control points until the stroke length reaches the maximum or the color
	/// error with the maximum becomes too great.
	/// 
	for(int stroke_length_count = 0; stroke_length_count < max_stroke_length; ++stroke_length_count) 
	{
		int gradient_x = 0;
		int gradient_y = 0;

		///
		/// Find the convolution of the sobel operator with the luminance of the
		/// reference image to determine the direction of the gradient which will
		/// tell us the current direction of the stroke.
		///
		int sobelX[3][3] = { {1, 0, -1}, {2, 0, -2}, {1, 0, -1} };
		int sobelY[3][3] = { {1, 2, 1}, {0, 0, 0}, {-1, -2, -1} };
		for(int y_incr = -1; y_incr < 2; ++y_incr) 
		{
			for(int x_incr = -1; x_incr < 2; ++x_incr) 
			{
				int temp_x = (int)x + x_incr < 0 ? 0 : 
					( (int)x + x_incr >= source->width() ? source->width() - 1 : (int)x + x_incr);
				int temp_y = (int)y + y_incr < 0 ? 0 : 
					( (int)y + y_incr >= source->height() ? source->height() - 1 : (int)y + y_incr );
				QColor reference_color = QColor(source->pixel(temp_x, temp_y));
				float reference_luminance = reference_color.red()*0.3 + reference_color.green()*0.59 + reference_color.blue()*0.11;
				gradient_x += reference_luminance*sobelX[x_incr + 1][y_incr + 1];
				gradient_y += reference_luminance*sobelY[x_incr + 1][y_incr + 1];
			}
		}
		
		///
		/// Why the 255? Its an integer, this wont scale it to 256...
		///
		gradient_x = gradient_x*255;
		gradient_y = gradient_y*255;

		///
		/// Calculate the position of the new control point.
		///
		if(control_point_distance*sqrt(gradient_x*gradient_x + gradient_y*gradient_y) >= 1) 
		{
			int new_dx = gradient_x;
			int new_dy = -1*gradient_y;

			if(stroke_length_count > 1 && new_dx*d_x + new_dy*d_y < 0) {
				new_dx = -1*new_dx;
				new_dy = -1*new_dy;
			}
			d_x = new_dx;
			d_y = new_dy;
		} 
		else 
		{
			break;
		}

		float length = sqrt(d_x*d_x + d_y*d_y);
		x += control_point_distance*(d_x/length);
		y += control_point_distance*(d_y/length);
		
		///
		/// Break if the new control point is off the edge of the canvas.
		///
		if(x < 0 || x >= destination->width() || y < 0 || y >= destination->height())
		{ 
			break;
		}
		
		///
		/// Calculate the color difference between the reference image and the canvas and
		/// the reference image and the stroke at the new control point.
		///
		QColor reference_color = QColor(source->pixel((int)x, (int)y));
		QColor canvas_color = QColor(destination->pixel((int)x, (int)y));
		double canvas_color_error = ColorDistance(reference_color, canvas_color);
		double stroke_color_error = ColorDistance(reference_color, color);

		///
		/// Break if the canvas is a better approximation of the reference image at this point
		/// than the stroke color.
		///
		if(stroke_length_count >= minimum_stroke_length && canvas_color_error < stroke_color_error ) 
		{
			break;
		}

		///
		/// Draw a circle at the control point.
		///
		DrawCircle(destination, QPoint(x, y), color, radius, z_depth, depth_buffer);
	}
}

void 
DrawHorizontalLine(QImage* canvas, int x_left, int x_right, int y, QColor color, int z_depth, uchar* depth_buffer) 
///
/// Draws a horizontal line on the canvas. Also stores the points on the line in the given depth buffer.
///
/// @param canvas
///  The image to paint the horizontal line onto.
///
/// @param x_left
///  The left most x point of the line.
///
/// @param x_right
///  The right most x point of the line.
///
/// @param y
///  The vertical position of the line.
///
/// @param color
///  The color of the line.
///
/// @param z_depth
///  The depth of the line in relation to any other lines that are drawn.
///
/// @param depth_buffer
///  The depth buffer to determine which point on the line should be drawn.
///
/// @return
///  Nothing
///
{
	
	if(y >= 0 && y < canvas->height()) 
	{
		if(x_left > x_right) 
		{
			int temp = x_left;
			x_left = x_right;
			x_right = temp;
		}
		
		while(x_left <= x_right) 
		{
			if(x_left >= canvas->width())
			{
				break;
			}
			
			if(x_left >= 0 && z_depth > depth_buffer[y*canvas->width() + x_left]) 
			{
				canvas->setPixel(QPoint(x_left, y), qRgb(color.red(), color.green(), color.blue()));
				depth_buffer[y*canvas->width() + x_left] = z_depth;
			}
			
			++x_left;
		}
	}
}

void 
DrawCircle(QImage* canvas, QPoint position, QColor color, int radius, int z_depth, uchar* depth_buffer) 
///
/// Draw a circle of a given color and radius on the given canvas.
///
/// @param canvas
///  The canvas to draw the circle on to.
///
/// @param position
///  The center point of the circle to be drawn.
///
/// @param color
///  The color of the circle to be drawn.
///
/// @param radius
///  The radius of the circle to be drawn.
///
/// @param z_depth
///  The depth of the circle being drawn within the depth buffer.
///
/// @param depth_buffer
///  The depth buffer to determine which pixels should be drawn.
///
/// @return
///  Nothing
///
{
	int x = -1;
	int y = radius;
	int d = 1 - radius;
	int delta_e = -1;
	int delta_se = (-radius << 1) + 3;

	while (y > x) 
	{
		delta_e += 2;
		x++;

		if (d < 0) 
		{
			d += delta_e;
			delta_se += 2;
		} 
		else 
		{
			d += delta_se;
			delta_se += 4;
			y--;
		}

		DrawHorizontalLine(canvas, position.x() + x, position.x() - x, position.y() + y, color, z_depth, depth_buffer);
		DrawHorizontalLine(canvas, position.x() + y, position.x() - y, position.y() + x, color, z_depth, depth_buffer);
		DrawHorizontalLine(canvas, position.x() + y, position.x() - y, position.y() - x, color, z_depth, depth_buffer);
		DrawHorizontalLine(canvas, position.x() + x, position.x() - x, position.y() - y, color, z_depth, depth_buffer);
	}
}

double absolute( double x)
{
	if( x >= 0) return x;
	else return -1.0*x;
}

double 
ColorDistance( QColor color1, QColor color2)
///
/// Just a simple rgb comparison at the moment, but is that the best way?
///
/// @todo: Research alternative color comparisons.
///  http://www.compuphase.com/cmetric.htm
///
{
	return absolute(	(color1.red() - color2.red())*(color1.red() - color2.red()) +
						(color1.green() - color2.green())*(color1.green() - color2.green()) +
						(color1.blue() - color2.blue())*(color1.blue() - color2.blue())	);
}
