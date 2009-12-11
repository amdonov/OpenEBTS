
#ifndef _COMMON_H
#define _COMMON_H

#include "stdafx.h"

#define RECORD_TYPE1  (int)1	// transaction information
#define RECORD_TYPE2  (int)2	// Descriptive text (user defined)
#define RECORD_TYPE3  (int)3	// Fingerprint (low resolution grayscale)
#define RECORD_TYPE4  (int)4	// fingerprint (high resolution grayscale)
#define RECORD_TYPE5  (int)5	// fingerprint (low resolution binary)
#define RECORD_TYPE6  (int)6	// fingerprint (high resolution binary)
#define RECORD_TYPE7  (int)7	// Image data (user defined)
#define RECORD_TYPE8  (int)8	// Signature image data
#define RECORD_TYPE9  (int)9	// Minutiae data
#define RECORD_TYPE10 (int)10	// Facial and SMT image data
#define RECORD_TYPE11 (int)11	// future use
#define RECORD_TYPE12 (int)12	// future use
#define RECORD_TYPE13 (int)13	// latent image data
#define RECORD_TYPE14 (int)14	// Tenprint fingerprint
#define RECORD_TYPE15 (int)15	// Palmprint image data
#define RECORD_TYPE16 (int)16	// Testing image data (user defined)
#define RECORD_TYPE17 (int)17	// Iris
#define RECORD_TYPE99 (int)99	// CBEFF

#define STR_PERIOD_SEP "."
#define STR_COLON_SEP	":"
#define STR_GS_SEP "(char)0x1D" // "(char)0x1D"

#define CHAR_FS	(char)0x1C		// ASCII Field Separator
#define CHAR_GS	(char)0x1D		// ASCII Group Separator
#define CHAR_RS	(char)0x1E		// ASCII Record Separator
#define CHAR_US	(char)0x1F		// ASCII Unit Separator

#define CHAR_PERIOD	'.'
#define CHAR_COLON ':'

// tags common across records(except for rectype 1)
#define REC_TAG_LEN 1
#define REC_TAG_VER	2
#define REC_TAG_IDC	2

#include "OpenEBTSErrors.h"

/***************
/* TYPE 1 Fields
/**************/
#define TYPE1_LEN	1
#define TYPE1_VER	2
#define TYPE1_CNT	3
#define TYPE1_TOT	4
#define TYPE1_DAT	5
#define TYPE1_PRY	6
#define TYPE1_DAI	7
#define TYPE1_ORI	8
#define TYPE1_TCN	9
#define TYPE1_TCR	10
#define TYPE1_NSR	11
#define TYPE1_NTR	12
#define TYPE1_DOM	13
#define TYPE1_GMT	14
#define TYPE1_DCS	15

/*****************
/* TYPE 3-6 Fields
/****************/
#define TYPE4_LEN	1
#define TYPE4_IDC	2
#define TYPE4_IMP	3
#define TYPE4_FGP	4
#define TYPE4_ISR	5
#define TYPE4_HLL	6
#define TYPE4_VLL	7
#define TYPE4_GCA	8
#define TYPE4_DAT	9

/***************
/* TYPE 7 Fields
/***************/
#define TYPE7_ISR	5
#define TYPE7_HLL	6
#define TYPE7_VLL	7
#define TYPE7_GCA	8
#define TYPE7_DAT	9

/***************
/* TYPE 8 Fields
/***************/
#define TYPE8_LEN	1
#define TYPE8_IDC	2
#define TYPE8_SIG	3
#define TYPE8_SRT	4
#define TYPE8_ISR	5
#define TYPE8_HLL	6
#define TYPE8_VLL	7
#define TYPE8_DAT	8

/***************
/* TYPE 10 Fields
/***************/
#define TYPE10_HLL	6
#define TYPE10_VLL	7
#define TYPE10_SLC	8
#define TYPE10_HPS	9
#define TYPE10_VPS	10
#define TYPE10_CGA	11
#define TYPE10_CSP	12
#define TYPE10_DAT	999

