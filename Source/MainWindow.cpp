#include "MainWindow.h"

const int NUM_ZOOM_SIZES = 9;
const int ZOOM_SIZES_ARRAY[] = { 12, 25, 50, 75, 100, 125, 200, 300, 400 };

MainWindow::MainWindow()
///
/// Constructor
///
: mOriginalImage(),
  mScrollArea(),
  mImageContainer()
{
	setMaximumSize( QSize( 1250, 650 ) );
    setMinimumSize( QSize( 200, 200 ) );


    mImageContainer.setBackgroundRole( QPalette::Base );
    mImageContainer.setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
    mImageContainer.setScaledContents( true );

    mScrollArea.setBackgroundRole( QPalette::Dark );
    mScrollArea.setWidget( &mImageContainer );

   	mScrollArea.setMaximumSize( QSize( 1250, 650 ) );
    mScrollArea.setMinimumSize( QSize( 200, 200 ) );

	setCentralWidget( &mScrollArea );

	setWindowTitle( tr( "Artistic Image Filters " ) );

	mOpenAction = new QAction( tr("&Open"), this );
	mSaveAction = new QAction( tr("&Save"), this );	

	///
	/// Ghost out actions that need an image to be loaded and connect these
	/// actions so they turn back on when the results widget sets an image sucessfully.
	///
	mSaveAction->setDisabled( true );
	//connect( mMainImagePane, SIGNAL( ImageLoaded(bool) ), mSaveAction, SLOT( setEnabled(bool) ) );

	///
	/// Connect the actions to their slots in mMainImagePane
	///
	connect( mOpenAction, SIGNAL( triggered() ), this, SLOT( Open() ) );
	connect( mSaveAction, SIGNAL( triggered() ), this, SLOT( Save() ) );

	///
	/// Add the actions to the menu
	///
	mFileMenu = menuBar()->addMenu( tr("&File") );
	mFileMenu->addAction( mOpenAction );
	mFileMenu->addAction( mSaveAction );
}

MainWindow::~MainWindow()
///
/// Destructor
///
{
	delete mOpenAction;
	delete mSaveAction;
	delete mFileMenu;
}

void
MainWindow::Open()
///
/// Displays an open file dialog and sets the current image to be the one selected by the user.
///
/// @return
///  Nothing
///
{
	const QString default_dir_key("default_dir");
	QSettings app_settings;

	// Display an open file dialog at the last folder opened by the application
	QString file_name = QFileDialog::getOpenFileName( this, tr("Open File"), app_settings.value(default_dir_key).toString(), "Images (*.png *.bmp *.jpg)" );

    if( file_name != "" )
    {
    	// Set the directory of the opened file as the default directory in the application settings
    	QDir current_dir;
        app_settings.setValue(default_dir_key, current_dir.absoluteFilePath(file_name));

        // Load the chosen file as the unprocessed image
        mOriginalImage.load( file_name );
        mImageContainer.setPixmap( QPixmap::fromImage( mOriginalImage ) );
        mImageContainer.adjustSize();
        Resize();
    }
}

void
MainWindow::Save()
///
/// Displays a save file dialog and saves the current image to a file specified by the user.
///
/// @return
///  Nothing
///
{
	QString file_name = QFileDialog::getSaveFileName( this, tr("Save File"), "", "Image (*.png *.bmp *.jpg" );
}

void
MainWindow::Resize()
///
/// Resizes the window to the size of the current image if it is within the window size limits.
/// Centers the window based on the new size.
///
/// @return
///  Nothing
///
{
	this->resize( mImageContainer.size() );
	Center();
}

void
MainWindow::Center()
///
/// Positions the window in the center of the screen.
///
/// @return
///  Nothing
///
{
  int x = (QApplication::desktop()->width() - this->width()) / 2;
  int y = (QApplication::desktop()->height()  - this->height()) / 2;
  this->move(x,y);
}
