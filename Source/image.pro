TARGET = ImageFilters
QT += widgets
DESTDIR = ../Build
RESOURCES = resources.qrc

HEADERS += \
	Filter.h \
	Filters/LayeredStrokesFilter.h \
	FilterProcessingThread.h \
	MainWindow.h \

SOURCES += \
	Filters/LayeredStrokesFilter.cpp \
	FilterProcessingThread.cpp \
    main.cpp \
    MainWindow.cpp \
    
