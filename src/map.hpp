/**
* implement a container like std::map
*/
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
   class Key,
   class T,
   class Compare = std::less<Key>
   > class map {
  public:
   typedef pair<const Key, T> value_type;

  private:
   struct Node {
     value_type data;
     bool color; // true = RED, false = BLACK
     Node *left, *right, *parent;
     Node(const value_type &d)
         : data(d), color(true), left(nullptr), right(nullptr), parent(nullptr) {}
   };

   Node *root = nullptr;
   size_t node_count = 0;
   Compare comp;

   // helpers
   static bool is_red(Node *x) { return x && x->color; }
   static bool is_black(Node *x) { return !x || !x->color; }

   static Node *min_node(Node *x) {
     if (!x) return nullptr;
     while (x->left) x = x->left;
     return x;
   }
   static Node *max_node(Node *x) {
     if (!x) return nullptr;
     while (x->right) x = x->right;
     return x;
   }

   bool eq_key(const Key &a, const Key &b) const {
     return !comp(a, b) && !comp(b, a);
   }

   Node *find_node(const Key &key) const {
     Node *cur = root;
     while (cur) {
       if (comp(key, cur->data.first)) cur = cur->left;
       else if (comp(cur->data.first, key)) cur = cur->right;
       else return cur;
     }
     return nullptr;
   }

   void left_rotate(Node *x) {
     Node *y = x->right; // must exist
     x->right = y->left;
     if (y->left) y->left->parent = x;
     y->parent = x->parent;
     if (!x->parent) root = y;
     else if (x == x->parent->left) x->parent->left = y;
     else x->parent->right = y;
     y->left = x;
     x->parent = y;
   }

   void right_rotate(Node *x) {
     Node *y = x->left; // must exist
     x->left = y->right;
     if (y->right) y->right->parent = x;
     y->parent = x->parent;
     if (!x->parent) root = y;
     else if (x == x->parent->right) x->parent->right = y;
     else x->parent->left = y;
     y->right = x;
     x->parent = y;
   }

   void insert_fix(Node *z) {
     while (z->parent && z->parent->color) { // parent red
       Node *p = z->parent;
       Node *g = p->parent;
       if (p == g->left) {
         Node *u = g->right; // uncle
         if (is_red(u)) {
           p->color = false; u->color = false; g->color = true; z = g;
         } else {
           if (z == p->right) { z = p; left_rotate(z); p = z->parent; g = p->parent; }
           p->color = false; g->color = true; right_rotate(g);
         }
       } else {
         Node *u = g->left;
         if (is_red(u)) {
           p->color = false; u->color = false; g->color = true; z = g;
         } else {
           if (z == p->left) { z = p; right_rotate(z); p = z->parent; g = p->parent; }
           p->color = false; g->color = true; left_rotate(g);
         }
       }
     }
     if (root) root->color = false;
   }

   void transplant(Node *u, Node *v) {
     if (!u->parent) root = v;
     else if (u == u->parent->left) u->parent->left = v;
     else u->parent->right = v;
     if (v) v->parent = u->parent;
   }

   void erase_fix(Node *x, Node *x_parent) {
     while (x != root && is_black(x)) {
       if (x == (x_parent ? x_parent->left : nullptr)) {
         Node *w = x_parent ? x_parent->right : nullptr;
         if (is_red(w)) { // case 1
           w->color = false;
           if (x_parent) { x_parent->color = true; left_rotate(x_parent); }
           w = x_parent ? x_parent->right : nullptr;
         }
         if (is_black(w ? w->left : nullptr) && is_black(w ? w->right : nullptr)) { // case 2
           if (w) w->color = true;
           x = x_parent;
           x_parent = x ? x->parent : nullptr;
         } else {
           if (is_black(w ? w->right : nullptr)) { // case 3
             if (w && w->left) w->left->color = false;
             if (w) { w->color = true; right_rotate(w); }
             w = x_parent ? x_parent->right : nullptr;
           }
           if (w) w->color = x_parent ? x_parent->color : false;
           if (x_parent) x_parent->color = false;
           if (w && w->right) w->right->color = false;
           if (x_parent) left_rotate(x_parent);
           x = root;
           x_parent = nullptr;
         }
       } else {
         Node *w = x_parent ? x_parent->left : nullptr;
         if (is_red(w)) {
           w->color = false;
           if (x_parent) { x_parent->color = true; right_rotate(x_parent); }
           w = x_parent ? x_parent->left : nullptr;
         }
         if (is_black(w ? w->right : nullptr) && is_black(w ? w->left : nullptr)) {
           if (w) w->color = true;
           x = x_parent;
           x_parent = x ? x->parent : nullptr;
         } else {
           if (is_black(w ? w->left : nullptr)) {
             if (w && w->right) w->right->color = false;
             if (w) { w->color = true; left_rotate(w); }
             w = x_parent ? x_parent->left : nullptr;
           }
           if (w) w->color = x_parent ? x_parent->color : false;
           if (x_parent) x_parent->color = false;
           if (w && w->left) w->left->color = false;
           if (x_parent) right_rotate(x_parent);
           x = root;
           x_parent = nullptr;
         }
       }
     }
     if (x) x->color = false;
   }

   void clear_node(Node *x) {
     if (!x) return;
     clear_node(x->left);
     clear_node(x->right);
     delete x;
   }

   Node *clone_subtree(Node *parent, Node *other) {
     if (!other) return nullptr;
     Node *x = new Node(other->data);
     x->color = other->color;
     x->parent = parent;
     x->left = clone_subtree(x, other->left);
     x->right = clone_subtree(x, other->right);
     ++node_count;
     return x;
   }

  public:
   class const_iterator;
   class iterator {
      friend class map;
      friend class const_iterator;
     private:
      map *owner = nullptr;
      Node *cur = nullptr;

      iterator(map *o, Node *c) : owner(o), cur(c) {}

      static Node *next_node(Node *x) {
        if (!x) return nullptr;
        if (x->right) return min_node(x->right);
        Node *p = x->parent;
        while (p && x == p->right) { x = p; p = p->parent; }
        return p;
      }
      static Node *prev_node(Node *x) {
        if (!x) return nullptr;
        if (x->left) return max_node(x->left);
        Node *p = x->parent;
        while (p && x == p->left) { x = p; p = p->parent; }
        return p;
      }

     public:
      iterator() = default;
      iterator(const iterator &other) = default;

      iterator operator++(int) {
        iterator tmp = *this;
        ++(*this);
        return tmp;
      }
      iterator &operator++() {
        if (!owner) throw invalid_iterator();
        if (!cur) throw invalid_iterator(); // ++ on end()
        Node *nxt = next_node(cur);
        cur = nxt;
        return *this;
      }
      iterator operator--(int) {
        iterator tmp = *this;
        --(*this);
        return tmp;
      }
      iterator &operator--() {
        if (!owner) throw invalid_iterator();
        if (!cur) { // --end() => last element if not empty
          if (!owner->root) throw invalid_iterator();
          cur = max_node(owner->root);
          return *this;
        }
        Node *prv = prev_node(cur);
        if (!prv) throw invalid_iterator(); // --begin()
        cur = prv;
        return *this;
      }

      value_type &operator*() const {
        if (!cur) throw invalid_iterator();
        return cur->data;
      }
      value_type *operator->() const noexcept {
        return &cur->data;
      }

      bool operator==(const iterator &rhs) const {
        return owner == rhs.owner && cur == rhs.cur;
      }
      bool operator==(const const_iterator &rhs) const { return owner == rhs.owner && cur == rhs.cur; }
      bool operator!=(const iterator &rhs) const { return !(*this == rhs); }
      bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }
   };

   class const_iterator {
      friend class map;
     private:
      const map *owner = nullptr;
      Node *cur = nullptr;

      const_iterator(const map *o, Node *c) : owner(o), cur(c) {}

      static Node *next_node(Node *x) { return iterator::next_node(x); }
      static Node *prev_node(Node *x) { return iterator::prev_node(x); }

     public:
      const_iterator() = default;
      const_iterator(const const_iterator &other) = default;
      const_iterator(const iterator &other) {
        owner = other.owner;
        cur = other.cur;
      }

      const_iterator operator++(int) {
        const_iterator tmp = *this;
        ++(*this);
        return tmp;
      }
      const_iterator &operator++() {
        if (!owner) throw invalid_iterator();
        if (!cur) throw invalid_iterator();
        cur = next_node(cur);
        return *this;
      }
      const_iterator operator--(int) {
        const_iterator tmp = *this;
        --(*this);
        return tmp;
      }
      const_iterator &operator--() {
        if (!owner) throw invalid_iterator();
        if (!cur) {
          if (!owner->root) throw invalid_iterator();
          cur = max_node(owner->root);
          return *this;
        }
        Node *prv = prev_node(cur);
        if (!prv) throw invalid_iterator();
        cur = prv;
        return *this;
      }

      const value_type &operator*() const {
        if (!cur) throw invalid_iterator();
        return cur->data;
      }
      const value_type *operator->() const noexcept { return &cur->data; }

      bool operator==(const const_iterator &rhs) const {
        return owner == rhs.owner && cur == rhs.cur;
      }
      bool operator==(const iterator &rhs) const {
        return rhs == *this;
      }
      bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }
      bool operator!=(const iterator &rhs) const { return !(rhs == *this); }
   };

   map() = default;

   map(const map &other) : root(nullptr), node_count(0), comp(other.comp) {
     root = clone_subtree(nullptr, other.root);
   }

   map &operator=(const map &other) {
     if (this == &other) return *this;
     clear();
     comp = other.comp;
     root = clone_subtree(nullptr, other.root);
     return *this;
   }

   ~map() { clear(); }

   T &at(const Key &key) {
     Node *x = find_node(key);
     if (!x) throw index_out_of_bound();
     return x->data.second;
   }
   const T &at(const Key &key) const {
     Node *x = find_node(key);
     if (!x) throw index_out_of_bound();
     return x->data.second;
   }

   T &operator[](const Key &key) {
     Node *cur = root, *parent = nullptr;
     while (cur) {
       parent = cur;
       if (comp(key, cur->data.first)) cur = cur->left;
       else if (comp(cur->data.first, key)) cur = cur->right;
       else return cur->data.second;
     }
     Node *z = new Node(value_type(key, T()));
     z->parent = parent;
     if (!parent) root = z;
     else if (comp(z->data.first, parent->data.first)) parent->left = z;
     else parent->right = z;
     ++node_count;
     insert_fix(z);
     return z->data.second;
   }

   const T &operator[](const Key &key) const { return at(key); }

   iterator begin() { return iterator(this, min_node(root)); }
   const_iterator cbegin() const { return const_iterator(this, min_node(root)); }

   iterator end() { return iterator(this, nullptr); }
   const_iterator cend() const { return const_iterator(this, nullptr); }

   bool empty() const { return node_count == 0; }
   size_t size() const { return node_count; }

   void clear() {
     clear_node(root);
     root = nullptr;
     node_count = 0;
   }

   pair<iterator, bool> insert(const value_type &value) {
     Node *cur = root, *parent = nullptr;
     while (cur) {
       parent = cur;
       if (comp(value.first, cur->data.first)) cur = cur->left;
       else if (comp(cur->data.first, value.first)) cur = cur->right;
       else return pair<iterator, bool>(iterator(this, cur), false);
     }
     Node *z = new Node(value);
     z->parent = parent;
     if (!parent) root = z;
     else if (comp(z->data.first, parent->data.first)) parent->left = z;
     else parent->right = z;
     ++node_count;
     insert_fix(z);
     return pair<iterator, bool>(iterator(this, z), true);
   }

   void erase(iterator pos) {
     if (pos.owner != this || pos.cur == nullptr) throw invalid_iterator();
     Node *z = pos.cur;

     Node *y = z;
     bool y_original_color = y->color;
     Node *x = nullptr; // the node that moves into y's position
     Node *x_parent = nullptr;

     if (!z->left) {
       x = z->right;
       x_parent = z->parent;
       transplant(z, z->right);
     } else if (!z->right) {
       x = z->left;
       x_parent = z->parent;
       transplant(z, z->left);
     } else {
       y = min_node(z->right); // successor
       y_original_color = y->color;
       x = y->right;
       if (y->parent == z) {
         x_parent = y;
         if (x) x->parent = y;
       } else {
         x_parent = y->parent;
         transplant(y, y->right);
         y->right = z->right;
         if (y->right) y->right->parent = y;
       }
       transplant(z, y);
       y->left = z->left;
       if (y->left) y->left->parent = y;
       y->color = z->color;
     }

     delete z;
     --node_count;

     if (!y_original_color) erase_fix(x, x_parent);
     if (root) root->color = false;
   }

   size_t count(const Key &key) const { return find_node(key) ? 1 : 0; }

   iterator find(const Key &key) { return iterator(this, find_node(key)); }
   const_iterator find(const Key &key) const { return const_iterator(this, find_node(key)); }
};

}

#endif