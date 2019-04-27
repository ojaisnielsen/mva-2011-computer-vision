#include "stdafx.h"
#include "FeatureExtractor.h"


using namespace ercf;

FeatureExtractor::FeatureExtractor(CImgList<double> *featureList, unsigned int *nDescriptorsPerImage, unsigned int maxNfeatures, CImgList<double> *images, CImgList<bool> *masks) 
	: _images(images), _featureList(featureList), _maxNFeatures(maxNfeatures), _masks(masks), _positions(NULL), _labels(NULL), _display(false),  _nDescriptorsPerImage(nDescriptorsPerImage)
{
	_init();	
}

FeatureExtractor::FeatureExtractor(CImgList<double> *featureList, unsigned int *nDescriptorsPerImage, vector<unsigned int> *labels, unsigned int maxNfeatures, CImgList<double> *images, CImgList<bool> *masks)
	: _images(images), _featureList(featureList), _maxNFeatures(maxNfeatures), _masks(masks), _positions(NULL), _labels(labels), _display(false),  _nDescriptorsPerImage(nDescriptorsPerImage)
{
	_init();
}

FeatureExtractor::FeatureExtractor(CImgList<double> *featureList, unsigned int *nDescriptorsPerImage, vector<unsigned int> *labels, CImg<double> *positions, unsigned int maxNfeatures, CImgList<double> *images, CImgList<bool> *masks)
	: _images(images), _featureList(featureList), _maxNFeatures(maxNfeatures), _masks(masks), _positions(positions), _labels(labels), _display(false),  _nDescriptorsPerImage(nDescriptorsPerImage)
{
	_init();
}

FeatureExtractor::FeatureExtractor(CImgList<double> *featureList, unsigned int *nDescriptorsPerImage, CImg<double> *positions, unsigned int maxNfeatures, CImgList<double> *images, CImgList<bool> *masks)
	: _images(images), _featureList(featureList), _maxNFeatures(maxNfeatures), _masks(masks), _positions(positions), _labels(NULL), _display(false),  _nDescriptorsPerImage(nDescriptorsPerImage)
{
	_init();
}

void FeatureExtractor::_init(void)
{
	if (_masks != NULL) 
	{
		_sortedMaskPositions.assign(_images->size(), AssociativeSortedList<unsigned int, unsigned int>());
	}
}

bool FeatureExtractor::useMasks(void) const
{
	return _masks != NULL;
}

bool FeatureExtractor::usePositions(void) const
{
	return _positions != NULL;
}

bool FeatureExtractor::useLabels(void) const
{
	return _labels != NULL;
}

void FeatureExtractor::setDisplay(bool display)
{
	_display = display;
}

void FeatureExtractor::_computeMaskIndices(unsigned int imageIndex)
{
	if (!_sortedMaskPositions.isNull(imageIndex)) return;
	_sortedMaskPositions.set(imageIndex);
	_sortedMaskPositions[imageIndex].assign(_masks->at(imageIndex).sum());
	unsigned int w = _images->at(imageIndex).width();
	unsigned int h = _images->at(imageIndex).height();
	for (unsigned int x = 0; x < w; ++x)
		for (unsigned int y = 0; y <h; ++y)
		{
			if (_masks->operator()(imageIndex, x, y))
			{			
				unsigned int dist = min(w - x, h - y);
				_sortedMaskPositions[imageIndex].insert(dist, packXY(x, y, w));
			}
		}
}

bool FeatureExtractor::getRandomPoint(unsigned int &x, unsigned int &y, unsigned int imageIndex, unsigned int patchSize)
{
	double r = RandomDouble::Default();

	if (useMasks())
	{		
		_computeMaskIndices(imageIndex);
		if (_sortedMaskPositions[imageIndex].getMaxKey() < patchSize) return false;

		unsigned int firstIndex = _sortedMaskPositions[imageIndex].lowerBound(patchSize);
		unsigned int n = firstIndex + round(r * (_sortedMaskPositions[imageIndex].size() - firstIndex - 1));
		unsigned int xy = _sortedMaskPositions[imageIndex][n];
		x = unpackX(xy, _images->at(imageIndex).width());
		y = unpackY(xy, _images->at(imageIndex).width());
	}
	else
	{
		unsigned int w = _images->at(imageIndex).width() - patchSize;
		unsigned int h = _images->at(imageIndex).height() - patchSize;
		if (w <= 0 || h <= 0) return false;

		unsigned int xy = round(r * (w * h - 1));
		x = unpackX(xy, w);
		y = unpackY(xy, w);
	}
	return true;
}

unsigned int FeatureExtractor::getHsl(unsigned int featureStartIndex, unsigned int imageIndex, unsigned int patchSize, unsigned int label)
{
	unsigned int x, y;	
	x = y = 0;
	Plot plot;
	if (_display) plot.assign(_images->at(imageIndex));

	for (unsigned int i = 0; i < _maxNFeatures; ++i)
	{
		double r = RandomDouble::Default();
		double scale = 0.5 + 0.5 * r;
		unsigned int scaledPatchSize = (unsigned int)(patchSize / scale);
		getRandomPoint(x, y, imageIndex, scaledPatchSize);

		if (_display) plot(x, y, scaledPatchSize, scaledPatchSize);

		_featureList->at(featureStartIndex + i).assign(_images->at(imageIndex).get_crop(x, y, x + patchSize - 1, y + patchSize - 1));
		_featureList->at(featureStartIndex + i).resize(patchSize, patchSize);
			
		_featureList->at(featureStartIndex + i).vector();
		if (useLabels()) _labels->at(featureStartIndex + i) = label;
		if (usePositions())
		{
			_positions->operator()(featureStartIndex + i, 0) = x + scaledPatchSize / 2;
			_positions->operator()(featureStartIndex + i, 1) = y + scaledPatchSize / 2;
		}
	}
	if (_display) plot();
	_nDescriptorsPerImage[imageIndex] = _maxNFeatures;
	return _maxNFeatures;
}

