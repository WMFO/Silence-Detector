#include <silenceDetector.hpp>
#include <detectorTypes.hpp>
#include <silenceException.hpp>
#include <curl/curl.h>
#include <fstream>
#include <ctime>

detectorAction::detectorAction()
{
	counts = 0;
	countsThreshold = 0;
	silenceThreshold = 0.0;
}

detectorAction::~detectorAction()
{
	//empty
}

double detectorAction::getSilenceThreshold()
{
	return silenceThreshold;
}

void detectorAction::setSilenceThreshold(const double in)
{
	if (in > 0.0)
	{
		silenceThreshold = in;
	}
	else
	{
		throw silenceException("silenceThreshold must be greater than 0.");
	}
}

unsigned long detectorAction::getSilenceCount()
{
	return countsThreshold;
}

void detectorAction::setSilenceCount(const unsigned long in)
{
	countsThreshold = in;
}

void detectorAction::sendMeasurement(const double in)
{
	if (in > silenceThreshold)
	{
		counts = 0;
		return;
	}

	if (++counts == countsThreshold)
	{
		doAction();
		return;
	}
}

void detectorAction::doAction()
{
	//empty
}

//detectorEmail class

detectorEmail::detectorEmail()
{
	//empty
}

detectorEmail::~detectorEmail()
{
	//empty
}

extern "C" size_t emailCallback(void *, size_t, size_t, void *);

detectorEmail::detectorEmail(const std::string &to, const std::string &Server, const std::string &msg)
{
	emailTo = to;
	emailServer = Server;
	emailMessage = msg;
}

void detectorEmail::doAction()
{
	sendEmail(emailTo, emailServer, emailMessage);
}

void detectorEmail::sendEmail(const std::string &to, const std::string &server, const std::string &message)
{
    CURL *curl;
    CURLcode res;
    curl_slist *recipients = NULL;

    curl = curl_easy_init();
    if (!curl)
    {
        throw silenceException("Unable to initialize curl while sending alert email.");
    }

    curl_easy_setopt(curl, CURLOPT_URL, ("smtp://" + server).c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, "silenceDetector@wmfo.org");
    recipients = curl_slist_append(recipients, to.c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    //Add the headers
    std::string message2 = "To: " + to + "\nFrom: silenceDetector@wmfo.org\nSubject: Silence Alert\n\n"
     + message;

    curl_easy_setopt(curl, CURLOPT_READFUNCTION, emailCallback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &message2);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
        throw silenceException("Curl initialized, but sending of emails failed.");
        return;
    }

    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);

}

//Internal Function, use to access C Code
//Will erase the std::string pointed to by messageString
extern "C" size_t emailCallback(void *buf, size_t size, size_t numbmemb, void *messageString)
{
    std::string *message = (std::string *)messageString;
    size_t numBytes;
    std::cout << message->c_str() << std::endl;

    if (message->length() == 0)
        return 0;

    if (size*numbmemb <= message->length())
    {
        numBytes = size * numbmemb;
    }
    else
    {
        numBytes = message->length();
    }

    size_t actualBytesSent = message->copy((char *)buf, numBytes, 0);
    message->erase(0, actualBytesSent);

    return actualBytesSent;
}

//log class

detectorLog::detectorLog()
{
	//empty
}

detectorLog::~detectorLog()
{
	//empty
}


detectorLog::detectorLog(const std::string &msg, const std::string &_logFn)
{
	logMessage = msg;
	logFn = _logFn;
}

void detectorLog::doAction()
{
	writeLog();
}

void detectorLog::writeLog()
{
	std::fstream file;
	file.open(logFn.c_str(), std::ios_base::out | std::ios_base::app);

	if (file.fail())
		throw silenceException("Unable to open log file.");

	time_t timer;
	time(&timer);
	std::string logTime = ctime(&timer);
	logTime[logTime.length() - 1] = ':'; //remove the new line
	file << logTime << " " << logMessage << std::endl;

	file.close();	
}

