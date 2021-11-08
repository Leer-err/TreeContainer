#pragma once
#include <functional>
using namespace std;

template<typename T>
class Map
{
public:
	class Iterator {
		using Node = typename Map<T>::Node;
		friend class Map;
	public:
		Iterator(Node* begin, Node* end, Node* currentObject = begin, function<int(T, T)> compareFunction = [](T a, T b) {return a - b; }) : 
			m_begin(begin), m_currentObject(currentObject), m_end(end), m_compareFunction(compareFunction) {}

		inline T* Get() { return &m_currentObject->value; }

		Iterator& operator++() {
			if (m_currentObject->right != nullptr) {
				m_currentObject = m_currentObject->right;
				while (m_currentObject->left != nullptr)
				{
					m_currentObject = m_currentObject->left;
				}
			}
			else if(m_currentObject != m_end){
				T tmp = m_currentObject->value;
				while (m_compareFunction(m_currentObject->parent->value, tmp) < 0) {
					m_currentObject = m_currentObject->parent;
					if (m_currentObject->parent == nullptr) {
						m_currentObject = m_end;
						return *this;
					}
				}
				m_currentObject = m_currentObject->parent;
			}
			return *this;
		}
		Iterator& operator--() {
			if (m_currentObject->left != nullptr) {
				m_currentObject = m_currentObject->left;
				while (m_currentObject->right != nullptr)
				{
					m_currentObject = m_currentObject->right;
				}
			}
			else if (m_currentObject != m_begin) {
				T tmp = m_currentObject->value;
				while (m_compareFunction(m_currentObject->parent->value, tmp) > 0) {
					m_currentObject = m_currentObject->parent;
					if (m_currentObject->parent == nullptr) {
						m_currentObject = m_end;
						return *this;
					}
				}
				m_currentObject = m_currentObject->parent;
			}
			return *this;
		}

		inline T& operator*() const { return m_currentObject->value; }
		inline T* operator->() const { return &m_currentObject->value; }
		inline bool operator==(Iterator other) { return m_currentObject == other.m_currentObject; }
		inline bool operator!=(Iterator other) { return m_currentObject != other.m_currentObject; }
	protected:
		inline Node* GetNode() { return m_currentObject; }
	private:
		Node* m_begin;
		Node* m_currentObject;
		Node* m_end;
		function<int(T, T)> m_compareFunction;
	};

	Map(function<int(T, T)> compareFunction = [](T a, T b) {return a - b; }) :
		m_compareFunction(compareFunction), m_root(nullptr), m_end(new Node(0, nullptr, nullptr, nullptr, -1)) {}

	~Map() {
		Clear(m_root);
		delete m_end;
	}

	void Insert(T value) {
		if (m_root == nullptr) {
			m_root = new Node(value);
			return;
		}
		Node* tmp = m_root;

		while (true) {
			int result = m_compareFunction(tmp->value, value);
			if (result > 0) { 
				if (tmp->left == nullptr) {
					tmp->left = new Node(value, tmp);
					break;
				}
				tmp = tmp->left; 
			}
			else if (result < 0) { 
				if (tmp->right == nullptr) {
					tmp->right = new Node(value, tmp);
					break;
				}
				tmp = tmp->right;	
			}
			else {
				tmp->count++;
				break;
			}
		}
		Balance();
	}

	Map<T>::Iterator Search(T value) {
		TreeIterator iter(m_root);
		while ((*iter).value != value) {
			int result = m_compareFunction((*iter).value, value);
			if (result > 0) {
				if ((*iter).left == nullptr) { return end(); }
				--iter;
			}
			else if (result < 0) { 
				if ((*iter).right == nullptr) { return end(); }
				++iter;
			}
		}
		Node* lowest = m_root;
		while (lowest->left) {
			lowest = lowest->left;
		}
		return Iterator(lowest, m_end, iter.Get(), m_compareFunction);
	}

	void Delete(T value) {
		Node* tmp;
		Node* replace = nullptr;

		auto deleteValue = Search(value);
		auto replaceParent = [&tmp](Node* ptr) { tmp->parent->left == tmp ? tmp->parent->left = ptr : tmp->parent->right = ptr; };
		tmp = deleteValue.GetNode();

		if (tmp->left == nullptr && tmp->right == nullptr) {
			replaceParent(nullptr);
		}
		else if (tmp->left == nullptr) {
			replace = (++deleteValue).GetNode();
			replaceParent(replace);
			if (replace->parent == tmp) {
				replace->left = tmp->left;
				replace->parent = tmp->parent;
				delete tmp;
				Balance();
				return;
			}
			replace->parent->left = replace->right;
			replace->parent = tmp->parent;
			replace->left = tmp->left;
			replace->right = tmp->right;
			replace->right->parent = replace;
		}
		else {
			replaceParent(replace);
			replace = (--deleteValue).GetNode();
			if (replace->parent == tmp) {
				replace->right = tmp->right;
				replace->parent = tmp->parent;
				delete tmp;
				Balance();
				return;
			}
			replace->parent->right = replace->left;
			replace->parent = tmp->parent;
			replace->left = tmp->left;
			replace->left->parent = replace;
			replace->right = tmp->right;
			replace->right->parent = replace;
		}
		if (tmp == m_root) {
			m_root = replace;
		}

		delete tmp;
		Balance();
		return;
	}

