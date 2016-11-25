#include "tw_raw.h"
#include "util/arg.h"
#include "util/util.h"
#include "http_reply.h"
#include "url/url.h"
#include <iostream>
#include <limits>
#include <sstream>
#include <set>

using namespace std;
ostream&
help(ostream &os)
{
    os<<"Tool for reading CTW_Raw pages and output them.\n"
      "\tUsage: cmd [--prefix=...]* [--select=...] [--from=...] [--num=...] \n"
      "\t\t  [--one-by-one] [--to-code=...]\n"
      "\t\t  [--http-reply | --body-only | --make-index --fn=] [-h|--help]\n"
      "\t\t--prefix= : select urls with curtain prefix.\n"
      "\t\t--select= : filename of selected url.\n"
      "\t\t--from= : skip the first ? pages.\n"
      "\t\t--num= : output num pages at most.\n"
          "\t\t--to-code= : http-body will be converted to this charset.\n"
      "\t\t--one-by-one : Write output to seperated files with names:\n"
      "\t\t\t\t \"1.\", \"2.\", \"3.\", ...\n"
      "\t\t\t\t extention will be raw, html, or reply\n"
      "\t\t--http-reply : output is in http-reply format, tw_raw headers are discarded.\n"
      "\t\t--body-only : only http body is output.\n"
      "\t\t--make-index : output url, date, filename, and offset of pages.\n"
      "\t\t--fn : filename of raw pages.\n"
      "\t\t-h|--help : print this message.\n"
      "\t\tcin : CTWRaw pages.\n"
      "\t\tcout : output if \"--one-by-one\" option is not set.\n"
      <<endl;
    return os;
}

int
main(int argc, char* argv[])
{
    CArg arg(argc, argv);
    if (arg.found("-h") || arg.found("--help"))
    {
        help(cout);
        return 1;
    }
    vector<string> prefixes = arg.find("--prefix=");
    string fn;
    set<string> selects;
    if (arg.findLast("--select=", fn))
    {
        ifstream ifs(fn.c_str());
        if (!ifs)
        {
            cerr<<"Can not open select file:\""<<fn<<"\"."<<endl;
            return -6;
        }
        string urlstr;
        while (ifs>>urlstr)
            selects.insert(urlstr);
    }
    string strNum;
    unsigned num=numeric_limits<unsigned>::max();
    if (arg.findLast("--num=", strNum))
        num = stoi(strNum);
    unsigned from=0;
    if (arg.findLast("--from=", strNum))
        from = stoi(strNum);
    string to_code = "";
    arg.findLast("--to-code=", to_code);
    bool oneByOne = arg.found("--one-by-one");
    bool body_only = arg.found("--body-only");
    bool http_reply = arg.found("--http-reply");
    bool make_index = arg.found("--make-index");
    if (make_index)
    {
        string filename;
        ifstream rawfile;
        if (arg.findLast("--fn=", filename))
            rawfile.open(filename);
        if (!rawfile.is_open())
        {
            cerr<<"Can not open twr file: "<<filename<<" to index."<<endl;
            return -1;
        }
        CTWRaw raw;
        for (unsigned pos = rawfile.tellg(); rawfile>>raw; pos = rawfile.tellg())
        {
            cout<<"url: "<<raw.url<<endl;
            cout<<"date: "<<raw.date<<endl;
            cout<<"pos: "<<filename<<'#'<<pos<<endl;
        }
    }
    for (unsigned nloop=0, count=0; count<num;  nloop++)
    {
        CTWRaw raw;
        if (!(cin>>raw))
            break;
        if (nloop < from)
            continue;
        if (prefixes.size() > 0 || selects.size() > 0)
        {
            bool match = false;
            for (size_t i=0; i<prefixes.size(); i++)
            {
                if (raw.url.compare(0, prefixes[i].length(), prefixes[i]) == 0)
                {
                    match = true;
                    break;
                }
            }
            if (!match && selects.find(raw.url)==selects.end())
                continue;
        }
        count++;

        if (to_code.length() > 0)
        raw.iconv(to_code);
        ostream os(cout.rdbuf());
        ofstream ofs;
        if (oneByOne)
        {
            if (body_only)
            {
                string ext = raw.reply.headers.ext();
                if (ext.empty())
                    ext = CURL(raw.url).local_ext();
                if (ext.empty())
                    ext = "html";
                ofs.open((to_string(count)+"."+ext).c_str());
            }
            else if (http_reply)
                ofs.open((to_string(count)+".reply").c_str());
            else
                ofs.open((to_string(count)+".raw").c_str());
               os.rdbuf(ofs.rdbuf());
        }
        if (body_only)
            os<<raw.reply.body<<endl;
        else if (http_reply)
            os<<raw.reply<<endl;
        else
            os<<raw<<endl;
    }
    return 0;
}
