// vi: ai nonu ts=2 sw=2

#include <sstream>
#include <string.h>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <cerrno>
#include <algorithm>
#include "GetOpt.H"
#include "Usage.H"

#define	FIX_SEPARATOR		((char) 0x01)

#define	PRG_OPTIONS					"w:i:o:m:ht:s:eE:f:IB"
#define	PRGOPT_NOOPT					  0
#define	PRGOPT_INPUT					  1
#define	PRGOPT_OUTPUT					  2
#define	PRGOPT_COMMASEP				  4
#define	PRGOPT_MSGLIST				  8
#define	PRGOPT_TAGWIDTH				 16
#define	PRGOPT_PRINTHDR				 32
#define	PRGOPT_TAGLIST				 64
#define	PRGOPT_FILTER					128
#define	PRGOPT_SEPARATOR			256
#define	PRGOPT_IGNMANDATORY		512
#define	PRGOPT_BAREDUMP			 1024

long	lOptValue = PRGOPT_NOOPT;

using namespace std;
 

const char *USAGESTR =
		"[-i file] [-o file] [-w size] "\
		"[-m type list] [-h] [-s sep] [-I] "\
		"[-t tag list] [-e] [-E sep] [-f filter on tag] [--help]";
const char *USAGEEXP =
		"   i: input file to use. Default stdin\n"\
		"   o: output file to use. Default stdout\n"\
		"   e: dump in comma separated format\n"\
		"   E: dump in comma separated format using sep instead of comma)\n"\
		"   f: filter on tag condition (in format 'tag value' means where tag equal value)\n"\
		"   h: print short header every message (tags 35, 49, 56)\n"\
		"   m: list of msg types to dump based on tag 35 value.\n"\
		"   s: use 'sep' as tag fields separator (default \\0x01).\n"\
		"   t: tag list comma separated.\n"\
		"   w: width for tag column ('tag' or 'tag:description').\n"\
		"      Notice that:\n"\
		"      - Tag size default is 7 char.\n"\
		"      - Description is fully dumped unless size is specified.\n"\
		"   I: ignore mandatory tag presence, just dump the content.\n"\
		"   B: dump all messages filtered as they were received on output "\
				"(similar to grep).\n"\
		"   help: show this help. Notice -h is not an abbreviation.\n";

std::map<int, string>	mFixMsgType;
std::map<int, string>::iterator mFMTit;
std::vector<int> tagList;

std::string		sInput, sOutput, sFixTypesList, sFilterOnTagValue = "";
int						iTagWidth, iDescWidth, iFilterOnTag;
ofstream			os;
char					cSep = FIX_SEPARATOR;
char					csvSep = ',';

/*
void	Usage()
{
	cerr << "Usage: FixDumper " << USAGESTR << endl;
	cerr << "Where:\n" << USAGEEXP << endl;
	std::exit(1);
}
*/

int	isFilterTrue(map<int, string> msg, int iTag, string sTagValue)
{
	std::map<int, string>::iterator it;
	for(it = msg.begin(); it != msg.end(); it++)
	{
		if ((*it).first == iTag)
			if ((*it).second.c_str() == sTagValue)
				return 1;
			else
				return 0;
	}
	return 0;
}

int	isInList(int tag)
{
	vector<int>::iterator it;
	for(it = tagList.begin(); it != tagList.end(); it++)
	{
		if (*it == tag)
			return 1;
	}
	return 0;
}

