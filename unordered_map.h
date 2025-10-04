#include <iostream>
#include <memory>
#include <vector>




template <typename Key, typename Value, typename Hash = std::hash<Key>,
          typename Equal = std::equal_to<Key>,
          typename Alloc = std::allocator<std::pair<const Key, Value>>>
class UnorderedMap {

 private:

  template <typename T,typename Allocator = Alloc>
  struct List {

    struct Node;

    struct BaseNode {
      BaseNode* prev;
      BaseNode* next;
      bool operator==(const BaseNode& other) const = default;
    };

    struct Node : BaseNode {
      T value;
      Node(const T& value) : value(value) {}
      Node(T&& value) : value(std::move(value)) {}
      Node() = default;
      ~Node() = default;
    };

    mutable BaseNode fakeNode{&fakeNode, &fakeNode};
    size_t count = 0;

    template <bool IsConst>
    struct base_iterator {
      using difference_type = std::ptrdiff_t;
      using value_type = std::conditional_t<IsConst, const T, T>;
      using pointer = std::conditional_t<IsConst, const T*, T*>;
      using reference = std::conditional_t<IsConst, const T&, T&>;
      using iterator_category = std::bidirectional_iterator_tag;

      BaseNode* node;

      base_iterator() = default;

      base_iterator(BaseNode* node) : node(node) {}
      base_iterator& operator=(const base_iterator& other) = default;
      base_iterator(const base_iterator& other) = default;
      base_iterator(base_iterator&& other) = default;
      base_iterator& operator=(base_iterator&& other) = default;
      pointer operator->() const { return &static_cast<Node*>(node)->value; }
      reference operator*() const { return static_cast<Node*>(node)->value; }

      base_iterator& operator++() {
        node = node->next;
        return *this;
      }

      base_iterator& operator--() {
        node = node->prev;
        return *this;
      }

      base_iterator operator++(int) {
        base_iterator copy = *this;
        ++*this;
        return copy;
      }

      base_iterator operator--(int) {
        base_iterator copy = *this;
        --*this;
        return copy;
      }

      operator base_iterator<true>() const {
        return base_iterator<true>(this->node);
      }

      ~base_iterator() = default;
    };

    friend bool operator==(const base_iterator<true>& first,
                           const base_iterator<true>& second) {
      return (first.node == second.node);
    }

    friend bool operator!=(const base_iterator<true>& first,
                           const base_iterator<true>& second) {
      return first.node != second.node;
    }


    using nodeAlloc =
        std::allocator_traits<Allocator>::template rebind_alloc<Node>;

    using allocator_traits = std::allocator_traits<nodeAlloc>;

    [[no_unique_address]] nodeAlloc alloc;

    nodeAlloc get_allocator() { return alloc; }

    using iterator = base_iterator<false>;
    using const_iterator = base_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin() { return iterator((fakeNode.next)); }

    const_iterator begin() const { return const_iterator((fakeNode.next)); }

    iterator end() { return iterator(&fakeNode); }

    const_iterator end() const { return const_iterator(&fakeNode); }

    const_iterator cbegin() const { return const_iterator(fakeNode.next); }

    const_iterator cend() const { return const_iterator(&fakeNode); }

    reverse_iterator rbegin() { return reverse_iterator(end()); }

    reverse_iterator rend() { return reverse_iterator(begin()); }

    const_reverse_iterator rbegin() const {
      return const_reverse_iterator(cend());
    }

    const_reverse_iterator rend() const {
      return const_reverse_iterator(cbegin());
    }

    const_reverse_iterator crbegin() const {
      return const_reverse_iterator(cend());
    }

    const_reverse_iterator crend() const {
      return const_reverse_iterator(cbegin());
    }

    List() = default;
    List(const Allocator& alloc) : alloc(alloc) {}

    List(size_t count, const Allocator& alloc = Allocator()) : List(alloc) {
      size_t index = 0;
      for (; index < count; ++index) {
        push_back();
      }
    }

    List(size_t count, const T& val, const Allocator& alloc = Allocator())
        : List(alloc) {
      size_t index = 0;

      for (; index < count; ++index) {
        push_back(val);
      }

    }

