/*******************************************************************************

License: 
This software was developed at the National Institute of Standards and 
Technology (NIST) by employees of the Federal Government in the course 
of their official duties. Pursuant to title 17 Section 105 of the 
United States Code, this software is not subject to copyright protection 
and is in the public domain. NIST assumes no responsibility  whatsoever for 
its use by other parties, and makes no guarantees, expressed or implied, 
about its quality, reliability, or any other characteristic. 

Disclaimer: 
This software was developed to promote biometric standards and biometric
technology testing for the Federal Government in accordance with the USA
PATRIOT Act and the Enhanced Border Security and Visa Entry Reform Act.
Specific hardware and software products identified in this software were used
in order to perform the software development.  In no case does such
identification imply recommendation or endorsement by the National Institute
of Standards and Technology, nor does it imply that the products and equipment
identified are necessarily the best available for the purpose.  

*******************************************************************************/

/*
 *
 * Stan Janet
 * December 4, 1990
 *
 */

#define XMGetSubImageDataDepth(_src,_x,_y,_srcw,_srch,_dst,_dstw,_dsth) \
{ \
int _i, _dst_index, _src_index; \
\
if (_x < 0) \
	_x = 0; \
else { \
	if (_x > _srcw - _dstw) \
		_x = (_srcw - _dstw); \
} \
\
if (_y < 0) \
	_y = 0; \
else { \
	if (_y > _srch - _dsth) \
		_y = _srch - _dsth; \
} \
\
_src_index = _y * _srcw + _x; \
_dst_index = 0; \
_i = _dsth; \
while (_i-- > 0) { \
	(void) memcpy(&(_dst[_dst_index]), &(_src[_src_index]), (int)(_dstw)); \
	_dst_index += _dstw; \
	_src_index += _srcw; \
} \
}
