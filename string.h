#include <cstring>
#include <iostream>

class String {

private:

    char* arr = nullptr;
    size_t sz_ = 0;
    size_t cap_ = 0;

    explicit String(size_t count) : arr(new char[count + 1]), sz_(count), cap_(count) {}

    void swap(String& other) {
        std::swap(arr, other.arr);
        std::swap(sz_, other.sz_);
        std::swap(cap_, other.cap_);
    }

    char* resize(char* arr,size_t new_cap){
        char* new_arr = new char[new_cap + 1];
        memcpy(new_arr, arr, sz_);
        delete[] arr;
        return new_arr;
    }

    size_t reverse_find(const String& other,bool reverse = false) const{

        if (other.sz_>sz_){
            return sz_;
        }

        for (size_t i = 0; i < sz_-other.sz_+1;++i){
            size_t index = reverse ? sz_-other.sz_-i : i;

            if (memcmp(arr+index,other.arr,other.sz_)==0){
                return index;
            }

        }

        return sz_;
    }

public:

    String() : arr(new char[1]) { arr[0] = '\0'; }

    String(size_t count, char c) : String(count) {
        memset(arr, c, count);
        arr[sz_] = '\0';
    }

    String(const char* str) : String(strlen(str)) {
        memcpy(arr, str, sz_);
        arr[sz_] = '\0';
    }

    String(const String& other) : String(other.sz_) {
        memcpy(arr, other.arr, sz_);
        arr[sz_] = '\0';
    }

    String& operator=(const String& other) {

        if (this==&other){
            return *this;
        }

        String new_str(other);
        swap(new_str);
        return *this;
    }

    char& operator[](size_t index) { return arr[index]; }

    const char& operator[](size_t index) const { return arr[index]; }

    size_t size() const { return sz_; }
    size_t length() const { return sz_; }
    size_t capacity() const { return cap_; }

    void push_back(char c) {

        if (sz_ == cap_) {

            cap_ *= 2;
            cap_ += 1;
            arr = resize(arr,cap_);

        }

        ++sz_;
        arr[sz_ - 1] = c;
        arr[sz_] = '\0';
    }

    size_t find(const String& other) const {
        return reverse_find(other);
    }

    size_t rfind(const String& other) const {
        return reverse_find(other,true);
    }

    String substr(size_t st, size_t count) const {
        size_t new_sz = 0;
        new_sz = st + count > sz_ ? sz_ - st : count;
        String str(new_sz);
        memcpy(str.arr, arr + st, str.sz_);
        str.arr[str.sz_] = '\0';
        return str;
    }

    void pop_back() {
        --sz_;
        arr[sz_] = '\0';
    }

    const char& front() const { return arr[0]; }

    char& front() { return arr[0]; }

    char* data() { return arr; }

    const char* data() const { return arr; }

    const char& back() const { return arr[sz_ - 1]; }

    char& back() { return arr[sz_ - 1]; }

    String& operator+=(const String& other) {

        if (cap_ < sz_ + other.sz_) {
            arr = resize(arr,sz_ + other.sz_);
        }

        memcpy(arr + sz_, other.arr, other.sz_);
        sz_ += other.sz_;
        arr[sz_] = '\0';
        return *this;
    }

    String& operator+=(char c) {
        push_back(c);
        return *this;
    }

    bool empty() const { return (sz_ == 0); }

    void clear() {
        sz_ = 0;
        arr[sz_] = '\0';
    }

    void shrink_to_fit() {
        cap_ = sz_;
        arr = resize(arr,cap_);
    }

    friend bool operator==(const String& str1, const String& str2);

    friend bool operator<(const String& str1, const String& str2);

    ~String() { delete[] arr; }

};

std::ostream& operator<<(std::ostream& out, const String& str) {
    out << str.data();
    return out;
}

std::istream& operator>>(std::istream& in, String& str) {
    str.clear();
    char c;

    while (in.get(c)) {

        if (isspace(c)) {

            if (str.empty()) {
                continue;
            }

            break;
        }
        str += c;
    }

    return in;
}

bool operator==(const String& str1, const String& str2) {

    if (str1.sz_ != str2.sz_) {
        return false;
    }

    for (size_t i = 0; i < str1.sz_; ++i) {

        if (str1.arr[i] != str2.arr[i]) {
            return false;
        }

    }

    return true;
}

bool operator!=(const String& str1, const String& str2) {
    return !(str1 == str2);
}

bool operator<(const String& str1, const String& str2) {
    size_t i = 0;

    while (i < str1.sz_ && i < str2.sz_) {

        if (str1.arr[i] != str2.arr[i]) {
            return (str1.arr[i] < str2.arr[i]);
        }

        ++i;
    }
    return (str1.sz_ < str2.sz_);
}

bool operator>(const String& str1, const String& str2) {
    return str2 < str1;
}

bool operator<=(const String& str1, const String& str2) {
    return !(str1 > str2);
}

bool operator>=(const String& str1, const String& str2) {
    return !(str1 < str2);
}

String operator+(const String& str1, const String& str2) {
    String new_str = str1;
    new_str += str2;
    return new_str;
}

String operator+(const String& str1, char str2) {
    String new_str = str1;
    new_str += str2;
    return new_str;
}

String operator+(char str1, const String& str2) {
    String new_str;
    new_str += str1;
    new_str += str2;
    return new_str;
}

