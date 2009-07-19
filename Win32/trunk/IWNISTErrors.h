#ifndef _IWNISTERRORS_H_
#define _IWNISTERRORS_H_

#define IW_SUCCESS                                   0
#define IW_ERR_LOADING_VERICATION                    1
#define IW_ERR_OPENING_FILE_FOR_READING              2
#define IW_ERR_OPENING_FILE_FOR_WRITING              3
#define IW_ERR_READING_FILE                          4
#define IW_ERR_WRITING_FILE                          5
#define IW_ERR_RECORD_NOT_FOUND                      6
#define IW_ERR_TRANSACTION_NOT_LOADED                7
#define IW_ERR_VERIFICATION_NOT_LOADED               8
#define IW_ERR_NULL_POINTER                          9
#define IW_ERR_NULL_TRANSACTION_POINTER             10
#define IW_ERR_UNSUPPORTED_IMAGE_FORMAT             11
#define IW_ERR_UNSUPPORTED_RECORD_TYPE              12
#define IW_ERR_INDEX_OUT_OF_RANGE                   13
#define IW_ERR_INVALID_SUBFIELD_NUM                 14
#define IW_ERR_MNEMONIC_NOT_FOUND                   15
#define IW_ERR_OUT_OF_MEMORY                        16
#define IW_ERR_WSQ_COMPRESS                         17
#define IW_ERR_WSQ_DECOMPRESS                       18
#define IW_ERR_IMAGE_CONVERSION                     19

#define IW_WARN_TRANSACTION_FAILED_VERIFICATION   1000	 
#define IW_WARN_INVALID_FIELD_NUM                 1001
#define IW_WARN_REQ_FIELD_MISSING                 1002
#define IW_WARN_INCORRECT_ITEM_COUNT              1003
#define IW_WARN_TOO_FEW_DATA_CHARS                1004
#define IW_WARN_TOO_MANY_DATA_CHARS               1005
#define IW_WARN_DATA_NOT_NUMERIC                  1006
#define IW_WARN_DATA_NOT_ALPHA                    1007
#define IW_WARN_DATA_NOT_ALPHANUMERIC             1008
#define IW_WARN_TOO_FEW_SUBFIELDS                 1009
#define IW_WARN_TOO_MANY_SUBFIELDS                1010
#define IW_WARN_UNSUPPORT_FIELD_PRESENT           1011
#define IW_WARN_TRANSACTION_FAILED_XML_PARSE      1012

#endif /* _IWNISTERRORS_H_ */