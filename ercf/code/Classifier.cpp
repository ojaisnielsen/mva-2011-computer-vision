#include "StdAfx.h"
#include "Classifier.h"

using namespace ercf;

Classifier::Classifier(const ErcForest *forest) 
	: _forest(forest)
{	
	vl_rand_init(&_random);
}

unsigned int Classifier::unmixedPoints(const CImg<double> &image, const CImg<double> &features, const CImg<double> &positions, unsigned int label) const
{

	Plot plot(image);
	unsigned int n = 0;
	for (unsigned int i = 0; i < features.width(); ++i)
	{
		if (_forest->isUnmixed(features.get_column(i), label))
		{
			plot(positions(i, 0), positions(i, 1));
			++n;
		}			
	}
	plot();
	return n;
}

void Classifier::train(const TrainingSet &set, const vector<unsigned int> &nDescriptorsPerImage)
{
	unsigned int nImages = nDescriptorsPerImage.size();
	_models.assign(_forest->getNLeaves() + 1, set.getNLabels());
	CImg<vl_int8> binaryLabels(nImages, set.getNLabels());

	CImg<double> histograms(_forest->getNLeaves(), nImages);
	histograms.fill(0.);
	unsigned int globalPoint = 0;
	for (unsigned int i = 0; i < nImages; ++i)
	{
		for (unsigned int p = 0; p < nDescriptorsPerImage[i]; ++p)
		{
			_forest->classify(histograms.data() + i * _forest->getNLeaves(), set.getPointFeature(globalPoint));
			++globalPoint;
		}
		for (unsigned int l = 0; l < set.getNLabels(); ++l)
		{
			binaryLabels(i, l) = (set.getPointLabel(globalPoint - 1) == l) ? 1 : -1;
		}
	}
	normalize(histograms);

	for (unsigned int l = 0; l < set.getNLabels(); ++l)
	{
		vl_pegasos_train_binary_svm_d(_models.data() + l * _models.width(), histograms.data(), _forest->getNLeaves(), nImages, binaryLabels.data() + l * binaryLabels.width(), 1., 1., 1, 100, &_random);
	}

}

double Classifier::classify(const CImg<double> &features, unsigned int label) const
{
	CImg<double> histogram(_forest->getNLeaves() + 1);
	for (unsigned int f = 0; f < features.width(); ++f)
	{
		_forest->classify(histogram.data(), features.get_column(f));
	}
	normalize(histogram);

	return _models.get_line(label).dot(histogram);
}

void Classifier::normalize(CImg<double> &histogram)
{
#pragma push_macro("min")
#undef min
	histogram.min(1.);
#pragma pop_macro("min")
}

void Classifier::save(string binFile) const
{
	ofstream bin;
	unsigned int nModels = _models.height();
	unsigned int nLeaves = _forest->getNLeaves();
	bin.open(binFile.c_str(), ios::trunc | ios::binary);
	bin.write((char *) &nModels, sizeof(unsigned int));
	bin.write((char *) &nLeaves, sizeof(unsigned int));
	bin.write((char *) _models.data(), _models.width() * _models.height() * sizeof(double));
	bin.close();

	cout << "Saved " << nModels << " models associated to a forest of " << nLeaves << " leaves." << endl;
}

void Classifier::load(string binFile)
{
	ifstream bin;
	unsigned int nModels;
	unsigned int nLeaves;
	bin.open(binFile.c_str(), ios::in | ios::binary);
	bin.read((char *) &nModels, sizeof(unsigned int));
	bin.read((char *) &nLeaves, sizeof(unsigned int));
	_models.assign(nLeaves + 1, nModels);
	bin.read((char *) _models.data(), _models.width() * _models.height() * sizeof(double));
	bin.close();

	cout << "Loaded " << nModels << " models associated to a forest of " << nLeaves << " leaves." << endl;

}

unsigned int Classifier::getNModels(void) const
{
	return _models.height();
}