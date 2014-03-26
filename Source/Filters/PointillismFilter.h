#ifndef _POINTILLISM_FILTER_H_
#define _POINTILLISM_FILTER_H_

#include "Filter.h"

class PointillismFilter : public Filter
{
	public:
		PointillismFilter();
		QImage* RunFilter( QImage* source );
};

#endif