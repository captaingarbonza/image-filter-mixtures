#include "MainWindow.h"

MainWindow::MainWindow()
///
/// Constructor
///
: mLayeredStrokesEnabled( false ),
  mPointillismEnabled( false ),
  mGlassPatternsEnabled( false )
{
	setMaximumSize( QSize( 1250, 650 ) );
    setMinimumSize( QSize( 200, 200 ) );

    mFilterProcessingThread = new FilterProcessingThread( this );
    connect( mFilterProcessingThread, SIGNAL( FilterProcessingComplete(QImage) ), this, SLOT( UpdateCurrentImage(QImage) ) );

    //mOriginalImage = new QImage;

    mScrollArea = new QScrollArea;
    mScrollArea->setBackgroundRole( QPalette::Dark );

    mImageContainer = new QLabel;
    mImageContainer->setBackgroundRole( QPalette::Base );
    mImageContainer->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
    mImageContainer->setScaledContents( true );

    mScrollArea->setWidget( mImageContainer );
   	mScrollArea->setMaximumSize( QSize( 1250, 650 ) );
    mScrollArea->setMinimumSize( QSize( 200, 200 ) );

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

	mOptionsWidget = new QWidget;
	mOptionsWidget->setMinimumSize( QSize( 200, 200 ) );
	mOptionsWidget->setMaximumWidth( 200 );

	mOptionsPaneLayout = new QVBoxLayout;
	mOptionsPaneLayout->setContentsMargins( 10, 10, 10, 10 );
	mOptionsWidget->setLayout( mOptionsPaneLayout );

	InitFilterControls( mOptionsPaneLayout );

	mMainLayout = new QHBoxLayout;
	mMainLayout->addWidget( mScrollArea );
	mMainLayout->addWidget( mOptionsWidget );
	mMainLayout->setContentsMargins( 0, 0, 0, 0 );

	mCentralWidget = new QWidget;
	mCentralWidget->setLayout( mMainLayout );
	mCentralWidget->setMinimumSize( mScrollArea->minimumWidth() + mOptionsWidget->minimumWidth(), mOptionsWidget->minimumHeight() );
	setCentralWidget( mCentralWidget );
	setMinimumSize( mCentralWidget->minimumSize() );
}

MainWindow::~MainWindow()
///
/// Destructor
///
{
	delete mOpenAction;
	delete mSaveAction;
	delete mFileMenu;

	delete mCentralWidget;

	delete mFilterProcessingThread;
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
        QImage* image = new QImage;
        image->load( file_name );
        mImageContainer->setPixmap( QPixmap::fromImage( *image ) );
        mImageContainer->adjustSize();
        mFilterProcessingThread->SetImage( image );
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
MainWindow::ApplyCurrentFilter()
{
	mFilterProcessingThread->BeginProcessing();
}

void
MainWindow::UpdateCurrentImage( QImage image )
{
	mImageContainer->setPixmap( QPixmap::fromImage( image ) );
    mImageContainer->adjustSize();
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
	this->resize( mImageContainer->size() );
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

void
MainWindow::InitFilterControls( QLayout* layout )
{
	QCheckBox* checkbox1 = new QCheckBox(tr("Layered Strokes"));
	QCheckBox* checkbox2 = new QCheckBox(tr("Pointilism"));
	QCheckBox* checkbox3 = new QCheckBox(tr("Glass Patterns"));

	checkbox1->setStyleSheet("QCheckBox::indicator { width: 50 px; height: 50 px; } \
							  QCheckBox::indicator:unchecked { image: url(:/images/layered-strokes-unchecked.png); } \
							  QCheckBox::indicator:unchecked:hover { image: url(:/images/layered-strokes-hover.png); } \
							  QCheckBox::indicator:checked { image: url(:/images/layered-strokes-checked.png); }");

	checkbox2->setStyleSheet("QCheckBox::indicator { width: 50 px; height: 50 px; } \
							  QCheckBox::indicator:unchecked { image: url(:/images/layered-strokes-unchecked.png); } \
							  QCheckBox::indicator:unchecked:hover { image: url(:/images/layered-strokes-hover.png); } \
							  QCheckBox::indicator:checked { image: url(:/images/layered-strokes-checked.png); }");

	checkbox3->setStyleSheet("QCheckBox::indicator { width: 50 px; height: 50 px; } \
							  QCheckBox::indicator:unchecked { image: url(:/images/layered-strokes-unchecked.png); } \
							  QCheckBox::indicator:unchecked:hover { image: url(:/images/layered-strokes-hover.png); } \
							  QCheckBox::indicator:checked { image: url(:/images/layered-strokes-checked.png); }");

	//layout->addWidget(checkbox1);
	//layout->addWidget(checkbox2);
	//layout->addWidget(checkbox3);

	QPushButton* apply_filters_button = new QPushButton(tr("Apply Filters"));
	connect( apply_filters_button, SIGNAL( clicked() ), this, SLOT( ApplyCurrentFilter() ) );
	layout->addWidget(apply_filters_button);
}
