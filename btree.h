#ifndef BTREE_H
#define BTREE_H

#include "node.h"
#include <iostream>
#include <stdexcept>

using namespace std;

template <typename TK>
class BTree {
private:
    Node<TK>* root;
    int M;
    int n;

	void dfs(Node<TK>* node, int depth, bool& ok) {
		int minK = (M + 1) / 2 - 1;
		if (node == root && node->count == 0) { return; }
		if (node != root && node->count < minK) { ok = false; }
		if (node->count > M - 1) { ok = false; }
		for (int i = 1; i < node->count; ++i)
			if (!(node->keys[i - 1] < node->keys[i])) { ok = false; }
		if (!node->leaf) {
			for (int i = 0; i <= node->count; ++i)
				if (node->children[i]) { dfs(node->children[i], depth + 1, ok); }
		}
	};

	int min_keys() { return (M+1)/2 - 1; }

	int find_key(Node<TK>* node, TK key) {
    	int idx = 0;
    	while (idx < node->count && node->keys[idx] < key) ++idx;
    	return idx;
	}

	void remove_from_leaf(Node<TK>* node, int idx) {
	    for (int i = idx + 1; i < node->count; ++i)
	        node->keys[i - 1] = node->keys[i];
	    node->count--;
	}

	TK get_successor(Node<TK>* node, int idx) {
	    Node<TK>* cur = node->children[idx + 1];
	    while (!cur->leaf)
	        cur = cur->children[0];
	    return cur->keys[0];
	}

	void borrow_from_prev(Node<TK>* node, int idx) {
	    Node<TK>* child = node->children[idx];
	    Node<TK>* sibling = node->children[idx - 1];
	    for (int i = child->count - 1; i >= 0; --i)
	        child->keys[i + 1] = child->keys[i];
	    if (!child->leaf)
	        for (int i = child->count; i >= 0; --i)
	            child->children[i + 1] = child->children[i];
	    child->keys[0] = node->keys[idx - 1];
	    if (!child->leaf)
	        child->children[0] = sibling->children[sibling->count];
	    node->keys[idx - 1] = sibling->keys[sibling->count - 1];
	    child->count++;
	    sibling->count--;
	}

	void borrow_from_next(Node<TK>* node, int idx) {
	    Node<TK>* child = node->children[idx];
	    Node<TK>* sibling = node->children[idx + 1];
	    child->keys[child->count] = node->keys[idx];
	    if (!child->leaf)
	        child->children[child->count + 1] = sibling->children[0];
	    node->keys[idx] = sibling->keys[0];
	    for (int i = 1; i < sibling->count; ++i)
	        sibling->keys[i - 1] = sibling->keys[i];
	    if (!sibling->leaf)
	        for (int i = 1; i <= sibling->count; ++i)
	            sibling->children[i - 1] = sibling->children[i];
	    child->count++;
	    sibling->count--;
	}

	void merge(Node<TK>* parent, int idx) {
	    Node<TK>* left = parent->children[idx];
	    Node<TK>* right = parent->children[idx + 1];
	    left->keys[left->count] = parent->keys[idx];
	    int oldLeftCount = left->count;
	    for (int i = 0; i < right->count; ++i)
	        left->keys[oldLeftCount + 1 + i] = right->keys[i];
	    if (!left->leaf)
	        for (int i = 0; i <= right->count; ++i)
	            left->children[oldLeftCount + 1 + i] = right->children[i];
	    left->count = oldLeftCount + right->count + 1;
	    for (int i = idx; i < parent->count - 1; ++i)
	        parent->keys[i] = parent->keys[i + 1];
	    for (int i = idx + 1; i < parent->count; ++i)
	        parent->children[i] = parent->children[i + 1];
	    parent->count--;
	    parent->children[parent->count + 1] = nullptr;
	    if (right->keys) delete[] right->keys;
	    if (right->children) delete[] right->children;
	    delete right;
	}

