#include <iostream>
#include <vector>

template <typename T>
class Deque {
private:
    static const size_t kBucketSize = 32;
    std::vector<T*> buckets_;
    std::pair<size_t, size_t> begin_{0, 0};
    std::pair<size_t, size_t> end_{0, 0};

    void reserve(size_t newsz_) {
        if (newsz_ <= buckets_.size()) {
            return;
        }

        std::vector<T*> newbuckets_(newsz_);

        if (buckets_.size() == 0) {
            for (size_t i = 0; i < newsz_; ++i) {
                newbuckets_[i] = reinterpret_cast<T*>(new char[kBucketSize * sizeof(T)]);
            }
        }

        else {
            size_t centrebegin_ = (newsz_ - buckets_.size()) / 2;

            for (size_t index = 0; index < newsz_; ++index) {
                newbuckets_[index] =
                        (index >= centrebegin_ && index < centrebegin_ + buckets_.size())
                        ? buckets_[index - centrebegin_]
                        : reinterpret_cast<T*>(new char[kBucketSize * sizeof(T)]);
            }

            begin_.first += centrebegin_;
            end_.first += centrebegin_;
        }

        std::swap(buckets_, newbuckets_);
    }

    void backward(std::pair<size_t, size_t>& coordinates, bool IsEnd) {
        if (coordinates.second + static_cast<size_t>(!IsEnd) != kBucketSize) {
            ++coordinates.second;
        }

        else {
            coordinates.second = static_cast<size_t>(IsEnd);
            ++coordinates.first;
        }
    }

    void forward(std::pair<size_t, size_t>& coordinates, bool IsEnd) {
        if (IsEnd && coordinates.second == 1 && size() == 1) {
            --coordinates.second;
            return;
        }

        if (coordinates.second != static_cast<size_t>(IsEnd)) {
            --coordinates.second;
        }

        else {
            coordinates.second = kBucketSize - 1 + static_cast<size_t>(IsEnd);
            --coordinates.first;
        }
    }

    template <bool IsConst>
    class base_iterator {
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = std::conditional_t<IsConst, const T, T>;
        using pointer = std::conditional_t<IsConst, const T*, T*>;
        using reference = std::conditional_t<IsConst, const T&, T&>;
        using iterator_category = std::random_access_iterator_tag;
        using vector =
                std::conditional_t<IsConst, const std::vector<T*>*, std::vector<T*>*>;

    private:
        vector buckets_ = nullptr;
        size_t out_index;
        size_t in_index;
        pointer bucket_ = nullptr;

    public:
        base_iterator() = default;
        base_iterator(vector buckets_, size_t out_index, size_t in_index)
                : buckets_(buckets_),
                  out_index(out_index),
                  in_index(in_index),
                  bucket_((*buckets_)[out_index]) {}
        base_iterator& operator=(const base_iterator& other) = default;
        base_iterator(const base_iterator& other) = default;

        pointer operator->() const { return bucket_ + in_index; }

        reference operator*() const { return *(bucket_ + in_index); }

        base_iterator& operator+=(difference_type number) {
            if (buckets_ != nullptr && bucket_ != nullptr) {
                difference_type new_index = kBucketSize * out_index + in_index;
                new_index += number;
                in_index = new_index % kBucketSize;
                out_index = new_index / kBucketSize;
                bucket_ = (*buckets_)[out_index];
            }
            return *this;
        }

        base_iterator& operator-=(difference_type number) {
            *this += (-number);
            return *this;
        }

        base_iterator& operator++() {
            *this += 1;
            return *this;
        }

        base_iterator& operator--() {
            *this += (-1);
            return *this;
        }

        base_iterator operator++(int) {
            base_iterator copy = *this;
            ++*this;
            return copy;
        }

        bool operator==(const base_iterator& other) const {
            return (out_index == other.out_index && in_index == other.in_index);
        }

        auto operator<=>(const base_iterator& other) const {
            if (out_index != other.out_index) {
                return out_index <=> other.out_index;
            }

            return in_index <=> other.in_index;
        }

        friend difference_type operator-(const base_iterator& first,
                                         const base_iterator& second) {
            return static_cast<difference_type>(first.out_index * kBucketSize +
                                                first.in_index) -
                   static_cast<difference_type>(second.out_index * kBucketSize +
                                                second.in_index);
        }

        friend base_iterator operator+(const base_iterator& first,
                                       difference_type number) {
            base_iterator copy = first;
            copy += number;
            return copy;
        }

        friend base_iterator operator-(const base_iterator& first,
                                       difference_type number) {
            base_iterator copy = first;
            copy -= number;
            return copy;
        }

        friend base_iterator operator+(difference_type number,
                                       const base_iterator& first) {
            base_iterator copy = first;
            copy += number;
            return copy;
        }

        friend base_iterator operator-(difference_type number,
                                       const base_iterator& first) {
            base_iterator copy = first;
            copy -= number;
            return copy;
        }

        operator base_iterator<true>() const { return *this; }

        ~base_iterator() = default;
    };

public:
    Deque() = default;

    Deque(const Deque<T>& another)
            : buckets_(another.buckets_.size()),
              begin_(another.begin_),
              end_(another.begin_) {
        for (size_t index = 0; index < another.buckets_.size(); ++index) {
            buckets_[index] = reinterpret_cast<T*>(new char[kBucketSize * sizeof(T)]);
        }
        size_t index = 0;
        try {
            for (; index < another.size(); ++index) {
                backward(end_, true);
                new (buckets_[end_.first] + end_.second - 1) T(another[index]);
            }

        } catch (...) {
            for (size_t i = 0; i < index; ++i) {
                (buckets_[end_.first] + end_.second - 1)->~T();
                forward(end_, true);
            }

            for (size_t index = 0; index < another.buckets_.size(); ++index) {
                delete[] reinterpret_cast<char*>(buckets_[index]);
            }

            throw;
        }
    }

