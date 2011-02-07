#include "Config.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <wchar.h>
#include <string.h>
#include <limits.h>
#include <vector>
#include <memory>
#include <map>
using namespace std;
#include "StdString.h"			// Smart CStdString type used throughout
#include "StringSlots.h"		// Self-destructing string allocation to respect string-returning interface
#include "StringResources.h"	// User-visible string table
#include "FreeImage.h"
#include "FreeImageHelpers.h"

#ifdef WIN32
#include <mbstring.h>
#else
#include <libgen.h>	// dirname()
#endif
