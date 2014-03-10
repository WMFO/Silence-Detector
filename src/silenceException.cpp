#include <silenceException.hpp>

silenceException::silenceException(std::string in)
{
	msg = in;
}

silenceException::~silenceException() throw()
{
	//empty
}

const char * silenceException::what() const throw()
{
	return (char *)msg.c_str();
}
