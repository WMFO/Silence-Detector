#ifndef SILENCEEXCEPTION_H
#define SILENCEEXCEPTION_H

#include <exception>
#include <string>

class silenceException : public std::exception
{
	public:
		silenceException(std::string);
		~silenceException() throw();
		virtual const char *what() const throw();
	
	private:
		std::string msg;
};


#endif
