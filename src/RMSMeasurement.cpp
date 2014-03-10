#include <cmath>
#include <RMSMeasurement.hpp>

#define TWO_TO_23RD 8388608

RMSMeasurement::RMSMeasurement()
{
    measurementValue = 0.0;
}

RMSMeasurement::~RMSMeasurement()
{

}

RMSMeasurement::RMSMeasurement(const unsigned char *in, unsigned long len)
{
    setData(in, len);
}

void RMSMeasurement::setData(const unsigned char *in, unsigned long len)
{
    //Create RMS sound level measurement off of two channel data
    long right;
    long left;
    measurementValue = 0.0;
    unsigned long num_samples = len/6; //6 is the 2*(size of one sample in one channel = 3)

    for (unsigned long i = 0; i < len; i += 6) //6 is the 2*(size of one sample = 3)
    {
        //Remember, 24 bit big endian samples, and they are signed (bleh)
        left = ((in[i] & 127) << 16)  | (in[i+1] << 8) | (in[i+2]);
        if (in[i] & 128)
        {
            //Must apply the two's complement since the MSB is set
            left += -TWO_TO_23RD;
        }

        right = ((in[i+3] & 127) << 16) | (in[i+4] << 8) | (in[i+5]);
        if (in[i+3] & 128)
        {
            //Must apply the two's complemennt since the MSB is set
            right += -TWO_TO_23RD;
        }

        //Right now using measurementValue to hold the mean of the squares
        measurementValue += std::pow(((left + right)/2.0), 2.0)/num_samples;
    }

    //Take square root to finish calculation
    measurementValue = std::sqrt(measurementValue);

}

double RMSMeasurement::getMeasurementValue() const
{
    return measurementValue;
}

