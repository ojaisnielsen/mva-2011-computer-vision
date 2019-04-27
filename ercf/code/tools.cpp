#include "stdafx.h"
#include "tools.h"

vector<string> getFileNames(const string &query)
{
	WIN32_FIND_DATA findData;
	HANDLE findHandle;

	char fullQuery[MAX_PATH];
	_fullpath(fullQuery, query.c_str(), MAX_PATH);

	char drive[3];
	char dir[MAX_PATH];
	_splitpath_s(fullQuery, drive, 3, dir, MAX_PATH, NULL, 0, NULL, 0); 

	findHandle = FindFirstFile(fullQuery, &findData);
	vector<string> fileNames;
	do
	{
		fileNames.push_back(string(drive) + string(dir) + string(findData.cFileName));
	}
	while (FindNextFile(findHandle, &findData) != 0);

	FindClose(findHandle);
	return fileNames;
}

template<>
void loadImages<bool>(CImgList<bool> &imList, const vector<string> &fileNames)
{	
	unsigned int nFiles = fileNames.size();
	imList.assign(nFiles);
	for (unsigned int i = 0; i < nFiles; ++i)
	{
		CImg<double> im(fileNames[i].c_str());
#pragma push_macro("round")
#undef round
		im.channel(0).normalize(0., 1.).round();
#pragma pop_macro("round")
		im *= -1.;
		im += 1.;
		imList[i].assign(CImg<bool>(im));
	}

}

Plot::Plot(void)
{
	_color[0] = 255;
	_color[1] = 0;
	_color[2] = 0;
	_nPoints = 0;
}

Plot::Plot(const CImg<double> &image) : _image(image.get_HSLtoRGB())
{
	_color[0] = 255;
	_color[1] = 0;
	_color[2] = 0;
	_nPoints = 0;
}

void Plot::assign(const CImg<double> &image)
{
	if (image.spectrum() == 3) _image = image.get_HSLtoRGB();
	else 
	{
		_image = image;
		_image.append(image, 'c');
		_image.append(image, 'c');
		_image.normalize(0., 255.);
	}
}

void Plot::operator()(unsigned int x, unsigned int y)
{
	_image.draw_circle(x, y, 2, _color);
}

void Plot::operator()(unsigned int x, unsigned int y, unsigned int radX, unsigned int radY)
{
	_image.draw_rectangle(x - radX, y - radY, x + radX, y + radY, _color, 0.5f);
}

void Plot::operator()(void) const
{
	_image.display();
}

Timer::Timer(void) : _start(clock()), _elapsed(0.)
{
}

double Timer::last(void) const
{
	return _elapsed;
}

void Timer::begin(void)
{
	_start = clock();
	_elapsed = 0.;
}

double Timer::end(void)
{
	_elapsed = (double)clock() - _start;
	_elapsed /= CLOCKS_PER_SEC;
	return _elapsed;
}
