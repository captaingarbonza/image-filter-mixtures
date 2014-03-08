#ifndef _FILTER_H_
#define _FILTER_H_

#include <QApplication>
#include <QtWidgets>

class Filter
{
	public:
		virtual ~Filter() {}
		virtual void RunFilter( QImage* source, QImage* destination ) = 0;
};
#endif