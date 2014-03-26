#ifndef _FILTER_PROCESSOR_H_
#define _FILTER_PROCESSOR_H_

#include <QApplication>
#include <QtWidgets>
#include <string>
#include <map>
#include <boost/shared_ptr.hpp>

#include "Filter.h"

class FilterProcessor : public QThread
{
	Q_OBJECT

	public:
		FilterProcessor();
		~FilterProcessor();

		void StartFilter( std::string filter_name, QImage image );

	signals:
		void FilterDone( QImage result );
		void FilterStatus( QString status_text );

	protected:
	    void run();

	private:
		void InitFilterLibrary();

		std::map<std::string, boost::shared_ptr<Filter> >  mFilterLibrary;

		QImage mImage;
		std::string mFilterName;

		QMutex mutex;
	    QWaitCondition condition;
};

#endif