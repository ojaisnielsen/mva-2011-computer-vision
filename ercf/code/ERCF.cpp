/*! \file */

#include "stdafx.h"
#include "ErcForest.h"
#include "tools.h"
#include "FeatureExtractor.h"
#include "Classifier.h"

using namespace ercf;

void train(vector<string> imageSearchPaths, vector<string> maskSearchPaths, unsigned int featureType, unsigned int maxNPictures)
{
	unsigned int nClasses = imageSearchPaths.size();
	unsigned int maxNDescriptorsPerImage = 67;	
	unsigned int patchSize = 16;
	unsigned int imageBucketSize = 20;
	CImgList<double> featureList(maxNDescriptorsPerImage * maxNPictures * nClasses);
	vector<unsigned int> labels(featureList.size(), 0);
	CImg<double> positions(featureList.size(), 2);
	unsigned int nDescriptors = 0;
	Timer totalTimer;
	vector<unsigned int> nDescriptorsPerImage(maxNPictures * nClasses);
	unsigned int nImages = 0;

	totalTimer.begin();

	for (int c = 0; c < nClasses; ++c)
	{
		bool useMasks = (maskSearchPaths[c].size() != 0);

		vector<string> imagePaths = getFileNames(imageSearchPaths[c]);
		unsigned int nPictures = min(imagePaths.size(), maxNPictures);
		vector<string> maskPaths;
		if (useMasks)
		{
			maskPaths = getFileNames(maskSearchPaths[c]);
			nPictures = min(nPictures, maskPaths.size());
		}

		cout << "Found " << nPictures << "/" << maxNPictures << " pictures ";
		if (useMasks) cout << "and masks ";
		cout << "of class " << c << "/" << nClasses << "." << endl;
			
		for (int i = 0; i < nPictures; i += imageBucketSize)
		{

			CImgList<double> imList;
			CImgList<bool> maskList;
			CImgList<bool> *maskListPtr;			
			Timer timer;

			maskListPtr = useMasks ? &maskList : NULL;		

			unsigned int i0 = i;
			unsigned int i1 = min(nPictures, i0 + imageBucketSize);

			timer.begin();
			loadImages<double>(imList, vector<string>(imagePaths.begin() + i0, imagePaths.begin() + i1));
			cout << "Pictures " << i0 << "->" << i1 << "/" << nPictures << " of class " << c << "/" << nClasses << " loaded in " << timer.end() << "s." << endl;

			if (useMasks)
			{
				timer.begin();
				loadImages<bool>(maskList, vector<string>(maskPaths.begin() + i0, maskPaths.begin() + i1));
				cout << "Masks " << i0 << "->" << i1 << "/" << nPictures << " of class " << c << "/" << nClasses << " loaded in " << timer.end() << "s." << endl;
			}


			timer.begin();

			FeatureExtractor featureExtractor(&featureList, nDescriptorsPerImage.data() + nImages, &labels, maxNDescriptorsPerImage, &imList, maskListPtr);
			
			cout << "Feature extractor created in " << timer.end() << "s" << endl;

			//featureExtractor.setDisplay(true);
			timer.begin();

			unsigned int nNewDecriptors;
			if (featureType == 0) nNewDecriptors = featureExtractor.getMultipleHsl(nDescriptors, 0, imList.size(), patchSize, c);
			else if (featureType == 1) nNewDecriptors = featureExtractor.getMultipleHslHaar(nDescriptors, 0, imList.size(), patchSize, c);
			else nNewDecriptors = featureExtractor.getMultipleSift(nDescriptors, 0, imList.size(), c);


			cout << "Descriptors " << nDescriptors << "->" << nDescriptors + nNewDecriptors << "/" << featureList.size() << " descriptors extracted in " << timer.end() << "s" << endl;
			nDescriptors += nNewDecriptors;

			nImages += i1 - i0;
		}		
	}

	cout << "Spent " << totalTimer.end() << "s loading data and extracting " << nDescriptors << "/" << featureList.size() << "features." << endl;

	while (featureList.size() > nDescriptors) featureList.pop_back();
	CImg<double> features = featureList.get_append('x');

	totalTimer.begin();
	TrainingSet set(&features, &labels, nClasses);
	cout << "Training set created in " << totalTimer.end() << "s." << endl;
	
	totalTimer.begin();
	ErcForest forest(5);
	forest.train(set, 0.5, set.getFeatureDim());
	forest.prune(1000);
	forest.save("forest.xml");
	cout << "Spent " << totalTimer.end() << "s training the forest and saving it to \"forest.xml\"." << endl;
	
	totalTimer.begin();
	Classifier classifier(&forest);
	classifier.train(set, nDescriptorsPerImage);
	classifier.save("classifier.bin");
	cout << "Spent " << totalTimer.end() << "s training the SVM classifier and saving it to \"classifier.bin\"." << endl;

}


