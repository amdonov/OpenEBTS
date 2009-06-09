#ifndef TRANSACTIONDEF_H
#define TRANSACTIONDEF_H

class CRecordTypeCount
{
public:
	int	nRecordType;
	int nMin;
	int nMax;
};

class CTransactionDefinition
{
public:
	CTransactionDefinition();
	~CTransactionDefinition();

	BOOL SetRuleString(CStdString& sRule);
	CStdString GetRuleString();
	std::vector<CRecordTypeCount> GetRecTypeCountAry() { return m_recTypeCountAry; }

	BOOL IsValid() { return ((m_TOTArray.size() > 0) && (m_sRule != _T(""))); }
	BOOL IsEmpty() { return ((m_TOTArray.size() == 0) && (m_sRule == _T(""))); }

	std::vector<CStdString> m_TOTArray;
	std::vector<CStdString> m_TOTLabelArray;
	CStdString m_sCategory;

private:
	CStdString m_sRule;
	void FreeRecTypeCounts();
	std::vector<CRecordTypeCount> m_recTypeCountAry;
};

#endif // TRANSACTIONDEF_H