#include <compare>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>


enum class Sign{
    negative,
    positive
};


class BigInteger {
private:
    static const int base_ = 1e9;
    static const int nine_ = 9;
    Sign sign_;
    std::vector<int> arr_;

    friend std::istream& operator>>(std::istream& in, BigInteger& number);
    friend std::ostream& operator<<(std::ostream& out, const BigInteger& number);
    friend BigInteger& operator/=(BigInteger& first, const BigInteger& second);
    friend BigInteger gcd(BigInteger first, BigInteger second);
    int bin_search(const BigInteger& second);
    friend class Rational;

    void clear_zero(){
        while (arr_.back() == 0 && arr_.size() > 1) {
            arr_.pop_back();
        }
    }

public:

    BigInteger() = default;

    explicit BigInteger(long long unsigned number) : sign_(Sign::positive) {
        while (number>=base_){
            arr_.push_back(number%base_);
            number/=base_;
        }
        if (number!=0){
            arr_.push_back(number);
        }

    }

    BigInteger(int integer) : sign_((integer >= 0 ? Sign::positive : Sign::negative)) {

        if (!static_cast<bool>(sign_)) {
            integer *= -1;
        }

        while (integer >= base_) {
            arr_.push_back(integer%base_);
            integer/=base_;
        }

       if (integer!=0||arr_.empty()){
           arr_.push_back(integer);
       }

    }

    BigInteger& operator+=(const BigInteger& other) {
        size_t end;
        if (arr_.size() < other.arr_.size()) {
            arr_.resize(other.arr_.size());
            end = arr_.size();
        }

        else {
            end = other.arr_.size();
        }

        if (sign_ == other.sign_) {

            for (size_t i = 0; i < end; ++i) {
                arr_[i] += other.arr_[i];
            }

            for (size_t i = 0; i < arr_.size() - 1; ++i) {

                if (arr_[i] >= base_) {
                    ++arr_[i + 1];
                    arr_[i] %= base_;
                }

            }

            if (arr_[arr_.size() - 1] >= base_) {
                arr_[arr_.size() - 1] %= base_;
                arr_.push_back(1);
            }

        }

        else {

            for (size_t i = 0; i < end; ++i) {
                arr_[i] -= other.arr_[i];
            }

            for (size_t i = 0; i < arr_.size() - 1; ++i) {

                if (arr_[i] < 0) {
                    arr_[i] += base_;
                    --arr_[i + 1];
                }

            }

        }
        int index = arr_.size() - 1;

        if (arr_[index] < 0) {

            sign_ = static_cast<bool>(sign_) ? Sign::negative : Sign::positive;
            arr_[index] *= -1;

            for (int i = index - 1; i >= 0; --i) {

                if (arr_[i] != 0) {
                    --arr_[i + 1];
                    arr_[i] -= base_;
                    arr_[i] *= -1;
                }

            }

        }
        clear_zero();
        if (arr_[arr_.size() - 1] == 0) {
            sign_ = Sign::positive;
        }
        return *this;
    }

    BigInteger& operator-=(const BigInteger& other) {
        sign_ = (static_cast<bool>(sign_)) ? Sign::negative : Sign::positive;
        *this+=other;
        sign_ = (static_cast<bool>(sign_)&&arr_[arr_.size()-1]!=0) ? Sign::negative : Sign::positive;
        return *this;
    }

    BigInteger& operator*=(const BigInteger& other) {

        sign_ = (static_cast<bool>(sign_) == static_cast<bool>(other.sign_)) ? Sign::positive : Sign::negative;
        long long number = 1;
        std::vector<long long> new_arr(arr_.size() + other.arr_.size() + 1);

        for (size_t i = 0; i < arr_.size(); ++i) {

            for (size_t j = 0; j < other.arr_.size(); ++j) {
                number = arr_[i];
                number *= other.arr_[j];
                new_arr[i + j] += (number % base_);
                new_arr[i + j + 1] += (number / base_);
                number = 1;
            }

        }

        arr_.resize(new_arr.size());

        for (size_t i = 0; i < arr_.size(); ++i) {

            if (new_arr[i] >= base_) {
                new_arr[i + 1] += new_arr[i] / base_;
                arr_[i] = (new_arr[i] % base_);
            }

            else {
                arr_[i] = new_arr[i];
            }

        }

        clear_zero();

        if (arr_[arr_.size() - 1] == 0) {
            sign_ = Sign::positive;
        }

        return *this;
    }

    BigInteger operator-() const {
        BigInteger result = *this;

        if (result == 0) {
            result.sign_ = Sign::positive;
        }

        else {
            result.sign_ = sign_==Sign::positive ? Sign::negative : Sign::positive;;
        }

        return result;
    }

    BigInteger& operator++() {
        *this += 1;
        return *this;
    }

    BigInteger operator++(int) {
        BigInteger result = *this;
        ++(*this);
        return result;
    }

    BigInteger& operator--() {
        *this -= 1;
        return *this;
    }

