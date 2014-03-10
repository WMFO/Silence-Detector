#ifndef SILENCEDETECTOR_H
#define SILENCEDETECTOR_H

#include <string>
#include <list>
#include <streamDumper.hpp>

class silenceDetector;
class detectorAction;

class silenceDetector
{
    public:

        silenceDetector();
        ~silenceDetector();
        void setIPAddress(const std::string&);
	void setBindingAddress(const std::string&);
	void setPort(const std::string&);
        void signalProcessingLoop();
        void addDetectorAction(detectorAction*);

    private:
        //Members
        streamDumper *sDump;
	std::string IPAddr;
	std::string bindAddr;
	unsigned int port;
        std::list<detectorAction*> actionList;

};


//base class
class detectorAction
{
	public:
		detectorAction();
		~detectorAction();
		double getSilenceThreshold();
		unsigned long getSilenceCount();
		void setSilenceCount(const unsigned long);
		void setSilenceThreshold(const double);
		void sendMeasurement(const double);

	private:
		virtual void doAction();
		unsigned long counts;
		unsigned long countsThreshold;
		double silenceThreshold;
};

#endif