void	MapAndDump(char* pcRow)
{
	std::map<int, string>	msg;
	std::map<int, string>::iterator it;
	string	sLocal(pcRow);
	string	sDummy;
	size_t	pos;

	if ((pos = sLocal.find(cSep)) == string::npos)
	{
		return;
	}
	sLocal = sLocal.substr(sLocal.substr(0, pos).find_last_of(' ') + 1);
	sLocal = sLocal.substr(strcspn(sLocal.c_str(), "0123456789"));

	string sFixString = sLocal;
	if ((pos = sFixString.find_last_of(cSep)) != string::npos)
	{
		sFixString = sFixString.substr(0, pos + 1);
	}

	string sTag;
	while((sLocal.length() > 0) && (isdigit(sLocal[0]) || isspace(sLocal[0])))
	{
		pos = sLocal.find(cSep);
		if (pos == string::npos)
		{
			sTag = sLocal;
			sLocal = "";
		}
		else
		{
			sTag = sLocal.substr(0, pos);
			sLocal = sLocal.substr(pos + 1);
		}
		// cerr << "New tag: " << sTag <<endl;
		pos = sTag.find("=");
		

		msg[atoi(sTag.substr(0, pos).c_str())] = sTag.substr(pos + 1);
	}

	if (lOptValue & PRGOPT_MSGLIST)
	{
		pos = sFixTypesList.find((char)(*msg[35].c_str()));
		if (pos == string::npos)
		{
			return;
		}
	}
	if (!(lOptValue & PRGOPT_SEPARATOR) && 
			((it = msg.find(35)) == msg.end()))
	{
		return;
	}

	if ((lOptValue & PRGOPT_FILTER) && !isFilterTrue(msg, iFilterOnTag, sFilterOnTagValue))
		return;

	if (lOptValue & PRGOPT_BAREDUMP)
	{
		os << sFixString << endl;
		return;
	}

	if (lOptValue & PRGOPT_PRINTHDR)
	{
		os << "\n----------------*" << endl;
		os << "Message Type: " << 
			mFixMsgType[(char)(*msg[35].c_str())] << endl;
		os << "Sent from   : " << msg[49] << endl;
		os << "Sent to     : " << msg[56] << endl;
		os << endl;
	}

	sDummy = "                                                                   ";
	string sSep = "";
	stringstream ssCsvSep;
	ssCsvSep << csvSep;
	for(it = msg.begin(); it != msg.end(); it++)
	{

		int iTag = (*it).first;
		if (!(lOptValue & PRGOPT_TAGLIST) || 
			  (lOptValue & PRGOPT_TAGLIST) && (isInList(iTag)))
		{
			if (!(lOptValue & PRGOPT_COMMASEP))
			{
				os.width(iTagWidth);
				os << (*it).first; 
				os << " => ";
				if (iDescWidth)
				{
					os << (*it).second.substr(0, iDescWidth);
					if ((*it).second.substr(0, iDescWidth).length() < iDescWidth) 
					{
							os << sDummy.substr(0, iDescWidth - 
							(*it).second.substr(0, iDescWidth).length());
					}
					os << endl;
				}
				else
				{
					os << (*it).second.c_str() << endl;
				}
			}
			else
			{
				os << sSep << (*it).second.c_str();
				ssCsvSep >> sSep;
			}
		}
	}
	os << endl;
}

