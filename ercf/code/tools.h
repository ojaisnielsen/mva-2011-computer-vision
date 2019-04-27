/*! \file */

#pragma once
#include "stdafx.h"

template<typename T, class Distribution>
class Random
{
private:
	typedef variate_generator<minstd_rand, Distribution> Generator;
	Generator *_generator;
	T _min;
	T _max;
	T _seed;
public:
	Random(void)
	{
		_generator = NULL;
	}
	Random(const Random &random) : _min(random._min), _max(random._max), _seed(random._seed)
	{
		if (random._generator != NULL) _generator = new Generator(minstd_rand(_seed), Distribution(_min, _max));
		else _generator = NULL;
	}

	Random(T min, T max, T seed = 999) : _min(min), _max(max), _seed(seed)
	{
		_generator = new Generator(minstd_rand(_seed), Distribution(_min, _max));
	}
	void assign(T min, T max, T seed = 999)
	{
		_min = min;
		_max = max;
		_seed = seed;
		_generator = new Generator(minstd_rand(_seed), Distribution(_min, _max));
	}
	~Random(void)
	{
		if (_generator != NULL) delete _generator;
	}
	T operator()(void) const 
	{
		return _generator->operator()();
	}
	static T Default(void)
	{
		static Random<T, Distribution> random((T)0, (T)1);
		return random();
	}
};

typedef Random<int, uniform_int_distribution<int>> RandomInt;
typedef Random<double, uniform_real_distribution<double>> RandomDouble;

vector<string> getFileNames(const string &query);

template<typename T>
void loadImages(CImgList<T> &imList, const vector<string> &fileNames)
{	
	unsigned int nFiles = fileNames.size();
	imList.assign(nFiles);
	for (unsigned int i = 0; i < nFiles; ++i)
	{
		imList.at(i).assign(CImg<T>(fileNames.at(i).c_str()).RGBtoHSL());
	}
}

template<>
void loadImages<bool>(CImgList<bool> &imList, const vector<string> &fileNames);

template<typename T>
void printMatrix(CImg<T> &matrix)
{
	for (unsigned int y = 0; y < matrix.height(); ++y)
	{
		for (unsigned int x = 0; x < matrix.width(); ++x)
		{
			cout << matrix.atXY(x,y) << " ";
		}
		cout << endl;
	}
}

class Plot
{
private:
	CImg<double> _image;
	unsigned char _color[3];
	unsigned int _nPoints;

public:
	Plot(void);
	Plot(const CImg<double> &image);
	void assign(const CImg<double> &image);
	void operator()(unsigned int x, unsigned int y);
	void operator()(unsigned int x, unsigned int y, unsigned int radX, unsigned int radY);
	void operator()(void) const;
};

class Timer 
{
public:
	Timer(void);
	double last() const;
	void begin(void);
	double end(void);

private:
	clock_t _start;
	double _elapsed;
};


template<typename T>
class NullableVector
{
private:
	vector<T> _data;
	vector<bool> _nullData;
public:
	NullableVector(void)
	{
	}
	NullableVector(unsigned int size, const T &val, bool isNull = true)
	{
		_data.assign(size, val);
		_nullData.assign(size, isNull);
	}
	void assign(unsigned int size, const T &val, bool isNull = true)
	{
		_data.assign(size, val);
		_nullData.assign(size, isNull);
	}
	bool isNull(unsigned int index)
	{
		return _nullData[index];
	}
	void set(unsigned int index)
	{
		_nullData[index] = false;
	}
	void set(unsigned int index, T val)
	{
		_data[index] = val;	
		_nullData[index] = false;
	}
	void unSet(unsigned int index)
	{
		_nullData[index] = true;
	}
	T &operator[](unsigned int i)
	{
		return _data[i];
	}
};



template<typename K, typename V>
class AssociativeSortedList
{
private:
	class Node
	{
		friend class AssociativeSortedList<K, V>;
	public:
		Node(void)
			: _leftChild(NULL), _rightChild(NULL), _size(0), _next(NULL), _prev(NULL), _parent(NULL), _depth(0)
		{
		}

		Node *assign(Node *parent, Node *prev, Node *next)
		{
			_parent = parent;
			_prev = prev;
			_next = next;
			return this;
		}
		unsigned int leftSize(void) const
		{
			return (_leftChild == NULL) ? 0 : _leftChild->_size;
		}
		unsigned int leftDepth(void) const
		{
			return (_leftChild == NULL) ? 0 : _leftChild->_depth;
		}
		unsigned int rightDepth(void) const
		{
			return (_rightChild == NULL) ? 0 : _rightChild->_depth;
		}
		int balanceFactor(void) const
		{
			return leftDepth() - rightDepth();
		}

	private:
		unsigned int _size;
		unsigned int _depth;
		K _key;
		V _val;
		Node *_leftChild;
		Node *_rightChild;
		Node *_next;
		Node *_prev;
		Node *_parent;
	};

