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
        if( mOriginalImage != NULL )
        {
            delete mOriginalImage;
        }
        if( mCanvas != NULL )
        {
            delete mCanvas;
        }
        // Wake the thread up so it can abort successfully
        mAbort = true;
		condition.wakeOne();
    }
    // Wait for the thread to finish
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
            // We are not ready to filter so put the thread to sleep
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
        // Pass the processed canvas to anyone who is interested
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

void 
FilterProcessingThread::BeginProcessing()
///
/// Starts the processing thread.
///
/// @return
///  Nothing
///
{
    QMutexLocker locker(&mutex);
    mFilterReady = true;
    condition.wakeOne();
}
