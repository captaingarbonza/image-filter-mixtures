#include "MainWindow.h"
//#include "LayeredStrokesFilter.h"

MainWindow::MainWindow()
///
/// Constructor
///
: mLayeredStrokesEnabled( false ),
  mPointillismEnabled( false ),
  mGlassPatternsEnabled( false )
{
    setWindowTitle( tr( "Artistic Image Filters " ) );
    
	setMaximumSize( QSize( 1250, 650 ) );
    setMinimumSize( QSize( 200, 200 ) );

    mFilterProcessingThread = new FilterProcessingThread( this );
    connect( mFilterProcessingThread, SIGNAL( FilterProcessingComplete(QImage) ), this, SLOT( UpdateCurrentImage(QImage) ) );
    
    mMainLayout = new QHBoxLayout;
    mMainLayout->setContentsMargins( 0, 0, 0, 0 );

    InitImagePane( mMainLayout );
	InitMenuBar();
	InitFilterControls( mMainLayout );

	mCentralWidget = new QWidget;
	mCentralWidget->setLayout( mMainLayout );
	mCentralWidget->setMinimumSize( mScrollArea->minimumWidth() + mOptionsWidget->minimumWidth(), mOptionsWidget->minimumHeight() );
	setCentralWidget( mCentralWidget );
	setMinimumSize( mCentralWidget->minimumSize() );

	mStatusText = new QLabel;
	statusBar()->addWidget(mStatusText);
	connect( mFilterProcessingThread, SIGNAL( FilterStatus(QString) ), this, SLOT( StatusBarUpdated(QString) ) );
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

	// Display an open file dialog at the default folder in the application setting
	QString file_name = QFileDialog::getOpenFileName( this, tr("Open File"), app_settings.value(default_dir_key).toString(), "Images (*.png *.bmp *.jpg)" );
    if( file_name != "" )
    {
    	// Save the chosen folder as the default in the application settings
    	QDir current_dir;
        app_settings.setValue(default_dir_key, current_dir.absoluteFilePath(file_name));

        // Load the chosen file as the unprocessed image
        QImage* image = new QImage;
        image->load( file_name );

        // Update the current image to this image
        UpdateCurrentImage( *image );

        // Pass ownership of the image to the filter processing thread
        mFilterProcessingThread->SetImage( image );

        // Resize window to fit new image
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
///
/// Sets up parameter values and triggers processing via the filter processing thread.
///
/// @return
///  Nothing
///
{
	StatusBarUpdated( QString("Processing...") );
	mFilterProcessingThread->BeginProcessing( new LayeredStrokesFilter() );
}

void
MainWindow::LayeredStrokesStateChange( bool state )
///
/// Receives state changes from a GUI element and toggles the layered strokes
/// filter accordingly.
///
/// @param state
///  True if the filter has been toggled on. False otherwise.
///
/// @return
///  Nothing
///
{
    mLayeredStrokesEnabled = state;
    emit LayeredStrokesToggled( mLayeredStrokesEnabled );
    UpdateFilterGhostedStates();
}

void
MainWindow::PointillismStateChange( bool state )
///
/// Receives state changes from a GUI element and toggles the pointillism
/// filter accordingly.
///
/// @param state
///  True if the filter has been toggled on. False otherwise.
///
/// @return
///  Nothing
///
{
    mPointillismEnabled = state;
    emit PointillismToggled( mPointillismEnabled );
    UpdateFilterGhostedStates();
}

void
MainWindow::GlassPatternsStateChange( bool state )
///
/// Receives state changes from a GUI element and toggles the glass patterns
/// filter accordingly.
///
/// @param state
///  True if the filter has been toggled on. False otherwise.
///
/// @return
///  Nothing
///
{
    mGlassPatternsEnabled = state;
    emit GlassPatternsToggled( mGlassPatternsEnabled );
    UpdateFilterGhostedStates();
}

void
MainWindow::UpdateFilterGhostedStates()
///
/// Updates filter controls to disable the last filter if two have already
/// been selected and reenable filters if a filter has been deselected.
/// Emits signals to the GUI to reflect this state.
///
/// @return
///  Nothing
///
{
    // A filter is ghosted if both other filters are selected.
    emit LayeredStrokesUnghosted( !( mPointillismEnabled && mGlassPatternsEnabled ) );
    emit PointillismUnghosted( !( mLayeredStrokesEnabled && mGlassPatternsEnabled ) );
    emit GlassPatternsUnghosted( !( mLayeredStrokesEnabled && mPointillismEnabled ) );
}

void
MainWindow::UpdateCurrentImage( QImage image )
///
/// Sets the pixmap displayed by the main window to a new image
///
/// @param image
///  QImage that will be converted giving us the new pixmap
///
/// @return
///  Nothing
{
	mImageContainer->setPixmap( QPixmap::fromImage( image ) );
    mImageContainer->adjustSize();
}

void
MainWindow::StatusBarUpdated( QString status_text )
{
	mStatusText->setText( status_text );
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
///
/// Sets up the GUI for the sidebar containing user parameters and controls
///
/// @param layout
///  Pointer to the layout the new controls are to be added to
///
/// @return
///  Nothing
///
{
    mOptionsWidget = new QWidget;
	mOptionsWidget->setMinimumSize( QSize( 200, 200 ) );
	mOptionsWidget->setMaximumWidth( 200 );
    
	mOptionsPaneLayout = new QVBoxLayout;
	mOptionsPaneLayout->setContentsMargins( 10, 10, 10, 10 );
	mOptionsWidget->setLayout( mOptionsPaneLayout );
    
    layout->addWidget( mOptionsWidget );
    
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

	mOptionsPaneLayout->addWidget(checkbox1);
	mOptionsPaneLayout->addWidget(checkbox2);
	mOptionsPaneLayout->addWidget(checkbox3);
    
    // Set up the checkboxes to change the state of the filters
    connect( checkbox1, SIGNAL( toggled(bool) ), this, SLOT( LayeredStrokesStateChange(bool) ) );
    connect( checkbox2, SIGNAL( toggled(bool) ), this, SLOT( PointillismStateChange(bool) ) );
    connect( checkbox3, SIGNAL( toggled(bool) ), this, SLOT( GlassPatternsStateChange(bool) ) );
    
    // Set up the checkboxes to receive state changes from the main window
    connect( this, SIGNAL( LayeredStrokesToggled(bool) ), checkbox1, SLOT( setChecked(bool) ) );
    connect( this, SIGNAL( PointillismToggled(bool) ), checkbox2, SLOT( setChecked(bool) ) );
    connect( this, SIGNAL( GlassPatternsToggled(bool) ), checkbox3, SLOT( setChecked(bool) ) );
    
    // Connect checkboxes so they can be ghosted out if too many are selected
    connect( this, SIGNAL( LayeredStrokesUnghosted(bool) ), checkbox1, SLOT( setEnabled(bool) ) );
    connect( this, SIGNAL( PointillismUnghosted(bool) ), checkbox2, SLOT( setEnabled(bool) ) );
    connect( this, SIGNAL( GlassPatternsUnghosted(bool) ), checkbox3, SLOT( setEnabled(bool) ) );

	QPushButton* apply_filters_button = new QPushButton(tr("Apply Filters"));
	connect( apply_filters_button, SIGNAL( clicked() ), this, SLOT( ApplyCurrentFilter() ) );
	mOptionsPaneLayout->addWidget(apply_filters_button);
}

void
MainWindow::InitMenuBar()
///
/// Initializes the menu bar and it's actions
///
/// @return
///  Nothing
///
{
	mOpenAction = new QAction( tr("&Open"), this );
	mSaveAction = new QAction( tr("&Save"), this );	

	///
	/// Ghost the save action as it needs an image to be loaded to function correctly.
	/// Connect the action so it turns back on when an image is set successfully.
	///
	mSaveAction->setDisabled( true );
	connect( mFilterProcessingThread, SIGNAL( ImageLoaded(bool) ), mSaveAction, SLOT( setEnabled(bool) ) );

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

void
MainWindow::InitImagePane( QLayout* layout )
///
/// Initializes the central pane the contains the image to be filtered.
///
/// @return
///  Nothing
///
{
    mScrollArea = new QScrollArea;
    mScrollArea->setBackgroundRole( QPalette::Dark );
    
    mImageContainer = new QLabel;
    mImageContainer->setBackgroundRole( QPalette::Base );
    mImageContainer->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
    mImageContainer->setScaledContents( true );
    
    mScrollArea->setWidget( mImageContainer );
   	mScrollArea->setMaximumSize( QSize( 1250, 650 ) );
    mScrollArea->setMinimumSize( QSize( 200, 200 ) );
    
    layout->addWidget( mScrollArea );
}