    List(const List& another)
        : List(allocator_traits::select_on_container_copy_construction(
              another.alloc)) {
      Node* current = static_cast<Node*>(another.fakeNode.next);
      size_t index = 0;

      for (; index < another.size(); ++index) {
        push_back(current->value);
        current = static_cast<Node*>(current->next);
      }

    }

    List(List&& another) {

      alloc = std::move(another.alloc);
      if (another.count == 0) {
        return;
      }

      fakeNode.next = another.fakeNode.next;
      fakeNode.prev = another.fakeNode.prev;

      another.fakeNode.next->prev = &fakeNode;
      another.fakeNode.prev->next = &fakeNode;

      another.fakeNode.prev = &another.fakeNode;
      another.fakeNode.next = &another.fakeNode;

      count = std::move(another.count);
    }


    List& operator=(const List& other) {

      if (this == &other) {
        return *this;
      }

      List<T, Allocator> copy(other);

      if (allocator_traits::propagate_on_container_copy_assignment::value) {
        alloc = other.alloc;
      }

      else {
        alloc =
            allocator_traits::select_on_container_copy_construction(other.alloc);
      }

      fakeNode = copy.fakeNode;
      copy.fakeNode.next = nullptr;
      copy.fakeNode.prev = nullptr;

      std::swap(count, copy.count);

      return *this;
    }

    List& operator=(List&& another) noexcept {
      if (this==&another){
        return *this;
      }
      while (count != 0) {
        pop_back();
      }
      if (allocator_traits::propagate_on_container_move_assignment::value &&
          alloc != another.alloc) {
        alloc = std::move(another.alloc);
      }

      else {
        alloc = another.alloc;
      }

      if (another.count == 0) {
        return *this;
      }

      fakeNode.next = another.fakeNode.next;
      fakeNode.prev = another.fakeNode.prev;

      another.fakeNode.next->prev = &fakeNode;
      another.fakeNode.prev->next = &fakeNode;

      another.fakeNode.prev = &another.fakeNode;
      another.fakeNode.next = &another.fakeNode;

      count = another.count;
      another.count = 0;
      return *this;

    }

    template <typename... Args>
    void emplace_back(Args&&... args) {
      Node* new_node = allocator_traits::allocate(alloc, 1);

      try {
        allocator_traits::construct(alloc, &new_node->value,
                                    std::forward<Args>(args)...);
      }

      catch(...){
        allocator_traits::deallocate(alloc,new_node,1);
      }

      if (count == 0) {
        fakeNode.prev = static_cast<BaseNode*>(new_node);
        fakeNode.next = static_cast<BaseNode*>(new_node);
        new_node->prev = &fakeNode;
      } else {
        fakeNode.prev->next = static_cast<BaseNode*>(new_node);
        new_node->prev = fakeNode.prev;
        fakeNode.prev = static_cast<BaseNode*>(new_node);
      }

      new_node->next = &fakeNode;
      ++count;
    }

    void push_back(T&& val) { emplace_back(std::move(val)); }
    void push_back(const T& val) { emplace_back(val); }

    void push_back() {
      emplace_back();
    }

    template <typename... Args>
    void emplace_front(Args&&... args) {
      Node* new_node = allocator_traits::allocate(alloc, 1);

      try {
        allocator_traits::construct(alloc, &new_node->value,
                                    std::forward<Args>(args)...);
      }

      catch(...){
        allocator_traits::deallocate(alloc,new_node,1);
      }

      if (count == 0) {
        fakeNode.prev = static_cast<BaseNode*>(new_node);
        fakeNode.next = static_cast<BaseNode*>(new_node);
        new_node->next = &fakeNode;
      }

      else {
        fakeNode.next->prev = static_cast<BaseNode*>(new_node);
        new_node->next = fakeNode.next;
        fakeNode.next = static_cast<BaseNode*>(new_node);
      }

      new_node->prev = &fakeNode;
      ++count;
    }

    void push_front(T&& val) { emplace_front(std::move(val)); }

    void push_front(const T& val) { emplace_front(val); }

    size_t size() const { return count; }

