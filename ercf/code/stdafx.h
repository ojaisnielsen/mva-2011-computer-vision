// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <sstream>
#include <fstream>
#include <tchar.h>
#include <random>
#include <time.h>

#define cimg_use_openmp

#ifdef _DEBUG
	#define cimg_verbosity 1
#else
	#define cimg_verbosity 0
#endif

#include <CImg.h>


extern "C" 
{
#include <vl/generic.h>
#include <vl/imopv.h>
#include <vl/mathop.h>
#include <vl/sift.h>
#include <vl/pegasos.h>
#include <vl/homkermap.h>
}

#include <tinyxml.h>

#define round(x) ((int)((x) + 0.5))
#define xLogX(x) ((x) > 0 ? ((x) * log(x)) : 0.)
#define square(x) ((x) * (x))
#define realEpsilon 0.//1e-10
#define realEqual(x, y) (((((x) - (y)) <= realEpsilon) && (((x) - (y)) >= -realEpsilon)) ? true : false)
#define packXY(x, y, w) ((x) + (w) * (y))
#define unpackX(xy, w) ((xy) % (w))
#define unpackY(xy, w) ((xy) / (w))

using namespace cimg_library;
using namespace std;


