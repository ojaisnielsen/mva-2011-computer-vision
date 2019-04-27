#pragma once
#include "stdafx.h"
#include "tools.h"

namespace ercf
{
	class FeatureExtractor
	{
	public:
		FeatureExtractor(CImgList<double> *featureList, unsigned int *nDescriptorsPerImage, unsigned int maxNfeatures, CImgList<double> *images, CImgList<bool> *masks = NULL);
		FeatureExtractor(CImgList<double> *featureList, unsigned int *nDescriptorsPerImage, vector<unsigned int> *labels, unsigned int maxNfeatures, CImgList<double> *images, CImgList<bool> *masks = NULL);
		FeatureExtractor(CImgList<double> *featureList, unsigned int *nDescriptorsPerImage, vector<unsigned int> *labels, CImg<double> *positions, unsigned int maxNfeatures, CImgList<double> *images, CImgList<bool> *masks = NULL);
		FeatureExtractor(CImgList<double> *featureList, unsigned int *nDescriptorsPerImage, CImg<double> *positions, unsigned int maxNfeatures, CImgList<double> *images, CImgList<bool> *masks = NULL);
		unsigned int getHsl(unsigned int featureStartIndex, unsigned int imageIndex, unsigned int patchSize, unsigned int label = 0);
		unsigned int getHslHaar(unsigned int featureStartIndex, unsigned int imageIndex, unsigned int patchSize, unsigned int label = 0);
		unsigned int getSift(unsigned int featureStartIndex, unsigned int imageIndex, unsigned int label = 0);
		unsigned int getMultipleHsl(unsigned int featureStartIndex, unsigned int imageFirstIndex, unsigned int nImages, unsigned int patchSize, unsigned int label = 0);
		unsigned int getMultipleHslHaar(unsigned int featureStartIndex, unsigned int imageFirstIndex, unsigned int nImages, unsigned int patchSize, unsigned int label = 0);
		unsigned int getMultipleSift(unsigned int featureStartIndex, unsigned int imageFirstIndex, unsigned int nImages, unsigned int label = 0);	
		bool getRandomPoint(unsigned int &x, unsigned int &y, unsigned int imageIndex, unsigned int patchSize = 0);
		bool useMasks(void) const;
		bool usePositions(void) const;
		bool useLabels(void) const;
		void setDisplay(bool display);

	private:
		void _init(void);		
		void _computeMaskIndices(unsigned int imageIndex);
		unsigned int _maxNFeatures;
		NullableVector<AssociativeSortedList<unsigned int, unsigned int>> _sortedMaskPositions;
		CImgList<double> *_images;
		CImgList<double> *_featureList;
		vector<unsigned int> *_labels;
		CImgList<bool> *_masks;	  
		CImg<double> *_positions;
		bool _display;
		unsigned int *_nDescriptorsPerImage;
	};
}
