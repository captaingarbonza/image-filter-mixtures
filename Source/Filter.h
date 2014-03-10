#ifndef _FILTER_H_
#define _FILTER_H_

#include <QApplication>
#include <QtWidgets>

class Filter
{
	public:
		virtual ~Filter() {}
		virtual QImage* RunFilter( QImage* source ) = 0;
};
#endif