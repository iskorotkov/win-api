﻿#include "GridPainter.h"
#include <stdexcept>
#include "GameState.h"

GridPainter::GridPainter(HWND& window, const int dimension) : window(window), dimension(dimension)
{
	StartDrawing();
}

void GridPainter::DrawImage(const WindowArea area, Image& img) const
{
	const auto hBitmap = CreateBitmap(img.Width, img.Height, 1, img.BitDepth, img.Buffer);
	const auto hdcMem = CreateCompatibleDC(tempHdc);

	SelectObject(hdcMem, hBitmap);
	const auto width = img.Width;
	const auto height = img.Height;
	const auto x = area.CenterX;
	const auto y = area.CenterY;
	TransparentBlt(tempHdc,
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

void GridPainter::DrawImageWhere(int value, GameState* gameState, Image& img) const
{
	// TODO: Remove duplicated code.
	const auto callback = [this, value, &img](auto line, auto column, auto val)
	{
		if (val == value)
		{
			const auto area = CalculateIconDimensions(line, column);
			DrawImage(area, img);
		}
	};
	ForEachCell(gameState, callback);
}

void GridPainter::DrawGrid(COLORREF gridColor) const
{
	RECT rect;
	GetClientRect(window, &rect);
	const auto pen = CreatePen(0, 2, gridColor);
	const auto prevBrush = SelectObject(tempHdc, pen);

	for (auto i = 1u; i < dimension; ++i)
	{
		const UINT x = rect.right * i / dimension;
		MoveToEx(tempHdc, x, 0, nullptr);
		LineTo(tempHdc, x, rect.bottom);

		const UINT y = rect.bottom * i / dimension;
		MoveToEx(tempHdc, 0, y, nullptr);
		LineTo(tempHdc, rect.right, y);
	}

	SelectObject(tempHdc, prevBrush);
	DeleteObject(pen);
}

GridPainter::~GridPainter()
{
	FinishDrawing();
}

void GridPainter::DrawCircle(const WindowArea area) const
{
	const auto hBrush = CreateSolidBrush(RGB(255, 255, 255));
	const auto prevBrush = SelectObject(tempHdc, hBrush);

	Ellipse(tempHdc,
		area.CenterX - area.Radius,
		area.CenterY - area.Radius,
		area.CenterX + area.Radius,
		area.CenterY + area.Radius);

	SelectObject(tempHdc, prevBrush);
	DeleteObject(hBrush);
}

void GridPainter::DrawCross(const WindowArea area) const
{
	// Draws '\'
	MoveToEx(tempHdc,
		area.CenterX - area.Radius,
		area.CenterY - area.Radius,
		nullptr);
	LineTo(tempHdc,
		area.CenterX + area.Radius,
		area.CenterY + area.Radius);
	// Draws '/'
	MoveToEx(tempHdc,
		area.CenterX + area.Radius,
		area.CenterY - area.Radius,
		nullptr);
	LineTo(tempHdc,
		area.CenterX - area.Radius,
		area.CenterY + area.Radius);
}

void GridPainter::DrawGradient(const COLORREF color) const
{
	const auto size = 2;
	TRIVERTEX vertices[size];
	GRADIENT_RECT gradientRect;
	gradientRect.UpperLeft = 0;
	gradientRect.LowerRight = 1;

	const auto colorValue = [](auto color) { return color << 8; };

	auto index = 0;
	vertices[index].Red = colorValue(GetRValue(color));
	vertices[index].Green = colorValue(GetGValue(color));
	vertices[index].Blue = colorValue(GetBValue(color));
	vertices[index].Alpha = colorValue(255);
	vertices[index].x = 0;
	vertices[index].y = 0;

	++index;
	// TODO: Use random color for second vertex.
	vertices[index].Red = colorValue(255);
	vertices[index].Green = colorValue(255);
	vertices[index].Blue = colorValue(255);
	vertices[index].Alpha = colorValue(255);
	RECT rect;
	GetClientRect(window, &rect);
	vertices[index].x = rect.right;
	vertices[index].y = rect.bottom;

	GradientFill(tempHdc, vertices, size, &gradientRect, 1, GRADIENT_FILL_RECT_H);
}

void GridPainter::DrawDynamicGradient(const COLORREF primary, const COLORREF secondary) const
{
	const auto size = 2;
	TRIVERTEX vertices[size];
	GRADIENT_RECT gradientRect;
	gradientRect.UpperLeft = 0;
	gradientRect.LowerRight = 1;

	const auto colorValue = [](auto color) { return color << 8; };

	auto index = 0;
	vertices[index].Red = colorValue(GetRValue(primary));
	vertices[index].Green = colorValue(GetGValue(primary));
	vertices[index].Blue = colorValue(GetBValue(primary));
	vertices[index].Alpha = colorValue(255);
	vertices[index].x = 0;
	vertices[index].y = 0;

	++index;
	// TODO: Use random color for second vertex.
	vertices[index].Red = colorValue(GetRValue(secondary));
	vertices[index].Green = colorValue(GetGValue(secondary));
	vertices[index].Blue = colorValue(GetBValue(secondary));
	vertices[index].Alpha = colorValue(255);
	RECT rect;
	GetClientRect(window, &rect);
	vertices[index].x = rect.right;
	vertices[index].y = rect.bottom;

	GradientFill(tempHdc, vertices, size, &gradientRect, 1, GRADIENT_FILL_RECT_H);
}

void GridPainter::StartDrawing()
{
	originalHdc = GetDC(window);
	tempHdc = CreateCompatibleDC(originalHdc);
	RECT rect;
	GetClientRect(window, &rect);
	const auto bitmap = CreateCompatibleBitmap(originalHdc, rect.right, rect.bottom);
	oldBitmap = SelectObject(tempHdc, bitmap);
	DeleteObject(oldBitmap);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void GridPainter::FinishDrawing()
{
	RECT rect;
	GetClientRect(window, &rect);
	BitBlt(originalHdc, 0, 0, rect.right, rect.bottom, tempHdc, 0, 0, SRCCOPY);
	const auto bitmap = SelectObject(tempHdc, oldBitmap);
	DeleteObject(bitmap);
	DeleteDC(tempHdc);
	ReleaseDC(window, originalHdc);
}

// ReSharper disable once CppInconsistentNaming
HDC& GridPainter::GetHDC()
{
	return tempHdc;
}

WindowArea GridPainter::CalculateIconDimensions(const UINT index) const
{
	const auto column = index % dimension;
	const auto line = index / dimension;
	return CalculateIconDimensions(line, column);
}

WindowArea GridPainter::CalculateIconDimensions(const int line, const int column) const
{
	RECT rect;
	GetClientRect(window, &rect);
	const UINT height = rect.bottom;
	const UINT width = rect.right;
	WindowArea area;
	area.Radius = min(height/dimension, width/dimension) / 3;
	area.CenterX = width * (2 * column + 1) / (2 * dimension);
	area.CenterY = height * (2 * line + 1) / (2 * dimension);
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

void GridPainter::ForEachCell(GameState* gameState, const std::function<void(CellIndex, CellIndex, CellValue)>& callback, bool ignoreZero) const
{
	for (auto i = 0; i < dimension; ++i)
	{
		for (auto j = 0; j < dimension; ++j)
		{
			const auto value = gameState->GetAt(i, j);
			if (value || !ignoreZero)
			{
				callback(i, j, value);
			}
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

void GridPainter::DrawIconsOnGrid(GameState* gameState) const
{
	// TODO: Duplicated lambda function.
	const auto callback = [this](auto line, int column, auto val)
	{
		const auto area = CalculateIconDimensions(line, column);
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
	ForEachCell(gameState, callback);
}
