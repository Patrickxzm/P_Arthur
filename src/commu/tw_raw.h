#ifndef _PAT_TW_RAW_H_032207
#define _PAT_TW_RAW_H_032207
#include <ostream>
#include <string>
#include "http_reply.h"
using std::string;
using std::ostream;

class CTWRaw
{
public:
    CTWRaw()
    {}
    CTWRaw(const string &u, const string &ip, const CHttpReply &r);
        bool iconv(const string &target);
    explicit operator bool() const { return url.size()>0; }
public:
    typedef map<string, string> ext_type;
    ext_type ext;
    string version;
    string url;
    string date;
    string ipaddr;
    CHttpReply reply;
private:
    bool get_charset_from_header(string &charset);
    void set_charset_to_header(const string &charset);
    bool get_charset_from_http_header(string &charset);
    bool get_charset_from_html(string &charset);
    bool convert_charset(const string &source, const string &target);
};

ostream& operator<<(ostream &os, const CTWRaw &raw);
istream& operator>>(istream &is, CTWRaw &raw);

#endif // _PAT_TW_RAW_H_032207

