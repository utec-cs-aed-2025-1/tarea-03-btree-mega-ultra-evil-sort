#ifndef BTree_H
#define BTree_H
#include <iostream>
#include <vector>
#include <stack>
#include "node.h"

using namespace std;

template <typename TK>
class BTree {
 private:
  Node<TK>* root;
  int M;  // grado u orden del arbol
  int n; // total de elementos en el arbol

 public:
  BTree(int _M) : root(nullptr), M(_M) {}

  bool search(TK key) {
   if (root == nullptr) return false;
   //if (!root->leaf) {
    //std::cout << "=====\nrootchildren firstkeys: ";
    //for (int i=0; i<=root->count; ++i) std::cout << root->children[i]/*->keys[0]*/ << " ";
    //std::cout << "\nroot address: " << root;
    //std::cout << "\n=====\n";
   //}
   auto curr = root;
   while (!curr->leaf) {
    //std::cout << "searching at node with first key " << curr->keys[0] << "\n";
    for (int i=0; i<curr->count; ++i) {
     //std::cout << "checking address " << curr->children[i] << "\n";
     if (curr->keys[i] == key) return true;
     if (curr->keys[i] > key) {curr=curr->children[i]; /*std::cout << "going through child " << i << " at address " << curr << "\n";*/ break;}
     if (i >= curr->count-1) {curr=curr->children[i+1]; /*std::cout << "going through child " << i+1 << " at address " << curr << "\n";*/ break;}
    }
    //std::cout << "keepgoing\n";
   }
   //std::cout << "searching at node with first key " << curr->keys[0] << "\n";
   for (int i=0; i<curr->count; ++i) if (curr->keys[i] == key) return true;
   return false;


  }//indica si se encuentra o no un elemento
  void insert(TK key) {
   //std::cout << "starting insertion of key " << key << "\n";
   if (root == nullptr) {
    root = new Node<TK>(M);
    root->count = 1; root->keys[0] = key; root->leaf = true; return;
   }
   //std::cout << "root first val: " << root->keys[0] <<  " root is leaf? ";
   //std::cout << std::boolalpha << root->leaf << "\n";
   std::stack<Node<TK>*> stack;
   auto curr = root;
   while (!curr->leaf) {
    //std::cout << "not reached the bottom yet\n";
    stack.push(curr);
    //std::cout << "curr->count: " << curr->count << "\n";
    for (int i=0; i<curr->count; ++i) {
     //std::cout << "starting arraycheck\n";
     if (curr->keys[i] == key) return;
     if (curr->keys[i] > key) {curr=curr->children[i]; break;}
     if (i >= curr->count-1) {curr=curr->children[i+1]; break;}
     //std::cout << "endingarraycheck\n";
    }
   } stack.push(curr);
   //std::cout << "stack size: " << stack.size() << "\n";
   //std::cout << "successful search of leaf node for key" << key << "\n";
   bool splitfinished = false;
   TK keytoadd = key;
   TK* newKeys = new TK[M];
   Node<TK>** newChildren = new Node<TK>*[M+1];
   Node<TK>* lastNewNode = nullptr;
   bool leaflevel = true;
   while (!stack.empty() && !splitfinished) {
    curr = stack.top();

    if (curr->count == M-1) {
     //std::cout << "starting backtrack at node with first key " << curr->keys[0] << "\n";
     auto* newNode = new Node<TK>(M);
     newNode->count = M-1-((M-1)/2);
     //newNode->leaf = leaflevel;
     newNode->leaf = curr->leaf;
     if (leaflevel) leaflevel = false;
     newChildren[0] = curr->children[0];
     int j=0; bool keyadded = false;
     //crear arrays temporales
     for (int i=0; i<M; ++i) {
      if (j >= curr->count || (keytoadd < curr->keys[j] && !keyadded)) {newKeys[i] = keytoadd; newChildren[i+1] = lastNewNode; keyadded = true;}
      else {newKeys[i] = curr->keys[j]; newChildren[i+1] = curr->children[j+1]; ++j;}
     }
     keytoadd = newKeys[(M-1)/2];
     //revisar cantidades de keys y children, posiblemente sean equívocas
     //copiado de valores de arrays temporales a nodos
     for (int i=0; i<(M-1)/2; ++i) {curr->keys[i] = newKeys[i];}
     for (int i=0; i<newNode->count; ++i) {newNode->keys[i] = newKeys[i+1+(M-1)/2];}
     if (!curr->leaf) {
      for (int i=0; i<(M+1)/2; ++i) {curr->children[i] = newChildren[i];}
      for (int i=(M+1)/2; i<M+1; ++i) {newNode->children[i-(M+1)/2] = newChildren[i];}
     } curr->count = (M-1)/2;
     for (int i=(M-1)/2+1; i<M; ++i) curr->children[i] = nullptr;
     for (int i=(M-1)/2; i<M-1; ++i) curr->keys[i] = 0;
     lastNewNode = newNode;
     stack.pop();
     //std::cout << "ended backtrack at node with first key " << curr->keys[0] << "\n";
    } else {
     //insertar normalmente
     int indextoinsert = curr->count;
     for (int i=0; i<curr->count; ++i) {if (keytoadd < curr->keys[i]) {indextoinsert = i; break;}}
     for (int i=curr->count; i>indextoinsert; --i) {
      curr->keys[i] = curr->keys[i-1];
      curr->children[i+1] = curr->children[i];
     }
     curr->keys[indextoinsert] = keytoadd;
     curr->children[indextoinsert+1] = lastNewNode;
     ++curr->count;
     splitfinished = true;
     //std::cout << "successful normal insertion of node " << key << " in node of first key " << curr->keys[0] << "\n";
    }
    ++n;
   }
   if (!splitfinished) {
    //crear nuevo nodo raíz
    Node<TK>* newRoot = new Node<TK>(M);
    newRoot->count = 1;
    newRoot->keys[0] = keytoadd;
    newRoot->children[1] = lastNewNode;
    newRoot->children[0] = root;
    newRoot->leaf = false;
    root = newRoot;
    //std::cout << "tree grew up\n";
   } //std::cout << "ending insertion of key " << key << "\n";
  }//inserta un elemento
  void remove(TK key);//elimina un elemento
  int height() {
   int counter = 0;
   auto curr = root; while (curr) {curr = curr->children[0]; ++counter;}
   return counter;
  }//altura del arbol. Considerar altura 0 para arbol vacio
 std::string keytostr(TK key) {
   if (std::is_same_v<TK, std::string>) return key;
   return std::to_string(key);
  }
  /*string toString(const string& sep) {

  }  // recorrido inorder
  string toString_inorder(const string& sep) {
   std::stack<Node<TK>*> stack;
   std::string output = "";
   auto curr = root;
   while (!stack.empty()) {
    while (!curr->leaf) {stack.push(curr); curr = curr->left;}
     for (int i=0; i<curr->count; ++i) output += curr->keys[i] + sep;



   }
  }*/

