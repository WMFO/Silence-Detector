#include <streamDumper.hpp>
#include <list>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

streamDumper::streamDumper()
{
    socketActive = false;
    socketDescriptor = -1;
    subscriptionActive = false;
    remainderData = NULL;
    remainderDataLength = 0;
}

streamDumper::streamDumper(const std::string &bindAddr, const std::string &ipaddr, int portNumber)
{
    socketActive = false;
    socketDescriptor = -1;
    subscriptionActive = false;
    remainderData = NULL;
    remainderDataLength = 0;
    openMulticastStream(bindAddr, ipaddr, portNumber);
}

streamDumper::~streamDumper()
{
    if (socketActive == true)
    {
        //Need to close the socket.
        closeSocket();
    }
}

void streamDumper::closeSocket()
{
    if (!socketActive)
        throw streamDumperException("Cannot close socket (never active).");

    close(socketDescriptor);
}

void streamDumper::openSocket()
{
    socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);

    //Check for errors
    if (socketDescriptor < 0)
    {
        //Throw an exception
        throw streamDumperException("Unable to create a socket.");
    }

    socketActive = true;
}

void streamDumper::openMulticastStream(const std::string &bindAddr, const std::string &ipaddr, int portnum)
{
    //Check
    if (socketActive)
        throw streamDumperException("Socket is already active.");
    if (subscriptionActive)
        throw streamDumperException("Already subscribed to a multicast stream.");

    //Setup the ip structure
    ip_mreq multicastSubscriptionReq;
    if (!inet_aton(ipaddr.c_str(), &multicastSubscriptionReq.imr_multiaddr))
        throw streamDumperException("Invalid IP address specified.");
    if (!inet_aton(bindAddr.c_str(), &multicastSubscriptionReq.imr_interface))
        throw streamDumperException("Invalid bind address specified.");
    //multicastSubscriptionReq.imr_interface.s_addr = htonl(INADDR_ANY);
    
    //Open the socket
    openSocket();

    //Bind the socket
    sockaddr_in bindStruct;
    memset(&bindStruct, 0, sizeof(bindStruct));
    bindStruct.sin_family = AF_INET;
    bindStruct.sin_addr.s_addr = htonl(INADDR_ANY);
    bindStruct.sin_port = htons(portnum);
    if (bind(socketDescriptor, (sockaddr *) &bindStruct, sizeof(bindStruct)) != 0)
        throw streamDumperException("Unable to bind socket to address.");

    //Attempt to join the multicast group
    if (setsockopt(socketDescriptor, IPPROTO_IP, IP_ADD_MEMBERSHIP, &multicastSubscriptionReq, sizeof(multicastSubscriptionReq)) != 0)
        throw streamDumperException("Unable to join multicast group.");

    subscriptionActive = true;
}

