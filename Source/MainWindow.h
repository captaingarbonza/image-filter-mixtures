#ifndef _MAIN_WINDOW_
#define _MAIN_WINDOW_

#include <QApplication>
#include <QtWidgets>

#include "FilterProcessingThread.h"
#include "Filters/LayeredStrokesFilter.h"

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

        void StatusBarUpdated( QString status_text );
    
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

		bool mLayeredStrokesEnabled;
		bool mPointillismEnabled;
		bool mGlassPatternsEnabled;

		QScrollArea* mScrollArea;
		QLabel* mImageContainer;

		QLabel* mStatusText;

		QMenu* mFileMenu;

		QAction* mOpenAction;
		QAction* mSaveAction;

		QWidget* mCentralWidget;
		QWidget* mOptionsWidget;
		QVBoxLayout* mOptionsPaneLayout;
		QHBoxLayout* mMainLayout;
};

#endif