#include "FilterProcessingThread.h"
#include "Filters/LayeredStrokesFilter.h"

FilterProcessingThread::FilterProcessingThread(QObject *parent)
///
/// Constructor
///
: QThread(parent),
  mAbort( false ),
  mOriginalImage( NULL ),
  mCanvas( NULL ),
  mFilterReady( false )
{
    start();
}

FilterProcessingThread::~FilterProcessingThread()
///
/// Destructor
///
{
    {
		QMutexLocker locker(&mutex);
		mAbort = true;
        if( mOriginalImage != NULL )
        {
            delete mOriginalImage;
        }
        if( mCanvas != NULL )
        {
            delete mCanvas;
        }
		condition.wakeOne();
    }
    wait();
}

void
FilterProcessingThread::run()
///
/// Runs the thread work for the filter processing thread. 
/// Runs a loop which continues until the thread is aborted.
/// The thread starts running by calling the start() function.
///
/// @return
///  Nothing
///
{
 	forever 
 	{
        if( mAbort )
            break;

		if( mOriginalImage == NULL || !mFilterReady )
		{
            QMutexLocker locker(&mutex);
			condition.wait(&mutex);
			continue;
		}

		{
			QMutexLocker locker(&mutex);
            if( mCanvas != NULL )
            {
                delete mCanvas;
            }
			mCanvas = RunLayeredStrokesFilter( mOriginalImage );
			mFilterReady = false;
		}
		emit FilterProcessingComplete( *mCanvas );
    }
}

void 
FilterProcessingThread::SetImage( QImage* image )
///
/// Sets the image that will be filter by this thread.
/// Resets the canvas if it still contains an image from the last filter run.
///
/// @param image
///  The QImage image to be processed
///
/// @return
///  Nothing
{
    QMutexLocker locker(&mutex);
    if( mOriginalImage != NULL )
    {
        delete mOriginalImage;
    }
	mOriginalImage = image;
}

/*QImage
FilterProcessingThread::GetImage()
///
/// Returns the processed image canvas.
///
/// @return
///  The filtered image canvas stored as a QImage.
///
{
    QMutexLocker locker(&mutex);
    
    return mCanvas->copy();
}*/

void 
FilterProcessingThread::BeginProcessing()
///
/// Sets the current filter to the filter passed in and starts the processing thread.
///
/// @param filter
///  The name of the filter
///
/// @return
///  Nothing
///
{
    QMutexLocker locker(&mutex);
    mFilterReady = true;
    condition.wakeOne();
}
