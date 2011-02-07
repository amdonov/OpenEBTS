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

	bool SetRuleString(CStdString& sRule);
	CStdString GetRuleString();
	std::vector<CRecordTypeCount> GetRecTypeCountAry() { return m_recTypeCountAry; }

	bool IsValid() { return ((m_TOTArray.size() > 0) && !m_sRule.IsEmpty()); }
	bool IsEmpty() { return ((m_TOTArray.size() == 0) && m_sRule.IsEmpty()); }

	std::vector<CStdString> m_TOTArray;
	std::vector<CStdString> m_TOTLabelArray;
	CStdString m_sCategory;

private:
	CStdString m_sRule;
	std::vector<CRecordTypeCount> m_recTypeCountAry;
};

#endif // TRANSACTIONDEF_H
