#include "Map.h"


Map::Map() : width { 160/5 }, height { 90/5 }, 
background_color { White }, border_color { LightGray }, border_width { 2 } {

}


void Map::draw(const HDC& hdc, const RECT& valid_area) const {
	HBRUSH bg_br = CreateSolidBrush(background_color);
	HPEN bd_pen = CreatePen(PS_SOLID, border_width, border_color);
	HBRUSH old_br = (HBRUSH)SelectObject(hdc, bg_br);
	HPEN old_pen = (HPEN)SelectObject(hdc, bd_pen);

	RECT rect = absoluteArea(valid_area);
	int w = rect.right - rect.left;
	int h = rect.bottom - rect.top;

	Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

	for(int i = 0; i <= height; ++i) {
		MoveToEx(hdc, rect.left, rect.top + i * h / height, NULL);
		LineTo(hdc, rect.right, rect.top + i * h / height);
	}
	for(int i = 0; i <= width; ++i) {
		MoveToEx(hdc, rect.left + i * w / width, rect.top, NULL);
		LineTo(hdc, rect.left + i * w / width, rect.bottom);
	}

	SelectObject(hdc, old_br);
	SelectObject(hdc, old_pen);
	DeleteObject(bd_pen);
	DeleteObject(bg_br);
}

RECT Map::absoluteArea(const RECT& valid_area) const {
	int w = valid_area.right - valid_area.left;
	int h = valid_area.bottom - valid_area.top;
	RECT rect = valid_area;
	if(double(w)/h < double(width)/height) {
		int n = h-w*double(height)/width;
		h -= n;
		rect.top = rect.top + n/2;
		rect.bottom = rect.bottom - n/2;
	}
	else if(double(w)/h > double(width)/height) {
		int n = w-h*double(width)/height;
		w -= n;
		rect.left = rect.left + n/2;
		rect.right = rect.right - n/2;
	}
	return rect;
}


int Map::getWidth() const {
    return width;
}

int Map::getHeight() const {
    return height;
}