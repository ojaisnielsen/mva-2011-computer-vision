#pragma once

#include "stdafx.h"
#include "tools.h"

namespace ercf
{

	class TrainingSet
	{
	public:
		TrainingSet(void);
		TrainingSet(const TrainingSet &set);
		TrainingSet(CImg<double> *features, vector<unsigned int> *labels, unsigned int nLabels);
		TrainingSet(CImg<double> *features, vector<unsigned int> *labels, unsigned int nLabels, vector<unsigned int> &indices, unsigned int nPoints);
		~TrainingSet(void);
		TrainingSet &operator=(const TrainingSet &set);
		void getSubset(unsigned int featureIndex, double featureThreshold, TrainingSet &leftSet, TrainingSet &rightSet) const;		
		unsigned int getFeatureDim(void) const;
		unsigned int getNLabels(void) const;
		unsigned int getNPoints(void) const;
		unsigned int getPointLabel(unsigned int index) const;
		double getPointFeature(unsigned int pointIndex, unsigned int featureIndex) const;
		CImg<double> getPointFeature(unsigned int pointIndex) const;
		double getMinFeature(unsigned int index);
		double getMaxFeature(unsigned int index);
		double getLabelEntropy(void) const;
		static double getPartitionEntropy(const TrainingSet &set1, const TrainingSet &set2);
		static double getLabelPartitionJointEntropy(const TrainingSet &set1, const TrainingSet &set2);
		void partition(unsigned int testFeatureIndex, double testThreshold, TrainingSet &set1, TrainingSet &set2) const;
		void addPointIndex(unsigned int index);
		bool isIndivisible(void) const;
		void flushIndices(unsigned int newMaxSize);
		unsigned int getLabelOccurences(unsigned int label) const;
		bool isUnmixed(void) const;
		unsigned int getUnmixedLabel(void) const;
		void computeLabelOccurences(void);

	private:
		void _computeMinMaxFeatures(void);		
		CImg<double> *_features;
		vector<unsigned int> *_labels;
		vector<unsigned int> _indices;
		unsigned int _nPoints;
		NullableVector<double> _minFeatures;
		NullableVector<double> _maxFeatures;
		unsigned int _nLabels;
		vector<unsigned int> _labelOccurences;
		bool _isUnmixed;

	};

}