    Deque(size_t number, const T& value) {
        reserve(number / kBucketSize + 1);

        try {
            for (size_t i = 0; i < number; ++i) {
                backward(end_, true);
                new (buckets_[end_.first] + end_.second - 1) T(value);
            }

        } catch (...) {
            size_t sz = size();

            for (size_t i = 0; i < sz - 1; ++i) {
                (buckets_[end_.first] + end_.second - 1)->~T();
                forward(end_, true);
            }

            for (size_t i = 0; i < buckets_.size(); ++i) {
                delete[] reinterpret_cast<char*>(buckets_[i]);
            }

            end_ = begin_;

            std::vector<T*> newbuckets_;

            std::swap(newbuckets_, buckets_);

            throw;
        }
    }

    Deque(int number) : Deque(number, T()) {}

    Deque& operator=(const Deque& deque) {
        try {
            Deque copy(deque);
            std::swap(copy.buckets_, buckets_);
            std::swap(copy.begin_, begin_);
            std::swap(copy.end_, end_);
        } catch (...) {
            throw;
        }

        return *this;
    }

    size_t size() const {
        return (kBucketSize * (end_.first - begin_.first) - begin_.second +
                end_.second);
    }

    T& operator[](size_t index) {
        return buckets_[begin_.first + (index + begin_.second) / kBucketSize]
        [(begin_.second + index) % kBucketSize];
    }

    const T& operator[](size_t index) const {
        return buckets_[begin_.first + (index + begin_.second) / kBucketSize]
        [(begin_.second + index) % kBucketSize];
    }

    T& at(size_t index) {
        if (index >= size()) {
            throw std::out_of_range("out of range");
        }

        return this->operator[](index);
    }

    const T& at(size_t index) const {
        if (index >= size()) {
            throw std::out_of_range("out of range");
        }

        return this->operator[](index);
    }

    void push_back(const T& element) {
        if (buckets_.size() == 0) {
            *this = Deque(1, element);
            return;
        }

        if (end_.first + 1 == buckets_.size() && end_.second == kBucketSize) {
            reserve(3 * buckets_.size());
        }

        backward(end_, true);
        try {
            new (buckets_[end_.first] + end_.second - 1) T(element);

        } catch (...) {
            forward(end_, true);
            throw;
        }
    }

    void push_front(const T& element) {
        if (size() == 0) {
            push_back(element);
            return;
        }

        if (begin_.first == 0 && begin_.second == 0) {
            reserve(3 * buckets_.size());
        }

        forward(begin_, false);

        try {
            new (buckets_[begin_.first] + begin_.second) T(element);

        } catch (...) {
            backward(begin_, false);
            throw;
        }
    }

    void pop_back() {
        (buckets_[end_.first] + end_.second - 1)->~T();
        forward(end_, true);
    }

    void pop_front() {
        (buckets_[begin_.first] + begin_.second)->~T();
        backward(begin_, false);
    }

    using iterator = base_iterator<false>;
    using const_iterator = base_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin() {
        if (buckets_.size() == 0) {
            return iterator();
        }

        return iterator(&buckets_, begin_.first, begin_.second);
    }

    const_iterator begin() const {
        if (buckets_.size() == 0) {
            return const_iterator();
        }

        return const_iterator(&buckets_, begin_.first, begin_.second);
    }

    const_iterator end() const {
        if (buckets_.size() == 0) {
            return const_iterator();
        }

        return const_iterator(&buckets_, end_.first, end_.second);
    }

    iterator end() {
        if (buckets_.size() == 0) {
            return iterator();
        }

        return iterator(&buckets_, end_.first, end_.second);
    }

    const_iterator cbegin() const {
        if (buckets_.size() == 0) {
            return const_iterator();
        }

        return const_iterator(&buckets_, begin_.first, begin_.second);
    }

    const_iterator cend() const {
        if (buckets_.size() == 0) {
            return const_iterator();
        }

        return const_iterator(&buckets_, end_.first, end_.second);
    }

    reverse_iterator rbegin() {
        if (buckets_.size() == 0) {
            return reverse_iterator();
        }

        return reverse_iterator(end());
    }

    reverse_iterator rend() {
        if (buckets_.size() == 0) {
            return reverse_iterator();
        }

        return reverse_iterator(begin());
    }

    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(cend());
    }

    const_reverse_iterator crend() const {
        return const_reverse_iterator(cbegin());
    }

    void insert(iterator iter, const T& val) {
        size_t index = iter - begin();
        iterator i;
        try {
            push_back(val);
            iterator new_iterator = iterator(
                    &buckets_, begin_.first + (index + begin_.second) / kBucketSize,
                    (index + begin_.second) % kBucketSize);
            i = end() - 1;
            for (; i != new_iterator; --i) {
                std::swap(*i, *(i - 1));
            }

        } catch (...) {
            for (;i!=end()-1;++i) {
                std::swap(*i,*(i+1));
            }
            pop_back();
            throw;
        }
    }

    void erase(iterator iter) {
        iterator i = iter;
        T cp = *iter;
        try {
            for (; i + 1 != end(); ++i) {
                *i = *(i + 1);
            }
            pop_back();

        } catch (...) {
            for (;i-1 != iter; --i) {
                *i = *(i-1);
            }
            *iter = cp;
            throw;
        }
    }

    ~Deque() {
        size_t sz = size();
        for (size_t i = 0; i < sz; ++i) {
            (buckets_[begin_.first] + begin_.second)->~T();
            backward(begin_, false);
        }

        for (size_t i = 0; i < buckets_.size(); ++i) {
            delete[] reinterpret_cast<char*>(buckets_[i]);
        }
    }
};
