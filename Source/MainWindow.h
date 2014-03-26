#ifndef _MAIN_WINDOW_
#define _MAIN_WINDOW_

#include <QApplication>
#include <QtWidgets>

#include "FilterProcessor.h"

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
    	void LoadImage( QImage image );

    	///
    	/// Temporary slots until there is enough functionality to use the checkboxes properly
    	///
    	void ApplyLayeredStrokes();
    	void ApplyPointillism();
    	void ApplyGlassPatterns();

    	void Undo();
    	void Redo();
    
        void LayeredStrokesStateChange( bool state );
        void PointillismStateChange( bool state );
        void GlassPatternsStateChange( bool state );

        void StatusBarUpdated( QString status_text );
    
    signals:
        void UndoIsActive(bool active);
    	void RedoIsActive(bool active);
    	void ImageLoaded(bool loaded);

        void LayeredStrokesToggled( bool enabled );
        void PointillismToggled( bool enabled );
        void GlassPatternsToggled( bool enabled );
    
        void LayeredStrokesUnghosted( bool ghosted );
        void PointillismUnghosted( bool ghosted );
        void GlassPatternsUnghosted( bool ghosted );

	private:
		void UpdateVisibleImage( QImage image );
		void UpdateEditMenuStates();

        void InitImagePane( QLayout* layout );
		void InitFilterControls( QLayout* layout );
		void InitMenuBar();

		FilterProcessor* mFilterProcessor;

		QImage* mCurrentImage;
		QImage* mPreviousImage;
		QImage* mNextImage;

		bool mLayeredStrokesEnabled;
		bool mPointillismEnabled;
		bool mGlassPatternsEnabled;

		QScrollArea* mScrollArea;
		QLabel* mImageContainer;

		QLabel* mStatusText;

		QMenu* mFileMenu;
		QMenu* mEditMenu;

		QAction* mOpenAction;
		QAction* mSaveAction;

		QAction* mUndoAction;
		QAction* mRedoAction;

		QWidget* mCentralWidget;
		QWidget* mOptionsWidget;
		QVBoxLayout* mOptionsPaneLayout;
		QHBoxLayout* mMainLayout;
};

#endif