unsigned int FeatureExtractor::getHslHaar(unsigned int featureStartIndex, unsigned int imageIndex, unsigned int patchSize, unsigned int label)
{
	unsigned int x, y;	
	x = y = 0;
	Plot plot;
	if (_display) plot.assign(_images->at(imageIndex));

	for (unsigned int i = 0; i < _maxNFeatures; ++i)
	{
		double r = RandomDouble::Default();
		double scale = 0.25 + 0.75 * r;
		unsigned int scaledPatchSize = (unsigned int)(patchSize / scale);
		getRandomPoint(x, y, imageIndex, scaledPatchSize);

		if (_display) plot(x, y, scaledPatchSize, scaledPatchSize);

		_featureList->at(featureStartIndex + i).assign(_images->at(imageIndex).get_crop(x, y, x + patchSize - 1, y + patchSize - 1));
		_featureList->at(featureStartIndex + i).resize(patchSize, patchSize);
			
		_featureList->at(featureStartIndex + i).haar().vector();
		if (useLabels()) _labels->at(featureStartIndex + i) = label;
		if (usePositions())
		{
			_positions->operator()(featureStartIndex + i, 0) = x + scaledPatchSize / 2;
			_positions->operator()(featureStartIndex + i, 1) = y + scaledPatchSize / 2;
		}
	}
	if (_display) plot();
	_nDescriptorsPerImage[imageIndex] = _maxNFeatures;
	return _maxNFeatures;
}



unsigned int FeatureExtractor::getSift(unsigned int featureStartIndex, unsigned int imageIndex, unsigned int label)
{

	CImg<float> im(_images->at(imageIndex).get_channel(2));
	if (useMasks() && !usePositions()) im.mul(_masks->at(imageIndex)).autocrop(0.F);
	else if (useMasks()) im.mul(_masks->at(imageIndex));

	Plot plot;
	if (_display) plot.assign(CImg<double>(im));

	VlSiftFilt *siftDetector = vl_sift_new(im.width(), im.height(), -1, 3, 0);
	vector<VlSiftKeypoint> points;
	vector<double> orientations;
	vector<array<vl_sift_pix, 128>> descriptors;

	unsigned int count = 0;
		
	vl_sift_process_first_octave(siftDetector, im.data());		
	do
	{
		vl_sift_detect(siftDetector);
		const VlSiftKeypoint *keypoints = vl_sift_get_keypoints(siftDetector);
		for (unsigned int k = 0; k < vl_sift_get_nkeypoints(siftDetector); ++k)
		{
			double angles[4];
			unsigned int nAngles = vl_sift_calc_keypoint_orientations(siftDetector, angles, keypoints + k);
			for (unsigned int o = 0; o < nAngles; ++o)
			{
				points.push_back(keypoints[k]);
				orientations.push_back(angles[o]);
				descriptors.push_back(array<vl_sift_pix, 128>());
				vl_sift_calc_keypoint_descriptor(siftDetector, descriptors.back().data(), keypoints + k, angles[o]);
			}
		}
	}
	while (vl_sift_process_next_octave(siftDetector) != VL_ERR_EOF);

	while (points.size() > _maxNFeatures)
	{
		unsigned int index = round((points.size() - 1) * RandomDouble::Default());
		points.erase(points.begin() + index);
		orientations.erase(orientations.begin() + index);
		descriptors.erase(descriptors.begin() + index);
	}

	for (unsigned int i = 0; i < points.size(); ++i)
	{		
		if (_display) plot(round(points[i].x), round(points[i].y));	
		_featureList->at(featureStartIndex + i).assign(CImg<vl_sift_pix>(descriptors[i].data(), 1, 128));

		if (useLabels()) _labels->at(featureStartIndex + i) = label;
		if (usePositions())
		{
			_positions->operator()(featureStartIndex + i, 0) = round(points[i].x);
			_positions->operator()(featureStartIndex + i, 1) = round(points[i].y);			
		}
	}

	vl_sift_delete(siftDetector);

	if (_display) plot();
	_nDescriptorsPerImage[imageIndex] = points.size();
	return points.size();
}

unsigned int FeatureExtractor::getMultipleHsl(unsigned int featureStartIndex, unsigned int imageFirstIndex, unsigned int nImages, unsigned int patchSize, unsigned int label)
{
	unsigned int nFeatures = 0;
	for (unsigned int i = imageFirstIndex; i < imageFirstIndex + nImages; ++i)
	{
		nFeatures += getHsl(featureStartIndex + nFeatures, i, patchSize, label);
	}
	return nFeatures;
}

unsigned int FeatureExtractor::getMultipleHslHaar(unsigned int featureStartIndex, unsigned int imageFirstIndex, unsigned int nImages, unsigned int patchSize, unsigned int label)
{
	unsigned int nFeatures = 0;
	for (unsigned int i = imageFirstIndex; i < imageFirstIndex + nImages; ++i)
	{
		nFeatures += getHslHaar(featureStartIndex + nFeatures, i, patchSize, label);
	}
	return nFeatures;
}

unsigned int FeatureExtractor::getMultipleSift(unsigned int featureStartIndex, unsigned int imageFirstIndex, unsigned int nImages, unsigned int label)
{
	unsigned int nFeatures = 0;
	for (unsigned int i = imageFirstIndex; i < imageFirstIndex + nImages; ++i)
	{
		nFeatures += getSift(featureStartIndex + nFeatures, i, label);
	}
	return nFeatures;
}