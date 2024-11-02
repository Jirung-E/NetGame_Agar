#pragma once

#include <Windows.h>
#include <random>
#include <string>

using tstring = std::basic_string<TCHAR>;
static std::random_device rd;


enum Color {
	Red = 0x0000ff, Green = 0x00ff00, Blue = 0xff0000, 
	Yellow = 0x00ffff, Cyan = 0xffff00, Magenta = 0xff00ff, 
	Black = 0x000000, White = 0xffffff, LightGray = 0xaaaaaa, Gray = 0x888888, DarkGray = 0x666666
};


class Range {
public:
	double min;
	double max;
	
public:
	Range(double min, double max);

public:
	bool isContain(double num) const;
};

double getRandomNumberOf(const Range& range, double interval = 1);
COLORREF getRandomColor();
COLORREF getRandomColor(const Range& r, const Range& g, const Range& b, const Range& base = { 0, 0 });
COLORREF getRandomColor(int r, int g, int b, const Range& base = { 0, 0xff });
POINT getCenterOf(const RECT& rect);
RECT rectToSquare(const RECT& rect);
RECT expand(const RECT& rect, int percentage);
int fps(int frame);

class RectSize {
public:
	double width;
	double height;

public:
	RectSize& operator*=(double n);
	RectSize& operator+=(double n);
	RectSize& operator-=(double n);
	RectSize operator/(double n);
	RectSize operator+(double n);
	RectSize operator+(const RectSize& rs);
	RectSize operator-(const RectSize& rs);
	RectSize& operator-=(const RectSize& rs);

public:
	RectSize expand(int percentage) const;
	RectSize square() const;
};


enum Direction {
	Right, Down, Left, Up
};

Direction& operator++(Direction& dir);
Direction operator++(Direction& dir, int);
Direction& operator--(Direction& dir);
Direction operator--(Direction& dir, int);

RECT& operator%=(RECT& rect, double per);