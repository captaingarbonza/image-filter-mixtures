include (boost.pri)

TARGET = ImageFilterMixtures
QT += widgets
DESTDIR = ../Build
RESOURCES = resources.qrc

HEADERS += \
	Filter.h \
	Filters/LayeredStrokesFilter.h \
	Filters/PointillismFilter.h \
	FilterProcessor.h \
	HelperFunctions/Drawing.h \
	HelperFunctions/ImageProcessing.h \
	MainWindow.h \

SOURCES += \
	Filters/LayeredStrokesFilter.cpp \
	Filters/PointillismFilter.cpp \
	FilterProcessor.cpp \
	HelperFunctions/Drawing.cpp \
	HelperFunctions/ImageProcessing.cpp \
    main.cpp \
    MainWindow.cpp \
    
