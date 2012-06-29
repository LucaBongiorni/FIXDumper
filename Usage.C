#include	<stdlib.h>
#include	<iostream>

#include	"Usage.H"

using namespace std;

void Usage::Notice(const char* USAGESTR,
					 char* pcPrgName)
{
	cerr << "Usage: " << pcPrgName << " " << USAGESTR << endl;
	exit(1);
};


void Usage::Notice(const char* USAGESTR,
					 char* pcPrgName,
					 std::string sMsg)
{
	cerr << sMsg << endl;
	cerr << "Usage: " << pcPrgName << " " << USAGESTR << endl;
	exit(1);
};


void Usage::Notice(const char* USAGESTR, 
					 const char* USAGEEXP,
					 char* pcPrgName)
{
	cerr << "Usage: " << pcPrgName << " " << USAGESTR << endl;
	cerr << "Where:\n" << USAGEEXP << endl;
	exit(1);
};


void Usage::Notice(const char* USAGESTR,
					 const char* USAGEEXP,
					 char* pcPrgName,
					 std::string sMsg)
{
	cerr << sMsg << endl;
	cerr << "Usage: " << pcPrgName << " " << USAGESTR << endl;
	cerr << "Where:\n" << USAGEEXP << endl;
	exit(1);
};
