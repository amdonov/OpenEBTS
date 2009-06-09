bool AddPrintHelper(CIWTransaction *pTrans, long lType, char *szFolder, char *szFile, char *szFmtIn, char *szFmtOut, int iImpressionType, int iFingerIndex);
bool AddMinutiaeHelper(CIWTransaction *pTrans, char *szFolder, char *szFile, int iImpressionType, int iFingerIndex);
bool AddMugshotHelper(CIWTransaction *pTrans, char *szFolder, char *szFile, char *szFmtIn, char *szFmtOut);
bool AddSignatureHelper(CIWTransaction *pTrans, char *szFolder, char *szFile, char *szFmt);
bool AddType16Helper(CIWTransaction *pTrans, char *szFolder, char *szFile, char *szFmtIn, char *szFmtOut, int iFeature);
bool AddIrisHelper(CIWTransaction *pTrans, char *szFolder, char *szFile, char *szFmt, int iEye);
bool AddCBEFFHelper(CIWTransaction *pTrans, char *szFolder, char *szFile, char *szBiometricType, char *szBDBOwner, char *szBDBType);

bool WriteBLOBToFile(char *szFilename, char* pBlob, long lBlobLen);
BYTE* ReadBLOBFromFile(char *szFilename, long *plFileSize);
bool CheckRet(char *szContext, long lRetVal);
