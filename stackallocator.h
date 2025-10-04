template <size_t N>
struct StackStorage {
    char arr[N];
    char* current = arr;
    size_t space = N;
    StackStorage() = default;
    StackStorage(const StackStorage& other) = delete;
    StackStorage& operator=(const StackStorage& other) = delete;
    ~StackStorage() = default;
};

template <typename T, size_t N>
struct StackAllocator {
    StackStorage<N>* storage = nullptr;
    using value_type = T;

    StackAllocator() = default;

    explicit StackAllocator(StackStorage<N>& storage) : storage(&storage) {}

    template <typename U>
    StackAllocator(const StackAllocator<U, N>& other) : storage(other.storage) {}

    template <typename U>
    StackAllocator& operator=(const StackAllocator<U, N>& other) {
        storage = other.storage;
        return *this;
    }

    template <typename U>
    bool operator==(const StackAllocator<U, N>& other) const {
        return storage == other.storage;
    }

    template <typename U>
    bool operator!=(const StackAllocator<U, N>& other) const {
        return !(storage == other.storage);
    }

    T* allocate(size_t number) {
        void* ptr = (storage->current);
        if (std::align(alignof(T), sizeof(T) * number, ptr, storage->space)){
            T* result = reinterpret_cast<T*>(ptr);
            storage->current = reinterpret_cast<char*>(ptr) + sizeof(T)*number;
            storage->space -= sizeof(T)*number;
            return result;
        }
        return nullptr;
    }

    void deallocate(T*, size_t) {}

    template <typename U>
    struct rebind {
        using other = StackAllocator<U, N>;
    };

    ~StackAllocator() = default;
};

template <typename T, typename Alloc = std::allocator<T>>
class List {

    struct Node;

    struct BaseNode {
        BaseNode* prev;
        BaseNode* next;

    };

    struct Node : BaseNode {
        T value;
        Node(const T& value) : value(value) {}
        Node() = default;
        ~Node() = default;
    };

    using nodeAlloc = std::allocator_traits<Alloc>::template rebind_alloc<Node>;
    nodeAlloc alloc;

    mutable BaseNode fakeNode{&fakeNode, &fakeNode};
    size_t count = 0;

    template <bool IsConst>
    class base_iterator {
    private:
        friend class List;
        BaseNode* node;

    public:

        using difference_type = std::ptrdiff_t;
        using value_type = std::conditional_t<IsConst, const T, T>;
        using pointer = std::conditional_t<IsConst, const T*, T*>;
        using reference = std::conditional_t<IsConst, const T&, T&>;
        using iterator_category = std::bidirectional_iterator_tag;

        base_iterator() = default;
        base_iterator(BaseNode* node) : node(node) {}
        base_iterator& operator=(const base_iterator& other) = default;
        base_iterator(const base_iterator& other) = default;
        pointer operator->() const { return node; }
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

        bool operator==(const base_iterator& other) const {
            return (node == other.node);
        }

        operator base_iterator<true>() const {
            return base_iterator<true>(this->node);
        }

        ~base_iterator() = default;
    };

public:


    using allocator_traits = std::allocator_traits<nodeAlloc>;


    Alloc get_allocator() { return alloc; }

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
    List(const Alloc& alloc) : alloc(alloc) {}

    List(size_t count, const Alloc& alloc = Alloc()) : List(alloc) {
        size_t index = 0;

        try {
            for (; index < count; ++index) {
                emplace_back();
            }

        } catch (...) {
            for (; index != 1; --index) {
                pop_back();
            }

            throw;
        }
    }
    List(size_t count, const T& val, const Alloc& alloc = Alloc()) : List(alloc) {
        size_t index = 0;

        try {
            for (; index < count; ++index) {
                push_back(val);
            }

        } catch (...) {
            for (; index != 1; --index) {
                pop_back();
            }

            throw;
        }
    }

    List(const List<T, Alloc>& another)
            : List(allocator_traits::select_on_container_copy_construction(
            another.alloc)) {
        Node* current = static_cast<Node*>(another.fakeNode.next);
        size_t index = 0;

        try {
            for (; index < another.size(); ++index) {
                push_back(current->value);
                current = static_cast<Node*>(current->next);
            }

        } catch (...) {
            for (; index != 1; --index) {
                pop_back();
            }

            throw;
        }
    }

    List& operator=(const List& other) {
        if (this == &other) {
            return *this;
        }

        List<T, Alloc> copy(other);

        if (allocator_traits::propagate_on_container_copy_assignment::value) {
            alloc = other.alloc;
        }

        else {
            alloc = allocator_traits::select_on_container_copy_construction(other.alloc);
        }

        std::swap(copy.fakeNode, fakeNode);
        std::swap(count, copy.count);

        return *this;
    }

    template <typename... Args>
    void emplace_back(const Args&... args) {
        Node* new_node = allocator_traits::allocate(alloc, 1);

        try {
            allocator_traits::construct(alloc, &new_node->value, args...);
        } catch (...) {
            throw;
        }

        if (count == 0) {
            fakeNode.prev = static_cast<BaseNode*>(new_node);
            fakeNode.next = static_cast<BaseNode*>(new_node);
            new_node->prev = &fakeNode;
        }

        else {
            fakeNode.prev->next = static_cast<BaseNode*>(new_node);
            new_node->prev = fakeNode.prev;
            fakeNode.prev = static_cast<BaseNode*>(new_node);
        }

        new_node->next = &fakeNode;
        ++count;
    }
    void push_back(const T& val){
        emplace_back(val);
    }

    void push_front(const T& val) {
        Node* new_node = allocator_traits::allocate(alloc, 1);

        try {
            allocator_traits::construct(alloc, &new_node->value, val);
        } catch (...) {
            throw;
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

    iterator insert(const_iterator iterator, const T& val) {
        Node* new_node = allocator_traits::allocate(alloc, 1);

        try {
            allocator_traits::construct(alloc, &new_node->value, val);
        } catch (...) {
            throw;
        }

        new_node->next = iterator.node;
        new_node->prev = iterator.node->prev;

        iterator.node->prev->next = static_cast<BaseNode*>(new_node);
        iterator.node->prev = static_cast<BaseNode*>(new_node);
        ++count;

        return {static_cast<BaseNode*>(new_node)};
    }

    iterator erase(const_iterator iterator) {

        Node* it = static_cast<Node*>(iterator.node->next);
        iterator.node->next->prev = iterator.node->prev;
        iterator.node->prev->next = iterator.node->next;

        allocator_traits::destroy(alloc,
                                  &(static_cast<Node*>(iterator.node)->value));

        allocator_traits::deallocate(alloc, static_cast<Node*>(iterator.node), 1);

        --count;

        return {static_cast<BaseNode*>(it)};
    }

    ~List() {
        while (count > 0) {
            pop_back();
        }
    }
};