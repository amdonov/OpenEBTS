bool AddPrintHelper(CIWTransaction *pTrans, long lType, const TCHARPATH *szFolder, const TCHARPATH *szFile, const TCHAR *szFmtIn, const TCHAR *szFmtOut, int iImpressionType, int iFingerIndex);
bool AddMinutiaeHelper(CIWTransaction *pTrans, const TCHARPATH *szFolder, const TCHARPATH *szFile, int iImpressionType, int iFingerIndex);
bool AddMugshotHelper(CIWTransaction *pTrans, const TCHARPATH *szFolder, const TCHARPATH *szFile, const TCHAR *szFmtIn, const TCHAR *szFmtOut);
bool AddSignatureHelper(CIWTransaction *pTrans, const TCHARPATH *szFolder, const TCHARPATH *szFile, const TCHAR *szFmt);
bool AddType16Helper(CIWTransaction *pTrans, const TCHARPATH *szFolder, const TCHARPATH *szFile, const TCHAR *szFmtIn, const TCHAR *szFmtOut, int iFeature);
bool AddIrisHelper(CIWTransaction *pTrans, const TCHARPATH *szFolder, const TCHARPATH *szFile, const TCHAR *szFmt, int iEye);
bool AddCBEFFHelper(CIWTransaction *pTrans, const TCHARPATH *szFolder, const TCHARPATH *szFile, const TCHAR *szBiometricType, const TCHAR *szBDBOwner, const TCHAR *szBDBType);

bool WriteBLOBToFile(const TCHARPATH *szFilename, BYTE* pBlob, long lBlobLen);
BYTE* ReadBLOBFromFile(const TCHARPATH *szFilename, long *plFileSize);
bool CheckRet(const TCHAR *szContext, long lRetVal);
