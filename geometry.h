#include <iostream>
#include <vector>
#include <cmath>
#include <math.h>

class Line;

namespace constants {

    static const double epsilon = 1e-6;

}

struct Point{

    double x;

    double y;

    Point();

    Point(double x,double y);

    void rotate(const Point& center, double angle);

    void scale(const Point& center, double coefficient);

    void reflect(const Point& center);

    void reflect(const Line& axis);

    double distance() const;

};

Point::Point() = default;

Point::Point(double x,double y): x(x),y(y){}

void Point::rotate(const Point &center, double angle) {
    double old_x = x;
    x = (x-center.x)* cos(angle)-(y-center.y) * sin(angle)+center.x;
    y = (old_x-center.x)* sin(angle)+(y-center.y) * cos(angle)+center.y;
}

double Point::distance() const{
    return sqrt(x*x+y*y);
}

void Point::scale(const Point& center, double coefficient){
    x=center.x+(x-center.x)*coefficient;
    y=center.y+(y-center.y)*coefficient;
}

void Point::reflect(const Point &center) {
    scale(center,-1);
}

bool operator==(const Point &first,const Point &second) {
    return (std::abs(first.x-second.x)<constants::epsilon&&std::abs(first.y-second.y)<constants::epsilon);
}

class Vector{

private:

    Point point;

public:

    Vector(Point point);

    double length() const;

    Vector& operator-=(const Vector& other);

    double operator*(const Vector& second);

    bool operator==(const Vector& Second) const;
    bool operator!=(const Vector& Second) const;

};

bool Vector::operator==(const Vector &Second) const {
    return (point==Second.point);
}

Vector::Vector(Point point): point(point){}

double Vector::operator*(const Vector &second) {
    return point.x*second.point.y-second.point.x*point.y;
}

Vector operator-(Point first, Point second){
    first.x-=second.x;
    first.y-= second.y;
    return {first};
}

Vector operator-(Vector first, Vector second){
    first-=second;
    return first;
}


bool operator!=(const Point& first, const Point& second){
    return !(first==second);
}

bool Vector::operator!=(const Vector& Second) const{
    return !(*this==Second);
}

double Vector::length() const{
    return sqrt(point.x*point.x+point.y*point.y);
}

Vector& Vector::operator-=(const Vector& other) {
    point.x-=other.point.x;
    point.y-=other.point.y;
    return *this;
}

class Line{

private:

    double a_;

    double b_;

    double c_;

    void base_form();

public:

    double distance(const Point& p_) const;

    bool operator==(const Line& second) const;

    bool operator!=(const Line& second) const;

    Line(Point first, Point second);

    Line(double coefficient, double c_);

    Line(Point p1, double coefficient);

    Point normal() const;

    double free_coeff() const;

    bool DoesPointBelong(Point p) const;


};

bool Line::DoesPointBelong(Point p) const{
    return std::abs(a_*p.x+b_*p.y+c_)<constants::epsilon;
}


double Line::free_coeff() const {
    return c_;
}

void Point::reflect(const Line& axis){

    double r = axis.distance(*this);
    Point normal = axis.normal();
    double coefficient = r/normal.distance();

    normal.x*=coefficient;
    normal.y*=coefficient;

    Point center_of_reflect=*this;

    center_of_reflect.x+=normal.x;
    center_of_reflect.y+=normal.y;

    if (axis.DoesPointBelong(center_of_reflect)){
        reflect(center_of_reflect);
    }

    else{

        center_of_reflect.x-=(2*normal.x);
        center_of_reflect.y-=(2*normal.y);

        reflect(center_of_reflect);
    }
}



Point Line::normal() const {
    return {a_,b_};
}

void Line::base_form() {
    if (c_!=0){

        a_/=c_;
        b_/=c_;
        c_ = 1;
    }

    else{

        if (b_!=0) {
            a_ /= b_;
            b_ = 1;
        }

        else{
            a_= 1;
        }

    }
    if (b_>0){

        a_*=-1;
        b_*=-1;
        c_*=-1;

    }
}


