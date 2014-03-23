#include "Drawing.h"

void 
Drawing::DrawHorizontalLine(QImage* canvas, int x_left, int x_right, int y, QColor color, int z_depth, uchar* depth_buffer) 
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
Drawing::DrawCircle(QImage* canvas, QPoint position, QColor color, int radius, int z_depth, uchar* depth_buffer) 
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