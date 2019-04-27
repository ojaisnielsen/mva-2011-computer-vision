#include "StdAfx.h"
#include "ErcTree.h"

using namespace ercf;

ErcTree::ErcTree(void)
{	
	_leftChild = NULL;
	_rightChild = NULL;
	_parent = NULL;
	_leaves = new vector<ErcTree *>();
	_featureIndexGen = NULL;
	leaf();
}

ErcTree::ErcTree(const TiXmlElement *xmlElement, ErcTree *parent)
{
	assign(xmlElement, parent);
}

void ErcTree::assign(const TiXmlElement *xmlElement, ErcTree *parent)
{
	_leftChild = NULL;
	_rightChild = NULL;
	_parent = parent;
	_leaves = (parent == NULL) ? new vector<ErcTree *>() : parent->getLeaves();
	leaf();
	if (xmlElement->NoChildren())
	{
		int leafIndex;
		xmlElement->QueryIntAttribute("index", &leafIndex);
		_leafIndex = (unsigned int)leafIndex;
		int isUnmixed;
		xmlElement->QueryIntAttribute("unmixed", &isUnmixed);
		_isUnmixed = (bool)isUnmixed;
		if (_isUnmixed)
		{
			int unmixedLabel;
			xmlElement->QueryIntAttribute("label", &unmixedLabel);
			_unmixedLabel = (unsigned int)unmixedLabel;
		}
	}
	else
	{
		int testFeatureIndex;
		xmlElement->QueryIntAttribute("testIndex", &testFeatureIndex);
		_testFeatureIndex = (unsigned int)testFeatureIndex;
		xmlElement->QueryDoubleAttribute("testThreshold", &_testThreshold);
		xmlElement->QueryDoubleAttribute("score", &_score);

		_leftChild = new ErcTree(xmlElement->FirstChild()->ToElement(), this);
		_rightChild = new ErcTree(xmlElement->LastChild()->ToElement(), this);	

		_isLeaf = false;
	}

	if (isRoot()) computeGlobalProperties();
}

ErcTree::ErcTree(const ErcTree &tree)
{
	_leftChild = NULL;
	_rightChild = NULL;
	_parent = NULL;
	_leaves = new vector<ErcTree *>();
	leaf();
}

ErcTree::ErcTree(ErcTree &tree, bool asChild)
{
	_featureIndexGen = tree._featureIndexGen;
	_leftChild = NULL;
	_rightChild = NULL;
	_parent = NULL;
	leaf();
	if (asChild)
	{
		_parent = &tree;
		_leaves = tree._leaves;
	}
}

ErcTree::ErcTree(const RandomInt *featureIndexGen, ErcTree *parent): _featureIndexGen(featureIndexGen), _parent(parent), _leaves(parent->getLeaves())
{
	_leftChild = NULL;
	_rightChild = NULL;
	leaf();
}

ErcTree::~ErcTree(void)
{
	leaf();
	if (isRoot()) delete _leaves;
}

void ErcTree::assign(const RandomInt *featureIndexGen, ErcTree *parent)
{
	_featureIndexGen = featureIndexGen;
	_parent = parent;
	_leaves = (parent == NULL) ? new vector<ErcTree *>() : parent->getLeaves();
	_leftChild = NULL;
	_rightChild = NULL;
	leaf();
}

void ErcTree::leaf(void)
{
	if (_leftChild != NULL)
	{
		_leftChild->leaf();
		free(_leftChild);
		_leftChild = NULL;
	}
	if (_rightChild != NULL)
	{
		_rightChild->leaf();	
		free(_rightChild);
		_rightChild = NULL;
	}
	_weakestFinalNode  = NULL;
	_nLeaves = 1;
	_score = 0.;
	_testFeatureIndex = 0;
	_testThreshold = 0.;
	_isLeaf = true;
}

double ErcTree::getPartitionScore(double entropy1, double entropy2, double jointEntropy)
{
	return (entropy1 + entropy2 == 0.) ? 0. : (2 * (1 - (jointEntropy / (entropy1 + entropy2))));
}

