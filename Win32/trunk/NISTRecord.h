
#ifndef _IW_NISTRECORD_H
#define _IW_NISTRECORD_H

#include "common.h"

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

	char* IWStrTok(char* pInStr, char cDelim, bool *pbEndofRecord = NULL);
	CNISTField *GetNISTField(int nField);

	void AddField(CNISTField *pField);
	int AddSubItems(CNISTField *pNISTField, char* szFieldData);

	int GetInfoFromImage(BYTE *pImage, long lImageLength, CStdString sFmt, IWNISTImageInfo *pInfo);
	void SetMandatoryImageFields(CNISTField *pNISTField, IWNISTImageInfo& info, long lRecordIndex, long lFieldLength);

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

	int GetNumSubfields(int nField, int *pCount);
	int GetNextField(int nField, int *pNextField);
	int GetNumItems(int Field, int Subfield, int* Count); //  { *Count = 0; return IW_SUCCESS; }
	int FindItem(int Field, int Subfield, int Item, CStdString& sData);
	int DeleteSubfield(int Field, int Subfield);
	int SetItem(CStdString sData, int Field, int Subfield, int Item);

	int ReadRecord(BYTE *pTransactionData, int nRecordType);
	int ReadBinaryRecord(BYTE *pTransactionData, int nRecordType);
	int Validate() { return IW_ERR_RECORD_NOT_FOUND; }

	int GetImage(CStdString& sStorageFormat, long *pLength, const BYTE **ppData);
	int SetImage(CStdString sInputFormat, int nRecordIndex, long nLength, BYTE *pData, CStdString sStorageFormat, float fCompression);
	int GetImageInfo(CStdString& spStorageFormat, long *pnLength, long *pnHLL, long *pnVLL, int *pnBitsPerPixel);
	int GetImageResolution(double *pfPPM);

	int Write(FILE *pFile);
	int WriteBinary(FILE *pFile);
	int WriteBinary( BYTE **ppBuffer, int *pnCurrentSize);
	int Write(TCHAR **ppBuffer, int *pnCurrentSize);


	static float CompressionToRate(float fCompression);
	static bool FormatSupportedInOut(CStdString sFormat);
	static bool FormatSupportedEBTS(CStdString sFormat);
	static int GetDATField(int nRecordType);
	static bool IsDATField(int nRecordType, int nField);
	static bool IsBinaryFingerType(int nRecordType);
	static bool IsBinaryType(int nRecordType);

	int GetRecordLen();
};

// Useful in determining width and heigh of WSQ images
typedef struct header_frm {
   unsigned char black;
   unsigned char white;
   unsigned short width;
   unsigned short height;
   float m_shift; 
   float r_scale;
   unsigned char wsq_encoder;
   unsigned short software;
} FRM_HEADER_WSQ;

// "IHDR" within JPEG2000 images
typedef struct {
	UINT width;
	UINT height;
	WORD numcmpts;
	BYTE bpc;
	BYTE comptype;
	BYTE csunk;
	BYTE ipr;
} jp2_ihdr_t;

#endif // _IW_NISTRECORD_H
