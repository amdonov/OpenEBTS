#ifndef IWVERIFICATION_H
#define IWVERIFICATION_H

class CIWTransaction;
class CRuleObj;

class CIWVerification
{
private:
	int LoadTOTDefinitions(TCHAR *pFile, CStdString sPath);
	int LoadRules(TCHAR *pFile, CStdString sPath, CStdString& sErr);

	CRuleObj *GetRule(CStdString sMNU);
	bool GetLocation(int *recordType, int *recordIndex, int *field, int *subField, int *item);

	CStringSlots	m_stringSlots;

	// begin Rule parsing functions
	CStdString GetTransactionList(TCHAR **ppRule);
	CStdString GetLocationIndex(TCHAR **ppRule);
	CStdString GetMneumonic(TCHAR **ppRule);
	CStdString GetCharType(TCHAR **ppRule);
	CStdString GetFieldSize(TCHAR **ppRule);
	CStdString GetOccurrences(TCHAR **ppRule);
	CStdString GetOptionalDescription(TCHAR **ppRule);
	CStdString GetOptionalSpecialChars(TCHAR **ppRule);
	CStdString GetOptionalDateFormat(TCHAR **ppRule);
	CStdString GetOptionalAdvancedRule(TCHAR **ppRule);
	CStdString GetOptionalMMap(TCHAR **ppRule);
	CStdString GetOptionalOMap(TCHAR **ppRule);
	CStdString GetOptionalLongDescription(TCHAR **ppRule);
	CStdString ExtractTagValue(TCHAR **ppRule, const TCHAR *szTag);
	CStdString ExtractAdvancedTagValue(TCHAR **ppRule, const TCHAR *szTag, bool bHasValue = false);

	CStdString GetNextToken(TCHAR **ppRule);
	CStdString GetRangeToken(TCHAR **ppRule);
	void SkipComments(TCHAR **ppRule);
	CStdString ReadLine(TCHAR** ppFile);
	TCHAR *GetRule(TCHAR **ppFile);
	// end Rule parsing functions

	bool VerifyFieldsForm1(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule);
	bool VerifyFieldsForm2(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule);
	bool VerifyFieldsForm3(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule);
	bool VerifyFieldsForm4(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule);
	bool VerifyFieldsForm5(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule);
	bool VerifyFieldsForm6(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule);
	bool VerifyFieldsForm7(CStdString& sTOT, CIWTransaction *pTrans, CRuleObj *pRule);
	bool VerifySubfieldOccurrences(CIWTransaction *pTrans, CRuleObj *pRule, int nSubfieldCount);
	bool VerifyfieldContents(CIWTransaction *pTrans, CRuleObj *pRule, CStdString sData);
	bool VerifyFieldLength(CIWTransaction *pTrans, CRuleObj *pRule, CStdString sData);
	bool VerifyFieldLengthTotal(CIWTransaction *pTrans, CRuleObj *pRule, int nTotalLen);
	bool VerifyFieldChars(CIWTransaction *pTrans, CRuleObj *pRule, CStdString sData);
	bool VerifyFieldDateFormat(CIWTransaction *pTrans, CRuleObj *pRule, CStdString sData);
	bool VerifyFieldValue(CIWTransaction *pTrans, CRuleObj *pRule, CStdString sData);
	long DaysInMonth(long y, long m);
	void FlagFieldError(CIWTransaction *pTrans, CRuleObj* pRule, int nErrCode, int nIDC, ...);

	bool IsAlpha(CStdString& s);
	bool IsNumeric(CStdString& s);
	bool IsAlphaNumeric(CStdString& s);
	bool IsAlphaSpecial(CStdString& s, CStdString& sSpecial);
	bool IsNumericSpecial(CStdString& s, CStdString& sSpecial);
	bool IsAlphaNumericSpecial(CStdString& s, CStdString& sSpecial);
	bool IsPrintable(CStdString& sIsPrintable, bool bAllowControlChars);

	std::vector<CTransactionDefinition> m_transactionDefAry;
	bool m_bVerificationLoaded;
	std::vector<CRuleObj> m_rulesAry;

public:
	CIWVerification();
	virtual ~CIWVerification();

	TCHAR* CreateNewStringSlot(CStdString s) { return m_stringSlots.AddNew(s); };

	int ReadVerificationFile(CStdString sPath, CStdString& sParseError);
	int VerifyTransaction(CIWTransaction *pTrans);
	bool IsLoaded() { return m_bVerificationLoaded; }

	void DebugOutputVerification();

	// Mnemonic based data access methods
	int GetMNULocation(CStdString sMNU, int inputIndex, int inputRecordIndex, int *recordType, int *recordIndex, int *field, int *subField, int *item);
	int GetTransactionCategories(int DataArraySize, const TCHAR **ppDataArray, int *pEntries);
	int GetTransactionTypes(int DataArraySize, const TCHAR **ppDataArray, 
							const TCHAR **ppDescArray, int *pEntries, const TCHAR *pCategory);
	int GetRecordTypeOccurrences(int DataArraySize, int *piRecordType, int *piMinOccurrences, int *piMaxOccurrences, int *pEntries, const TCHAR *pTOT);
	int GetNumRulesPerMNU(CStdString &sMNU);
	int GetMnemonics(const TCHAR* TransactionType, int DataArraySize, const TCHAR** ppDataArray, const TCHAR** ppDescArray, int* pEntries);
	int GetRuleRestrictions(const TCHAR* TransactionType, const TCHAR* pMnemonic, int* pRecordType,
							int* pField, int* pSubfield, int* pItem, const TCHAR** ppDesc, const TCHAR** ppLongDesc, const TCHAR** ppCharType,
							const TCHAR** ppSpecialChars, const TCHAR** ppDateFormat, const TCHAR** ppAdvancedRule, int* pSizeMin, int* pSizeMax,
							int* pOccMin, int* pOccMax, int* pOffset, bool* pAutomaticallySet, bool* pMandatory);
	int GetValueList(const TCHAR* TransactionType, const TCHAR* Mnemonic, bool *Mandatory,
					 int DataArraySize, const TCHAR** DataArray, const TCHAR** DescArray, int *Entries);
};

#endif // IWVERIFICATION_H
