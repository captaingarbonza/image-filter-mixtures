#include "PointillismFilter.h"
#include "HelperFunctions/ImageProcessing.h"
#include "HelperFunctions/Drawing.h"

void Pointillize( QImage * img, QImage * canvas, int radius, double strength );
void BaseLayer( QImage* img, QImage* canvas, int radius, double strength );
void MainLayer( QImage* img, QImage * canvas, int radius, double strength );
void EdgeLayer( QImage* img, QImage * canvas, int radius, double hue_distortion, double strength );

void DrawRandomCircle( QImage * img, QPoint pos, QColor color, int radius, int z, uchar* depth_buffer );
int GetPaletteHuePosition( int hue );
int GetRandomNeighbour( int pos );
int ChangeSaturation( int sat, double val, double t, double scale );
int ChangeHue(double v);

int chevreul[12] = 	{5, 20, 35, 45, 58, 80, 140, 170, 215, 244, 265, 285};

PointillismFilter::PointillismFilter()
{

}

QImage*
PointillismFilter::RunFilter( QImage* source )
///
/// Runs the pointillistic image filter that attempts to make a photograph
/// look like a pointillistic painting.
///
/// @param source
///  The reference image.
///
/// @return
///  The filtered result.
///
{
	QImage* canvas = new QImage(source->size(), QImage::Format_ARGB32);
	Pointillize( source, canvas, 5, 1.0 );
	return canvas;
}

void 
Pointillize(QImage * img, QImage * canvas, int radius, double strength)
/// 
/// Changes a given image to a pointillistic painting style.
/// Uses poisson disks for point placement. 
/// Colors are chosen which are isoluminant and approximate the average local color.
///
/// @param img
///  The reference image.
///
/// @param canvas
///  The image to store the result of the filter.
///
/// @param radius
///  The radius of the points to be painted.
///
/// @param strength
///  The strength of the pointillistic filter, where 1.0 is very strong and 0.0 is very weak.
///
/// @return
///  Nothing.
///
{
	*canvas = img->copy();
	if( strength > 0.0 ) 
	{
		BaseLayer( img, canvas, radius*3, strength );
		MainLayer( img, canvas, radius, strength );
		EdgeLayer( img, canvas, radius, 0.2, strength);
	}
}

void 
BaseLayer( QImage* img, QImage* canvas, int radius, double strength)
///
/// Covers the canvas in large points. Hues are taken from the palette
///  but no color distortion is added at this point.
///
/// @param img
///  The reference image.
///
/// @param canvas
///  The canvas to store the filtered image.
///
/// @param radius
///  The radius of the points being used for the pointillism algorithm 
///  (actual point radius used will be larger for this stage of the algorithm).
///
/// @param strength
///  The strength of the pointillistic filter, where 1.0 is very strong and 0.0 is very weak.
///
/// @return
///  Nothing.
///
{
	// Adjust the point radius based on the strength of the filter
	if(strength < 0.5) {
		int new_radius = (int)radius*strength*2;
		if(1.0*new_radius < radius*strength*2) new_radius++;
		radius = new_radius;
		if(radius < 3) radius = 3;
	}

	// Clear the depth buffer ready for drawing
	uchar* depth_buffer = new uchar[img->width()*img->height()];
	for( int i = 0; i < img->width()*img->height(); i++ )
	{
		depth_buffer[i] = 0;
	}

	// Get a poisson disk sampling of the area, and repaint the sampled areas with a brush of small radius
	int spacing = radius*2;
	std::vector<QPoint> poisson = ImageProcessing::GetPoissonDisks(canvas->width(), canvas->height(), spacing);

	while(!poisson.empty()) {
		QPoint pos = poisson.back();
		poisson.pop_back();

		// Get the hue at this point and find the closest hue in the color palette
		QColor hsv = QColor(img->pixel(pos)).toHsv();
		int hue = hsv.hue();
		int sat = hsv.saturation();
		int val = hsv.value();

		hue = chevreul[ GetPaletteHuePosition( hue ) ];

		// Paint a point of the chosen hue at a random depth value
		hsv.setHsv(hue, sat, val);
		int z = rand()%256;
		DrawRandomCircle(canvas, pos, hsv.toRgb(), radius, z, depth_buffer);
	}
	poisson.clear();
	delete [] depth_buffer;
}



