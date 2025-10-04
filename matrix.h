#include <iostream>

#define CPP23 1


#include <compare>
#include <string>
#include <vector>
#include <chrono>
#include <array>


enum class Sign{
    negative,
    positive
};


class BigInteger {
private:
    static const int base_ = 1e9;
    static const int nine_ = 9;
    Sign sign_;
    std::vector<long long> arr_;

    friend std::istream& operator>>(std::istream& in, BigInteger& number);
    friend std::ostream& operator<<(std::ostream& out, const BigInteger& number);
    friend BigInteger gcd(BigInteger first, BigInteger second);
    int bin_search(const BigInteger& second);
    friend class Rational;

    void clear_zero(){
        while (arr_.back() == 0 && arr_.size() > 1) {
            arr_.pop_back();
        }
    }

public:

    BigInteger() : BigInteger(0){}



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
        int i = arr_.size() - 1;

        if (arr_[i] < 0) {

            sign_ = static_cast<bool>(sign_) ? Sign::negative : Sign::positive;
            arr_[i] *= -1;

            for (int o = i - 1; o >= 0; --o) {

                if (arr_[o] != 0) {
                    --arr_[o + 1];
                    arr_[o] -= base_;
                    arr_[o] *= -1;
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







    BigInteger& operator/=(const BigInteger& other) {
        if (arr_.empty() && !other.arr_.empty()) {
            *this = 0;
            return *this;
        }

        BigInteger thispositive = (sign_==Sign::positive) ? *this : -*this;

        BigInteger quotient;
        BigInteger rest;
        size_t length = std::to_string(thispositive.arr_.back()).size();
        size_t maxtenpow = 1;
        for (size_t i = 0; i < length-1; ++i) {
            maxtenpow*=10;
        }
        for (size_t i = (thispositive.arr_.size() - 1) * nine_ + length; i > 0; --i) {
            rest*=10;
            rest += thispositive.arr_.back() / maxtenpow;
            thispositive.arr_.back() %= maxtenpow;

            if (rest >= other) {
                
                size_t multiply = 0;
                while (rest >= other){
                    ++multiply;
                    rest -= other;
                }

                BigInteger add;
                add.arr_.assign(thispositive.arr_.size() - 1, 0);
                add.arr_.push_back(multiply * maxtenpow);
                quotient += add;
            }

            if (i % nine_ == 1) {
                maxtenpow = base_;
                thispositive.arr_.pop_back();
            }
            maxtenpow /= 10;
        }

        if (!quotient.arr_.empty()) {
            quotient.sign_ =
                    (static_cast<bool>(sign_) ^ static_cast<bool>(other.sign_)) == 0 ? Sign::positive : Sign::negative;
        }
        *this = quotient;
        return *this;
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

    Rational() : Rational(0){}

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
        BigInteger nod1 = gcd(dividend, other.divisor);
        BigInteger nod2 = gcd(other.dividend, divisor);
        dividend *= other.dividend;
        divisor *= other.divisor;
        divisor /= nod1;
        dividend /= nod1;
        divisor /= nod2;
        dividend /= nod2;
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
    Rational abs(){
        Rational new_rational = *this;
        if (dividend<0){
            new_rational.dividend*=-1;
        }
        return new_rational;
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

    bool operator==(const Rational& other) const {

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

std::istream& operator>>(std::istream& in, Rational& num){
    BigInteger nw;
    in >> nw;
    num = nw;
    return in;
}

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



template <size_t N,size_t U>
struct HelpSqrt{
    static constexpr size_t val = ((N/U)>U) ? U * 2 : HelpSqrt<N,U/2>::val;
};
template <size_t N>
struct HelpSqrt<N,0>{
    static constexpr size_t val = 1;
};

template <size_t N>
struct Sqrt{
    static constexpr size_t val = HelpSqrt<N,N>::val;
};

template <size_t N,size_t U>
struct helpPrime{
    static constexpr bool val = N % U != 0 && helpPrime<N, U - 1>::val;
};
template <size_t N>
struct helpPrime<N,1>{
    static constexpr bool val = true;
};

template <size_t N>
struct isPrime{
    static constexpr bool val = helpPrime<N,Sqrt<N>::val>::val;
};
template <>
struct isPrime<1>{
    static constexpr bool val = false;
};
template <>
struct isPrime<2>{
    static constexpr bool val = true;
};


template <size_t N>
class Residue {
private:
    size_t num_;
    size_t pow(size_t number, size_t power) {
        if (power == 0) {
            return 1;
        }
        if (power == 1) {
            return number;
        }
        if (power % 2 == 1) {
            return (pow(number, power / 2) * pow(number, power / 2) * number) % N;
        }
        return (pow(number, power / 2) * pow(number, power / 2)) % N;


    };
public:


    Residue() = default;
    Residue(int num){
        if (num>=0){
            num_ = num % N;
        }
        else{
            num_ = num % static_cast<int>(N)+N;
        }
    }

    Residue<N>& operator+=(const Residue<N>& other){
        num_+=other.num_;
        num_%=N;
        return *this;
    }
    Residue<N>& operator-=(const Residue<N>& other){
        if (num_>=other.num_) {
            num_ -= other.num_;
            return *this;
        }
        num_=N - (other.num_-num_);
        return *this;
    }
    Residue<N>& operator*=(const Residue<N>& other){
        num_*=other.num_;
        num_%=N;
        return *this;
    }

    Residue<N>& operator/=(const Residue<N>& other){
        *this *= static_cast<Residue<N>>(pow(other.num_,N-2));
        return *this;
    }

    explicit operator int() const {
        return static_cast<int>(num_);
    }

    bool operator==(const Residue<N>& other) const{
        return num_==other.num_;
    }
    bool operator!=(const Residue<N>& other) const{
        return !(*this==other);
    }
    ~Residue() = default;
};




template <size_t N>
Residue<N> operator+(const Residue<N>& first,const Residue<N>& second){
    first+=second;
    return first;
}
template <size_t N>
Residue<N> operator-(const Residue<N>& first,const Residue<N>& second){
    Residue<N> new_first = first;
    new_first-=second;
    return new_first;
}
template <size_t N>
Residue<N> operator*(const Residue<N>& first,const Residue<N>& second){
    Residue<N> new_first = first;
    new_first*=second;
    return new_first;
}
template <size_t N>
Residue<N> operator/(const Residue<N>& first,const Residue<N>& second){
    Residue<N> new_first = first;
    new_first/=second;
    return new_first;
}

template <size_t N, size_t M, typename Field = Rational>
class Matrix{
private:
    std::array<std::array<Field,M>,N> arr_;

public:



    std::pair<Matrix,int> Gauss() const{
        size_t a = 0;
        size_t b = 0;
        size_t permutation = 0;
        Matrix<N,M,Field> answ = *this;
        while (a<N&&b<M){
            for (size_t i = a + 1; i < N; ++i) {
                if (answ[i,b]==static_cast<Field>(0)){
                    continue;
                }
                if (answ[a,b]==static_cast<Field>(0)){
                    for (size_t j = b; j < M; ++j) {
                        std::swap(answ[i,j],answ[a,j]);
                    }
                    ++permutation;
                    continue;
                }
                Field subproduct = static_cast<Field>(-1)*answ[i,b]/answ[a,b];
                for (size_t j = b; j < M; ++j) {
                    answ[i,j]+=subproduct*answ[a,j];
                }

            }
            if (answ[a,b]!=static_cast<Field>(0)){
                ++a;
            }
            ++b;
        }
        return {answ,permutation};
    }

    Field& operator[](size_t i,size_t j){
        return arr_[i][j];
    }
    const Field& operator[](size_t i,size_t j) const{
        return arr_[i][j];
    }

    Matrix(){
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                arr_[i][j]=static_cast<Field>(0);
            }
        }
    }

    Matrix(const std::initializer_list<std::vector<int>>& arr) {
        size_t i = 0;
        for (auto element : arr) {
            for (size_t j = 0; j < M; ++j) {
                arr_[i][j] = static_cast<Field>(element[j]);
            }
            ++i;
        }
    }

    Matrix<N,M,Field>& operator+=(const Matrix<N,M,Field>& other){
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                arr_[i][j]+=other[i,j];
            }
        }
        return *this;
    }
    Matrix<N,M,Field>& operator-=(const Matrix<N,M,Field>& other){
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                arr_[i][j]-=other[i,j];
            }
        }
        return *this;
    }
    Matrix<N, M, Field>& operator*=(const Field& field){
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                arr_[i][j]*=field;
            }
        }
        return *this;
    }

    Matrix<N,M,Field>& operator*=(const Matrix<M,M,Field>& other){
        Matrix<N,M,Field> new_matrix;
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                for (size_t l = 0; l < M; ++l) {
                    new_matrix[i,j]+=arr_[i][l]*other[l,j];
                }
            }
        }
        *this = new_matrix;
        return *this;
    }

    Field det() const {
        std::pair<Matrix<N,N,Field>,int> temp_matrix = this->Gauss();

        Field det = temp_matrix.second % 2 == 0 ? static_cast<Field>(1) : static_cast<Field>(-1);
        if (temp_matrix.first[N-1,N-1]==static_cast<Field>(0)){
            return static_cast<Field>(0);
        }
        for (size_t i = 0; i < N; ++i) {
            det*=temp_matrix.first[i,i];
        }
        return det;
    }
    Matrix<M, N, Field> transposed() const {
        Matrix<M,N, Field> transposed;
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j) {
                transposed[i,j] = arr_[j][i];
            }
        }
        return transposed;
    }
    size_t rank() const{
        Matrix<N,M,Field> temp_matrix = this->Gauss().first;
        size_t rank = 0;
        for (size_t i = 0; i < std::min(N,M); ++i) {
            for (size_t j = 0; i+j < M; ++j) {
                if (temp_matrix[i,i+j]!=static_cast<Field>(0)){
                    rank++;
                    break;
                }
            }

        }
        return rank;
    }
    void invert(){
        static_assert(N==M);
        Matrix<N,2*N,Field> matrix;
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < N; ++j) {
                matrix[i,j]=arr_[i][j];
            }
            matrix[i,i+N] = static_cast<Field>(1);
        }
        matrix = matrix.Gauss().first;
        for (long long i = N-1; i >= 0; --i) {
            Field subproduct = static_cast<Field>(1)/matrix[i,i];
            for (size_t j = i; j < 2*N; ++j) {
                matrix[i,j]*=subproduct;
            }
            for (long long j = i-1; j >= 0 ; --j) {
                Field row_subproduct = static_cast<Field>(-1)*matrix[j,i]/matrix[i,i];
                for (size_t k = i; k < 2*N; ++k) {
                    matrix[j,k]+=row_subproduct*matrix[i,k];
                }

            }

        }
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < N; ++j) {
                arr_[i][j] = matrix[i,j+N];
            }
        }
    }
    Matrix<N,N,Field> inverted() const{
        Matrix<N,N,Field> inverted = *this;
        inverted.invert();
        return inverted;
    }
    Field trace() const{
        static_assert(N==M);
        Field answ = static_cast<Field>(0);
        for (size_t i = 0; i < N; ++i) {
            answ+=arr_[i][i];
        }
        return answ;
    }
    std::vector<Field> getRow(size_t index) const{
        std::vector<Field> row(M);
        for (size_t i = 0; i < M; ++i) {
            row[i]=arr_[index][i];
        }
        return row;
    }
    std::vector<Field> getColumn(size_t index) const{
        std::vector<Field> column(N);
        for (size_t i = 0; i < N; ++i) {
            column[i]=arr_[i][index];
        }
        return column;
    }






};
template <size_t N, size_t M, typename Field = Rational>
bool operator==(const Matrix<N,M,Field>& first,const Matrix<N,M,Field>& second){
    if (&first==&second){
        return true;
    }
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < M; ++j) {
            if (first[i,j]!=second[i,j]){
                return false;
            }
        }
    }
    return true;
}
template <size_t N, size_t M, typename Field = Rational>