    void pop_back() {
      Node* delete_node = static_cast<Node*>(fakeNode.prev);
      fakeNode.prev = delete_node->prev;

      if (count == 1) {
        fakeNode.next = &fakeNode;
      }

      else {
        fakeNode.prev->next = &fakeNode;
      }

      allocator_traits::destroy(alloc, &delete_node->value);
      allocator_traits::deallocate(alloc, delete_node, 1);
      --count;
    }

    void pop_front() {
      Node* delete_node = static_cast<Node*>(fakeNode.next);
      fakeNode.next = delete_node->next;

      if (count == 1) {
        fakeNode.prev = &fakeNode;
      }

      else {
        fakeNode.next->prev = &fakeNode;
      }

      allocator_traits::destroy(alloc, &delete_node->value);
      allocator_traits::deallocate(alloc, delete_node, 1);
      --count;
    }



    iterator erase(const_iterator iter) {
      Node* it = static_cast<Node*>(iter.node->next);
      cut_node(iter);
      destroy_node(iter);
      return {static_cast<BaseNode*>(it)};
    }


    ~List() {
      while (count > 0) {
        pop_back();
      }
    }

    template <typename... Args>
    Node* create_node(Args&&... args) {
      Node* new_node = allocator_traits::allocate(alloc, 1);

      try {
        allocator_traits::construct(alloc, &new_node->value,
                                    std::forward<Args>(args)...);
      }

      catch (...) {
        throw;
      }

      return new_node;
    }

    void insert_node(const_iterator iterator, Node* new_node) {

      if (iterator == end()) {
        new_node->next = iterator.node;
        new_node->prev = iterator.node;
        iterator.node->next = static_cast<BaseNode*>(new_node);
        iterator.node->prev = static_cast<BaseNode*>(new_node);
      }
      else {
        new_node->next = iterator.node;
        new_node->prev = iterator.node->prev;

        iterator.node->prev->next = static_cast<BaseNode*>(new_node);
        iterator.node->prev = static_cast<BaseNode*>(new_node);
      }

      ++count;
    }

    template <typename... Args>
    iterator move_insert(const_iterator iterator, Args&&... args) {
      Node* new_node = create_node(std::forward<Args>(args)...);
      insert_node(iterator, new_node);
      return {static_cast<BaseNode*>(new_node)};
    }

    iterator insert(const_iterator iterator, T&& val) {
      return move_insert(iterator, std::move(val));
    }

    iterator insert(const_iterator iterator, const T& val) {
      return move_insert(iterator, val);
    }

    void cut_node(const_iterator iter) {
      iter.node->next->prev = iter.node->prev;
      iter.node->prev->next = iter.node->next;
      --count;
    }

    void destroy_node(const_iterator iter) {
      allocator_traits::destroy(alloc, &(static_cast<Node*>(iter.node)->value));
      allocator_traits::deallocate(alloc, static_cast<Node*>(iter.node), 1);
    }

  };


  struct Node {
    static Hash hash;
    std::pair<const Key, Value> key_value;
    size_t hash_element;

    template <typename... Args>
    Node(Args&&... args)
        : key_value{std::forward<Args>(args)...}, hash_element(hash(key_value.first)) {}

    Node& operator=(const Node& other) = default;
    Node& operator=(Node&& other) = default;

    Node(const Node& other) = default;
    Node(Node&& other) = default;

    Node() = default;
    ~Node() = default;
  };

  using allocNodetype =
      typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
  using allocNodeiter =
      typename std::allocator_traits<Alloc>::template rebind_alloc<
          typename List<Node, allocNodetype>::iterator>;
  using allocator_traits_nodetype = std::allocator_traits<allocNodetype>;


  template <bool IsConst>
  struct base_iterator {
    using iter =
        std::conditional_t<IsConst,
                           typename List<Node, allocNodetype>::const_iterator,
                           typename List<Node, allocNodetype>::iterator>;

    iter it;
    using difference_type = std::ptrdiff_t;
    using value_type =
        std::conditional_t<IsConst, const std::pair<const Key, Value>,
                           std::pair<const Key, Value>>;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::forward_iterator_tag;
    pointer operator->() const { return &it->key_value; }
    reference operator*() const { return it->key_value; }

    base_iterator& operator++() {
      ++it;
      return *this;
    }

