#include <silenceDetector.hpp>

//192.168.0.200 239.192.39.123, 5004

int main(int argc, char *argv[])
{
	silenceDetector sd;
	sd.setIPAddress("239.192.39.123");
	sd.setBindingAddress("192.168.0.200");
	sd.setPort("5004");

	sd.signalProcessingLoop();

}
