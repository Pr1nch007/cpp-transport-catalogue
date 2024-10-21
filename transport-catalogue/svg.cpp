#include "svg.h"

namespace svg {

using namespace std::literals;
    
std::ostream& operator<<(std::ostream &os, const StrokeLineCap &cap) {
    switch (cap) {
        case StrokeLineCap::BUTT:
            return os << "butt"sv;
        case StrokeLineCap::ROUND:
            return os << "round"sv;
        case StrokeLineCap::SQUARE:
            return os << "square"sv;
    }
    return os;
}
    
std::ostream& operator<<(std::ostream &os, const StrokeLineJoin &join) {
    switch (join) {
        case StrokeLineJoin::ARCS:
            return os << "arcs"sv;
        case StrokeLineJoin::BEVEL:
            return os << "bevel"sv;
        case StrokeLineJoin::MITER:
            return os << "miter"sv;
        case StrokeLineJoin::MITER_CLIP:
            return os << "miter-clip"sv;
        case StrokeLineJoin::ROUND:
            return os << "round"sv;
    }
    return os;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();
    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;
    
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ------------------
    
Polyline& Polyline::AddPoint(Point point){
    points_.push_back(point);
    return *this;
}
    
void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    size_t x = 0;
    
    for(auto& i : points_){
        ++x;
        out << i.x << ","sv << i.y;
        if(x != points_.size()){
            out << ' ';
        }
    }
    
    out << "\" "sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}
    
// ---------- Text ------------------
    
Text& Text::SetPosition(Point pos){
    position_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset){
    offset_ = offset;
    return *this;
}
    
Text& Text::SetFontSize(uint32_t size){
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family){
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight){
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data){
    data_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext& context) const{
    auto& out = context.out;
    out << "<text "s;
    RenderAttrs(context.out);
    
    out << " x=\""sv << position_.x << "\" y=\""sv << position_.y << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" font-size=\""sv << size_;
    
    if(!font_family_.empty()){
        out << "\" font-family=\""sv << font_family_;
    }
    if(!font_weight_.empty()){
        out << "\" font-weight=\""sv << font_weight_;
    }
    
    out << "\">"sv << data_;
    out  << "</text>"sv;
}
    
// ---------- Document ------------------
    
void Document::AddPtr(std::unique_ptr<Object>&& obj){
    objects_.emplace_back(std::move(obj));
}
    
void Document::Render(std::ostream& out) const{
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    
    for(auto& i : objects_){
        RenderContext ctx(out,2,2);
        i->Render(ctx);
    }
    
    out << "</svg>"sv;
}

}  // namespace svg