	const Node *_getNode(unsigned int i) const
	{
		const Node *current = _nodes;
		while (current->leftSize() != i)
		{
			if (i < current->leftSize()) current = current->_leftChild;
			else 
			{
				i -= current->leftSize() + 1;
				current = current->_rightChild;
			}
		}
		return current;
	}

	//void _rightRotate(Node *node)
	//{
	//	Node *parent = node->_parent;
	//	Node *node2 = node->_leftChild;
	//	node->_leftChild = node2->_rightChild;
	//	node2->_rightChild = node;
	//	if (node == parent->_leftChild) parent->_leftChild = node2;
	//	else parent->_rightChild = node2;
	//	node->_parent = node2;
	//	node2->_parent = parent;
	//	_updateDepth(node);
	//}

	//void _leftRotate(Node *node)
	//{
	//	Node *parent = node->_parent;
	//	Node *node2 = node->_rightChild;
	//	node->_rightChild = node2->_leftChild;
	//	node2->_leftChild = node;
	//	if (node == parent->_leftChild) parent->_leftChild = node2;
	//	else parent->_rightChild = node2;
	//	node->_parent = node2;
	//	node2->_parent = parent;
	//	_updateDepth(node);
	//}

	void _updateDepth(Node *node)
	{
		Node *current = node;
		while (current != NULL)
		{
			current->_depth = 1 + max(current->leftDepth(), current->rightDepth());
			current = current->_parent;			
		}
	}

	//void _rebalance(Node *node)
	//{
	//	Node *current = node->_parent;
	//	while (current != NULL && current->_parent != NULL)
	//	{
	//		if (current->balanceFactor() < -1)
	//		{
	//			if (current->_rightChild->balanceFactor() > 0) _rightRotate(current->_rightChild);				
	//			_leftRotate(current);
	//		}
	//		else if (current->balanceFactor() > 1)
	//		{
	//			if (current->_leftChild->balanceFactor() < 0) _leftRotate(current->_leftChild);
	//			_rightRotate(current);
	//		}
	//		current = current->_parent;			
	//	}
	//}

	unsigned int _maxSize;
	Node *_nodes;
	K _minKey;
	K _maxKey;

public:
	AssociativeSortedList(void)
		: _maxSize(0)
	{
	}

	AssociativeSortedList(unsigned int maxSize)
		: _maxSize(maxSize)
	{
		_nodes = new Node[maxSize];
	}

	void assign(unsigned int maxSize)
	{
		_maxSize = maxSize;
		_nodes = new Node[maxSize];
	}

	~AssociativeSortedList(void)
	{
		if (_maxSize > 0) delete[] _nodes;
	}

	void insert(const K &key, const V &val)
	{
		_minKey = (size() == 0) ? key : min(_minKey, key);
		_maxKey = (size() == 0) ? key : max(_maxKey, key);		

		Node *current = _nodes;
		while (current->_size++ != 0)
		{
			if (key < current->_key)
			{
				if (current->_leftChild == NULL)
				{
					int s = size() - 1;
					current->_leftChild = _nodes[size() - 1].assign(current, current->_prev, current);
					if (current->_prev != NULL) current->_prev->_next = current->_leftChild;
					current->_prev = current->_leftChild;
				}
				current = current->_leftChild;
			}
			else
			{
				if (current->_rightChild == NULL) 
				{
					int s = size() - 1;
					current->_rightChild = _nodes[size() - 1].assign(current, current, current->_next);
					if (current->_next != NULL) current->_next->_prev = current->_rightChild;
					current->_next = current->_rightChild;
				}
				current = current->_rightChild;
			}
		}
		_updateDepth(current);
		//_rebalance(current);
		current->_key = key;
		current->_val = val;
	}

	unsigned int lowerBound(const K &key) const
	{
		if (key <= _minKey) return 0;
		if (key > _maxKey) return size();
		Node *current = _nodes;
		unsigned int pos = 0;
		while (current->_key < key || (current->_prev != NULL && current->_prev->_key >= key))
		{
			if (current->_prev != NULL && current->_key >= key) current = current->_leftChild;
			else
			{
				pos += current->leftSize() + 1;
				current = current->_rightChild;
			}
		}
		return pos + current->leftSize();
	}

	unsigned int size(void) const
	{
		return (_maxSize == 0) ? 0 : _nodes->_size;
	}

	unsigned int depth(void) const
	{
		return (_maxSize == 0) ? 0 : _nodes->_depth;
	}

	unsigned int maxSize(void) const
	{
		return _maxSize;
	}

	const V &operator[](unsigned int i) const
	{		
		return _getNode(i)->_val;
	}

	const K &getKey(unsigned int i) const
	{
		return _getNode(i)->_key;
	}

	const K &getMaxKey(void) const
	{
		return _maxKey;
	}

	const K &getMinKey(void) const
	{
		return _minKey;
	}

};






