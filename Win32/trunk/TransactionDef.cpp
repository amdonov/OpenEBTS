#include "stdafx.h"
#include "TransactionDef.h"
#include "RuleObj.h"


CTransactionDefinition::CTransactionDefinition()
{
}

CTransactionDefinition::~CTransactionDefinition()
{
	FreeRecTypeCounts();
}

void CTransactionDefinition::FreeRecTypeCounts()
{
	//TODO REMOVE
	/*
	int nCount;
	CRecordTypeCount *pRecTypeCount;

	nCount = m_recTypeCountAry.GetSize();

	for (int i=0; i<nCount; i++)
	{
		pRecTypeCount = (CRecordTypeCount*)m_recTypeCountAry.GetAt(i);
		if (pRecTypeCount)
		{
			delete pRecTypeCount;
		}
	}
	m_recTypeCountAry.RemoveAll();
	*/
}

BOOL CTransactionDefinition::SetRuleString(CStdString& sRule)
// Parse out array of ranges, e.g., "1:1 2:1 4:0-99 9:0-14 10:0-5 14:0-14 16:0-6"
{
	BOOL bRet = FALSE;
	CRuleObj rule; // We use this class just for SetRange
	int nMin;
	int nMax;
	int i;
	int nCounts;
	int nPos;
	CStdString sTemp;
	CStdString sRecType;
	CStdString sRecRange;

	FreeRecTypeCounts();

	m_sRule = sRule;

	// first count number of ranges by counting the ':'
	nCounts = 0;
	for (i=0; i<m_sRule.GetLength(); i++)
	{
		if (m_sRule.GetAt(i) == ':') nCounts++;
	}

	sTemp = m_sRule;
	sTemp += ' ';	// This simplifies the parsing code
	for (i=0; i<nCounts; i++)
	{
		nPos = sTemp.Find(':');
		if (nPos == -1) break;
		sRecType = sTemp.Left(nPos);
		sTemp = sTemp.Right(sTemp.GetLength()-nPos-1);

		nPos = sTemp.Find(' ');
		if (nPos == -1) break;
		sRecRange = sTemp.Left(nPos);
		sTemp = sTemp.Right(sTemp.GetLength()-nPos-1);

		if (!rule.SetRange(sRecRange, &nMin, &nMax))
		{
			break;
		}

		CRecordTypeCount* pRecTypeCount = new CRecordTypeCount;
		pRecTypeCount->nRecordType = atol(sRecType);
		pRecTypeCount->nMin = nMin;
		pRecTypeCount->nMax = nMax;
		m_recTypeCountAry.push_back(*pRecTypeCount);
	}

	// Make sure each ':' created a CRecordTypeCount
	if (m_recTypeCountAry.size() == nCounts)
	{
		bRet = TRUE;
	}

	return bRet;
}

CStdString CTransactionDefinition::GetRuleString()
{
	return m_sRule;
}
