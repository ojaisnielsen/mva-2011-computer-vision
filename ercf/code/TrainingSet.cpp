#include "StdAfx.h"
#include "TrainingSet.h"
#include "tools.h"

using namespace ercf;

TrainingSet::TrainingSet(void)
{
}

TrainingSet::TrainingSet(CImg<double> *features, vector<unsigned int> *labels, unsigned int nLabels): _features(features), _labels(labels), _nLabels(nLabels)
{
	_nPoints = _features->width();
	_indices.assign(_nPoints, 0);
	_maxFeatures.assign(_features->height(), 0.);
	_minFeatures.assign(_features->height(), 0.);
	
	for (int i = 0; i < _nPoints; ++i)
	{
		_indices[i] = i;
	}
	computeLabelOccurences();
}

void TrainingSet::computeLabelOccurences(void)
{
	_labelOccurences.assign(_nLabels, 0);
	for (unsigned int i = 0; i < getNPoints(); ++i)
	{		
		++(_labelOccurences[getPointLabel(i)]);
	}
}


TrainingSet::TrainingSet(CImg<double> *features, vector<unsigned int> *labels, unsigned int nLabels, vector<unsigned int> &indices, unsigned int nPoints): _features(features), _labels(labels), _nLabels(nLabels), _indices(indices), _nPoints(nPoints)
{
	_maxFeatures.assign(_features->height(), 0.);
	_minFeatures.assign(_features->height(), 0.);
	computeLabelOccurences();
}

TrainingSet::TrainingSet(const TrainingSet &set): _features(set._features), _labels(set._labels), _nLabels(set._nLabels), _nPoints(0)
{
	_maxFeatures.assign(_features->height(), 0.);
	_minFeatures.assign(_features->height(), 0.);
}

TrainingSet &TrainingSet::operator=(const TrainingSet &set)
{
	_features = set._features;
	_labels = set._labels;
	_nLabels = set._nLabels;
	_nPoints = 0;
	_maxFeatures.assign(_features->height(), 0.);
	_minFeatures.assign(_features->height(), 0.);
	return *this;
}

TrainingSet::~TrainingSet(void)
{
}

unsigned int TrainingSet::getFeatureDim() const
{
	return _features->height();
}

double TrainingSet::getMinFeature(unsigned int index)
{
	if (_minFeatures.isNull(index))
	{
		double m = getPointFeature(0, index);
		for (unsigned int p = 1; p < getNPoints(); ++p)
		{
			m = min(m, getPointFeature(p, index));
		}
		_minFeatures.set(index, m);
	}
	return _minFeatures[index];
}

double TrainingSet::getMaxFeature(unsigned int index)
{
	if (_maxFeatures.isNull(index))
	{
		double m = getPointFeature(0, index);
		for (unsigned int p = 1; p < getNPoints(); ++p)
		{
			m = max(m, getPointFeature(p, index));
		}
		_maxFeatures.set(index, m);
	}
	return _maxFeatures[index];
}

unsigned int TrainingSet::getLabelOccurences(unsigned int label) const
{
	return _labelOccurences[label];
}

double TrainingSet::getLabelEntropy(void) const
{
	double entropy = 0.;
	for (unsigned int i = 0; i < _nLabels; ++i)
	{
		double p = getLabelOccurences(i) / (double)getNPoints();
		entropy += -xLogX(p);
	}
	return entropy;
}

double TrainingSet::getPartitionEntropy(const TrainingSet &set1, const TrainingSet &set2)
{
	double p1 = set1.getNPoints() / (double)(set1.getNPoints() + set2.getNPoints());
	double entropy = -xLogX(p1) - xLogX(1. - p1);
	return entropy;
}


double TrainingSet::getLabelPartitionJointEntropy(const TrainingSet &set1, const TrainingSet &set2)
{
	CImg<unsigned int> labelSetOccurences(set1.getNLabels(), 2);
	labelSetOccurences.fill(0);
	for (unsigned int i = 0; i < set1.getNPoints(); ++i)
	{		
		++labelSetOccurences(set1.getPointLabel(i), 0);
	}

	for (unsigned int i = 0; i < set2.getNPoints(); ++i)
	{		
		++labelSetOccurences(set2.getPointLabel(i), 1);
	}

	double entropy = 0.;
	for (unsigned int l = 0; l < set1.getNLabels(); ++l)
		for (unsigned int s = 0; s < 2; ++s)
		{
			double p = labelSetOccurences(l, s) / (double)(set1.getNPoints() + set2.getNPoints());
			entropy += -xLogX(p);
		}

	return entropy;
}

unsigned int TrainingSet::getPointLabel(unsigned int index) const
{
	return _labels->at(_indices.at(index));
}

double TrainingSet::getPointFeature(unsigned int pointIndex, unsigned int featureIndex) const
{
	return _features->operator()(_indices[pointIndex], featureIndex);
}

unsigned int TrainingSet::getNLabels() const
{
	return _nLabels;
}

unsigned int TrainingSet::getNPoints() const
{
	return _nPoints;
}

void TrainingSet::addPointIndex(unsigned int index)
{
	_indices[_nPoints++] = index;
}

void TrainingSet::partition(unsigned int testFeatureIndex, double testThreshold, TrainingSet &set1, TrainingSet &set2) const
{
	set1.flushIndices(getNPoints());
	set2.flushIndices(getNPoints());

	for (int i = 0; i < getNPoints(); ++i)
	{
		if (getPointFeature(i, testFeatureIndex) < testThreshold)
		{
			set1.addPointIndex(_indices[i]);
		}
		else
		{
			set2.addPointIndex(_indices[i]);
		}
	}
	set1.computeLabelOccurences();
	set2.computeLabelOccurences();
}


bool TrainingSet::isIndivisible(void) const
{
	if (isUnmixed()) return true;
	for (unsigned int i = 0; i < _nLabels; ++i)
	{
		double val = getPointFeature(0, i);
		for (unsigned int j = 0; j < getNPoints(); ++j)
		{
			if (!realEqual(getPointFeature(j, i), val)) return false;
		}
	}
	return true;
}

void TrainingSet::flushIndices(unsigned int newMaxSize)
{
	_nPoints = 0;
	_indices.assign(newMaxSize, 0);
	_labelOccurences.clear();
}

bool TrainingSet::isUnmixed(void) const
{
	int emptyCount = 0;
	for (unsigned int i = 0; i < _nLabels; ++i)
	{
		emptyCount += (unsigned int)(getLabelOccurences(i) == 0);
	}
	return emptyCount == _nLabels - 1;
}

CImg<double> TrainingSet::getPointFeature(unsigned int pointIndex) const
{
	return _features->get_column(_indices[pointIndex]);
}