	void fill(Node<TK>* node, int idx) {
    	if (idx > 0 && node->children[idx - 1]->count > min_keys())
        	borrow_from_prev(node, idx);
	    else if (idx < node->count && node->children[idx + 1]->count > min_keys())
    	    borrow_from_next(node, idx);
	    else {
    	    if (idx < node->count)
        	    merge(node, idx);
	        else
	            merge(node, idx - 1);
    	}
	}

	void remove_internal(Node<TK>* node, TK key) {
	    int idx = findKey(node, key);
	    if (idx < node->count && !(key < node->keys[idx]) && !(node->keys[idx] < key)) {
 	       	if (node->leaf) {
	            remove_from_leaf(node, idx);
	            return;
	        } else {
				Node<TK>* rightChild = node->children[idx + 1];
				if (rightChild->count >= min_keys() + 1) {
 	            	TK succ = get_successor(node, idx);
  	            	node->keys[idx] = succ;
  	              	remove_internal(rightChild, succ);
  	          	}
				else {
    	            merge(node, idx);
   	             	remove_internal(node->children[idx], key);
    	        }
            	return;
     		}
		}
   		if (node->leaf) { return; }
   		int i = idx;
    	remove_internal(node->children[i], key);
    	if (i > node->count) i = node->count;
    	if (!node->children[i]) return;
    	if (node->children[i]->count < min_keys()) {
        	if (i > 0 && node->children[i - 1]->count > min_keys()) { borrow_from_prev(node, i); }
	        else if (i < node->count && node->children[i + 1]->count > min_keys()) { borrow_from_next(node, i); }
    	    else {
        	    if (i < node->count) { merge(node, i); }
            	else { merge(node, i - 1); }
        	}
    	}
	}

	void remove_root_if_empty() {
	    if (root->count == 0) {
	        Node<TK>* oldRoot = root;
	        if (root->leaf) {
	            if (oldRoot->keys) delete[] oldRoot->keys;
	            if (oldRoot->children) delete[] oldRoot->children;
	            root = nullptr;
	            delete oldRoot;
	        } else {
	            root = root->children[0];
	            if (oldRoot->keys) delete[] oldRoot->keys;
	            if (oldRoot->children) delete[] oldRoot->children;
  	          delete oldRoot;
	        }
	    }
	}


public:
    BTree(int _M) : root(nullptr), M(_M), n(0) { if (M < 3) throw invalid_argument("M too small"); }

    bool search(TK key) {
        Node<TK>* cur = root;
        while (cur) {
            int i = 0;
            while (i < cur->count && key > cur->keys[i]) ++i;
            if (i < cur->count && !(key > cur->keys[i]) && !(cur->keys[i] > key)) return true;
            if (cur->leaf) return false;
            cur = cur->children[i];
        }
        return false;
    }

	bool check_properties() {
		if (!root) return true;
		bool ok = true;
		dfs(root, 0, ok);
		return ok;
	}


	void remove(TK key) {
	    if (!root) return;
	    if (!search(key)) return;
	    remove_internal(root, key);
	    n = max(0, n - 1);
	    if (root->count == 0)
	        remove_root_if_empty();
	}

	int size() { return n; }

	void clear() {
		if (root) {
			root->killSelf();
			delete root;
			root = nullptr;
		}
		n = 0;
	}

 	int height() {
		if (!root) return -1;
		int h = -1;
		Node<TK>* cur = root;
		while (!cur->leaf) { cur = cur->children[0]; h++; }
		return h + 1;
 	}

	TK minKey() {
		if (!root || root->count == 0) throw runtime_error("Empty tree");
		Node<TK>* cur = root;
		while (!cur->leaf) cur = cur->children[0];
		return cur->keys[0];
	}

	TK maxKey() {
		if (!root || root->count == 0) throw runtime_error("Empty tree");
		Node<TK>* cur = root;
		while (!cur->leaf) cur = cur->children[cur->count];
		return cur->keys[cur->count - 1];
	}

    ~BTree() {
		if (root) { root->killSelf(); }
		root = nullptr;
		n = 0;
	}
};

#endif

