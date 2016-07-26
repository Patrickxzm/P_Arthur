#include "util/arg.h"
#include "tw_raw.h"
#include <stdexcept>
#include <time.h>
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

ostream&
help(ostream& os)
{
    os<<"Output web pages in Tianwang Raw format to multiple files with time infomation as file name suffix, such as MyWebRaw.20100509.\n"
       "\tUsage: Cmd --prefix= --time-format= [--file-size=] [-h|--help]. \n"
       "\t\t --prefix= : prefix of output file names.\n"
       "\t\t --time-format= : string used in \"strftime()\" function to format a time.\n"
       "\t\t\t example: %y%m%d%H%M produce 1005091305 \n"
       "\t\t --file-size= : max size of single output file. (default: 1000000000)\n"
       "\t\t -h|--help : print this message.\n"
       "\t\t cin : web pages in Tianwang Raw format.\n"
    <<endl;
    return os;
}

namespace {
    bool
    open(ofstream &ofs, const string &prefix, const string &time_format)
    {
        const int bufsize = 256;
        char buf[bufsize];
        time_t now;
        time(&now);
        strftime(buf, bufsize, time_format.c_str(), gmtime(&now));
        string fn = prefix+"."+buf;
        ifstream test(fn.c_str());
        if (test)
        {
            ostringstream oss;
            oss<<"output file \""<<fn<<"\" already exists";
            throw runtime_error(oss.str());
        }
        ofs.open((prefix+"."+buf).c_str(), ios::out|ios::trunc);
        if (!ofs)
        {
            ostringstream oss;
            oss<<"Can not open output file:\""<<fn<<"\"";
            throw runtime_error(oss.str());
        }
        return true;
    }
}

int
main(int argc, char* argv[])
try {
    CArg arg(argc, argv);
    if (arg.found("-h") || arg.found("--help"))
    {
        help(cout);
        return 1;
    }
    string prefix;
    if (!arg.findLast("--prefix=", prefix))
        {
        cerr<<"\"--prefix=\" is required."<<endl;
        return -1;
    }
    string time_format;
    if (!arg.findLast("--time-format=", time_format))
    {
        cerr<<"\"--time-format=\" is required."<<endl;
        return -2;
    }
    unsigned file_size;
    string strNum;
    if (arg.findLast("--file-size=", strNum))
        file_size = stoul(strNum);
    else
        file_size = 1000000000;

    CTWRaw raw;
    ofstream ofs;
    while (cin >> raw)
    {
        if (ofs.is_open() && ofs.tellp() > file_size)
            ofs.close();
        if (!ofs.is_open())
            open(ofs, prefix, time_format);
        ofs << raw << endl;
    }
    if (!cin.eof())
    {
        cerr<<"input in wrong format, job not finished."<<endl;
        return -4;
    }
    return 0;
}
catch (exception &e)
{
    cerr<<"Catch exception:"<<e.what()<<endl;
    return -3;
}
