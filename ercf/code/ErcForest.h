#pragma once

#include "StdAfx.h"
#include "ErcTree.h"
#include "tools.h"


namespace ercf
{

	class ErcForest
	{
	private:
		vector<ErcTree> _trees;
		RandomInt _featureIndexGen;

	public:
		ErcForest::ErcForest(string xmlFile);
		ErcForest(unsigned int size);
		~ErcForest(void);

		unsigned int getNLeaves(void) const;
		void train(TrainingSet &set, double sMin, unsigned int tMax);
		void classify(double *histogram, const CImg<double> &feature) const;
		bool isUnmixed(const CImg<double> &feature, unsigned int unmixedLabel) const;
		void prune(unsigned int maxNLeaves);
		string xml(void) const;
		void save(string xmlFile) const;
		bool verbose;
	};

}

