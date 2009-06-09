
#ifdef __cplusplus
extern "C" {
#endif

int NISTBMPtoWSQ(char *pBMP, long lLenBMP, HGLOBAL *phWSQ, float fRate);
int NISTWSQtoBMP(char *pWSQ, long lLenWSQ, HGLOBAL *phBMP);
int NISTRAWtoWSQ(char *pRAW, long lWidth, long lHeight, long lDPI, HGLOBAL *phWSQ, float fRate);
int NISTWSQtoRAW(char *pWSQ, long lLenWSQ, HGLOBAL *phRAW, long *plWidth, long *plHeight, long *plDPI);

#ifdef __cplusplus
}
#endif
