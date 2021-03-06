
#ifndef _IW_NISTRECORD_H
#define _IW_NISTRECORD_H

#include "Common.h"

class CNISTField;

class CNISTRecord
{
protected:
	std::vector<CNISTField*> m_FieldList;
	int m_nRecordType;
	int m_nRecordLen;
	int m_nRecordOffset;
	double m_dNativeResolutionPPMM;

	bool m_bGetImage;

	char* IWStrTok(char** ppCurPos, char** ppString, char** ppEndString, int* pnCurPos, char* pInStr, char cDelim, bool *pbEndofRecord = NULL);
	CNISTField *GetNISTField(int nField);

	void AddField(CNISTField *pField);
	int AddSubItems(CNISTField *pNISTField, char* szFieldData);
	void RemoveField(CNISTField *pField);

	int GetInfoFromImage(BYTE *pImage, int cbLength, CStdString sFmt, OpenEBTSImageInfo *pInfo);
	void SetMandatoryImageFields(CNISTField *pNISTField, OpenEBTSImageInfo& info, int recordIndex, int fieldLength);

	int GetFingerprintInfo(int nRecordType, BYTE* pTransactionData);
	int GetSignatureInfo(BYTE* pTransactionData);

	// binary record helper routines
	int GetDecimalValue(BYTE* pInStr, int nStrLen);
	char *BytetoHexString(BYTE lDecimalValue, char *pszHexString);

public:
	CNISTRecord();
	virtual ~CNISTRecord();

	int ReadLogicalRecordLen(BYTE* pTransactionData, int nRecordType, int nRecordIndex = 1);
	int GetLogicalRecordLen() { return m_nRecordLen; }

	void InitializeNewRecord(int nRecordType);
	
	void SetNativeScanningResolution(double dNativeResolutionPPM);

	int GetRecordType() { return m_nRecordType; }
	int SetRecordType(int nRecordType) { m_nRecordType = nRecordType; return IW_SUCCESS; }

	int GetRecordOffset() { return m_nRecordOffset; }
	int SetRecordOffset(int nRecordOffset) { m_nRecordOffset = nRecordOffset; return IW_SUCCESS; }

	int GetNumFields(int *pCount);
	int GetNumSubfields(int nField, int *pCount);
	int GetNextField(int nField, int *pNextField);
	int GetNumItems(int Field, int Subfield, int* Count); //  { *Count = 0; return IW_SUCCESS; }
	int FindItem(int Field, int Subfield, int Item, CStdString& sData);
	int DeleteSubfield(int Field, int Subfield);
	int SetItem(CStdString sData, int Field, int Subfield, int Item);
	int RemoveItem(int Field, int Subfield, int Item);

	int ReadRecord(BYTE *pTransactionData, int nRecordType);
	int ReadBinaryRecord(BYTE *pTransactionData, int nRecordType);
	int Validate() { return IW_ERR_RECORD_NOT_FOUND; }

	int GetImage(CStdString& sStorageFormat, int *pLength, const BYTE **ppData);
	int SetImage(CStdString sInputFormat, int nRecordIndex, int nLength, BYTE *pData, CStdString sStorageFormat, float fCompression);
	int GetImageInfo(CStdString& spStorageFormat, int *pnLength, int *pnHLL, int *pnVLL, int *pnBitsPerPixel);
	int GetImageResolution(double *pfPPM);

	int WriteBinary(BYTE *pBuffer, int *pnCurrentSize);
	int Write(BYTE *pBuffer, int *pnCurrentSize);


	static float CompressionToRate(float fCompression);
	static bool FormatSupportedInput(CStdString sFormat);
	static bool FormatSupportedEBTS(CStdString sFormat);
	static int GetDATField(int nRecordType);
	static bool IsDATField(int nRecordType, int nField);
	static bool IsBinaryFingerType(int nRecordType);
	static bool IsBinaryType(int nRecordType);

	void AdjustRecordLength();
	int GetLength();
};

#endif // _IW_NISTRECORD_H