Line::Line(Point first, Point second) {
    a_ = second.y-first.y;
    b_ = -second.x+first.x;
    c_ = first.y*(second.x-first.x)-first.x*(second.y-first.y);
    base_form();
}


Line::Line(double coefficient, double c_):a_(coefficient),b_(-1),c_(c_) {
    base_form();
}

Line::Line(Point p1, double coefficient):a_(coefficient),b_(-1){
       c_ = -a_*p1.x-b_*p1.y;
       base_form();
}

bool Line::operator==(const Line& second) const{
    return (std::abs(a_-second.a_)<constants::epsilon&&std::abs(b_-second.b_)<constants::epsilon&&std::abs(c_-second.c_)<constants::epsilon);
}


bool Line::operator!=(const Line& second) const {
    return !(*this==second);
}

double Line::distance(const Point& p_) const{
    return std::abs(a_*p_.x+b_*p_.y+c_)/sqrt(a_*a_+b_*b_);
}


Point intersection(Line first, Line second){
    Point p1 = first.normal();
    Point p2 = second.normal();
    double c1 = first.free_coeff();
    double c2 = second.free_coeff();
    return {(p1.y*c2-p2.y*c1)/(p1.x*p2.y-p2.x*p1.y),-(p1.x*c2-p2.x*c1)/(p1.x*p2.y-p2.x*p1.y)};
}


class Shape{

public:

    virtual double perimeter() const = 0;

    virtual double area() const = 0;

    virtual bool operator==(const Shape& another) const = 0;

    virtual bool operator!=(const Shape& another) const = 0;

    virtual bool isCongruentTo(const Shape& another) const = 0;

    virtual bool isSimilarTo(const Shape& another) const = 0;

    virtual bool containsPoint(const Point& point) const = 0;

    virtual void rotate(const Point& center, double angle)  = 0;

    virtual void reflect(const Point& center)  = 0;

    virtual void reflect(const Line& axis)  = 0;

    virtual void scale(const Point& center, double coefficient)  = 0;

    virtual ~Shape() = default;

};



class Polygon : public Shape{

protected:

    std::vector<Point> vertices;

    bool congruent(const Shape& another, bool equality) const;

    template<typename T, typename... Args>
    void build(T value, Args... args);

    template<typename T>
    void build(T value);

public:

    Polygon();

    Polygon(const std::vector<Point>& vertices);

    template<typename... Args>
    Polygon(Args... args);

    size_t verticesCount() const;

    const std::vector<Point>& getVertices() const;

    bool isConvex() const;

    double perimeter() const override;

    double area() const override;

    bool operator==(const Shape& another) const override;

    bool operator!=(const Shape& another) const override;

    bool isSimilarTo(const Shape& another) const override;

    bool isCongruentTo(const Shape& another) const override;

    bool containsPoint(const Point& point) const override;

    void rotate(const Point& center, double angle) override;

    void reflect(const Point& center) override;

    void reflect(const Line& axis) override;

    void scale(const Point& center, double coefficient) override;

};

Polygon::Polygon() = default;
Polygon::Polygon(const std::vector<Point> &vertices):vertices(vertices) {}

template<typename T>
void Polygon::build(T value){
    vertices.push_back(value);
}

template<typename T, typename... Args>
void Polygon::build(T value, Args... args){
    vertices.push_back(value);
    build(args...);
}

template<typename... Args>
Polygon::Polygon(Args... args){
    build(args...);
}

size_t Polygon::verticesCount() const{
    return vertices.size();
}

const std::vector<Point>& Polygon::getVertices() const {
    return vertices;
}

bool Polygon::isConvex() const {

    bool cross_product = (vertices[0]-vertices[vertices.size()-1])*(vertices[1]-vertices[0])>0;

    if (cross_product!=(vertices[vertices.size()-1]-vertices[vertices.size()-2])*(vertices[0]-vertices[vertices.size()-1])>0){
        return false;
    }

    for (size_t i = 0; i < vertices.size()-2; ++i){

        if (cross_product!=(vertices[i+1]-vertices[i])*(vertices[i+2]-vertices[i+1])>0){
            return false;
        }

    }

    return true;

}