 string toString(const string& sep) {
    return toStringRecur(root, sep, false);
  }  // recorrido inorder
  string toStringRecur(Node<TK>* node, const string& sep, bool lastsep) {
   if (node->leaf) {
    std::string outputleaf = "";
    for (int i=0; i<node->count; ++i) {
     outputleaf += keytostr(node->keys[i]);
     if (lastsep) outputleaf += sep;
    }
    return outputleaf;
   }
   std::string output = "";
   for (int i=0; i<node->count; ++i) {
    output += toStringRecur(node->children[i], sep, true);
    output += keytostr(node->keys[i]) + sep;
   } output += toStringRecur(node->children[node->count], sep, lastsep);
   return output;
  }


  vector<TK> rangeSearch(TK begin, TK end) {
    vector<TK> output;
    rangeSearchRecur(root, output, begin, end);
   return output;
  }
  void rangeSearchRecur(Node<TK>* node, vector<TK>& vec, TK begin, TK end) {
   if (node->leaf) {
    for (int i=0; i<node->count; ++i) if (node->keys[i] >= begin && node->keys[i] <= end) vec.push_back(node->keys[i]);
    return;
   }
   //fijar i's de comienzo y final de los nodos a retornar.
   int firsti = -1; int lasti = -1;
   for (int i=0; i<node->count; ++i) {
    if (begin < node->keys[i]) firsti = i;
    if (end < node->keys[i] && lasti == -1) lasti = i;
   }
   if (firsti != -1 && lasti == -1) lasti = node->count;
   //si ninguna de las llaves está en el rango, añade lo que pueda del hijo mayor.
   //si este no tiene nada simplemente no lo añadirá ya que solo se añaden elementos entre begin y end
   if (firsti == -1) {
    if (node->keys[node->count-1] == begin) vec.push_back(node->keys[node->count-1]);
     rangeSearchRecur(node->children[node->count], vec, begin, end); return;
   }
   //realiza la inserción en el vector de los valores de los nodos e hijos entre begin y end. incluye las llaves
   //exteriores en caso sean iguales a begin y end
   if (firsti != 0 && node->keys[firsti-1] == begin) vec.push_back(node->keys[firsti-1]);
   for (int i=firsti; i<lasti; ++i) {
    rangeSearchRecur(node->children[i], vec, begin, end);
    vec.push_back(node->keys[i]);
   } rangeSearchRecur(node->children[lasti], vec, begin, end);
   if (lasti != node->count && node->keys[lasti+1] == end) vec.push_back(node->keys[lasti+1]);
  }

