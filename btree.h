#ifndef BTree_H
#define BTree_H
#include <iostream>
#include <optional>
#include <vector>
#include <stdexcept>
#include "node.h"

using namespace std;

//implementación de un stack

template<typename T>
struct ListNode {
    T val;
    ListNode *next = nullptr;

    ListNode() = default;

    explicit ListNode(T val): val(val) {
    }
};

template<typename T>
struct ForwardList {
    ListNode<T> *head;
    ListNode<T> *tail;
    int sz;

    ForwardList(): head(nullptr), tail(nullptr), sz(0) {
    }

    explicit ForwardList(T val): head(new ListNode(val)), sz(1) { tail = head; }

    void push_front(T val) {
        auto *node = new ListNode(val);
        node->next = head;
        head = node;
        ++sz;
        if (sz == 1) tail = head;
    }

    void pop_front() {
        if (head == nullptr) throw std::runtime_error("List is empty");
        if (sz == 1) tail = nullptr;
        auto temp = head;
        head = head->next;
        delete temp;
        --sz;
    }

    void push_back(T val) {
        auto *node = new ListNode(val);
        if (tail != nullptr) tail->next = node;
        tail = node;
        ++sz;
        if (sz == 1) head = tail;
    }

    int size() { return sz; }
    bool empty() { return sz == 0; }

    ForwardList(const ForwardList &other) {
        ListNode<T> *curr = other.head;
        head = nullptr;
        tail = nullptr;
        sz = 0;
        while (curr) {
            push_back(curr->val);
            curr = curr->next;
        }
    }

    ForwardList &operator=(const ForwardList &other) {
        while (head) pop_front();
        ListNode<T> *curr = other.head;
        while (curr) {
            push_back(curr->val);
            curr = curr->next;
        }
        return *this;
    }

    ~ForwardList() { while (head != nullptr) pop_front(); }
};

template<typename T>
struct Stack {
    ForwardList<T> fl;

    Stack() = default;

    void push(T val) { fl.push_front(val); }
    void pop() { fl.pop_front(); }

    T top() {
        if (fl.empty()) throw std::runtime_error("Stack is empty");
        return fl.head->val;
    }

    int size() { return fl.sz; }
    bool empty() { return fl.sz == 0; }
};


//BTree

template<typename TK>
class BTree {
private:
    Node<TK> *root;
    int M; // grado u orden del arbol
    int n; // total de elementos en el arbol


    int min_keys() { return (M + 1) / 2 - 1; }

    int find_key(Node<TK> *node, TK key) {
        int idx = 0;
        while (idx < node->count && node->keys[idx] < key) ++idx;
        return idx;
    }

    void remove_from_leaf(Node<TK> *node, int idx) {
        for (int i = idx + 1; i < node->count; ++i)
            node->keys[i - 1] = node->keys[i];
        node->count--;
    }

    TK get_successor(Node<TK> *node, int idx) {
        Node<TK> *cur = node->children[idx + 1];
        while (!cur->leaf)
            cur = cur->children[0];
        return cur->keys[0];
    }