void test(string forestPath, string classifierPath, string testImagePath, unsigned int featureType)
{
	ErcForest forest(forestPath);
	Classifier classifier(&forest);
	classifier.load(classifierPath);

	unsigned int maxNDescriptors = 8000;

	unsigned int nDescriptorsPerImage;
	CImgList<double> imList;
	CImgList<double> featureList(maxNDescriptors);
	CImg<double> positions(featureList.size(), 2);

	vector<string> imagePaths;
	imagePaths.push_back(testImagePath);
	loadImages<double>(imList, imagePaths);
	FeatureExtractor featureExtractor(&featureList, &nDescriptorsPerImage, &positions, maxNDescriptors, &imList);

	unsigned int nDescriptors;
	if (featureType == 0) nDescriptors = featureExtractor.getHsl(0, 0, 16);
	else if (featureType == 1) nDescriptors = featureExtractor.getHslHaar(0, 0, 16);
	else nDescriptors = featureExtractor.getSift(0, 0);

	while (featureList.size() > nDescriptors) featureList.pop_back();
	CImg<double> features = featureList.get_append('x');

	for (unsigned int c = 0; c < classifier.getNModels(); ++c)
	{
		cout << classifier.unmixedPoints(imList.at(0), features, positions, c) << " unmixed points for label " << c <<  endl;
		cout << "Decision function for label " << c << ": " << classifier.classify(features, c) << endl;
	}
}

int main(unsigned int argc, char* argv[])
{	
	if (argc == 2)
	{
		vector<string> imageSearchPaths;
		vector<string> maskSearchPaths;
		cout << "Training model from paths in \"" << argv[1] << "\"" << endl;
	
		FILE *pathFile = fopen(argv[1], "r");
		char line[MAX_PATH];
		while (fgets(line, MAX_PATH, pathFile))
		{		
			if (line[strlen(line) - 1] == '\n') line[strlen(line) - 1] = '\0';
			imageSearchPaths.push_back(string(line));
			if (!fgets(line, MAX_PATH, pathFile)) 
			{
				maskSearchPaths.push_back("");
				break;
			}			
			if (line[strlen(line) - 1] == '\n') line[strlen(line) - 1] = '\0';
			maskSearchPaths.push_back(string(line));
		}
		fclose(pathFile);

		unsigned int maxNPictures;
		cout << "Number of images to use: ";
		cin >> maxNPictures;	

		unsigned int featureType;
		cout << "Type of features (0: HSL, 1: Haar transform of HSL, 2: SIFT): ";
		cin >> featureType;	
		if (featureType == 0) cout << "Using HSL." << endl;
		else if (featureType == 1) cout << "Using Haar transform of HSL." << endl;
		else cout << "Using SIFT." << endl;

		train(imageSearchPaths, maskSearchPaths, featureType, maxNPictures);
	}
	else if (argc == 4)
	{
		string forestPath = argv[1];
		string classifierPath = argv[2];
		string testImagePath = argv[3];
		cout << "Testing image \"" << testImagePath << "\" with forest \"" << forestPath << "\" and classifier \"" << classifierPath << "\"." << endl;
		
		unsigned int featureType;
		cout << "Type of features (0: HSL, 1: Haar transform of HSL, 2: SIFT): ";
		cin >> featureType;			
		
		test(forestPath, classifierPath, testImagePath, featureType);
	}
	else
	{
		cout << "Usage" << endl << endl;
		cout << "For training models to \"forest.xml\" and \"classifier.bin\" with image search paths indicated in \"paths.txt\" :" << endl;
		cout << "ERCF.exe \"paths.txt\"" << endl << endl;
		cout << "For testing image \"image.jpg\" with models \"forest.xml\" and \"classifier.bin\" :" << endl;
		cout << "ERCF.exe \"forest.xml\" \"clasifier.bin\" \"image.jpg\"" << endl << endl;
	}

	return 0;

}

