#ifndef _PAT_TCPBUF_H_030807
#define _PAT_TCPBUF_H_030807
#include <streambuf>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>
#include <cstdio>
#include <cstring>

class TcpBuf : public std::streambuf 
{
    friend class CTCPServer;
    friend class CTCPClient;
    friend class CUltraServer;
  protected:
    /* data buffer:
     * - at most, four characters in putback area plus
     * - at most, six characters in ordinary read buffer
     */
    static const int rbufSize = 1024;    // size of the data buffer
    static const int putbackSize = 4;
    char rbuf[rbufSize];             // data buffer

    static const int wbufSize = 1024;
    char wbuf[wbufSize];

    int sockfd;
    int recv_timeout;

    int recv(void* data, int len);
    int send(const void* data, unsigned len);
  public:
    /* constructor
     * - initialize empty read data buffer
     * - no putback area
     * => force underflow()
     * 
     * - initialize write data buffer
     * - one character less to let the bufferSizeth character
     *    cause a call of overflow()
     */
    TcpBuf(int r=-1) : sockfd(-1), recv_timeout(r)
    {
	reset();
    }
    
    bool close()
    {
	if (sockfd >= 0)
	{
	    sync();
	    ::close(sockfd);
	    sockfd = -1;
	}
	reset();
	return true;
    }

    bool is_open()
    {
	return sockfd >= 0;
    }

    virtual ~TcpBuf()
    {
	close();
    }
     

    int timeout(int recv_timeout)
    {
	    int tmp = this->recv_timeout;
	    this->recv_timeout = recv_timeout;
	    return tmp;
    }
private:
	void reset()
	{
        	setg (rbuf+putbackSize,     // beginning of putback area
              	rbuf+putbackSize,     // read position
              	rbuf+putbackSize);    // end position
        	setp (wbuf, wbuf+(wbufSize-1));  // One more byte for overflow
		return;
	}
  protected:
    /* buffer full
     * - write c and all previous characters
     */
    virtual int_type overflow (int_type c) {
        if (c != EOF) {
            // insert character into the buffer
	    //assert(pptr() < epptr());
            *pptr() = c;
            pbump(1);
        }
        // flush the buffer
        if (flushBuffer() == EOF) {
            // ERROR
            return EOF;
        }
        return c;
    }

    /* synchronize data with file/destination
     * - flush the data in the buffer
     */
    virtual int sync () {
        if (flushBuffer() == EOF) {
            // ERROR
            return -1;
        }
        return 0;
    }

    // insert new characters into the buffer
    virtual int_type underflow () {

        // is read position before end of buffer?
        if (gptr() < egptr()) {
            return traits_type::to_int_type(*gptr());
        }

        /* process size of putback area
         * - use number of characters read
         * - but at most four
         */
        int numPutback;
        numPutback = gptr() - eback();
        if (numPutback > putbackSize) {
            numPutback = putbackSize;
        }

        /* copy up to four characters previously read into
         * the putback buffer (area of first four characters)
         */
        std::memmove (rbuf+(putbackSize-numPutback), gptr()-numPutback,
                      numPutback);

        // read new characters
        int num;
	num = recv(rbuf+putbackSize, rbufSize-putbackSize);
        //num = read (0, buffer+putbackSize, bufferSize-putbackSize);
        if (num <= 0) {
            // ERROR or EOF
            return EOF;
        }

        // reset buffer pointers
        setg (rbuf+(putbackSize-numPutback),   // beginning of putback area
              rbuf+putbackSize,                // read position
              rbuf+putbackSize+num);           // end of buffer

        // return next character
        return traits_type::to_int_type(*gptr());
    }

    // flush the characters in the buffer
    int flushBuffer () {
        int num = pptr()-pbase();
        if (send(wbuf, num) != num) {
            return EOF;
        }
        pbump (-num);    // reset put pointer accordingly
        return num;
    }

};

#endif // _PAT_TCPBUF_H_030807
