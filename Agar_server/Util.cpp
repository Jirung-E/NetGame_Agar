#include "Util.h"


COLORREF getRandomColor() {
	std::uniform_int_distribution<> random_color { 0 + 0x11, 0xFF - 0x11 };
	return RGB(random_color(rd), random_color(rd), random_color(rd));
}


COLORREF getRandomColor(const Range& r, const Range& g, const Range& b, const Range& base) {
	std::uniform_int_distribution<> r_base { int(base.min), int(base.max) };
	std::uniform_int_distribution<> r_r { int(r.min), int(r.max) };
	std::uniform_int_distribution<> r_g { int(g.min), int(g.max) };
	std::uniform_int_distribution<> r_b { int(b.min), int(b.max) };
	int bs = r_base(rd);
	return RGB(bs + r_r(rd), bs + r_g(rd), bs + r_b(rd));
}

COLORREF getRandomColor(int r, int g, int b, const Range& base) {
	std::uniform_int_distribution<> r_base { int(base.min), int(base.max) };
	int bs = r_base(rd);
	return RGB(r + bs, g + bs, b + bs);
}


POINT getCenterOf(const RECT& rect) {
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	return { rect.left + width/2, rect.top + height/2 };
}

RECT rectToSquare(const RECT& rect) {
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	int n = width < height ? width/2 : height/2;
	POINT center = getCenterOf(rect);
	return { center.x-n, center.y-n, center.x+n, center.y+n };
}

RECT expand(const RECT& rect, int percentage) {
	int w = rect.right - rect.left;
	int h = rect.bottom - rect.top;
	RECT r;
	r.left = rect.left - w*percentage/100;
	r.right = rect.right + w*percentage/100;
	r.top = rect.top - h*percentage/100;
	r.bottom = rect.bottom + h*percentage/100;
	return r;
}

int fps(int frame) {
	return round(1000.0/frame);
}


RectSize& RectSize::operator*=(double n) {
	this->width *= n;
	this->height *= n;
	return *this;
}

RectSize& RectSize::operator+=(double n) {
	this->width += n;
	this->height += n;
	return *this;
}

RectSize& RectSize::operator-=(double n) {
	this->width -= n;
	this->height -= n;
	return *this;
}

RectSize RectSize::operator/(double n) {
	return RectSize { width / n, height / n };
}

RectSize RectSize::operator+(double n) {
	return RectSize { width + n, height + n };
}

RectSize RectSize::operator+(const RectSize& rs) {
	return { width + rs.width, height + rs.height };
}

RectSize RectSize::operator-(const RectSize& rs) {
	return { width - rs.width, height - rs.height };
}

RectSize& RectSize::operator-=(const RectSize& rs) {
	width -= rs.width;
	height -= rs.height;
	return *this;
}

RectSize RectSize::expand(int percentage) const {
	RectSize r;
	r.width = width + width*percentage/100;
	r.height = height + height*percentage/100;
	return r;
}

RectSize RectSize::square() const {
	if(width < height) {
		return RectSize { width, width };
	}
	else {
		return RectSize { height, height };
	}
}


Direction& operator++(Direction& dir) {
	dir = Direction((int(dir) + 1) % 4);
	return dir;
}

Direction operator++(Direction& dir, int) {
	Direction temp = dir;
	dir = Direction((int(dir) + 1) % 4);
	return temp;
}

Direction& operator--(Direction& dir) {
	dir = Direction((int(dir) + 3) % 4);
	return dir;
}

Direction operator--(Direction& dir, int) {
	Direction temp = dir;
	dir = Direction((int(dir) + 3) % 4);
	return temp;
}


RECT& operator%=(RECT& rect, double per) {
	int w = rect.right - rect.left;
	int h = rect.bottom - rect.top;
	rect.left += w * (100 - per)/2 / 100;
	rect.right -= w * (100 - per)/2 / 100;
	rect.top += h * (100 - per)/2 / 100;
	rect.right -= h * (100 - per)/2 / 100;

	return rect;
}


Range::Range(double min, double max) : min { min }, max { max } {
	
}

bool Range::isContain(double num) const {
	if(min <= num && num <= max) {
		return true;
	}
	return false;
}


double getRandomNumberOf(const Range& range, double interval) {
	int count = (range.max - range.min) / interval;
	std::uniform_int_distribution<> uid { 0, count };
	return range.min + interval * uid(rd);
}