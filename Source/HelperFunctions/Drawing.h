#ifndef _DRAWING_H_
#define _DRAWING_H_

#include <QtWidgets>

class Drawing
{
	public:
		static void DrawHorizontalLine(QImage* canvas, int x_left, int x_right, int y, QColor color, int z_depth, uchar* depth_buffer);
		static void DrawCircle(QImage* canvas, QPoint position, QColor color, int radius, int z_depth, uchar* depth_buffer);
};

#endif