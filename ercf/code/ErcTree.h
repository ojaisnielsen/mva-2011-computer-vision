#pragma once

#include "StdAfx.h"
#include "tools.h"
#include "TrainingSet.h"

namespace ercf
{

	class ErcTree
	{
	public:
		ErcTree(void);
		ErcTree(const ErcTree &tree);
		ErcTree(const TiXmlElement *xmlElement, ErcTree *parent = NULL);
		ErcTree(ErcTree &tree, bool asChild);
		ErcTree(const RandomInt *featureIndexGen,  ErcTree *parent = NULL);
		~ErcTree(void);
		void assign(const RandomInt *featureIndexGen,  ErcTree *parent = NULL);
		void assign(const TiXmlElement *xmlElement, ErcTree *parent = NULL);
		void leaf(void);
		void train(TrainingSet &set, double sMin, unsigned int tMax);
		void prune(unsigned int maxNLeaves);
		ErcTree *getWeakestFinalNode(void);
		double getScore(void) const;
		bool isFinal(void) const;
		bool isLeaf(void) const;
		bool isRoot(void) const;
		unsigned int getNLeaves(void) const;
		void removeChild(ErcTree *child);		
		ErcTree *getParent(void);
		unsigned int getIndex(void) const;
		const ErcTree *test(const CImg<double> &feature) const;
		static double getPartitionScore(double entropy1, double entropy2, double jointEntropy);
		string xml(void) const;
		bool isUnmixed(void) const;
		unsigned int getUnmixedLabel(void) const;
		void computeGlobalProperties(bool fromRoot = false);
		vector<ErcTree *> *getLeaves(void) const;
		unsigned int getLeafIndex(void) const;
		bool verbose;


	private:		
		bool _isLeaf;
		unsigned int _testFeatureIndex;
		double _testThreshold;
		ErcTree *_leftChild;
		ErcTree *_rightChild;
		ErcTree *_parent;
		ErcTree *_weakestFinalNode;
		const RandomInt *_featureIndexGen;
		double _score;
		unsigned int _nLeaves;
		bool _isUnmixed;
		unsigned int _unmixedLabel;
		vector<ErcTree *> *_leaves;
		unsigned int _leafIndex;
	};

}