    base_iterator operator++(int) {
      base_iterator it2{it};
      ++it;
      return it2;
    }

    operator base_iterator<true>() const { return base_iterator<true>(it); }

  };

  friend bool operator==(const base_iterator<true>& first,
                         const base_iterator<true>& second) {
    return first.it == second.it;
  }

  friend bool operator!=(const base_iterator<true>& first,
                         const base_iterator<true>& second) {
    return !(first == second);
  }

  static Equal equal;
  static Hash hash;
  [[no_unique_address]] allocNodetype alloc;
  std::vector<typename List<Node, allocNodetype>::iterator, allocNodeiter> hashmap;
  List<Node, allocNodetype> list;
  double max_load_factor = 1;


 public:

  using NodeType = std::pair<const Key, Value>;
  using iterator = base_iterator<false>;
  using const_iterator = base_iterator<true>;

  double load_factor() const { return list.size() / hashmap.size(); }
  double max_factor() const { return max_load_factor; }
  void set_max_load_factor(double max_load) { max_load_factor = max_load; }

  void reserve(size_t count) {
    if (static_cast<double>(count) / hashmap.size() <= max_load_factor) {
      return;
    }
    rehash(count);
  }

  void rehash(size_t count) {
    if (count < size() / max_load_factor) {
      count = size() / max_load_factor;
    }

    std::vector<typename List<Node, allocNodetype>::iterator, allocNodeiter>
        hashmap_new(count, list.end());
    List<Node, allocNodetype> list_new;
    auto temp_element = list.begin();

    for (; temp_element != list.end();) {
      auto temp = temp_element;
      ++temp_element;
      list.cut_node(temp);

      if (hashmap_new[temp->hash_element % count] != list.end()) {
        list_new.insert_node(
            hashmap_new[temp->hash_element % count],
            static_cast<List<Node, allocNodetype>::Node*>(temp.node));
      }

      else {
        list_new.insert_node(
            list_new.begin(),
            static_cast<List<Node, allocNodetype>::Node*>(temp.node));
      }

      hashmap_new[temp->hash_element % count] = temp;
    }

    list = std::move(list_new);
    hashmap = std::move(hashmap_new);
  }

  UnorderedMap(const Alloc& alloc = Alloc())
      : alloc(alloc), hashmap(alloc), list(alloc) {
    reserve(1);
  }

  UnorderedMap(const UnorderedMap& other)
      : alloc(allocator_traits_nodetype::select_on_container_copy_construction(
            other.alloc)),
        hashmap(other.hashmap),
        list(other.list) {}

  UnorderedMap(UnorderedMap&& other) noexcept {
    alloc = other.alloc;
    hashmap = std::move(other.hashmap);
    list = std::move(other.list);
  }

  UnorderedMap& operator=(const UnorderedMap& other) {
    if (this==&other){
      return *this;
    }
    hashmap = other.hashmap;
    list = other.list;
    if (allocator_traits_nodetype::propagate_on_container_copy_assignment::
            value) {
      alloc = other.alloc;
    }
    return *this;
  }

  UnorderedMap& operator=(UnorderedMap&& other) noexcept {
    if (this==&other){
      return *this;
    }
    if (allocator_traits_nodetype::propagate_on_container_move_assignment::
            value) {
      alloc = std::move(other.alloc);
    }
    hashmap = std::move(other.hashmap);
    list = std::move(other.list);
    rehash(hashmap.size());
    return *this;
  }

  iterator begin() { return {list.begin()}; }
  const_iterator begin() const { return {list.cbegin()}; }
  iterator end() { return {list.end()}; }
  const_iterator end() const { return {list.cend()}; }
  const_iterator cbegin() const { return {list.cbegin()}; }
  const_iterator cend() const { return {list.cend()}; }

  iterator find(const Key& key) {
      return help_find(key);
  }

  const_iterator find(const Key& key) const {
    return help_find(key);
  }



