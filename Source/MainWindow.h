#ifndef _MAIN_WINDOW_
#define _MAIN_WINDOW_

#include <QApplication>
#include <QtWidgets>

#include "FilterProcessingThread.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

	public:
		MainWindow();
		~MainWindow();

		void Resize();
		void Center();
    
        void UpdateFilterGhostedStates();

	public slots:
    	void Open();
    	void Save();
    	void ApplyCurrentFilter();
    	void UpdateCurrentImage( QImage image );
    
        void LayeredStrokesStateChange( bool state );
        void PointillismStateChange( bool state );
        void GlassPatternsStateChange( bool state );
    
    signals:
        void LayeredStrokesToggled( bool enabled );
        void PointillismToggled( bool enabled );
        void GlassPatternsToggled( bool enabled );
    
        void LayeredStrokesUnghosted( bool ghosted );
        void PointillismUnghosted( bool ghosted );
        void GlassPatternsUnghosted( bool ghosted );

	private:
        void InitImagePane( QLayout* layout );
		void InitFilterControls( QLayout* layout );
		void InitMenuBar();

		FilterProcessingThread* mFilterProcessingThread;

		/// Or could have an enum of filter types and store the ones that are enabled. If more than two, ghost others out.
		/// Objects could store filter id? Overkill?
		bool mLayeredStrokesEnabled;
		bool mPointillismEnabled;
		bool mGlassPatternsEnabled;

		//QImage* mOriginalImage;
		QScrollArea* mScrollArea;
		QLabel* mImageContainer;

		QMenu* mFileMenu;

		QAction* mOpenAction;
		QAction* mSaveAction;

		QWidget* mCentralWidget;
		QWidget* mOptionsWidget;
		QVBoxLayout* mOptionsPaneLayout;
		QHBoxLayout* mMainLayout;
};

#endif