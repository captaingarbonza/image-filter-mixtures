#ifndef _MAIN_WINDOW_
#define _MAIN_WINDOW_

#include <QApplication>
#include <QDesktopWidget>
#include <QMainWindow>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QLabel>
#include <QScrollArea>
#include <QImage>
#include <QFileDialog>
#include <QSettings>

class MainWindow : public QMainWindow
{
	Q_OBJECT

	public:
		MainWindow();
		~MainWindow();

		void Resize();
		void Center();

	public slots:
    	void Open();
    	void Save();

	private:
		QImage mOriginalImage;
		QScrollArea mScrollArea;
		QLabel mImageContainer;

		QMenu* mFileMenu;

		QAction* mOpenAction;
		QAction* mSaveAction;
};

#endif