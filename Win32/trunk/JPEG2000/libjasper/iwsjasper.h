
#ifdef __cplusplus
extern "C" {
#endif

bool JasperBMPtoJP2(char *pBMP, long lLenBMP, HGLOBAL *phJP2, float fRate);
bool JasperJP2toBMP(char *pJP2, long lLenJP2, HGLOBAL *phBMP);

#ifdef __cplusplus
}
#endif
