
#ifndef _RULE_OBJ_
#define _RULE_OBJ_

//#pragma warning(disable : 4786)
//#include <boost/regex.hpp>

#define TRANS_UNKNOWN	0
#define TRANS_OPTIONAL	1
#define TRANS_MANDATORY 2

#define RANGE_INVALID		 2
#define RANGE_NOTSPECIFIED	-1

#define charTypeDigit	1
#define charTypeDone	2

#define LOC_FORM_UNKOWN		0
#define LOC_FORM_1		1
#define LOC_FORM_2		2
#define LOC_FORM_3		3
#define LOC_FORM_4		4
#define LOC_FORM_5		5
#define LOC_FORM_6		6
#define LOC_FORM_7		7

class CRuleObj
{
private:
	bool SetTransactions(CStdString& sTransactionList);
	bool SetLocation(CStdString& sLocation);
	bool SetMNU(CStdString& sMNU);
	bool SetCharType(CStdString& sCharType);
	bool SetFieldSize(CStdString& sFieldSize);
	bool SetOccurrences(CStdString& sOccurence);
	bool SetOptionalDescription(CStdString& sDescription);
	bool SetOptionalLongDescription(CStdString& sLongDescription);
	bool SetOptionalSpecialChars(CStdString& sSpecialChars);
	bool SetOptionalDateFormat(CStdString& sDateFormat);
	bool SetOptionalMMap(CStdString& sMMap, CStdString& sFilePath);
	bool SetOptionalOMap(CStdString& sOMap, CStdString& sFilePath);
	bool SetOptionalMap(CStdString& sMap, CStdString& sFilePath, std::vector<CStdString>& mapValNames, std::vector<CStdString>& mapValDescriptions);
	bool SetTags(CStdString& sTags);

	void SetAllowedChars();

	bool GetRangeValue(CStdString& sToken, int *pValue);

	bool TestRegEx(CStdString& sInput, CStdString& sRegEx);
	bool ExtractValues(CStdString& sLocation, std::vector<UINT> *pNumAry);

	void DumpObject();

	map<CStdString,UINT> m_transactionList;
	UINT m_transactionDefault;

	CStdString m_sMNU;
	CStdString m_sLocation;
	UINT m_nLocFormType;
	int m_nRecordType;
	int m_nField;
	int m_nSubField;
	int m_nItem;
	int m_nOffset;

	CStdString m_sCharType;
	int m_nMinFieldSize;
	int m_nMaxFieldSize;
	int m_nMinOccurrences;
	int m_nMaxOccurrences;
	CStdString m_sDescription;			// desc tag
	CStdString m_sLongDescription;		// long_desc tag
	CStdString m_sSpecialChars;			// sca tag
	CStdString m_sDateFormat;			// from date tag
	std::vector<CStdString> m_mmapName;	// from mmap tag
	std::vector<CStdString> m_omapName;	// from omap tag
	std::vector<CStdString> m_mmapDesc;	// from mmap tag
	std::vector<CStdString> m_omapDesc;	// from omap tag

	CStdString m_sAllowedChars;			// Set of chars based on m_sCharType/m_sSpecialChars

public:
	CRuleObj();
	virtual ~CRuleObj();

	// This function is also used by other classes since it's so useful, so we keep it public
	bool SetRange(CStdString& sRange, int *pMin, int *pMax);

	bool SetData(CStdString sFilePath, CStdString& sTransactionList, CStdString& sLocation, CStdString& sMNU, CStdString& sCharType,
				 CStdString& sFieldSize, CStdString& sOccurrences, CStdString& sDescription, CStdString& sLongDescription,
				 CStdString& sSpecialChars, CStdString& sDateFormat, CStdString& sMMap, CStdString& sOMap, CStdString& sTags,
				 CStdString& sErr);
	bool GetLocation(int inputIndex, int inputRecordIndex, int *recordType, int *recordIndex, int *field, int *subField, int *item);

	CStdString GetMNU() { return m_sMNU; }
	CStdString GetLocation() { return m_sLocation; }
	CStdString GetCharType() { return m_sCharType; }
	CStdString GetSpecialChars() { return m_sSpecialChars; }
	CStdString GetDescription() { return m_sDescription; }
	CStdString GetLongDescription() { return m_sLongDescription; }
	CStdString GetDateFormat() { return m_sDateFormat; }
	std::vector<CStdString> GetMMapValNames() { return m_mmapName; }
	std::vector<CStdString> GetOMapValNames() { return m_omapName; }
	std::vector<CStdString> GetMMapValDescriptions() { return m_mmapDesc; }
	std::vector<CStdString> GetOMapValDescriptions() { return m_omapDesc; }
	CStdString GetMMap();
	int GetLocFormType() { return m_nLocFormType; }
	int	GetRecordType() { return m_nRecordType; }
	int	GetField() { return m_nField; }
	int	GetSubField() { return m_nSubField; }
	int	GetItem() { return m_nItem; }
	int GetMinFieldSize() { return m_nMinFieldSize; }
	int GetMaxFieldSize() { return m_nMaxFieldSize; }
	int GetMinOccurrences() { return m_nMinOccurrences; }
	int GetMaxOccurrences() { return m_nMaxOccurrences; }
	int GetOffset() { return m_nOffset; }

	CStdString GetTransactionListString();
	CStdString GetAllowedChars()  { return m_sAllowedChars; }

	bool IsMandatory(CStdString& sTOT);
	bool IsOptional(CStdString& sTOT);
	bool AppliesTo(int iRecType, int iRecord, int iField, int iSubfield, int iItem);
};

#endif // _RULE_OBJ_
