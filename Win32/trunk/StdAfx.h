// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _WIN32_IE 0x0400

#include "windows.h"

#include <vector>
#include <memory>
#include <map>
using namespace std;

// WSQ support
#include "iwswsq.h"
// JPEG2000 lib
#include "iwsjasper.h"
// JPEG support
#include "iwsjpeg.h"
// TIFF support for Group 4 Fax
#include "iwstiff.h"
// UTF8 from SourceForge
#include "utf8.h"

#include "StdString.h"		// smart CStdString type used throughout
#include "StringSlots.h"	// Self-destructing string allocation to rsepect string-returning interface

/*
For future use: XML support

#include "ITL-2007f-Package.hxx"
#include "fbi_ebts_beta_1.0.0.hxx"

using namespace standard_22008;
using namespace xercesc;
using namespace std;

#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xsd/cxx/xml/string.hxx>
#include <xsd/cxx/xml/dom/auto-ptr.hxx>
#include <xsd/cxx/xml/dom/serialization-source.hxx>
#include <xsd/cxx/xml/dom/bits/error-handler-proxy.hxx>
#include <xsd/cxx/tree/exceptions.hxx>
#include <xsd/cxx/tree/error-handler.hxx>
*/