void 
MainLayer( QImage* img, QImage* canvas, int radius, double strength )
///
/// Paint the main pointillism layer, adding smaller details and more color distortion.
/// Points are painted where the color error between the canvas and the original image
/// is high. Color difference is based on color intensity (i.e. brightness) to avoid
/// difference due to hue distortion. Saturation distortion and divisionism are applied
/// in addition to palette restriction.
///
/// @param img
///  The reference image.
///
/// @param canvas
///  The canvas to be painted to.
///
/// @param radius
///  The radius of the points being painted.
///
/// @param strength
///  The strength of the pointillistic filter, where 1.0 is very strong and 0.0 is very weak.
///
/// @return
///  Nothing.
///
{
	// Adjust the point radius based on the strength of the filter
	if(strength < 0.5) 
	{
		int new_radius = (int)radius*strength*2;
		if(1.0*new_radius < radius*strength*2) new_radius++;
		radius = new_radius;
		if(radius < 1) radius = 1;
	}

	// Get gray scale of original image
	uchar* gray = new uchar[img->width()*img->height()];
	ImageProcessing::ConvertToOneChannel( img->bits(), gray, img->width(), img->height() );

	// Blur the grayscale image
	uchar* smoothed_gray = new uchar[img->width()*img->height()];
	int kernel = radius;
	if(kernel%2 == 0) kernel++;
	if(kernel < 3) kernel = 3;
	ImageProcessing::GaussianBlur( gray, smoothed_gray, img->width(), img->height(), 1, kernel );
	delete [] gray;

	// Clear the depth buffer ready for painting
	uchar* depth_buffer = new uchar[img->width()*img->height()];
	for( int i = 0; i < img->width()*img->height(); i++ )
	{
		depth_buffer[i] = 0;
	}

	// At each grid point, find maximum error based on difference
	// between intensity at canvas and intensity of blurred image
	// Paint stroke at this location
	for( int y = (int)radius/2; y < img->height(); y += radius ) 
	{
		for( int x = (int)radius/2; x < img->width(); x += radius ) 
		{
			int total_error = 0;
			int max_error = 0;
			QPoint max_error_at = QPoint(0, 0);

			// Get error of each pixel in the neighbourhood
			int min_x = x - radius/2;
			int min_y = y - radius/2;
			int max_x = x + radius/2;
			int max_y = y + radius/2;

			if(min_x < 0) min_x = 0;
			if(min_y < 0) min_y = 0;
			if(max_x >= img->width()) max_x = img->width() - 1;
			if(max_y >= img->height()) max_y = img->height() - 1;

			for( int j = min_y; j <= max_y; j++ ) 
			{
				for( int i = min_x; i <= max_x; i++ ) 
				{
					
					// Get error at this pixel
					int intensity = QColor(canvas->pixel( i, j )).toHsv().value();
					int error = abs(intensity - smoothed_gray[j*img->width() + i]);

					// Update error stats
					total_error += error;
					if( error > max_error ) 
					{
						max_error = error;
						max_error_at = QPoint( i, j );
					}
				}
			}

			// If the total error is above a threshold
			// Paint a stroke at the area of max error
			if( total_error > 10*strength ) 
			{
				QColor hsv = QColor(img->pixel( x, y )).toHsv();
				int hue = hsv.hue();
				int sat = hsv.saturation();
				int v = hsv.value();

				// Find closest hue in palette, make a method that does this
				int new_pos = GetPaletteHuePosition( hsv.hue() );
				if( (rand()%100)/100.0 < strength ) 
				{
					hue = GetRandomNeighbour(new_pos);
				} 
				else 
				{
					hue = chevreul[new_pos];
				}
				
				sat = ChangeSaturation(sat, v, 0.35*strength, strength);
				hsv.setHsv( hue, sat, v );

				int z = rand()%256;
				DrawRandomCircle(canvas, max_error_at, hsv.toRgb(), radius, z, depth_buffer);
			}
		}
	}
	delete [] smoothed_gray;
	delete [] depth_buffer;
}

