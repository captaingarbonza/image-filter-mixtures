#ifndef _LAYERED_STROKES_FILTER_H
#define _LAYERED_STROKES_FILTER_H

#include "Filter.h"

class LayeredStrokesFilter: public Filter
{
	public:
		static const int MAX_BRUSH_SIZE_DEFAULT;
		static const int MIN_BRUSH_SIZE_DEFAULT;
		static const int FIDELITY_THRESHOLD_DEFAULT;
		static const int MINIMUM_POSSIBLE_BRUSH_SIZE;
		static const int MAXIMUM_POSSIBLE_BRUSH_SIZE;
		static const int MINIMUM_FIDELITY_THRESHOLD;
		static const int MAXIMUM_FIDELITY_THRESHOLD;

		LayeredStrokesFilter();

		QImage* RunFilter( QImage* source );
};

#endif