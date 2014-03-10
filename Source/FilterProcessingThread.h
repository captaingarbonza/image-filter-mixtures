#ifndef _FILTER_PROCESSING_THREAD_H_
#define _FILTER_PROCESSING_THREAD_H_

#include <QtGui>
#include "Filters/LayeredStrokesFilter.h"

class Filter;

class FilterProcessingThread : public QThread
{
	Q_OBJECT

	public:
	    FilterProcessingThread(QObject *parent = 0);
	    ~FilterProcessingThread();

	    void SetImage( QImage* image );
	    void BeginProcessing( Filter* filter );

	signals:
        void ImageLoaded( bool loaded);
	    void FilterProcessingComplete( QImage result );

	protected:
	    void run();

	private:

	    QMutex mutex;
	    QWaitCondition condition;
	    bool mAbort;

	    QImage* mOriginalImage;
	    QImage* mCanvas;

	    Filter* mFilter;
};

#endif