/***************
/* TYPE 13 Fields
/***************/
#define TYPE13_HLL	6
#define TYPE13_VLL	7
#define TYPE13_SLC	8
#define TYPE13_HPS	9
#define TYPE13_VPS  10
#define TYPE13_CGA	11
#define TYPE13_BPX  12
#define TYPE13_DAT	999

/***************
/* TYPE 14 Fields
/***************/
#define TYPE14_HLL	6
#define TYPE14_VLL	7
#define TYPE14_SLC	8
#define TYPE14_HPS	9
#define TYPE14_VPS	10
#define TYPE14_CGA	11
#define TYPE14_BPX	12
#define TYPE14_DAT	999

/***************
/* TYPE 15 Fields
/***************/
#define TYPE15_HLL	6
#define TYPE15_VLL	7
#define TYPE15_SLC	8
#define TYPE15_HPS	9
#define TYPE15_VPS	10
#define TYPE15_CGA	11
#define TYPE15_BPX	12
#define TYPE15_DAT	999

/***************
/* TYPE 16 Fields
/***************/
#define TYPE16_IFM  5
#define TYPE16_HLL	6
#define TYPE16_VLL	7
#define TYPE16_SLC	8
#define TYPE16_HPS	9
#define TYPE16_VPS	10
#define TYPE16_CGA	11
#define TYPE16_BPX	12
#define TYPE16_DAT	999

/***************
/* TYPE 17 Fields
/***************/
#define TYPE17_HLL	6
#define TYPE17_VLL	7
#define TYPE17_SLC	8
#define TYPE17_HPS	9
#define TYPE17_VPS	10
#define TYPE17_CGA	11
#define TYPE17_BPX	12
#define TYPE17_CSP	13
#define TYPE17_DAT	999

/***************
/* TYPE 99 Fields
/***************/
#define TYPE99_DAT	999

// Redefinitions in "short format"
#define T1_LEN	TYPE1_LEN	
#define T1_VER	TYPE1_VER	
#define T1_CNT	TYPE1_CNT	
#define T1_TOT	TYPE1_TOT	
#define T1_DAT	TYPE1_DAT	
#define T1_PRY	TYPE1_PRY	
#define T1_DAI	TYPE1_DAI	
#define T1_ORI	TYPE1_ORI	
#define T1_TCN	TYPE1_TCN	
#define T1_TCR	TYPE1_TCR	
#define T1_NSR	TYPE1_NSR	
#define T1_NTR	TYPE1_NTR	
#define T1_DOM	TYPE1_DOM	
#define T1_GMT	TYPE1_GMT	
#define T1_DCS	TYPE1_DCS	
#define T4_LEN	TYPE4_LEN
#define T4_IDC	TYPE4_IDC
#define T4_IMP	TYPE4_IMP
#define T4_FGP	TYPE4_FGP
#define T4_ISR	TYPE4_ISR
#define T4_HLL	TYPE4_HLL
#define T4_VLL	TYPE4_VLL
#define T4_GCA	TYPE4_GCA
#define T4_DAT	TYPE4_DAT

/***************************
/* Exception handling macros
/***************************/

#define IWS_BEGIN_EXCEPTION_METHOD(FROMSTRING) \
	CStdString sFrom(FROMSTRING); \
	CStdString sException;

#define IWS_BEGIN_CATCHEXCEPTION_BLOCK() \
	try \
	{

#define IWS_END_CATCHEXCEPTION_BLOCK() \
	} \
	catch (...) \
	{ \
		sException.Format("[%s] Unknown exception occured.",sFrom); \
		LogFile(LF_OPENEBTS,sException); \
	}

#define LF_OPENEBTS	"OpenEBTS.log"

extern BOOL g_bTraceOn; 
extern BOOL g_bLogErrors; 

void LogFile(char *pLogFile, CStdString& sException);
void TraceMsg(CStdString& sTraceMsg);

void SetLogFlags();

#endif // _COMMON_H