double Polygon::perimeter() const {
    double perimeter = (vertices[0]-vertices[vertices.size()-1]).length();
    for (size_t i = 0; i < vertices.size()-1;++i){
        perimeter+=(vertices[i+1]-vertices[i]).length();
    }
    return perimeter;
}

bool Polygon::congruent(const Shape& another, bool equality) const{
    const Polygon* oth_polygon = dynamic_cast<const Polygon*>(&another);

    if (oth_polygon == nullptr){
        return false;
    }

    const Polygon& oth = *oth_polygon;

    if (vertices.size()!=oth.vertices.size()){
        return false;
    }

    for (size_t i = 0; i < vertices.size();++i){

        double coefficient = (vertices[0]-vertices[vertices.size()-1]).length()/(oth.vertices[(i+1)%vertices.size()]-oth.vertices[i]).length();

        if (equality&&std::abs(coefficient-1)>constants::epsilon){
            continue;
        }

        bool flag = true;



        for (size_t j = 0; j < vertices.size(); ++j){

            if (std::abs(coefficient-((vertices[j]-vertices[(vertices.size()+j-1)%vertices.size()]).length()/(oth.vertices[(i+j+1)%vertices.size()]-oth.vertices[(i+j)%vertices.size()]).length()))>constants::epsilon){
                flag = false;
                break;
            }

        }



        if (!flag){
            flag = true;

            for (size_t j = 0; j < vertices.size(); ++j){
                if (std::abs(coefficient-(vertices[j]-vertices[(vertices.size()+j-1)%vertices.size()]).length()/(oth.vertices[(vertices.size()+i-j+1)%vertices.size()]-oth.vertices[(vertices.size()+i-j)%vertices.size()]).length())>constants::epsilon){
                    flag = false;
                    break;
                }
            }

            if (!flag){
                continue;
            }
        }



        for (size_t j = 0; j < vertices.size(); ++j) {

            double coeff_of_cross_product = ((vertices[j]-vertices[(vertices.size()+j-1)%vertices.size()])*(vertices[(j+1)%vertices.size()]-vertices[j]))/((oth.vertices[(i+j+1)%vertices.size()]-oth.vertices[(i+j)%vertices.size()])*(oth.vertices[(i+j+2)%vertices.size()]-oth.vertices[(i+j+1)%vertices.size()]));

            if (std::abs(coefficient*coefficient-std::abs(coeff_of_cross_product))>constants::epsilon){
                    flag = false;
                    break;
            }

        }


        if (!flag){

            flag = true;

            for (size_t j = 0; j < vertices.size(); ++j) {
                    double coeff_of_cross_product = ((vertices[j]-vertices[(vertices.size()+j-1)%vertices.size()])*(vertices[(j+1)%vertices.size()]-vertices[j]))/((oth.vertices[(vertices.size()+i-j+1)%vertices.size()]-oth.vertices[(vertices.size()+i-j)%vertices.size()])*(oth.vertices[(vertices.size()+i-j)%vertices.size()]-oth.vertices[(vertices.size()+i-j-1)%vertices.size()]));

                    if (std::abs(coefficient*coefficient-std::abs(coeff_of_cross_product))>constants::epsilon){
                        flag = false;
                        break;
                    }

            }
        }

        if (flag) {
            return flag;
        }

    }
    return false;
}


double Polygon::area() const {

    double res = 0;
    for (size_t i=0; i<vertices.size(); i++) {
        Point p1 = i ? vertices[i-1] : vertices.back();
        Point p2 = vertices[i];
        res += (p1.x - p2.x) * (p1.y + p2.y);
    }
    return std::abs (res) / 2;
}

bool Polygon::isSimilarTo(const Shape& another) const{
    return congruent(another, false);
}

