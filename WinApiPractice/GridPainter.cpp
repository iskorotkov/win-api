﻿#include "GridPainter.h"
#include <stdexcept>

GridPainter::GridPainter(HWND& window, const int dimension) : window(window), dimension(dimension)
{
	hdc = BeginPaint(window, &ps);
}

void GridPainter::DrawImage(const WindowArea area, Image& img) const
{
	const auto hBitmap = CreateBitmap(img.Width, img.Height, 1, img.BitDepth, img.Buffer);
	const auto hdcMem = CreateCompatibleDC(hdc);

	SelectObject(hdcMem, hBitmap);
	const auto width = img.Width;
	const auto height = img.Height;
	const auto x = area.CenterX;
	const auto y = area.CenterY;
	TransparentBlt(hdc,
		x - width / 2,
		y - height / 2,
		width,
		height,
		hdcMem,
		0,
		0,
		width,
		height,
		RGB(0, 0, 0));

	DeleteDC(hdcMem);
	DeleteObject(hBitmap);
}

void GridPainter::DrawImageWhere(int value, const int* values, Image& img) const
{
	const auto callback = [this, value, &img](auto index, auto val)
	{
		if (val == value)
		{
			const auto area = CalculateIconDimensions(index);
			DrawImage(area, img);
		}
	};
	ForEachCell(values, callback);
}

void GridPainter::DrawGrid(COLORREF gridColor) const
{
	RECT rect;
	GetClientRect(window, &rect);
	const auto pen = CreatePen(0, 2, gridColor);
	const auto prevBrush = SelectObject(hdc, pen);

	for (auto i = 1u; i < dimension; ++i)
	{
		const UINT x = rect.right * i / dimension;
		MoveToEx(hdc, x, 0, nullptr);
		LineTo(hdc, x, rect.bottom);

		const UINT y = rect.bottom * i / dimension;
		MoveToEx(hdc, 0, y, nullptr);
		LineTo(hdc, rect.right, y);
	}

	SelectObject(hdc, prevBrush);
	DeleteObject(pen);
}

GridPainter::~GridPainter()
{
	EndPaint(window, &ps);
}

void GridPainter::DrawCircle(const WindowArea area) const
{
	const auto hBrush = CreateSolidBrush(RGB(255, 255, 255));
	const auto prevBrush = SelectObject(hdc, hBrush);

	Ellipse(hdc,
		area.CenterX - area.Radius,
		area.CenterY - area.Radius,
		area.CenterX + area.Radius,
		area.CenterY + area.Radius);

	SelectObject(hdc, prevBrush);
	DeleteObject(hBrush);
}

void GridPainter::DrawCross(const WindowArea area) const
{
	// Draws '\'
	MoveToEx(hdc,
		area.CenterX - area.Radius,
		area.CenterY - area.Radius,
		nullptr);
	LineTo(hdc,
		area.CenterX + area.Radius,
		area.CenterY + area.Radius);
	// Draws '/'
	MoveToEx(hdc,
		area.CenterX + area.Radius,
		area.CenterY - area.Radius,
		nullptr);
	LineTo(hdc,
		area.CenterX - area.Radius,
		area.CenterY + area.Radius);
}

HDC& GridPainter::GetHDC()
{
	return hdc;
}

WindowArea GridPainter::CalculateIconDimensions(const UINT index) const
{
	const auto row = index % dimension;
	const auto col = index / dimension;
	RECT rect;
	GetClientRect(window, &rect);
	const UINT height = rect.bottom;
	const UINT width = rect.right;
	WindowArea area;
	area.Radius = min(height/dimension, width/dimension) / 3;
	area.CenterX = width * (2 * row + 1) / (2 * dimension);
	area.CenterY = height * (2 * col + 1) / (2 * dimension);
	return area;
}

void GridPainter::ForEachCell(const int* values, const std::function<void(CellIndex, CellValue)>& callback, const bool ignoreZero) const
{
	for (UINT i = 0u, len = dimension * dimension; i < len; ++i)
	{
		if (values[i] || !ignoreZero)
		{
			callback(i, values[i]);
		}
	}
}

void GridPainter::DrawIconsOnGrid(const int* values) const
{
	const auto callback = [this](auto index, auto val)
	{
		const auto area = CalculateIconDimensions(index);
		switch (val)
		{
			case 1:
			{
				DrawCircle(area);
				break;
			}
			case 2:
			{
				DrawCross(area);
				break;
			}
			default:
				throw std::logic_error("Unknown grid value.");
		}
	};
	ForEachCell(values, callback);
}
