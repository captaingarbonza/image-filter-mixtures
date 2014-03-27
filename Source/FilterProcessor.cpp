///
/// Stores a library of possible image filters.
/// Calls filters in a separate thread when instructed.
///
/// Created by Crystal Valente.
///

#include "FilterProcessor.h"

#include "Filters/GlassPatternsFilter.h"
#include "Filters/LayeredStrokesFilter.h"
#include "Filters/PointillismFilter.h"

typedef boost::shared_ptr<Filter> filter_ptr;
typedef boost::shared_ptr<uchar> uchar_ptr;

using namespace std;

FilterProcessor::FilterProcessor()
///
/// Constructor.
///
{
	InitFilterLibrary();
}

FilterProcessor::~FilterProcessor()
///
/// Destructor.
///
{
    wait();
    mFilterLibrary.clear();
}

void
FilterProcessor::run()
///
/// Runs the thread work for the filter processing thread.
/// The thread starts running by calling the start() function.
/// Thread stops at the end of the function.
///
/// @return
///  Nothing.
///
{
	if( !mImage.isNull() )
	{
		QMutexLocker locker(&mutex);
		if( mFilterLibrary.find(mFilterName) != mFilterLibrary.end() )
		{
			QImage* result = mFilterLibrary[mFilterName]->RunFilter(&mImage);
			if( *result != mImage )
			{
		        mImage = *result;

		        // Pass the processed canvas to anyone who is interested
				emit FilterDone( mImage );
				emit FilterStatus( QString("Done!") );
			}
			else
			{
				emit FilterStatus( QString("Problem with results. Filter canceled!") );
			}
			delete result;
		}
		else
		{
			emit FilterStatus( QString("Filter not found. Filter canceled!") );
		}
	}
	else
	{
		emit FilterStatus( QString("No image to filter. Filter canceled!") );
	}
}

void
FilterProcessor::InitFilterLibrary()
///
/// Adds the default filters to the filter collection.
///
/// @return
///  Nothing.
{
	mFilterLibrary["glass_patterns"] = filter_ptr( new GlassPatternsFilter() );
	mFilterLibrary["layered_strokes"] = filter_ptr( new LayeredStrokesFilter() );
	mFilterLibrary["pointillism"] = filter_ptr( new PointillismFilter() );
}

void
FilterProcessor::StartFilter( string filter_name, QImage image )
///
/// Sets the current image and filter name and starts the thread to begin filtering.
///
/// @param filter_name
///  The name of the filter to be applied.
///
/// @param image
///  The image to be filtered.
///
/// @return
///  Nothing.
///
{
	QMutexLocker locker(&mutex);
	mImage = image.copy();
	mFilterName = filter_name;
	start();
}