bool Polygon::operator==(const Shape& another) const {
    const Polygon* oth_polygon = dynamic_cast<const Polygon*>(&another);

    if (oth_polygon == nullptr){
        return false;
    }

    const Polygon& oth = *oth_polygon;

    if (vertices.size()!=oth.vertices.size()){
        return false;
    }

    for (size_t i = 0;i < vertices.size();++i){

        if (oth.vertices[i]==vertices[0]){

            bool flag = true;

            for (size_t j = 0; j < vertices.size(); ++j) {

                if (vertices[j]!=oth.vertices[(i+j)%vertices.size()]){
                    flag = false;
                    break;
                }

            }

            if (!flag){

                flag = true;
                for (size_t j = 0; j < vertices.size(); ++j) {

                    if (vertices[j]!=oth.vertices[(vertices.size()+i-j)%vertices.size()]){
                        flag = false;
                        break;
                    }

                }
            }

            return flag;
        }
    }
    return false;
}

bool Polygon::operator!=(const Shape& another) const {
    return !(*this==another);
}

bool Polygon::isCongruentTo(const Shape& another) const {
    return congruent(another,true);
}

bool Polygon::containsPoint(const Point& point) const{
    bool result = false;
    int j = vertices.size() - 1;

    for (size_t i = 0; i < vertices.size(); i++) {

        if ( ((vertices[i].y < point.y && vertices[j].y >= point.y) || (vertices[j].y < point.y && vertices[i].y >= point.y)) &&
             (vertices[i].x + (point.y - vertices[i].y) / (vertices[j].y - vertices[i].y) * (vertices[j].x - vertices[i].x) < point.x) ) {

            result = !result;

        }

        j = i;
    }
    return result;
}

void Polygon::rotate(const Point &center, double angle) {
    for (Point& point : vertices){
        point.rotate(center,angle);
    }
}

void Polygon::reflect(const Point &center) {
    for (Point& point : vertices){
        point.reflect(center);
    }
}

void Polygon::reflect(const Line &axis) {
    for (Point& point : vertices){
        point.reflect(axis);
    }
}

void Polygon::scale(const Point &center, double coefficient) {
    for (Point& point : vertices){
        point.scale(center,coefficient);
    }
}

class Ellipse : public Shape{

protected:

    Point f1_;

    Point f2_;

    double length_;

    bool similar(const Shape& another, bool equality) const;

public:
    Ellipse();

    Point center() const;

    Ellipse(Point f1_,Point f2_,double length_);

    std::pair<Point,Point> focuses() const;

    std::pair<Line, Line> directrices() const;

    double a_() const;

    double b_() const;

    double c_() const;

    double eccentricity() const;

    double perimeter() const override;

    double area() const override;

    bool operator==(const Shape& another) const override;

    bool operator!=(const Shape& another) const override;

    bool isSimilarTo(const Shape& another) const override;

    bool isCongruentTo(const Shape& another) const override;

    bool containsPoint(const Point& point) const override;

    void rotate(const Point& center, double angle) override;

    void reflect(const Point& center) override;

    void reflect(const Line& axis) override;

    void scale(const Point& center, double coefficient) override;

};

Ellipse::Ellipse() = default;

Point Ellipse::center() const{
    return {(f1_.x+f2_.x)/2,(f1_.y+f2_.y)/2};
}

double Ellipse::a_() const{
    return length_/2;
}

double Ellipse::c_() const{
    return (f1_-f2_).length()/2;
}

double Ellipse::b_() const{
    return sqrt(a_()*a_()-c_()*c_());
}

double Ellipse::eccentricity() const{
    return c_()/a_();
}

Ellipse::Ellipse(Point f1_, Point f2_, double length_): f1_(f1_), f2_(f2_), length_(length_) {}

std::pair<Point,Point> Ellipse::focuses() const{
    return {f1_,f2_};
}

std::pair<Line, Line> Ellipse::directrices() const{
    return {Line({a_()*a_()/c_(),0},{a_()*a_()/c_(),1}),Line({-a_()*a_()/c_(),0},{-a_()*a_()/c_(),1})};
}

double Ellipse::perimeter() const{
    return 4 * a_() * std::comp_ellint_2(eccentricity());
}

double Ellipse::area() const {
    return M_PI*a_()*b_();
}

