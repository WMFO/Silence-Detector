#ifndef STREAMDUMPER_H
#define STREAMDUMPER_H

#include <exception>
#include <string>
#include <iostream>
#include <list>

class streamDumper
{
     public:
        streamDumper();
        streamDumper(const std::string &bindAddr, const std::string &ipaddr, int portNumber);
        void openMulticastStream(const std::string &bindAddr, const std::string &ipaddr, int portNumber);
        ~streamDumper();
        void getSocketData(unsigned char* buf, unsigned long bufLen);

    private:
        std::string multicastAddr;
        int port;
        bool socketActive;
        bool subscriptionActive;
        int socketDescriptor;
        unsigned char *remainderData;
        unsigned long remainderDataLength;
        void openSocket();
        void closeSocket();
        void readSocket();

        class RTPHeader
        {
            public:
                RTPHeader();
                RTPHeader(unsigned char*);
                ~RTPHeader();
                void readData(unsigned char*);
                int getVersion() const;
                bool hasPadding() const;
                bool hasExtension() const;
                unsigned int getCSRCCount() const;
                unsigned int getPayloadType() const;
                unsigned long getSequenceNumber() const;
                unsigned long getTimestamp() const;
                unsigned long getSSRCIdentifier() const;
                unsigned long getHeaderLength() const;

            private:
                int version;
                bool padding;
                bool extensions;
                unsigned int CSRCCount;
                unsigned int payloadType;
                unsigned long sequenceNumber;
                unsigned long timeStamp;
                unsigned long SSRCIdentifier;
                unsigned long extensionLength;

        };

        class RTPPacket
        {
                public:
                    RTPHeader header;
                    RTPPacket();
                    RTPPacket(unsigned char*, unsigned long);
                    ~RTPPacket();
                    void readData(unsigned char*, unsigned long);
                    unsigned long getContentLength() const;
                    unsigned char *getContent() const;
                    bool operator<(const RTPPacket &b) const;
                    RTPPacket(const RTPPacket &b);
                    RTPPacket &operator=(const RTPPacket &b);

                private:
                    unsigned char *content;
                    unsigned long contentLength;

        };

        void setRemainderData(const unsigned char *, unsigned long, const std::list<RTPPacket>&);

};

//Exceptions
class streamDumperException : public std::exception
{
    public:
        streamDumperException(const std::string &reason) throw()
        {
            exceptionWhat = reason;
        }

        virtual const char *what() const throw()
        {
            return exceptionWhat.c_str();
        }

        virtual ~streamDumperException() throw() {};

    private:
        std::string exceptionWhat;

};

#endif