void ErcTree::train(TrainingSet &set, double sMin, unsigned int tMax)
{
	_isUnmixed = (!isRoot() && (_parent->isUnmixed())) ? true : set.isUnmixed();
	if (_isUnmixed) _unmixedLabel = (!isRoot() && (_parent->isUnmixed())) ? _parent->getUnmixedLabel() : set.getPointLabel(0);

	if (verbose) cout << "Training tree with " << set.getNPoints() << " points... ";
	leaf();

	if (set.isIndivisible()) 
	{
		if (verbose) cout << "tree is a leaf." << endl;
		return;
	}	

	TrainingSet set1(set);
	TrainingSet set2(set);
	double entropy = set.getLabelEntropy();
	unsigned int testFeatureIndex;
	double testThreshold;

	
	unsigned int t = 0;
	do
	{
		double score;
		testFeatureIndex = _featureIndexGen->operator()();
		double testFeatureMin = set.getMinFeature(testFeatureIndex);
		double testFeatureMax = set.getMaxFeature(testFeatureIndex);
		testThreshold = testFeatureMin + RandomDouble::Default() * (testFeatureMax - testFeatureMin);

		set.partition(testFeatureIndex, testThreshold, set1, set2);

		score = getPartitionScore(entropy, TrainingSet::getPartitionEntropy(set1, set2), TrainingSet::getLabelPartitionJointEntropy(set1, set2));

		if (score >=_score || t == 0)
		{
			_score = score;
			_testFeatureIndex = testFeatureIndex;
			_testThreshold = testThreshold;
		}
	} while (_score < sMin && ++t <= tMax);

	if (set1.getNPoints() == 0 || set2.getNPoints() == 0)
	{
		leaf();
		if (verbose) cout << "tree is a leaf." << endl;
	}
	else
	{
		if (verbose)
		{
			cout << "score: " << _score << endl;
			cout << "Creating children trees..." << endl;
		}


		_leftChild = new ErcTree(*this, true);
		_rightChild = new ErcTree(*this, true);

		_isLeaf = false;

		set.partition(_testFeatureIndex, _testThreshold, set1, set2);

		_leftChild->train(set1, sMin, tMax);

		_rightChild->train(set2, sMin, tMax);
	
		_isUnmixed = set.isUnmixed();
	}
	if (isRoot()) computeGlobalProperties();
}

void ErcTree::computeGlobalProperties(bool fromRoot)
{
	if (isRoot())
	{
		_leaves->clear();
	}
	if (fromRoot && !isRoot())
	{
		_parent->computeGlobalProperties(true);
		return;
	}
	if (isLeaf())
	{
		_leafIndex = _leaves->size();
		_leaves->push_back(this);
		return;
	}

	_leftChild->computeGlobalProperties();
	_rightChild->computeGlobalProperties();
	if (isFinal())
	{
		_weakestFinalNode = this;
	}
	else if (_leftChild->isLeaf())
	{		
		_weakestFinalNode = _rightChild->getWeakestFinalNode();
	}
	else if (_rightChild->isLeaf())
	{		
		_weakestFinalNode = _leftChild->getWeakestFinalNode();
	}
	else 
	{
		double leftWeakestScore = _leftChild->getWeakestFinalNode()->getScore();
		double rightWeakestScore = _rightChild->getWeakestFinalNode()->getScore();
		_weakestFinalNode = (leftWeakestScore < rightWeakestScore) ? _leftChild->getWeakestFinalNode() : _rightChild->getWeakestFinalNode();	
	}
	_nLeaves = _rightChild->getNLeaves() + _leftChild->getNLeaves();
}

bool ErcTree::isLeaf(void) const 
{
	return _isLeaf;
}

bool ErcTree::isFinal(void) const 
{
	if (_leftChild == NULL || _rightChild == NULL) return false;
	return _leftChild->isLeaf() && _rightChild->isLeaf();
}

bool ErcTree::isRoot(void) const 
{
	return _parent == NULL;
}

ErcTree *ErcTree::getWeakestFinalNode(void) 
{
	return _weakestFinalNode;
}

unsigned int ErcTree::getNLeaves(void) const 
{
	return _nLeaves;
}

void ErcTree::prune(unsigned int maxNLeaves)
{
	if (_nLeaves <= maxNLeaves) return;
	while (_nLeaves > maxNLeaves)
	{
		ErcTree *parent = _weakestFinalNode->getParent();
		parent->removeChild(_weakestFinalNode);
	}
}

void ErcTree::removeChild(ErcTree *child)
{
	if (child == _leftChild)
	{
		_leftChild->leaf();
		computeGlobalProperties(true);
	}
	else if (child == _rightChild)
	{
		_rightChild->leaf();
		computeGlobalProperties(true);
	}
}

double ErcTree::getScore(void) const 
{
	return _score;
}

ErcTree *ErcTree::getParent(void)
{
	return _parent;
}

unsigned int ErcTree::getLeafIndex(void) const 
{
	return _leafIndex;
}

const ErcTree *ErcTree::test(const CImg<double> &feature) const
{
	if (isLeaf()) return this;
	if (feature.atX(_testFeatureIndex) < _testThreshold) return _leftChild->test(feature);
	return _rightChild->test(feature);
}

string ErcTree::xml(void) const 
{
	stringstream output;

	if (isLeaf())
	{
		output << "<leaf index=\"" << _leafIndex << "\" unmixed=\"" << isUnmixed() << "\"";
		if (isUnmixed()) output << " label=\"" << getUnmixedLabel() << "\"";
		output << "/>";
	}
	else 
	{
		output << "<node score=\"" << _score << "\" testIndex=\"" << _testFeatureIndex << "\" testThreshold=\"" << _testThreshold << "\">";
		output << _leftChild->xml() << _rightChild->xml() << "</node>";
	}
	return output.str();
}

bool ErcTree::isUnmixed(void) const 
{
	return _isUnmixed;
}

unsigned int ErcTree::getUnmixedLabel(void) const 
{
	return _unmixedLabel;
}

vector<ErcTree *> *ErcTree::getLeaves(void) const
{
	return _leaves;
}