bool Ellipse::operator==(const Shape& another) const{
    const Ellipse* oth_polygon = dynamic_cast<const Ellipse*>(&another);

    if (oth_polygon == nullptr){
        return false;
    }

    const Ellipse& oth = *oth_polygon;

    if (((f1_==oth.f1_&&f2_==oth.f2_)||(f1_==oth.f2_&&f2_==oth.f1_))&&std::abs(length_-oth.length_)<constants::epsilon){

        return true;

    }
    return false;
}

bool Ellipse::operator!=(const Shape& another) const{
    return !(*this==another);
}

bool Ellipse::isSimilarTo(const Shape &another) const {
    return similar(another,false);
}

bool Ellipse::similar(const Shape &another, bool equality) const {

    const Ellipse* oth_polygon = dynamic_cast<const Ellipse*>(&another);

    if (oth_polygon == nullptr){
        return false;
    }

    const Ellipse& oth = *oth_polygon;

    if (a_()*oth.b_()==b_()*oth.a_()||(a_()*oth.a_()==a_()==oth.a_())){

        if (equality&&(a_()!=oth.a_()&&a_()!=oth.b_())){
            return false;
        }

        return true;
    }
    return false;
}

bool Ellipse::isCongruentTo(const Shape &another) const {
    return similar(another, true);

}

bool Ellipse::containsPoint(const Point &point) const {
    return (f1_-point).length()+(f2_-point).length()<length_+constants::epsilon;
}

void Ellipse::rotate(const Point &center, double angle) {
    f1_.rotate(center,angle);
    f2_.rotate(center,angle);
}

void Ellipse::reflect(const Point &center) {
    f1_.reflect(center);
    f2_.reflect(center);
}

void Ellipse::reflect(const Line &axis) {
    f1_.reflect(axis);
    f2_.reflect(axis);
}

void Ellipse::scale(const Point &center, double coefficient) {
    f1_.scale(center,coefficient);
    f2_.scale(center,coefficient);
    length_*=std::abs(coefficient);
}


class Circle : public Ellipse{
public:
    Circle(Point center, double r_);

    Circle(Point first, Point second, Point third);

    double radius() const;

    Point center_of_three_point(Point first, Point second, Point third) const;

};

Point Circle::center_of_three_point(Point first, Point second, Point third) const {
    double x = -(0.5)*(first.y*(second.x*second.x-third.x*third.x+second.y*second.y-third.y*third.y)+second.y*(-first.x*first.x+third.x*third.x-first.y*first.y+third.y*third.y)+third.y*(first.x*first.x-second.x*second.x+first.y*first.y-second.y*second.y))/(first.x*(second.y-third.y)+second.x*(third.y-first.y)+third.x*(first.y-second.y));
    double y = (0.5)*(first.x*(second.x*second.x-third.x*third.x+second.y*second.y-third.y*third.y)+second.x*(-first.x*first.x+third.x*third.x-first.y*first.y+third.y*third.y)+third.x*(first.x*first.x-second.x*second.x+first.y*first.y-second.y*second.y))/(first.x*(second.y-third.y)+second.x*(third.y-first.y)+third.x*(first.y-second.y));
    return {x,y};
}

Circle::Circle(Point center, double r_): Ellipse(center,center,2*r_) {}

Circle::Circle(Point first, Point second, Point third) : Ellipse(center_of_three_point(first,second,third),center_of_three_point(first,second,third),(center_of_three_point(first,second,third)-first).length()*2) {}

double Circle::radius() const {
    return length_/2;
}

class Rectangle : public Polygon{

public:
    Point center() const;

    std::pair<Line, Line> diagonals() const;

    Rectangle(Point first, Point second, double coefficient);

};

Rectangle::Rectangle(Point first, Point second, double coefficient) {
    if (coefficient>1){
        coefficient=1/coefficient;
    }

    vertices.push_back(first);
    double angle = std::atan(coefficient);
    Point old_second(second);
    second.rotate(first,angle);
    second.scale(first,coefficient);

    vertices.push_back(second);
    vertices.push_back(old_second);

    Point fourth = {(vertices[0].x+vertices[2].x)/2,(vertices[0].y+vertices[2].y)/2};
    fourth.scale(second,2);

    vertices.push_back(fourth);
}

