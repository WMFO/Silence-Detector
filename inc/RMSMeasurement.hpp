#ifndef RMSMEASUREMENT_H
#define RMSMEASUREMENT_H

class RMSMeasurement
{
    public:
        RMSMeasurement();
        ~RMSMeasurement();
        RMSMeasurement(const unsigned char*, unsigned long);
        void setData(const unsigned char*, unsigned long);
        double getMeasurementValue() const;

    private:
        double measurementValue;

};

#endif

