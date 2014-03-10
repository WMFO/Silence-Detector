#ifndef DETECTORTYPES_H
#define DETECTORTYPES_H

#include <silenceDetector.hpp>

class detectorEmail : public detectorAction
{
	public:
		detectorEmail(const std::string&, const std::string&, const std::string&);
		detectorEmail();
		~detectorEmail();
		
	private:
		std::string emailTo;
		std::string emailServer;
		std::string emailMessage;
		void doAction();
		void sendEmail(const std::string&, const std::string&, const std::string&);

};

class detectorLog : public detectorAction
{
	public:
		detectorLog(const std::string&, const std::string&);
		detectorLog();
		~detectorLog();
		
	private:
		std::string logFn;
		std::string logMessage;
		void doAction();
		void writeLog();
};

#endif
