#include "Scene.h"


Scene::Scene(const SceneID& id) : id { id }, valid_area { 0, 0, 0, 0 }, 
horizontal_length { 0 }, vertical_length { 0 } {

}


void Scene::drawBackground(const HDC& hdc, const COLORREF& color) const {
	HBRUSH br = CreateSolidBrush(color);
	HBRUSH old = (HBRUSH)SelectObject(hdc, br);
	Rectangle(hdc, valid_area.left, valid_area.top, valid_area.right, valid_area.bottom);
	SelectObject(hdc, old);
	DeleteObject(br);
}

void Scene::show(const HDC& hdc) const {
	draw(hdc);
}

void Scene::syncSize(const HWND& hWnd) {
	RECT rect;
	GetClientRect(hWnd, &rect);
	valid_area = rect;
	horizontal_length = valid_area.right - valid_area.left;
	vertical_length = valid_area.bottom - valid_area.top;
}

SceneID Scene::getID() const {
    return id;
}

ButtonID Scene::clickL(const POINT& point) const {
	return None;
}

ButtonID Scene::clickR(const POINT& point) const {
	return None;
}