Point Rectangle::center() const {
    return {(vertices[0].x+vertices[2].x)/2,(vertices[0].y+vertices[2].y)/2};
}


std::pair<Line, Line> Rectangle::diagonals() const{
    return {Line(vertices[0],vertices[2]),Line(vertices[1],vertices[3])};
}

class Square : public Rectangle{
public:
    Square(Point first, Point second);

    Circle circumscribedCircle() const;

    Circle inscribedCircle() const;

};

Square::Square(Point first, Point second): Rectangle(first,second,1){}

Circle Square::circumscribedCircle() const{
    return {vertices[0],vertices[1],vertices[2]};
}

Circle Square::inscribedCircle() const{
    return {{(vertices[0].x+vertices[2].x)/2,(vertices[0].y+vertices[2].y)/2},(vertices[0]-vertices[1]).length()};
}

class Triangle : public Polygon{
public:
    using Polygon::Polygon;

    Circle circumscribedCircle() const;

    Circle inscribedCircle() const;

    Point centroid() const;

    Point orthocenter() const;

    Line EulerLine() const;

    Circle ninePointsCircle() const;

};

Circle Triangle::circumscribedCircle() const{
    return Circle{vertices[0],vertices[1],vertices[2]};
}

Circle Triangle::inscribedCircle() const{

    double coefficient1 = (vertices[0]-vertices[1]).length()/(vertices[1]-vertices[2]).length();

    Point bis1 = {(vertices[0].x+coefficient1*vertices[2].x)/(1+coefficient1),(vertices[0].y+coefficient1*vertices[2].y)/(1+coefficient1)};

    Line first{vertices[1],bis1};

    double coefficient2 = (vertices[0]-vertices[1]).length()/(vertices[0]-vertices[2]).length();

    Point bis2 = {(vertices[1].x+coefficient2*vertices[2].x)/(1+coefficient2),(vertices[1].y+coefficient2*vertices[2].y)/(1+coefficient2)};

    Line second{vertices[0],bis2};

    Point center = intersection(first,second);

    double radius = Line(vertices[0],vertices[1]).distance(center);

    return {center,radius};
}

Point Triangle::centroid() const{
    Point mid = {(vertices[0].x+vertices[1].x)/2,(vertices[0].y+vertices[1].y)/2};
    double coefficient = 2;
    return {(vertices[2].x+coefficient*mid.x)/(1+coefficient),(vertices[2].y+coefficient*mid.y)/(1+coefficient)};
}

Point Triangle::orthocenter() const{

    Line side1(vertices[0],vertices[1]);
    Line side2(vertices[0],vertices[2]);
    Line side3(vertices[1],vertices[2]);

    Point normal1 = side1.normal();
    Point normal2 = side2.normal();
    Point normal3 = side3.normal();

    Line first(normal1.y/normal1.x,vertices[2].y-vertices[2].x*normal1.y/normal1.x);
    Line second(normal2.y/normal2.x,vertices[1].y-vertices[1].x*normal2.y/normal2.x);
    Line third(normal3.y/normal3.x,vertices[0].y-vertices[0].x*normal3.y/normal3.x);

    if (normal1.x!=0){

        if (normal2.x!=0){
            return intersection(first,second);
        }

        return intersection(first, third);
    }

    return intersection(second,third);
}

Line Triangle::EulerLine() const{

    return {circumscribedCircle().center(),orthocenter()};

}

Circle Triangle::ninePointsCircle() const{
    Point mid1 = {(vertices[0].x+vertices[1].x)/2,(vertices[0].y+vertices[1].y)/2};
    Point mid2 = {(vertices[0].x+vertices[2].x)/2,(vertices[0].y+vertices[2].y)/2};
    Point mid3 = {(vertices[1].x+vertices[2].x)/2,(vertices[1].y+vertices[2].y)/2};
    return {mid1,mid2,mid3};
}
