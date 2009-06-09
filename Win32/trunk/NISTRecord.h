
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

	BOOL m_bGetImage;

	char *IWStrTok(char *pInStr, char cDelim, BOOL *pbEndofRecord = NULL);
	CNISTField *GetNISTField(int nField);

	void AddField(CNISTField *pField);
	int AddSubItems(CNISTField *pNISTField, char *pszFieldData);

	int GetInfoFromImage(BYTE *pImage, long lImageLength, const char* szFmt, IWNISTImageInfo *pInfo);
	void SetMandatoryImageFields(CNISTField *pNISTField, IWNISTImageInfo& info, long lRecordIndex, long lFieldLength);

	int GetFingerprintInfo(int nRecordType, char *pTransactionData);
	int GetSignatureInfo(char *pTransactionData);

	// binary record helper routines
	long GetDecimalValue(char *pInStr, int nStrLen);
	char *BytetoHexString(BYTE lDecimalValue, char *pszHexString);

public:
	CNISTRecord();
	virtual ~CNISTRecord();

	int ReadLogicalRecordLen(char *pTransactionData, int nRecordType, int nRecordIndex = 1);
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
	int FindItem(int Field, int Subfield, int Item, const char** ppData);
	int DeleteSubfield(int Field, int Subfield);
	int SetItem(const char *pData, int Field, int Subfield, int Item);

	int ReadRecord(char *pTransactionData, int nRecordType);
	int ReadBinaryRecord(char *pTransactionData, int nRecordType);
	int Validate() { return IW_ERR_RECORD_NOT_FOUND; }

	int GetImage(const char **ppStorageFormat, long *pLength, const void **ppData);
	int SetImage(const char *pInputFormat, int RecordIndex, long Length, 
		void *pData, const char *pStorageFormat, float Compression);
	int GetImageInfo(const char **ppStorageFormat, long *pLength, long *phll, long *pvll,
		int *pBitsPerPixel);
	int GetImageResolution(double *pfPPM);

	int Write(FILE *pFile);
	int WriteBinary(FILE *pFile);

	static float CompressionToRate(float Compression);
	static BOOL FormatSupportedInOut(const char *pFormat);
	static BOOL FormatSupportedEBTS(const char *pFormat);
	static int GetDATField(int nRecordType);
	static BOOL IsDATField(int nRecordType, int nField);
	static BOOL IsBinaryFingerType(int nRecordType);
	static BOOL IsBinaryType(int nRecordType);

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