  TK minKey() {
   auto curr = root;
   while (!curr->leaf) curr = curr->children[0];
   return curr->keys[0];
  }  // minimo valor de la llave en el arbol
  TK maxKey() {
   auto curr = root;
   while (!curr->leaf) curr = curr->children[curr->count];
   return curr->keys[curr->count-1];
  }  // maximo valor de la llave en el arbol
  void clear() {root->killSelf();} // eliminar todos lo elementos del arbol
  int size() {return n;} // retorna el total de elementos insertados
  
  // Construya un árbol B a partir de un vector de elementos ordenados
 template<typename T>
  static BTree* build_from_ordered_vector(vector<T> elements, int m) {

  }
  // Verifique las propiedades de un árbol B
  bool check_properties() {
   int globalheight = -1;
   return check_properties_node(root, -1, -1, false, false, globalheight, 0);
  }

 //falta verificar que no haya dos valores repetidos en el árbol en nodos diferentes, la verificación
  bool check_properties_node(Node<TK>* node, int bottomcap, int topcap, bool usebottomcap, bool usetopcap,
   int& globalheight, int height) {
   //casos de si hay 0 nodos o 1 nodo
   if (root == nullptr) return true;
   if (node == nullptr) return false;
   if (node == root && root->leaf) {
    TK rootprevkey;
    for (int i=0; i<node->count; ++i) {
     if (i != 0 && rootprevkey >= node->keys[i]) return false;
     rootprevkey = node->keys[i];
    } return true;
   }
   //compara height (altura propia) con globalheight (altura de la primera hoja encontrada)
   if (node->leaf && globalheight != -1 && height != globalheight) return false;
   if (node->leaf && globalheight == -1) globalheight = height;

   TK prevkey;
   if (node->count < (M-1)/2) return false;
   for (int i=0; i<node->count; ++i) {
    //los valores de las llaves deben estar dentro de los valores posibles
    if ((node->keys[i] >= topcap && usetopcap) || (node->keys[i] <= bottomcap && usebottomcap)) return false;
    //debe haber todos los hijos posibles
    if (!node->leaf && node->children[i] == nullptr) return false;
    //el hijo izquierdo del nodo i debe cumplir con estar dentro de los valores y todas las propiedades (llamada recursiva)
    if (i == 0) {
     if (!check_properties_node(node->children[i], bottomcap, node->keys[i], usebottomcap, true,
      globalheight, height+1)) return false;
    } else {
     if (!check_properties_node(node->children[i], node->keys[i-1], node->keys[i], true, true,
      globalheight, height+1)) return false;
    }
    //los elementos deben estar ordenados ascendentemente sin repetidos en un mismo nodo
    if (i != 0 && prevkey >= node->keys[i]) return false;
    prevkey = node->keys[i];
   }
   if (!node->leaf) {
    //verificación de que cumpla el último hijo
    if (node->children[node->count] == nullptr) return false;
    if (!check_properties_node(node->children[node->count], node->keys[node->count-1], topcap, true, usetopcap, globalheight, height+1)) return false;
    //fuerza a que el resto de hijos después de count+1 sean nullptr (se puede quitar)
    for (int i=node->count+1; i<M; ++i) {
     if (node->children[i] != nullptr) return false;
    }
   }
   return true;
  }

  ~BTree() {
   if (root != nullptr) root->killSelf();
  }     // liberar memoria
};

#endif