  template <typename... Args>
  std::pair<iterator, bool> emplace(Args&&... args) {


    Node* new_node = allocator_traits_nodetype::allocate(alloc, 1);
    try {
      allocator_traits_nodetype::construct(alloc, &new_node->key_value,
                                           std::forward<Args>(args)...);
    }
    catch(...){
      allocator_traits_nodetype::deallocate(alloc, new_node, 1);
      throw;
    }

    new_node->hash_element = hash(new_node->key_value.first);
    if (load_factor() >= max_load_factor) {
      rehash(2 * hashmap.size());
    }
    list.move_insert(list.begin(),
                     std::move(const_cast<Key&>(new_node->key_value.first)),
                     std::move(new_node->key_value.second));
    list.begin()->hash_element = new_node->hash_element;


    auto it = list.begin();
    auto find_element = find_hash_value(new_node);
    allocator_traits_nodetype::destroy(alloc, &new_node->key_value);
    allocator_traits_nodetype::deallocate(alloc, new_node, 1);

    if (find_element != end()) {
      list.pop_front();
      return {find_element, false};
    }

    else {
      hashmap[it->hash_element % hashmap.size()] = list.begin();
      return {{it}, true};
    }

  }

  std::pair<iterator, bool> insert(const NodeType& node) {
    return emplace(node);
  }

  std::pair<iterator, bool> insert(NodeType&& node) {
    return emplace(std::move(const_cast<Key&>(node.first)),
                   std::move(node.second));
  }

  template <typename InputIt>
  void insert(InputIt first, InputIt last) {
    for (auto it = first; it != last; ++it) {
      insert(*it);
    }
  }

  Value& at(const Key& key) {
    auto it = find(key);
    if (it == end()) {
      throw std::out_of_range("");
    }
    return it->second;
  }

  const Value& at(const Key& key) const {
    auto it = find(key);
    if (it == end()) {
      throw std::out_of_range("");
    }
    return it->key_value.second;
  }

  Value& operator[](const Key& key) {
    auto find_element = find(key);
    if (find_element == end()) {
      auto it = insert({key, Value()}).first;
      return it.it->key_value.second;
    }
    return find_element->second;
  }

  const Value& operator[](const Key& key) const { return find(key)->second; }

  const_iterator erase(const_iterator it) {
    auto next = it;
    ++next;
    if (list.size()==1){
      hashmap[it.it->hash_element % hashmap.size()] = list.end();
    }
    else if (it.it == hashmap[it.it->hash_element % hashmap.size()]) {
      typename List<Node, allocNodetype>::iterator next_it(it.it.node);
      ++next_it;
      if (next_it==list.end()||next_it->hash_element % hashmap.size() !=
                                       it.it->hash_element % hashmap.size()||next_it==list.end()) {
        hashmap[it.it->hash_element % hashmap.size()] = list.end();
      }

      else {
        hashmap[it.it->hash_element % hashmap.size()] = next_it;
      }
    }
    list.erase(it.it);
    return next;
  }
  const_iterator erase(const_iterator first, const_iterator second) {
    while (first != second) {
      first = erase(first);
    }
    return first;
  }

  size_t size() const { return list.size(); }

  ~UnorderedMap() = default;

 private:
  iterator find_hash_value(Node* new_node) {

    auto it = hashmap[new_node->hash_element % hashmap.size()];
    while (it.node != list.end().node && it.node != NULL) {

      if (it->hash_element % hashmap.size() !=
          new_node->hash_element % hashmap.size()) {
        return end();
      }

      if (equal(new_node->key_value.first, it->key_value.first)) {
        return {it};
      }

      ++it;
    }
    return end();
  }

  auto help_find(const Key& key){
    if (list.size() == 0) {
      return end();
    }

    size_t element_hash = hash(key) % hashmap.size();
    auto it = hashmap[element_hash];
    while (it != end().it) {

      if (it->hash_element % hashmap.size() != element_hash) {
        return end();
      }

      if (equal(key, it->key_value.first)) {
        return iterator {it};
      }

      ++it;
    }
    return end();
  }
};


template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
Equal UnorderedMap<Key,Value,Hash,Equal,Alloc>::equal{};

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
Hash UnorderedMap<Key,Value,Hash,Equal,Alloc>::hash{};

template <typename Key, typename Value, typename Hash, typename Equal,
          typename Alloc>
Hash UnorderedMap<Key,Value,Hash,Equal,Alloc>::Node::hash{};