	Map<T>::Iterator begin() {
		Node* lowest = m_root;
		while (lowest && lowest->left) {
			lowest = lowest->left;
		}
		return Iterator(lowest, m_end, lowest, m_compareFunction);
	}

	Map<T>::Iterator end() { return Iterator(m_end, m_end, m_end, m_compareFunction); }
protected:
	class TreeIterator {
		using Node = typename Map<T>::Node;
	public:
		TreeIterator(Node* begin) : m_currentObject(begin) {}

		inline Node* Get() { return m_currentObject; }

		TreeIterator& Back() {
			if (m_currentObject->parent != nullptr) {
				m_currentObject = m_currentObject->parent;
			}
			return *this;
		}
		TreeIterator& operator++() {
			if (m_currentObject->right != nullptr) {
				m_currentObject = m_currentObject->right;
			}
			return *this;
		}
		TreeIterator& operator--() {
			if (m_currentObject->left != nullptr) {
				m_currentObject = m_currentObject->left;
			}
			return *this;
		}

		inline Node& operator*() const { return *m_currentObject; }
		inline Node* operator->() const { return &m_currentObject; }
		inline bool operator==(TreeIterator other) { return m_currentObject == other.m_currentObject; }
		inline bool operator!=(TreeIterator other) { return m_currentObject != other.m_currentObject; }
	private:
		Node* m_currentObject;
	};

	struct Node
	{
		Node(T val, Node* prev = nullptr, Node* lower = nullptr, Node* greater = nullptr, int num = 0, int bf = 0) :
			value(val), count(num), balanceFactor(bf), parent(prev), left(lower), right(greater) {}

		T value;
		int count;
		int balanceFactor;
		Node* parent;
		Node* left;
		Node* right;

		inline bool operator==(Node b) {
			return value == b.value && count == b.count;
		}
	};

	int CountBalanceFactor(Node* root, int depth = 0) {
		depth++;
		int ldepth = depth;
		int rdepth = depth;
		if (root->right == nullptr && root->left == nullptr) {
			root->balanceFactor = 0;
			return depth;
		}
		if (root->left != nullptr) { ldepth = CountBalanceFactor(root->left, depth); }
		if (root->right != nullptr) { rdepth = CountBalanceFactor(root->right, depth); }
		root->balanceFactor = rdepth - ldepth;
		return ldepth >= rdepth ? ldepth : rdepth;
	}

	Node* RotateRight(Node* p) // правый поворот вокруг p
	{
		Node* q = p->left;
		q->parent = p->parent;
		if (!q->parent) { m_root = q; }
		if (q->right != nullptr) { q->right->parent = p; }
		p->parent = q;
		p->left = q->right;
		q->right = p;
		return q;
	}

	Node* RotateLeft(Node* q) // левый поворот вокруг q
	{
		Node* p = q->right;
		p->parent = q->parent;
		if (!p->parent) { m_root = p; }
		if (p->left != nullptr) { p->left->parent = q; }
		q->parent = p;
		q->right = p->left;
		p->left = q;
		return p;
	}

	Node* SearchUnbalanced(Node* root) {
		if (root == nullptr) { return nullptr; }
		if (root->balanceFactor >= 2 || root->balanceFactor <= -2) { return root; }
		if (SearchUnbalanced(root->right) == nullptr) {
			return SearchUnbalanced(root->left);
		}
		else {
			return SearchUnbalanced(root->right);
		}
	}

	void Balance() {
		CountBalanceFactor(m_root);
		Node* unbalanced = SearchUnbalanced(m_root);
		if (unbalanced) {
			Node* unbalancedParent = unbalanced->parent;
			auto replaceParent = [unbalancedParent, unbalanced](Node* ptr) { unbalancedParent->left == unbalanced ? unbalancedParent->left = ptr : unbalancedParent->right = ptr; };
			if (unbalanced->balanceFactor >= 2)
			{
				if (unbalanced->right->balanceFactor < 0) {
					unbalanced->right = RotateRight(unbalanced->right);
				}
				Node* tmp = RotateLeft(unbalanced);
				if (unbalancedParent) {
					unbalancedParent->left == unbalanced ? unbalancedParent->left = tmp : unbalancedParent->right = tmp;
				}
			}
			else if (unbalanced->balanceFactor <= -2)
			{
				if (unbalanced->left->balanceFactor > 0) {
					unbalanced->left = RotateLeft(unbalanced->left);
				}
				Node* tmp = RotateRight(unbalanced);
				if (unbalancedParent) {
					unbalancedParent->left == unbalanced ? unbalancedParent->left = tmp : unbalancedParent->right = tmp;
				}
			}
		}
	}

	void Clear(Node* node) {
		if (node->left) { Clear(node->left); }
		if (node->right) { Clear(node->right); }
		delete(node);
	}
private:
	Node* m_root;
	Node* m_end;

	function<int(T, T)> m_compareFunction;
};