int main (int argc, char** argv)
{
	char					acRow[64*1024];
	filebuf 			fbIn, fbOut;
	int						iOpt, iDummy;
	string				sDummy;
	Option				longOpt[2];
	string::size_type lastPos;
	string::size_type pos;
	
	longOpt[0].name = "help";
	longOpt[0].has_arg = NO_ARG;
	longOpt[0].flag = 0;
	longOpt[0].val = '?';
	longOpt[0].valid = NULL;
	longOpt[0].descr = NULL;

	longOpt[1].name = NULL;

	mFixMsgType['8'] = "Execution Report     ";
	mFixMsgType['D'] = "New Order Single     ";
	mFixMsgType['E'] = "Order List           ";
	mFixMsgType['F'] = "Order Cancel Request ";
	mFixMsgType['G'] = "Order Replace Request";

	GetOpt	GetAppParms(argc, argv, PRG_OPTIONS,
											&(longOpt[0]), &iDummy, 0);

	while((iOpt = GetAppParms()) != EOF)
	{
		switch(iOpt)
		{
			case 'e':
			case 'E':
				lOptValue |= PRGOPT_COMMASEP;
				if (iOpt == 'E')
				{
					csvSep = *(GetAppParms.optarg);
				}
				break;

			case 'f':
				sDummy = GetAppParms.optarg;
				lastPos = sDummy.find(' ');
				iFilterOnTag = (int)(atoi(sDummy.substr(0, lastPos).c_str()));
				sFilterOnTagValue = sDummy.substr(lastPos + 1).c_str();
				lOptValue |= PRGOPT_FILTER;
				break;

			case 'o':
				sOutput = GetAppParms.optarg;
				lOptValue |= PRGOPT_OUTPUT;
				break;

			case 'i':
				sInput = GetAppParms.optarg;
				lOptValue |= PRGOPT_INPUT;
				break;

			case 'm':
				sFixTypesList = GetAppParms.optarg;
				lOptValue |= PRGOPT_MSGLIST;
				break;

			case 's':
				cSep = *(GetAppParms.optarg);
				lOptValue |= PRGOPT_SEPARATOR;
				break;

			case 't':
				lOptValue |= PRGOPT_TAGLIST;
				sDummy = GetAppParms.optarg;
				lastPos = sDummy.find_first_not_of(",", 0);
				pos = sDummy.find_first_of(",", lastPos);
				while (string::npos != pos || string::npos != lastPos)
				{
					tagList.push_back(
						atoi(sDummy.substr(lastPos, pos - lastPos).c_str()));
					lastPos = sDummy.find_first_not_of(",", pos);
					pos = sDummy.find_first_of(",", lastPos);
				}
				sort(tagList.begin(), tagList.end());
				break;

			case 'w':
				lOptValue |= PRGOPT_TAGWIDTH;
				sDummy = GetAppParms.optarg;
				pos = sDummy.find(':');
				if (pos == string::npos)
				{
					iTagWidth = atoi(GetAppParms.optarg);
				}
				else
				{
					iTagWidth = atoi(sDummy.substr(0, pos).c_str());
					iDescWidth = atoi(sDummy.substr(pos + 1).c_str());
				}
				break;

			case 'I':
				lOptValue |= PRGOPT_SEPARATOR;
				break;

			case 'B':
				lOptValue |= PRGOPT_BAREDUMP;
				break;

			case 'h':
				lOptValue |= PRGOPT_PRINTHDR;
				break;

			case '?':
				Usage::Notice(USAGESTR, USAGEEXP, argv[0]);
				break;

			default:
				Usage::Notice(USAGESTR, USAGEEXP, argv[0]);
			}
	}

	if(!(lOptValue & PRGOPT_INPUT))
	{
		sInput = "/dev/stdin";
	}
	if (!(lOptValue & PRGOPT_OUTPUT))
	{
		sOutput = "/dev/stdout";
	}
	if (!(lOptValue & PRGOPT_MSGLIST))
	{
		cerr << "No msg types list provided. Will consider all" << endl;
	}
	else
	{
		cerr << "Filtering on: ";
		for (char *pc = (char *) sFixTypesList.c_str(); *pc != '\0'; pc++)
			cerr << mFixMsgType[*pc] << " ";
		cerr << endl;
	}
	if (!(lOptValue & PRGOPT_TAGWIDTH))
	{
		iTagWidth = 7;
	}

	if(!fbIn.open(sInput.c_str(), ios::in))
	{
		cerr << "Error " << errno <<
			" opening " << sInput << endl;
		std::exit(0);
	}
	if(!fbOut.open(sOutput.c_str(), ios::out))
	{
		cerr << "Error " << errno <<
			" opening " << sOutput << endl;
		std::exit(0);
	}
	fbOut.close();

	istream is(&fbIn);
	os.open(sOutput.c_str());
	memset(acRow, sizeof(acRow), '\0');
	string sSep = "";
	if (lOptValue & PRGOPT_COMMASEP)
	{
		vector<int>::iterator it;
		for(it = tagList.begin(); it != tagList.end(); it++)
		{
			os << sSep << *it;
			sSep = ",";
		}
		os << endl;
	}
	while(is.getline(acRow, sizeof(acRow) - 1))
	{
		MapAndDump(acRow);
		memset(acRow, sizeof(acRow), '\0');
	}

	fbIn.close();
	std::exit (0);
}
