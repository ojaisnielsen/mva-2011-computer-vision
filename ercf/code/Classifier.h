#pragma once
#include "stdafx.h"
#include "ErcForest.h"
#include "tools.h"

namespace ercf
{
	class Classifier
	{
	public:
		Classifier(const ErcForest *forest);
		void train(const TrainingSet &set, const vector<unsigned int> &nDescriptorsPerImage);
		unsigned int unmixedPoints(const CImg<double> &image, const CImg<double> &features, const CImg<double> &positions, unsigned int label) const;
		double classify(const CImg<double> &features, unsigned int label) const;
		static void normalize(CImg<double> &histogram);
		void save(string binFile) const;
		void load(string binFile);
		unsigned int getNModels(void) const;

	private:
		const ErcForest *_forest;
		VlRand _random;
		CImg<double> _models;
	};
}