void streamDumper::getSocketData(unsigned char *buf, unsigned long buflen)
{
    if (!socketActive)
        throw streamDumperException("Cannot receive data. Socket not active.");
    if (!subscriptionActive)
        throw streamDumperException("Cannot receive data. Not subscribed to multicast group.");
    if (buflen < 1)
        return;

    unsigned char packetBuf[2048];
    unsigned long curCache = remainderDataLength;
    unsigned char *pos = buf;
    std::list<RTPPacket> packets;

    while (curCache < buflen)
    {
        ssize_t num_bytes = recvfrom(socketDescriptor, packetBuf, 2048, 0, NULL, NULL);
        RTPPacket rtpPacket(packetBuf, num_bytes);
        curCache += rtpPacket.getContentLength();
        packets.push_back(rtpPacket);
    }

    packets.sort();

    //Add the remainder data to the buffer
    if (remainderDataLength > 0)
    {
        if (remainderDataLength < buflen)
        {
            memcpy(pos, remainderData, remainderDataLength);
            pos += remainderDataLength;
            remainderDataLength = 0;
            delete[] remainderData;
            remainderData = NULL;
        }
        else
        {
            //Buffer is not big enough to hold entirety of remainder data
            memcpy(pos, remainderData, buflen);
            unsigned char *leftOverBytes = new unsigned char[remainderDataLength-buflen];
            memcpy(leftOverBytes, remainderData+buflen, remainderDataLength-buflen);
            setRemainderData(leftOverBytes, remainderDataLength-buflen, packets);
            delete[] leftOverBytes;
            return;
        }
    }

    //Add the packet data
    std::list<RTPPacket>::iterator it = packets.begin();
    while (pos < (buf + buflen))
    {
        if ((*it).getContentLength() < ((buf + buflen) - pos))
        {
            //Copy the whole packet in
            memcpy(pos, (*it).getContent(), (*it).getContentLength());
            pos += (*it).getContentLength();
            it++;
            packets.pop_front();
        }
        else
        {
            //Need to copy only some of the packet in
            unsigned char *endbuf = buf + buflen;
            unsigned long numLeftOverBytes = (*it).getContentLength() - (endbuf - pos);
            unsigned long numBytesToWrite = endbuf - pos;
            memcpy(pos, (*it).getContent(), numBytesToWrite);
            unsigned char *leftOverBytes = new unsigned char[numLeftOverBytes];
            memcpy(leftOverBytes, (*it).getContent() + numBytesToWrite, numLeftOverBytes);
            it++;
            packets.pop_front();
            setRemainderData(leftOverBytes, numLeftOverBytes, packets);
            delete[] leftOverBytes;
            return;
        }
    }
    

//    std::cout << "Recv'd " << num_bytes << " from the socket." << std::endl;
//    std::cout << "Version: " << rtpHead.getVersion() << std::endl;
//    std::cout << "Padding: " << rtpHead.hasPadding() << std::endl;
//    std::cout << "Extensions: " << rtpHead.hasExtension() << std::endl;
//    std::cout << "CSRCCount: " << rtpHead.getCSRCCount() << std::endl;
//    std::cout << "PayloadType: " << rtpHead.getPayloadType() << std::endl;
//    std::cout << "Sequence #: " << rtpPacket.header.getSequenceNumber() << std::endl;
//    std::cout << "Timestamp: " << rtpHead.getTimestamp() << std::endl;
//    std::cout << "SSRC: " << rtpHead.getSSRCIdentifier() << std::endl << std::endl;
//    std::cout << "Header Length: " << rtpPacket.header.getHeaderLength() << std::endl;
//    std::cout << "Content Length: " << rtpPacket.getContentLength() << std::endl;
}

void streamDumper::setRemainderData(const unsigned char *data, unsigned long data_len, const std::list<RTPPacket>& l)
{
    //Determine how much data we have here
    unsigned long newRemainderDataLength = data_len;
    for (std::list<RTPPacket>::const_iterator i = l.begin(); i != l.end(); i++)
    {
        newRemainderDataLength += (*i).getContentLength();
    }

    unsigned char *newRemainderData = new unsigned char[newRemainderDataLength];
    unsigned char *pos = newRemainderData;

    //Copy the raw data
    memcpy(pos, data, data_len);
    pos += data_len;

    //Copy the data from full packets
    for (std::list<RTPPacket>::const_iterator i = l.begin(); i != l.end(); i++)
    {
        memcpy(pos, (*i).getContent(), (*i).getContentLength());
        pos += (*i).getContentLength();
    }

    //Clear the old remainder data and set new data
    delete[] remainderData;
    remainderData = newRemainderData;
    remainderDataLength = newRemainderDataLength;
}

streamDumper::RTPHeader::RTPHeader()
{
    version = 0;
    padding = false;
    extensions = false;
    CSRCCount = 0;
    payloadType = 0;
    sequenceNumber = 0;
    timeStamp = 0;
    SSRCIdentifier = 0;
    extensionLength = 0;
}

streamDumper::RTPHeader::~RTPHeader() { };

streamDumper::RTPHeader::RTPHeader(unsigned char *in)
{
    version = 0;
    padding = false;
    extensions = false;
    CSRCCount = 0;
    payloadType = 0;
    sequenceNumber = 0;
    timeStamp = 0;
    SSRCIdentifier = 0;
    extensionLength = 0;
    readData(in);
}

