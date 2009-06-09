
#ifdef __cplusplus
extern "C" {
#endif

int LIBTIFFRAWtoGroup4Fax(char *pRAW, long lWidth, long lHeight, long lDPI, HGLOBAL *phFX4);
int LIBTIFFGroup4FaxtoRAW(char *pFX4, long lLenFX4, HGLOBAL *phRAW, long *plWidth, long *plHeight, long *plDPI);
int LIBTIFFGetInfo(char *pTIF, long lLenTIF, long *plWidth, long *plHeight, long *plDPI);

#ifdef __cplusplus
}
#endif
