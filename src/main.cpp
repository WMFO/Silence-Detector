#include <silenceDetector.hpp>
#include <detectorTypes.hpp>
#include <rapidxml.hpp>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <map>
#include <unistd.h>

void processDetectorAction(rapidxml::xml_node<> *, silenceDetector &);

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: silenceDetector <xml_config>" << std::endl;
		return 1;
	}

	//Daemonize
	if (fork() != 0)
		return 0; 

	setsid();

	if (fork() != 0)
		return 0;
	
	std::fstream f;
	f.open(argv[1], std::ios_base::in);
	if (f.fail())
	{
		std::cerr << "Unable to open config file." << std::endl;
		return 1;
	}

	std::string config_xml;
	while (!f.eof())
	{
		char c = f.get();
		if (!f.eof())
			config_xml += c;
	}

	char *buf = new char[config_xml.length()+1];
	strncpy(buf, config_xml.c_str(), config_xml.length());
	buf[config_xml.length()] = 0;
	rapidxml::xml_document<> xml;
	try
	{
		xml.parse<0>(buf);
	}
	catch (std::exception e)
	{
		std::cerr << "Unable to parse xml configuration file." << std::endl;
		std::cerr << e.what() << std::endl;
		return 1;
	}

	silenceDetector sd;
	rapidxml::xml_node<> *node = xml.first_node();
	if (node == 0)
	{
		std::cerr << "Empty document." << std::endl;
		return 1;
	}

	while (node != 0)
	{
		if (!strcmp(node->name(), "BindingAddress"))
		{
			sd.setBindingAddress(node->first_node()->value());
		}
		else if ( !strcmp(node->name(), "IPAddress")  )
		{
			sd.setIPAddress(node->first_node()->value());
		}
		else if ( !strcmp(node->name(), "Port")  )
		{
			sd.setPort(node->first_node()->value());
		}
		else if ( !strcmp(node->name(), "Action")  )
		{
			processDetectorAction(node, sd);
		}

		node = node->next_sibling();
	}

	sd.signalProcessingLoop();
	return 0;
}

void processDetectorAction(rapidxml::xml_node<> *node, silenceDetector &sd)
{
	std::string actionType;
	std::map<std::string, std::string> params;
	detectorAction *newAction;
	bool makeAction = true;
	for (rapidxml::xml_node<> *n = node->first_node(); n != 0; n = n->next_sibling())
	{
		detectorAction *act;
		if ( !strcmp(n->name(), "Type") )
		{
			actionType = n->first_node()->value();
		}
		else
		{
			params[n->name()] = n->first_node()->value();
		}
	}

	if (actionType == "email")
	{
		newAction = new detectorEmail(params["to"], params["server"], params["message"]);
 	}
	else if (actionType == "log")
	{
		newAction = new detectorLog(params["message"], params["file"]);
	}
	else
	{
		makeAction = false;
	}

	if (makeAction)
	{
		newAction->setSilenceThreshold(atoi(params["threshold"].c_str()));
		newAction->setSilenceCount(atoi(params["counts"].c_str()));
		std::cout << newAction->getSilenceCount() << " " << newAction->getSilenceThreshold() << std::endl;
		sd.addDetectorAction(newAction);
	}
}