void 
EdgeLayer(QImage* img, QImage* canvas, int radius, double hue_distortion, double strength)
///
/// This final layer repaints over areas determined to be edges in order to bring smaller details
/// that have been covered by points back into the picture. The same color distortions are used
/// as in the main layer.
///
/// @param img
///  The reference image.
///
/// @param canvas
///  The canvas to be painted to.
///
/// @param radius
///  The radius of the points to be painted.
///
/// @param hue_distortion
///  Determines the strength of the hue distortion where 1.0 is very distorted and 0.0 is not distorted.
///
/// @param strength
///  The strength of the pointillistic filter, where 1.0 is very strong and 0.0 is very weak.
///
/// @return
///  Nothing.
///
{
	hue_distortion = hue_distortion*strength;
	if( strength < 0.5 ) 
	{
		int new_radius = (int)radius*strength*2;
		if( 1.0*new_radius < radius*strength*2 ) new_radius++;
		radius = new_radius;
		if( radius < 1 ) radius = 1;
	}
	
	uchar* edges = new uchar[img->width()*img->height()];
	ImageProcessing::CannyEdgeDetection( img->bits(), edges, img->width(), img->height(), 4 );

	uchar* gray = new uchar[img->width()*img->height()];
	ImageProcessing::ConvertToOneChannel( img->bits(), gray, img->width(), img->height());

	uchar* smoothed_gray = new uchar[img->width()*img->height()];
	int kernel = radius;
	if(kernel%2 == 0) kernel++;
	if(kernel < 3) kernel = 3;
	ImageProcessing::GaussianBlur( gray, smoothed_gray, img->width(), img->height(), 1, kernel );
	delete [] gray;

	// Clear the depth buffer ready for painting
	uchar* depth_buffer = new uchar[img->width()*img->height()];
	for( int i = 0; i < img->width()*img->height(); i++ )
	{
		depth_buffer[i] = 0;
	}

	// If there is an edge, find the greatest error in the edge's neighbourhood
	// and at a new stroke at this point.
	for( int y = 0; y < img->height(); y++ ) 
	{
		for( int x = 0; x < img->width(); x++ ) 
		{
			if( edges[y*img->width() + x] > 0 ) 
			{

				// Find the brightest and and darkest spots in the neighbourhood
				int brightest = 0;
				int darkest = 255;
				QPoint brightest_pos = QPoint(0, 0);
				QPoint darkest_pos = QPoint(0, 0);
				for( int j = y - radius; j <= y + radius; j++ ) {
					for( int i = x - radius; i <= x + radius; i++ ) {
						if( i >= 0 && j >= 0 && i < img->width() && j < img->height() ) {
							int intensity = smoothed_gray[j*img->width() + i];
							if(intensity > brightest) {
								brightest_pos = QPoint(i, j);
								brightest = intensity;
							}
							if(intensity < darkest) {
								darkest_pos = QPoint(i, j);
								darkest = intensity;
							}
						}
					}
				}

				// For each position in the neighbourhood, find if most spots
				// are closer to the brightest or darkest
				int dark = 0;
				int bright = 0;
				for( int j = y - radius; j <= y + radius; j++ ) 
				{
					for( int i = x - radius; i <= x + radius; i++ ) 
					{
						if( i >= 0 && j >= 0 && i < img->width() && j < img->height() ) {
							int intensity = smoothed_gray[j*img->width() + i];
							int bright_diff = brightest - intensity;
							int dark_diff = intensity - darkest;
							if(bright_diff < dark_diff) 
							{
								bright++;
							} 
							else 
							{
								dark++;
							}
						}
					}
				}

				// Paint at the side that needs defining
				QPoint new_point;
				if(bright < dark && bright != 0) 
				{
					new_point = brightest_pos;
				} 
				else if(dark != 0) 
				{
					new_point = darkest_pos;
				} 
				else 
				{
					new_point = QPoint(x, y);
				}

				// Paint circle at this position
				QColor hsv = QColor(img->pixel( new_point )).toHsv();
				int hue = hsv.hue();
				int val = hsv.value();
				int sat = hsv.saturation();

				// Find closest hue in palette
				int new_pos = GetPaletteHuePosition( hsv.hue() );

				if( ( rand()%100)/100.0 < strength ) 
				{
					hue = GetRandomNeighbour( new_pos );
				} 
				else 
				{
					hue = chevreul[new_pos];
				}

				// Hue distortion
				double r = ( rand()%100 )/100.0;
				if( ( r < hue_distortion && new_pos != 8 ) || r < hue_distortion/3 ) 
				{
					int n = ChangeHue(smoothed_gray[new_point.y()*img->width() + new_point.x()]/256.0)*2;
					if( n > -1 ) 
					{
						new_pos = n;
						if( sat < 70 && val < 0.3 ) sat = 70;
					}
					hue = chevreul[new_pos];
				}
				sat = ChangeSaturation( sat, val, 0.35*strength, strength );
				int z = rand()%256;
				hsv.setHsv( hue, sat, val );
				DrawRandomCircle( canvas, new_point, hsv.toRgb(), radius - 1, z, depth_buffer );
			}
		}
	}

	delete [] smoothed_gray;
	delete [] edges;
	delete [] depth_buffer;
}

void 
DrawRandomCircle( QImage * img, QPoint pos, QColor color, int radius, int z, uchar* depth_buffer )
///
/// Draws a circle of random size.
///
/// @param img
///  The image to draw the circle to.
///
/// @param pos
///  The position of the center of the circle.
///
/// @param color
///  The color of the circle.
///
/// @param radius
///  The rough radius of the circle. Will either be increased by 1, decreased by 1, or left the same.
///
/// @param z
///  The depth of the circle in the depth_buffer
///
/// @param depth_buffer
///  The depth_buffer of the image.
///
/// @return
///  Nothing.
/// 
{
	int prob = rand()%4;
	if( prob < 1 ) 
	{
		radius++;
	} 
	else if( prob < 2 )  
	{
		radius--;
	}
	Drawing::DrawCircle( img, pos, color, radius, z, depth_buffer );
}

