bool AddPrintHelper(CIWTransaction *pTrans, long lType, TCHAR *szFolder, TCHAR *szFile, TCHAR *szFmtIn, TCHAR *szFmtOut, int iImpressionType, int iFingerIndex);
bool AddMinutiaeHelper(CIWTransaction *pTrans, TCHAR *szFolder, TCHAR *szFile, int iImpressionType, int iFingerIndex);
bool AddMugshotHelper(CIWTransaction *pTrans, TCHAR *szFolder, TCHAR *szFile, TCHAR *szFmtIn, TCHAR *szFmtOut);
bool AddSignatureHelper(CIWTransaction *pTrans, TCHAR *szFolder, TCHAR *szFile, TCHAR *szFmt);
bool AddType16Helper(CIWTransaction *pTrans, TCHAR *szFolder, TCHAR *szFile, TCHAR *szFmtIn, TCHAR *szFmtOut, int iFeature);
bool AddIrisHelper(CIWTransaction *pTrans, TCHAR *szFolder, TCHAR *szFile, TCHAR *szFmt, int iEye);
bool AddCBEFFHelper(CIWTransaction *pTrans, TCHAR *szFolder, TCHAR *szFile, TCHAR *szBiometricType, TCHAR *szBDBOwner, TCHAR *szBDBType);

bool WriteBLOBToFile(TCHAR *szFilename, BYTE* pBlob, long lBlobLen);
BYTE* ReadBLOBFromFile(TCHAR *szFilename, long *plFileSize);
bool CheckRet(TCHAR *szContext, long lRetVal);
