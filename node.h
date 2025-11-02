#ifndef NODE_H
#define NODE_H

using namespace std;

template <typename TK>
struct Node {
  // array de keys
  TK* keys;
  // array de punteros a hijos
  Node** children;
  // cantidad de keys
  int count;
  // indicador de nodo hoja
  bool leaf;

  Node() : keys(nullptr), children(nullptr), count(0) {}
  Node(int M) {
    keys = new TK[M - 1]();
    for (int i=0; i<M-1; ++i) keys[i] = 0;
    children = new Node<TK>*[M]();
    for (int i=0; i<M; ++i) children[i] = nullptr;
    count = 0;
    leaf = true;
  }

  void killSelf() {
    for (int i=0; i<count; ++i) {delete children[i]; children[i]=nullptr;}
    delete[] children; children = nullptr;
    delete[] keys; keys = nullptr;
    delete this;
  }
};

#endif