int 
GetPaletteHuePosition( int hue )
///
/// Returns the position in the color palette of the hue closest to a given hue.
///
/// @param hue
///  We want to find the hue in the palette that is closest to this hue.
///
/// @return
///  The position in the palette of the hue closest to a given hue.
///
{
	int new_pos = 0;
	int min_dist = 360;
	for( int j = 0; j < 12; j++ ) 
	{
		int dist1 = abs( hue - chevreul[j] );
		int dist2 = abs( chevreul[j] - hue + 360 );
		if( dist1 < min_dist ) 
		{
			min_dist = dist1;
			new_pos = j;
		}
		if(dist2 < min_dist)
		{
			min_dist = dist2;
			new_pos = j;
		}
	}

	return new_pos;
}

int 
GetRandomNeighbour(int pos) 
///
/// Find a random neighbor close to a given position in the chevreul color wheel.
///
/// @param pos
///  The position in the color wheel to look for a neighbor close to.
///
/// @return
///  The resulting random near hue that was found.
///
{
	bool blue = pos == 8 || pos == 9;
	int prob = rand()%4;
	if( prob < 1 ) 
	{
		pos--;
	} 
	else if( (blue && prob > 4) || (!blue && prob > 2 ) )
	{
		pos++;
	}

	if(pos < 0) pos = 12 + pos;
	return chevreul[pos];
}

// Changes the saturation depending on the saturation and brightness of the pixel
int 
ChangeSaturation(int sat, double v, double t, double scale)
///
/// Distorts a given saturation value depending on the saturation and brightness
/// and brightness of the pixel.
///
/// @param sat
///  The saturation of the pixel.
///
/// @param v
///  The brightness of the pixel.
///
/// @param t
///  @todo: what is this function actually doing?
///
/// @param scale
///
///
/// @return
///  The distorted saturation value.
///
{
	double prob = (rand()%100)/100.0;
	if( prob < t ) 
	{
		// Increase relative to how low luminance is
		// Only decrease saturation if luminance greater than 0.9
		int thresholds [4] = {(int)(220*scale), (int)(150*scale), (int)(80*scale), (int)(30*scale)};
		if( v < 0.2 ) 
		{
			int change = thresholds[0];
			if( sat < change) sat = change;
		} 
		else if( v < 0.25 ) 
		{
			int min_sat = thresholds[1];
			double increase = (0.25 - v)*10.0;
			min_sat += (int)((thresholds[0] - thresholds[1])*increase);
			if(sat < min_sat) sat = min_sat;
		} 
		else if( v < 0.4 ) 
		{
			int min_sat = thresholds[2];
			double increase = (0.4 - v)*10.0/1.5;
			min_sat += (int)((thresholds[1] - thresholds[2])*increase);
			if( sat < min_sat ) sat = min_sat;
		} 
		else if( v < 0.9 ) 
		{
			int min_sat = thresholds[3];
			double increase = (0.9 - v)*10.0/5.0;
			min_sat += (int)((thresholds[2] - thresholds[3])*increase);
			if( sat < min_sat ) sat = min_sat;
		} 
		else 
		{
			int max_sat = 30;
			double decrease = (1.0 - v)*10.0;
			max_sat -= 30*decrease;
			if(sat > max_sat) sat = max_sat;
		}	
	}
	return sat;
}

int 
ChangeHue(double v)
///
/// Returns a random hue where the probability of certain colors is relative
/// to a brightness value.
///
/// @param brightness
///  The brightness to determine the probability of each hue from.
///
/// @return
///  The chosen hue position in the chevreul color palette.
///
{
	// Change the hue with the probability of blue, red, or yellow, dependent on luminance

	double blue_prob = 0.0;
	double yellow_prob = 0.0;
	
	// Get blue's probability
	if( v < 0.30 ) 
	{
		blue_prob = 0.6;
	} 
	else 
	{
		double decrease = (v - 0.4)/0.1*0.5;
		blue_prob = 0.6 - decrease;
	} 

	// Get yellow's probability
	if( v > 0.55 ) 
	{
		yellow_prob = 0.6;
	} 
	else 
	{
		double decrease = (0.6 - v)/0.1*0.5;
		yellow_prob = 0.6 - decrease;
	}
	double random = (rand()%100)/100.0;
	if( random < blue_prob) 
	{
		// blue
		return 4;
	} 
	else if( random > 1.0 - yellow_prob ) 
	{
		// yellow
		return 2;
	} 
	else 
	{
		// red
		return 0;
	}
	return -1;
}