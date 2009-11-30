#ifndef IWVERIFICATION_H
#define IWVERIFICATION_H

class CIWTransaction;
class CRuleObj;

class CIWVerification
{
private:
	int LoadTOTDefinitions(CStdString& sFilePath);
	int LoadRules(CStdString& sFilePath, CStdString& sErr);

	CRuleObj *GetRule(const char *pMNU);
	BOOL GetLocation(int *recordType, int *recordIndex, int *field, int *subField, int *item);

	// begin Rule parsing functions
	CStdString GetTransactionList(char **ppRule);
	CStdString GetLocationIndex(char **ppRule);
	CStdString GetMneumonic(char **ppRule);
	CStdString GetCharType(char **ppRule);
	CStdString GetFieldSize(char **ppRule);
	CStdString GetOccurrences(char **ppRule);
	CStdString GetOptionalDescription(char **ppRule);
	CStdString GetOptionalSpecialChars(char **ppRule);
	CStdString GetOptionalDateFormat(char **ppRule);
	CStdString GetOptionalMMap(char **ppRule);
	CStdString GetTags(char **ppRule);
	CStdString ExtractTagValue(char **ppRule, const char *szTag);

	CStdString GetNextToken(char **ppRule);
	CStdString GetRangeToken(char **ppRule);
	void SkipComments(char **ppRule);
	CStdString ReadLine(char **ppFile);
	char *GetRule(char **ppFile);
	// end Rule parsing functions

	BOOL VerifyFieldsForm1(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule);
	BOOL VerifyFieldsForm2(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule);
	BOOL VerifyFieldsForm3(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule);
	BOOL VerifyFieldsForm4(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule);
	BOOL VerifyFieldsForm5(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule);
	BOOL VerifyFieldsForm6(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule);
	BOOL VerifyFieldsForm7(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule);
	BOOL VerifySubfieldOccurences(CIWTransaction *pTrans, CRuleObj *pRule, int nSubfieldCount);
	BOOL VerifyfieldContents(CIWTransaction *pTrans, CRuleObj *pRule, const char *pData);
	BOOL VerifyFieldLength(CIWTransaction *pTrans, CRuleObj *pRule, const char *pData);
	BOOL VerifyFieldLengthTotal(CIWTransaction *pTrans, CRuleObj *pRule, int nTotalLen);
	BOOL VerifyFieldChars(CIWTransaction *pTrans, CRuleObj *pRule, const char *pData);
	BOOL VerifyFieldDateFormat(CIWTransaction *pTrans, CRuleObj *pRule, const char *pData);
	BOOL VerifyFieldValue(CIWTransaction *pTrans, CRuleObj *pRule, const char *pData);
	long DaysInMonth(long y, long m);
	void FlagFieldError(CIWTransaction *pTrans, CRuleObj* pRule, int nErrCode, const TCHAR *szFormat, ...);

	BOOL IsAlpha(CStdString& s);
	BOOL IsNumeric(CStdString& s);
	BOOL IsAlphaNumeric(CStdString& s);
	BOOL IsAlphaSpecial(CStdString& s, CStdString& sSpecial);
	BOOL IsNumericSpecial(CStdString& s, CStdString& sSpecial);
	BOOL IsAlphaNumericSpecial(CStdString& s, CStdString& sSpecial);
	BOOL IsPrintable(CStdString& sIsPrintable, BOOL bAllowControlChars);

	std::vector<CTransactionDefinition> m_transactionDefAry;
	BOOL m_bVerificationLoaded;
	std::vector<CRuleObj> m_rulesAry;

public:
	CIWVerification();
	virtual ~CIWVerification();

	int ReadVerificationFile(const char* Path, int MaxParseError, char* ParseError);
	int VerifyTransaction(CIWTransaction *pTrans);
	BOOL IsLoaded() { return m_bVerificationLoaded; }

	void DebugOutputVerification();

	// Mneumonic based data access methods
	int GetMNULocation(const char *pMNU, int inputIndex, int inputRecordIndex, int *recordType, int *recordIndex, int *field, int *subField, int *item);
	int GetTransactionCategories(int DataArraySize, const char **ppDataArray, int *pEntries);
	int GetTransactionTypes(int DataArraySize, const char **ppDataArray, 
							const char **ppDescArray, int *pEntries, const char *pCategory);
	int GetRecordTypeOccurences(int DataArraySize, int *piRecordType, int *piMinOccurences, int *piMaxOccurences, int *pEntries, const char *pCategory);
	int GetNumRulesPerMNU(CStdString &sMNU);
};

#endif // IWVERIFICATION_H
