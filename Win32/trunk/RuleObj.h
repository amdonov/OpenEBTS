
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
	BOOL SetTransactions(CStdString& sTransactionList);
	BOOL SetLocation(CStdString& sLocation);
	BOOL SetMNU(CStdString& sMNU);
	BOOL SetCharType(CStdString& sCharType);
	BOOL SetFieldSize(CStdString& sFieldSize);
	BOOL SetOccurrences(CStdString& sOccurence);
	BOOL SetTags(CStdString& sTags);

	BOOL GetRangeValue(CStdString& sToken, int *pValue);

	BOOL TestRegEx(CStdString& sInput, CStdString& sRegEx);
	BOOL ExtractValues(CStdString& sLocation, std::vector<UINT> *pNumAry);

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

public:
	CRuleObj();
	virtual ~CRuleObj();

	// This function is also used by other classes since it's so useful, so we keep it public
	BOOL SetRange(CStdString& sRange, int *pMin, int *pMax);

	BOOL SetData(CStdString& sTransactionList, CStdString& sLocation, CStdString& sMNU, CStdString& sCharType,
								CStdString& sFieldSize, CStdString& sOccurrences, CStdString& sTags);
	BOOL IsValid();
	BOOL GetLocation(int inputIndex, int inputRecordIndex, int *recordType, int *recordIndex, int *field, int *subField, int *item);

	CStdString GetMNU() { return m_sMNU; }
	CStdString GetLocation() { return m_sLocation; }
	CStdString GetCharType() { return m_sCharType; }
	int GetLocFormType() { return m_nLocFormType; }
	int	GetRecordType() { return m_nRecordType; }
	int	GetField() { return m_nField; }
	int	GetItem() { return m_nItem; }
	int GetMinFieldSize() { return m_nMinFieldSize; }
	int GetMaxFieldSize() { return m_nMaxFieldSize; }
	int GetMinOccurrences() { return m_nMinOccurrences; }
	int GetMaxOccurrences() { return m_nMaxOccurrences; }
	CStdString GetTransactionListString();

	BOOL IsMandatory(CStdString& sTOT);
	BOOL IsOptional(CStdString& sTOT);
	BOOL AppliesTo(int iRecType, int iRecord, int iField, int iSubfield, int iItem);
};

#endif // _RULE_OBJ_
