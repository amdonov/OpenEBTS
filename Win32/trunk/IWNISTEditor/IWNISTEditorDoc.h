// IWNISTEditorDoc.h : interface of the CIWNISTEditorDoc class
//


#pragma once

#include "..\OpenEBTS.h"
#include "IWJPGLib.h"

#define READ_ALL_RECORDS -1

class CIWNistSubfield
{
public:
	CStringArray m_Items;
};

class CIWNistSubfieldArray : public CPtrArray
{
public:
	CIWNistSubfieldArray() {}
	~CIWNistSubfieldArray() { Empty(); }

	CIWNistSubfield* GetAt(int nIndex) { return ((CIWNistSubfield*) CPtrArray::GetAt(nIndex)); }
	void Remove(int nIndex) { delete GetAt(nIndex); RemoveAt(nIndex); }
	void Empty() { while(GetSize()) Remove(0); }
};

class CIWNistField
{
public:
	int m_ID;
	CIWNistSubfieldArray m_SubFields;
};

class CIWNistFieldArray : public CPtrArray
{
public:
	CIWNistFieldArray() {}
	~CIWNistFieldArray() { Empty(); }

	CIWNistField* GetAt(int nIndex) { return ((CIWNistField*) CPtrArray::GetAt(nIndex)); }
	void Remove(int nIndex) { delete GetAt(nIndex); RemoveAt(nIndex); }
	void Empty() { while(GetSize()) Remove(0); }
};

class CIWNistRecord
{
public:
	CIWNistRecord() { m_hDIB = m_hImage = NULL; m_nRecordType = m_nImageType = -1; }
	~CIWNistRecord()
	{
		if(m_hDIB) GlobalFree(m_hDIB);
		if(m_hImage) GlobalFree(m_hImage);
		m_arrFields.Empty();
	}

	int m_nRecordType;
	int m_nImageType;
	CIWNistFieldArray m_arrFields;
	CStringArray m_arrStrings;
	HGLOBAL m_hDIB;
	HGLOBAL m_hImage;
};


class CIWNISTEditorDoc : public CDocument
{
protected: // create from serialization only
	CIWNISTEditorDoc();
	DECLARE_DYNCREATE(CIWNISTEditorDoc)

// Attributes
public:
	CIWTransaction* m_pIWTrans;
	CPtrArray m_arrRecords;
	int m_nCurrentRecord;

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CIWNISTEditorDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);

public:
	// reads the entire file into a transaction object
	int ReadNIST(void);

	// reads transaction information record
	int ReadType1Record();

	// reads descriptive text records
	int ReadType2Record();

	// reads fingerprint records: Types 3,4,5,6,13,14
	int ReadFingerprintRecords(int nRecordType = RECORD_TYPE4, int nPos = READ_ALL_RECORDS, BOOL bIsCompressed = FALSE);

	// reads user-defined image records
	int ReadType7Records();

	// reads signature records
	int ReadType8Records();

	// reads minutiae records
	int ReadType9Records();

	// reads facial/smt records
	int ReadType10Records();

	// reads palmprint records
	int ReadType15Records();

	// reads user-defined testing image record
	int ReadType16Records();

	// reads iris records
	int ReadType17Records();

	int ReadType1617Records(int nRecordType);

	// reads CBEFF biometric data records
	int ReadType99Records();

	HGLOBAL GetThumbnail(int nType, int nPos);
	int GetImageType(const char* szType);
	int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

	HGLOBAL PackageAsHGLOBAL(char* p, long n);

public:
	CIWNistRecord* GetRecord(int nIndex);
public:
	virtual void DeleteContents();
};


