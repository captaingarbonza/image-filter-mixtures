#include "MainWindow.h"

const int NUM_ZOOM_SIZES = 9;
const int ZOOM_SIZES_ARRAY[] = { 12, 25, 50, 75, 100, 125, 200, 300, 400 };

MainWindow::MainWindow()
///
/// Constructor
///
{
	setMaximumSize( QSize( 1250, 650 ) );
    setMinimumSize( QSize( 200, 200 ) );

	//setCentralWidget( mResults );

	setWindowTitle( tr( "Artistic Image Filters " ) );
}

MainWindow::~MainWindow()
///
/// Destructor
///
{

	//delete mResults;
}