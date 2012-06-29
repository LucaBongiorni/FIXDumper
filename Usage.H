#include	<string>
class Usage
{
	public:
		static	void Notice(const char* pcUsageStr, char* pcPrgName);
		static	void Notice(const char* pcUsageStr, const char* pcUsageExp, 
												char* pcPrgName);
		static	void Notice(const char* pcUsageStr, char* pcPrgName, 
												std::string pcErrorMsg);
		static	void Notice(const char* pcUsageStr, const char* pcUsageExp, 
					char* pcPrgName, std::string pcErrorMsg);
};
