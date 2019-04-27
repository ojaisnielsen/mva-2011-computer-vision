#include "StdAfx.h"
#include "ErcForest.h"

using namespace ercf;

ErcForest::ErcForest(unsigned int size)
{	
	_trees.assign(size, ErcTree());
	for (unsigned int i = 0; i < size; ++i)
	{
		_trees[i].assign(&_featureIndexGen);
	}
}

ErcForest::~ErcForest(void)
{
}

void ErcForest::train(TrainingSet &set, double sMin, unsigned int tMax)
{
	_featureIndexGen.assign(0, set.getFeatureDim() - 1);	
	for (unsigned int i = 0; i < _trees.size(); ++i)
	{
		_trees[i].train(set, sMin, tMax);
		_trees[i].verbose = verbose;
	}
	if (verbose)
	{
		for (unsigned int i = 0; i < _trees.size(); ++i)
		{
			cout << "Tree " << i << " trained: " << _trees[i].getNLeaves() << " leaves" << endl;
		}
	}
	
}

unsigned int ErcForest::getNLeaves(void) const
{
	unsigned int n = 0;
	for (unsigned int t = 0; t < _trees.size(); ++t)
	{
		n += _trees[t].getNLeaves();
	}
	return n;
}

void ErcForest::classify(double *histogram, const CImg<double> &feature) const
{
	unsigned int histOffset = 0;
	for (unsigned int t = 0; t < _trees.size(); ++t)		
	{
		histogram[histOffset + _trees[t].test(feature)->getLeafIndex()] += 1.;
		histOffset += _trees[t].getNLeaves();
	}
}

bool ErcForest::isUnmixed(const CImg<double> &feature, unsigned int unmixedLabel) const
{
	bool test = false;
	for (unsigned int i = 0; i < _trees.size(); ++i)
	{
		if(_trees[i].test(feature)->isUnmixed() && (_trees[i].test(feature)->getUnmixedLabel() == unmixedLabel)) return true;
		//test &= _trees[i].test(feature)->isUnmixed() && (_trees[i].test(feature)->getUnmixedLabel() == unmixedLabel);
	}
	return false;
	//return test;
}

void ErcForest::prune(unsigned int maxNLeaves)
{
	for (unsigned int i = 0; i < _trees.size(); ++i)
	{
		_trees[i].prune(maxNLeaves);
		_trees[i].verbose = verbose;
	}
	if (verbose)
	{
		for (unsigned int i = 0; i < _trees.size(); ++i)
		{
			cout << "Tree " << i << " pruned: " << _trees[i].getNLeaves() << " leaves" << endl;
		}
	}
}

string ErcForest::xml(void) const
{
	stringstream output;
	output << "<forest>";
	for (unsigned int i = 0; i < _trees.size(); ++i)
	{
		output << _trees[i].xml();
	}
	output << "</forest>";
	return output.str();
}

ErcForest::ErcForest(string xmlFile)
{
	TiXmlDocument doc(xmlFile.c_str());
	doc.LoadFile();

	TiXmlHandle docHandle(&doc);
	TiXmlElement *root = docHandle.FirstChildElement("forest").Element();
	unsigned int n = 0;	
	for(TiXmlElement *element = root->FirstChildElement(); element; element = element->NextSiblingElement())
	{		
		++n;
	}

	_trees.assign(n, ErcTree());
	unsigned int i = 0;
	for(TiXmlElement *element = root->FirstChildElement(); element; element = element->NextSiblingElement())
	{		
		_trees[i].assign(element);
		++i;
	}

	if (verbose)
	{
		for (unsigned int i = 0; i < _trees.size(); ++i)
		{
			cout << "Tree " << i << " read from XML: " << _trees[i].getNLeaves() << " leaves" << endl;
		}	
	}
}

void ErcForest::save(string xmlFile) const
{
	ofstream file;
	file.open(xmlFile.c_str(), ios::trunc);
	file << xml();
	file.close();
}