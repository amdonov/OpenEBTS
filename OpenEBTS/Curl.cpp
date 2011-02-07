#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <curl/easy.h>

#include <string>
using namespace std;

typedef struct _DATA
{       
    std::string* pstr;       
    bool bGrab;   /* set to FALSE when don't want to download */
} DATA; 

static size_t writefunction( void *ptr , size_t size , size_t nmemb , void *stream ) 
{
    if ( !((DATA*) stream)->bGrab )
		return -1; 
	std::string* pStr = ((DATA*) stream)->pstr;
	if ( size * nmemb ) 
		pStr->append((const char*) ptr, size * nmemb);
	return nmemb * size; 
}

bool DownloadURLContent(std::string strUrl , std::string & strContent,
						std::string &headers,bool grabHeaders = true,
						bool grabUrl = true )
{       
	CURL *curl_handle;
	DATA data = { &strContent, grabUrl };
	DATA headers_data = {&headers , grabHeaders}; 
	if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK)
		return false;  
	if ((curl_handle = curl_easy_init()) == NULL)
		return false;   

	char stdError[CURL_ERROR_SIZE] = { '\0' };   
	if (curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER , stdError) != CURLE_OK)
		goto clean_up; 
	if (curl_easy_setopt(curl_handle, CURLOPT_URL, strUrl.c_str()) != CURLE_OK)
		goto clean_up;  
	if (curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writefunction) != CURLE_OK)
		goto clean_up;  
	if (grabHeaders)
	{ 
		if (curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, writefunction) != CURLE_OK)
			goto clean_up; 
		if (curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, (void *)&headers_data) != CURLE_OK)
			goto clean_up;  
	}  
	if (curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&data) != CURLE_OK)
		goto clean_up;  
	//if ( curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, MY_USR_AGENT) != CURLE_OK )  
	if (curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0") != CURLE_OK)
		goto clean_up; 
	if (curl_easy_perform(curl_handle) != CURLE_OK)
		if (grabUrl)
			goto clean_up;  
	curl_easy_cleanup(curl_handle); 
	curl_global_cleanup();  

	return true;

clean_up:
	printf("(%s %d) error: %s", __FILE__,__LINE__, stdError); 
	curl_easy_cleanup(curl_handle);  
	curl_global_cleanup();

	return false;
}
