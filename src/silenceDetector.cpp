#include <silenceDetector.hpp>
#include <silenceException.hpp>
#include <RMSMeasurement.hpp>
#include <cstdlib>

silenceDetector::silenceDetector()
{
	sDump = NULL;
	actionList.empty();
}

silenceDetector::~silenceDetector()
{
	if (sDump != NULL)
		delete sDump;

	std::list<detectorAction*>::iterator it;

	for (it = actionList.begin(); it != actionList.end(); ++it)
	{
		delete (*it);
	}
}

void silenceDetector::setIPAddress(const std::string &in)
{
	IPAddr = in;
}

void silenceDetector::setBindingAddress(const std::string &in)
{
	bindAddr = in;
}

void silenceDetector::setPort(const std::string &in)
{
	port = atoi(in.c_str());
}

void silenceDetector::addDetectorAction(detectorAction *in)
{
	if (in == NULL)
		throw silenceException("Attempted to add null detectorAction to queue.");

	actionList.push_back(in);
}

void silenceDetector::signalProcessingLoop()
{
	unsigned char buf[3*2*48000]; //Believe this is one second
	double measurementValue;
	try
	{
		sDump = new streamDumper(bindAddr, IPAddr, port);
	}
	catch (std::exception e)
	{
		std::cerr << "Unable to begin streamDumping..." << std::endl;
		std::cerr << e.what() << std::endl;
		return;
	}

	while(1)
	{
		try
		{
			sDump->getSocketData(buf, 3*2*48000);
			measurementValue = RMSMeasurement(buf, 3*2*48000).getMeasurementValue();
			//std::cout << "RMS Value: " << measurementValue << std::endl;

			std::list<detectorAction*>::iterator it;
			for (it = actionList.begin(); it != actionList.end(); ++it)
			{
				(*it)->sendMeasurement(measurementValue);
			}
		}
		catch (std::exception e)
		{
			std::cerr << "Error while monitoring stream..." << std::endl;
			std::cerr << e.what() << std::endl;
		}
	}
}