bool operator!=(const Matrix<N,M,Field>& first,const Matrix<N,M,Field>& second){
    return !(first == second);
}

template <size_t N, size_t M, typename Field = Rational>
Matrix<N,M,Field> operator+(const Matrix<N,M,Field>& first,const Matrix<N,M,Field>& second){
    Matrix<N,M,Field> new_first = first;
    new_first+=second;
    return new_first;
}

template <size_t N, size_t M, typename Field = Rational>
Matrix<N,M,Field> operator-(const Matrix<N,M,Field>& first,const Matrix<N,M,Field>& second){
    Matrix<N,M,Field> new_first = first;
    new_first-=second;
    return new_first;
}
template <size_t N, size_t M,size_t K, typename Field = Rational>
Matrix<N,K,Field> operator*(const Matrix<N,M,Field>& first,const Matrix<M,K,Field>& second){
    Matrix<N,K,Field> new_matrix;
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < K; ++j) {
            for (size_t k = 0; k < M; ++k) {
                new_matrix[i,j]+=first[i,k]*second[k,j];
            }
        }
    }
    return new_matrix;
}

template <size_t N, size_t M, typename Field = Rational>
Matrix<N, M, Field> operator*(Matrix<N,M,Field> matrix,const Field& field){
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < M; ++j) {
            matrix[i,j]*=field;
        }
    }
    return matrix;
}
template <size_t N, size_t M, typename Field = Rational>
Matrix<N, M, Field> operator*(const Field& field,Matrix<N,M,Field> matrix){
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < M; ++j) {
            matrix[i,j]*=field;
        }
    }
    return matrix;
}


template<size_t N, typename Field = Rational>
using SquareMatrix = Matrix<N, N, Field>;