    BigInteger operator--(int) {
        BigInteger result = *this;
        --(*this);
        return result;
    }

    explicit operator bool() const { return (arr_[arr_.size() - 1] != 0); }

    std::string toString() const {
        std::string str;

        if (!static_cast<bool>(sign_)) {
            str += "-";
        }

        str += std::to_string(arr_[arr_.size() - 1]);
        int st = arr_.size() - 2;

        for (int i = st; i >= 0; --i) {
            std::string arr_i = std::to_string(arr_[i]);
            int count_of_null = nine_ - arr_i.size();
            str += std::string(count_of_null, '0');
            str += arr_i;
        }

        return str;
    }

    std::weak_ordering operator<=>(const BigInteger& other) const {
        if (sign_!=other.sign_){
            return (sign_ <=> other.sign_);
        }

        if (arr_.size() != other.arr_.size()) {

            return (static_cast<bool>(sign_)) ? arr_.size() <=> other.arr_.size() : other.arr_.size() <=> arr_.size();

        }

        for (int i = arr_.size() - 1; i >= 0; --i) {

            if (arr_[i] != other.arr_[i]) {

                return (static_cast<bool>(sign_)) ? arr_[i] <=> other.arr_[i] : other.arr_[i] <=> arr_[i];

            }

        }
        return std::weak_ordering::equivalent;

    }
    bool operator==(const BigInteger& second) const {
        if (sign_ == second.sign_ && arr_.size() == second.arr_.size()) {
            int sz = arr_.size();

            for (int i = sz - 1; i >= 0; --i) {

                if (arr_[i] != second.arr_[i]) {
                    return false;
                }

            }

            return true;
        }
        return false;
    }

    ~BigInteger() = default;
};

BigInteger operator""_bi(long long unsigned number) {
    return number;
}

BigInteger operator*(const BigInteger& first, const BigInteger& second) {
    BigInteger new_first = first;
    new_first *= second;
    return new_first;
}

std::ostream& operator<<(std::ostream& out, const BigInteger& number) {
    std::string str = number.toString();
    out << str;
    return out;
}

std::istream& operator>>(std::istream& in, BigInteger& number) {
    number.arr_.clear();
    std::string str;
    in >> str;
    if (str[0] != '-') {
        number.sign_ = Sign::positive;
    } else {
        number.sign_ = Sign::negative;
    }
    std::string s;
    int sz = str.size();
    int end = (static_cast<bool>(number.sign_)) ? 0 : 1;

    for (int i = sz - 1; i >= end; i--) {

        if (s.size() < BigInteger::nine_) {
            s += str[i];
        }

        else {
            reverse(s.begin(), s.end());
            number.arr_.push_back(std::stoi(s));
            s = str[i];
        }

    }

    if (!s.empty()) {
        reverse(s.begin(), s.end());
        number.arr_.push_back(std::stoi(s));
    }
    return in;
}

BigInteger operator+(const BigInteger& first, const BigInteger& second) {
    BigInteger result = first;
    result += second;
    return result;
}

int BigInteger::bin_search(const BigInteger& second) {
    int st = -1;
    int end = BigInteger::base_;
    int middle;
    BigInteger mult;

    while ((end - st) > 1) {
        middle = (st + end) / 2;
        mult = second;
        mult *= middle;
        if (mult > *this) {
            end = middle;
        }

        else {
            st = middle;
        }
    }

    return st;
}
BigInteger& operator/=(BigInteger& first, const BigInteger& second) {
    int index = first.arr_.size() - 1;
    int search_index;
    BigInteger temp = first.arr_[index];
    BigInteger answ = 0;
    BigInteger second_change = second;
    second_change.sign_ = Sign::positive;

    while (index >= 0) {
        search_index = temp.bin_search(second_change);
        answ += search_index;
        temp -= search_index * second_change;

        if (index > 0) {
            --index;
            answ *= BigInteger::base_;
            temp *= BigInteger::base_;
            temp += first.arr_[index];
        }

        else {
            --index;
        }
    }
    if (answ == 0) {
        first = answ;
        first.sign_ = Sign::positive;
    }

    else {
        answ.sign_ = (static_cast<bool>(first.sign_) == static_cast<bool>(second.sign_)) ? Sign::positive : Sign::negative;
        first = answ;
    }

    return first;
}

BigInteger operator/(const BigInteger& first, const BigInteger& second) {
    BigInteger temp = first;
    temp /= second;
    return temp;
}

BigInteger& operator%=(BigInteger& first, const BigInteger& second) {
    first -= (first / second) * second;
    return first;
}

BigInteger gcd(BigInteger first, BigInteger second) {
    first.sign_ = Sign::positive;
    second.sign_ = Sign::positive;

    while (second) {
        first %= second;
        std::swap(first, second);
    }

    if (!static_cast<bool>(first.sign_)) {
        first.sign_ = Sign::positive;
    }

    return first;
}