    void borrow_from_prev(Node<TK> *node, int idx) {
        Node<TK> *child = node->children[idx];
        Node<TK> *sibling = node->children[idx - 1];
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

    void borrow_from_next(Node<TK> *node, int idx) {
        Node<TK> *child = node->children[idx];
        Node<TK> *sibling = node->children[idx + 1];
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

    void merge(Node<TK> *parent, int idx) {
        Node<TK> *left = parent->children[idx];
        Node<TK> *right = parent->children[idx + 1];

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

    void fill(Node<TK> *node, int idx) {
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

    void remove_internal(Node<TK> *node, TK key) {
        int idx = find_key(node, key);
        if (idx < node->count && !(key < node->keys[idx]) && !(node->keys[idx] < key)) {
            if (node->leaf) {
                remove_from_leaf(node, idx);
                return;
            } else {
                Node<TK> *rightChild = node->children[idx + 1];
                if (rightChild->count >= min_keys() + 1) {
                    TK succ = get_successor(node, idx);
                    node->keys[idx] = succ;
                    remove_internal(rightChild, succ);
                } else {
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
            if (i > 0 && node->children[i - 1]->count > min_keys()) { borrow_from_prev(node, i); } else if (
                i < node->count && node->children[i + 1]->count > min_keys()) { borrow_from_next(node, i); } else {
                if (i < node->count) { merge(node, i); } else { merge(node, i - 1); }
            }
        }
    }

    void remove_root_if_empty() {
        if (root->count == 0) {
            Node<TK> *oldRoot = root;
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
        if (root == nullptr) return false;
        auto curr = root;
        while (!curr->leaf) {
            for (int i = 0; i < curr->count; ++i) {
                if (curr->keys[i] == key) return true;
                if (curr->keys[i] > key) {
                    curr = curr->children[i];
                    break;
                }
                if (i >= curr->count - 1) {
                    curr = curr->children[i + 1];
                    break;
                }
            }
        }
        for (int i = 0; i < curr->count; ++i) if (curr->keys[i] == key) return true;
        return false;
    } //indica si se encuentra o no un elemento

    void insert(TK key) {
        if (root == nullptr) {
            root = new Node<TK>(M);
            root->count = 1;
            root->keys[0] = key;
            n = 1;
            root->leaf = true;
            return;
        }
        Stack<Node<TK> *> stack;
        auto curr = root;
        while (!curr->leaf) {
            stack.push(curr);
            for (int i = 0; i < curr->count; ++i) {
                if (curr->keys[i] == key) return;
                if (curr->keys[i] > key) {
                    curr = curr->children[i];
                    break;
                }
                if (i >= curr->count - 1) {
                    curr = curr->children[i + 1];
                    break;
                }
            }
        }
        for (int i = 0; i < curr->count; ++i) {
            if (curr->keys[i] == key) return;
        }
        stack.push(curr);

        bool splitfinished = false;
        TK keytoadd = key;
        std::vector<TK> newKeys(M);
        std::vector<Node<TK> *> newChildren(M + 1, nullptr);
        Node<TK> *lastNewNode = nullptr;
        while (!stack.empty() && !splitfinished) {
            curr = stack.top();

            if (curr->count == M - 1) {
                auto *newNode = new Node<TK>(M);
                newNode->count = M - 1 - ((M - 1) / 2);
                newNode->leaf = curr->leaf;
                newChildren[0] = curr->children[0];
                int j = 0;
                bool keyadded = false;
                //crear arrays temporales
                for (int i = 0; i < M; ++i) {
                    if (j >= curr->count || (keytoadd < curr->keys[j] && !keyadded)) {
                        newKeys[i] = keytoadd;
                        newChildren[i + 1] = lastNewNode;
                        keyadded = true;
                    } else {
                        newKeys[i] = curr->keys[j];
                        newChildren[i + 1] = curr->children[j + 1];
                        ++j;
                    }
                }
                keytoadd = newKeys[(M - 1) / 2];
                //copiado de valores de arrays temporales a nodos
                for (int i = 0; i < (M - 1) / 2; ++i) { curr->keys[i] = newKeys[i]; }
                for (int i = 0; i < newNode->count; ++i) { newNode->keys[i] = newKeys[i + 1 + (M - 1) / 2]; }
                if (!curr->leaf) {
                    for (int i = 0; i < (M + 1) / 2; ++i) { curr->children[i] = newChildren[i]; }
                    for (int i = (M + 1) / 2; i < M + 1; ++i) { newNode->children[i - (M + 1) / 2] = newChildren[i]; }
                }
                curr->count = (M - 1) / 2;
                for (int i = (M - 1) / 2 + 1; i < M; ++i) curr->children[i] = nullptr;
                lastNewNode = newNode;
                stack.pop();
            } else {
                //insertar normalmente (sin split)
                int indextoinsert = curr->count;
                for (int i = 0; i < curr->count; ++i) {
                    if (keytoadd < curr->keys[i]) {
                        indextoinsert = i;
                        break;
                    }
                }
                for (int i = curr->count; i > indextoinsert; --i) {
                    curr->keys[i] = curr->keys[i - 1];
                    curr->children[i + 1] = curr->children[i];
                }
                curr->keys[indextoinsert] = keytoadd;
                curr->children[indextoinsert + 1] = lastNewNode;
                ++curr->count;
                splitfinished = true;
            }
        }
        if (!splitfinished) {
            //crear nuevo nodo raíz
            Node<TK> *newRoot = new Node<TK>(M);
            newRoot->count = 1;
            newRoot->keys[0] = keytoadd;
            newRoot->children[1] = lastNewNode;
            newRoot->children[0] = root;
            newRoot->leaf = false;
            root = newRoot;
        }
        ++n;
    } //inserta un elemento


    void remove(TK key) {
        if (!root) return;
        if (!search(key)) return;
        remove_internal(root, key);
        n = std::max(0, n - 1);
        if (root->count == 0)
            remove_root_if_empty();
    } //elimina un elemento

    int height() {
        if (root == nullptr) return 0;
        //Es lo mismo que si solo tuviera el nodo raíz, pero los test sugieren esto: no puede con M=3 haber 7 valores diferentes en el árbol
        //y que la altura esperada sea 1. Por lo que solo con el nodo raíz la altura también debe ser 0
        int counter = 0;
        auto curr = root;
        while (!curr->leaf) {
            curr = curr->children[0];
            ++counter;
        }
        return counter;
    } //altura del arbol. Considerar altura 0 para arbol vacio //LOS OTROS TEST NO CONCUERDAN CON ALTURA 0 PARA ARBOL VACÍO.
    //Es imposible que un árbol tenga altura 1 (1 nodo más que vacío, o sea 1 nivel de altura / 1 nodo) y 7 elementos insertados
    std::string keytostr(string key) { return key; }

    std::string keytostr(TK key) {
        return std::to_string(key);
    }

    string toString(const string &sep) {
        return toStringRecur(root, sep, false);
    } // recorrido inorder
    string toStringRecur(Node<TK> *node, const string &sep, bool lastsep) {
        if (node->leaf) {
            std::string outputleaf = "";
            for (int i = 0; i < node->count; ++i) {
                outputleaf += keytostr(node->keys[i]);
                if (lastsep || i != node->count - 1) outputleaf += sep;
            }
            return outputleaf;
        }
        std::string output = "";
        for (int i = 0; i < node->count; ++i) {
            output += toStringRecur(node->children[i], sep, true);
            output += keytostr(node->keys[i]) + sep;
        }
        output += toStringRecur(node->children[node->count], sep, lastsep);
        return output;
    }

    vector<TK> rangeSearch(TK begin, TK end) {
        vector<TK> output;
        rangeSearchRecur(root, output, begin, end);
        return output;
    }

private:
    void rangeSearchRecur(Node<TK> *node, vector<TK> &vec, TK begin, TK end) {
        if (node->leaf) {
            for (int i = 0; i < node->count; ++i) if (node->keys[i] >= begin && node->keys[i] <= end) vec.push_back(
                node->keys[i]);
            return;
        }
        // calcular límites en el nodo interno
        // firsti = primer índice con keys[i] >= begin
        int firsti = node->count;
        for (int i = 0; i < node->count; ++i) {
            if (node->keys[i] >= begin) {firsti = i; break; }
        }

        // lasti = primer índice con keys[i] > end
        int lasti = node->count;
        for (int i = 0; i < node->count; ++i) {
            if (node->keys[i] >= end) {lasti = i; break; }
        }
        //si todas las llaves son < begin, baja por el hijo más a la derecha
        if (firsti == node->count) {
            rangeSearchRecur(node->children[node->count], vec, begin, end);
        }

        rangeSearchRecur(node->children[firsti], vec, begin, end);
        for (int i = firsti; i < lasti; ++i) {
            vec.push_back(node->keys[i]);
            rangeSearchRecur(node->children[i+1], vec, begin, end);
        }
    }

public:
    TK minKey() {
        if (!root) throw std::runtime_error("minKey on empty tree");
        auto curr = root;
        while (!curr->leaf) curr = curr->children[0];
        return curr->keys[0];
    } // minimo valor de la llave en el arbol
    TK maxKey() {
        if (!root) throw std::runtime_error("maxKey on empty tree");
        auto curr = root;
        while (!curr->leaf) curr = curr->children[curr->count];
        return curr->keys[curr->count - 1];
    } // maximo valor de la llave en el arbol
    void clear() {
        if (root == nullptr) return;
        root->killSelf();
        root = nullptr;
        n = 0;
    } // eliminar todos lo elementos del arbol
    int size() { return n; } // retorna el total de elementos insertados

    // Construya un árbol B a partir de un vector de elementos ordenados
    template<typename T>
    static BTree *build_from_ordered_vector(vector<T> elements, int m) {
        auto *tree = new BTree<T>(m);
        if (elements.empty()) return tree;
        build_recursive<T>(tree, elements, 0, elements.size() - 1);
        return tree;
    }

private:
    template<typename T>
    static void build_recursive(BTree<T> *tree, vector<T> &vec, int left, int right) {
        if (left == right) {
            tree->insert(vec[left]);
            return;
        }
        if (right - left == 1) {
            tree->insert(vec[left]);
            tree->insert(vec[right]);
            return;
        }
        tree->insert(vec[(right + left) / 2]);
        build_recursive(tree, vec, left, (right + left) / 2 - 1);
        build_recursive(tree, vec, (right + left) / 2 + 1, right);
    }

public:
    // Verifique las propiedades de un árbol B
    bool check_properties() {
        if (!root) return true;
        int leaf_h = -1;
        return check_properties_node(root, nullopt, nullopt, leaf_h, 0);
    }

private:
    bool check_properties_node(
        Node<TK> *node,
        optional<TK>& bottom,
        optional<TK>& top,
        int &leaf_h,
        int height) {

        if (node == root && root->leaf) {
            TK rootprevkey;
            for (int i = 0; i < node->count; ++i) {
                if (i != 0 && !(rootprevkey < node->keys[i])) {
                    cout << "internal root keys not ordered\n";
                    return false;
                }
                rootprevkey = node->keys[i];
            }
            return true;
        }
        //compara height (altura propia) con globalheight (altura de la primera hoja encontrada)
        if (node->leaf && leaf_h != -1 && height != leaf_h) {
            cout << "different leaf heights\n";
            return false;
        }
        if (node->leaf && leaf_h == -1) leaf_h = height;

        TK prevkey;
        if (node != root && node->count < min_keys()) {
            cout << "node count too small\n";
            return false;
        }

        if (node->count > M - 1) {
            cout << "node count too large \n";
            return false;
        }

        // check unificado (todos los hijos posibles en un solo check)
        if (!node->leaf) {
            for (int i = 0; i <= node->count; ++i) {
                if (node->children[i] == nullptr) {
                    cout << "missing child pointer\n";
                    return false;
                }
            }
        }

        for (int i = 0; i < node->count; ++i) {
            //los valores de las llaves deben estar dentro de los valores posibles
            if ( (bottom && !(*bottom < node->keys[i])) ||
                 (top && !(node->keys[i] < *top)) ) {
                cout << "node values out of cap range\n";
                return false;
            }

            //el hijo izquierdo del nodo i debe cumplir con estar dentro de los valores y todas las propiedades (llamada recursiva)
            if (!node->leaf) {
                if (i == 0) {
                    if (!check_properties_node(node->children[i], bottom, node->keys[i],leaf_h, height + 1))
                        return false;
                } else {
                    if (!check_properties_node(node->children[i], node->keys[i - 1], node->keys[i], leaf_h, height + 1))
                        return false;
                }
            }
            //los elementos deben estar ordenados ascendentemente sin repetidos en un mismo nodo
            if (i != 0 && !(prevkey < node->keys[i])) {
                cout << "internal non-root node keys not ordered\n";
                return false;
            }
            prevkey = node->keys[i];
        }
        if (!node->leaf) {
            if (!check_properties_node(node->children[node->count], node->keys[node->count - 1], top, leaf_h, height + 1)) return false;
        }
        return true;
    }

public:
    ~BTree() {
        if (root != nullptr) root->killSelf();
        root = nullptr;
        n = 0;
    } // liberar memoria
};


#endif