void streamDumper::RTPHeader::readData(unsigned char *in)
{
    version = (in[0] & 192) >> 6;
    padding = in[0] & (1 << 5);
    extensions = in[0] & (1 << 4);
    CSRCCount = ((1 << 3) | (1 << 2) | (1 << 1) | 1) & in[0];
    payloadType = in[1] & ~128;
    sequenceNumber = (in[2] << 8) | (in[3]);
    timeStamp = (in[4] << 24) | (in[5] << 16) | (in[6] << 8) | in[7];
    SSRCIdentifier = (in[8] << 24) | (in[9] << 16) | (in[10] << 8) | in[11];

    //Extenstion length code
    if (extensions)
    {
        unsigned long ind = 16 + 4*CSRCCount + 2;
        extensionLength = (in[ind] << 8) | in[ind+1];
        extensionLength = (extensionLength+1)*4;
    }
    else
    {
        extensionLength = 0;
    }
}

unsigned long streamDumper::RTPHeader::getHeaderLength() const
{
    return 12 + 4*CSRCCount + extensionLength;
}

int streamDumper::RTPHeader::getVersion() const
{
    return version;
}

bool streamDumper::RTPHeader::hasPadding() const
{
    return padding;
}

bool streamDumper::RTPHeader::hasExtension() const
{
    return extensions;
}

unsigned int streamDumper::RTPHeader::getCSRCCount() const
{
    return CSRCCount;
}

unsigned int streamDumper::RTPHeader::getPayloadType() const
{
    return payloadType;
}

unsigned long streamDumper::RTPHeader::getSequenceNumber() const
{
    return sequenceNumber;
}

unsigned long streamDumper::RTPHeader::getTimestamp() const
{
    return timeStamp;
}

unsigned long streamDumper::RTPHeader::getSSRCIdentifier() const
{
    return SSRCIdentifier;
}

streamDumper::RTPPacket::RTPPacket()
{
    content = NULL;
    contentLength = 0;
}

streamDumper::RTPPacket::~RTPPacket()
{
    if (content != NULL)
        delete[] content;
}

streamDumper::RTPPacket::RTPPacket(unsigned char *in, unsigned long in_len)
{
    content = NULL;
    contentLength = 0;
    readData(in, in_len);
}

void streamDumper::RTPPacket::readData(unsigned char *in, unsigned long in_len)
{
    if (in_len < 12)
        throw streamDumperException("RTP Packet must be at least 12 bytes long.");

    header.readData(in);
    contentLength = in_len - header.getHeaderLength();
    if (contentLength > 0)
    {
        content = new unsigned char[contentLength];
        memcpy(content, in+header.getHeaderLength(), contentLength);
    }
}

unsigned long streamDumper::RTPPacket::getContentLength() const
{
    return contentLength;
}

unsigned char *streamDumper::RTPPacket::getContent() const
{
    return content;
}

streamDumper::RTPPacket::RTPPacket(const RTPPacket &b)
{
    //Copy constructor
    if (b.contentLength > 0)
    {
        contentLength = b.contentLength;
        header = b.header;
        content = new unsigned char[contentLength];
        memcpy(content, b.content, contentLength);
    }
    else
    {
        contentLength = 0;
        content = NULL;
    };

}

streamDumper::RTPPacket& streamDumper::RTPPacket::operator=(const streamDumper::RTPPacket &b)
{
    //Assignment operator
    if (contentLength > 0)
    {
        //Clear old memory
        delete[] content;
    }

    if (b.contentLength > 0)
    {
        header = b.header;
        contentLength = b.contentLength;
        content = new unsigned char[contentLength];
        memcpy(content, b.content, contentLength);
    }
    else
    {
        header = b.header;
        contentLength = 0;
        content = NULL;
    }

    return *this;
}

bool streamDumper::RTPPacket::operator<(const RTPPacket &b) const
{
    unsigned long seq_a, seq_b;
    seq_a = header.getSequenceNumber();
    seq_b = b.header.getSequenceNumber();

    if (seq_a < seq_b)
    {
        if ((seq_b - seq_a) < 32768)
        {
            //No wrap around
            return true;
        }
        else
        {
            //Wrap around
            return false;
        }
    }
    else
    {
        // b < a
        if ((seq_a - seq_b) < 32768)
        {
            //No wrap around
            return false;
        }
        else
        {
            //Wrap around
            return true;
        }
    }

}
