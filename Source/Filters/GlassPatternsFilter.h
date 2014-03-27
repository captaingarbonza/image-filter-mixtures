#ifndef _GLASS_PATTERNS_FILTER_H
#define _GLASS_PATTERNS_FILTER_H

#include "Filter.h"

class GlassPatternsFilter: public Filter
{
	public:
		static const double FILTER_STRENGTH_DEFAULT;
		GlassPatternsFilter();
		QImage* RunFilter( QImage* source );

};

#endif