BigInteger operator-(const BigInteger& first, const BigInteger& second) {
    BigInteger result = first;
    result -= second;
    return result;
}

BigInteger operator%(const BigInteger& first, const BigInteger& second) {
    BigInteger result = first;
    result %= second;
    return result;
}

class Rational {

private:
    static const int decimal_const = 15;
    BigInteger dividend;
    BigInteger divisor;

public:

    Rational() = default;

    Rational(const BigInteger& first, const BigInteger& second) {
        dividend = first;
        divisor = second;

        if (!static_cast<bool>(divisor.sign_)) {
            divisor.sign_ = (static_cast<bool>(divisor.sign_)) ? Sign::negative : Sign::positive;
            dividend.sign_ = (static_cast<bool>(dividend.sign_)) ? Sign::negative : Sign::positive;
        }

        BigInteger nod = gcd(first, second);
        divisor /= nod;
        dividend /= nod;
    }

    Rational(const BigInteger& number) {
        dividend = number;
        divisor = 1;
    }

    Rational(int number) {
        dividend = number;
        divisor = 1;
    }

    Rational& operator+=(const Rational& other) {
        dividend *= other.divisor;
        dividend += (other.dividend * divisor);
        divisor *= other.divisor;
        BigInteger nod = gcd(dividend, divisor);
        divisor /= nod;
        dividend /= nod;
        return *this;
    }

    Rational& operator-=(const Rational& other) {
        this->dividend.sign_ = (static_cast<bool>(this->dividend.sign_)) ? Sign::negative : Sign::positive;
        *this+=other;
        this->dividend.sign_ = (static_cast<bool>(this->dividend.sign_)) ? Sign::negative : Sign::positive;
        return *this;
    }

    Rational& operator*=(const Rational& other) {
        dividend *= other.dividend;
        divisor *= other.divisor;
        BigInteger nod = gcd(dividend, divisor);
        divisor /= nod;
        dividend /= nod;
        return *this;
    }

    Rational& operator/=(const Rational& other) {
        dividend *= other.divisor;
        divisor *= other.dividend;
        BigInteger nod = gcd(dividend, divisor);
        divisor /= nod;
        dividend /= nod;

        if (divisor < 0) {

            divisor.sign_ = Sign::positive;

            if (dividend != 0) {
                dividend.sign_ = (static_cast<bool>(dividend.sign_)) ? Sign::negative : Sign::positive;
            }

        }

        return *this;
    }

    Rational operator-() const {
        Rational result = *this;
        result.dividend *= -1;
        return result;
    }

    std::weak_ordering operator<=>(const Rational& other) {

        if (dividend.sign_ != other.dividend.sign_) {
            return dividend.sign_ <=> other.dividend.sign_;
        }

        return dividend * other.divisor <=> divisor * other.dividend;
    }

    bool operator==(const Rational& other) {

        if (dividend.sign_ != other.dividend.sign_) {
            return static_cast<bool>(dividend.sign_);
        }

        return (dividend == other.dividend && divisor == other.divisor);
    }

    std::string toString() const {
        std::string str = dividend.toString();

        if (divisor == 1 || dividend == 0) {
            return str;
        }

        str += '/';
        str += divisor.toString();
        return str;
    }

    std::string asDecimal(size_t precision = 0) const {

        std::string str;

        if (divisor == 1) {
            str += dividend.toString();
            return str;
        }

        if (!static_cast<bool>(dividend.sign_)) {
            str += '-';
        }

        BigInteger posivite_dividend = dividend;
        posivite_dividend.sign_ = Sign::positive;
        BigInteger temp_devidend = posivite_dividend % divisor;
        BigInteger temp_result = posivite_dividend / divisor;
        str += temp_result.toString();
        int len_int = str.size();

        if (str[0] == '-') {
            --len_int;
        }

        str += '.';
        temp_devidend *= 10;
        temp_result = temp_devidend;

        while (str.size() < precision + len_int + 2) {
            temp_result /= divisor;
            str += temp_result.toString();
            temp_devidend %= divisor;
            temp_devidend *= 10;
            temp_result = temp_devidend;
        }

        if (str[0] == '-' && str.size() > len_int + precision + 2) {
            str.resize(precision + len_int + 1);
        }

        else if (str[0] != '-' && str.size() > precision + len_int + 1) {
            str.resize(precision + len_int + 1);
        }

        return str;
    }

    explicit operator double() const {
        double result = std::stod(asDecimal(decimal_const));
        return result;
    }

    ~Rational() = default;
};

Rational operator+(const Rational& first, const Rational& second) {
    Rational result = first;
    result += second;
    return result;
}

Rational operator-(const Rational& first, const Rational& second) {
    Rational result = first;
    result -= second;
    return result;
}

Rational operator*(const Rational& first, const Rational& second) {
    Rational result = first;
    result *= second;
    return result;
}

Rational operator/(const Rational& first, const Rational& second) {
    Rational result = first;
    result /= second;
    return result;
}
