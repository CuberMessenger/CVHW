#include <iostream>
#include <cmath>
#include "CImg.h"
using namespace std;
using namespace cimg_library;

class HW1Utils {
private:
	static float L2Distance(float x0, float y0, float x1, float y1) {
		float deltaX = x0 - x1;
		float deltaY = y0 - y1;
		return sqrt(deltaX * deltaX + deltaY * deltaY);
	}

	static void RotatePoint(float& x, float& y, float x0, float y0, float angle, bool forward = true) {
		angle = (angle / 180.0) * 3.1415926;
		float answerX = 0.0, answerY = 0.0;

		if (forward) {
			answerX = (x - x0) * cos(angle) - (y - y0) * sin(angle) + x0;
			answerY = (x - x0) * sin(angle) + (y - y0) * cos(angle) + y0;
		}
		else {
			answerX = (x - x0) * cos(angle) + (y - y0) * sin(angle) + x0;
			answerY = -(x - x0) * sin(angle) + (y - y0) * cos(angle) + y0;
		}

		x = answerX;
		y = answerY;
	}
public:
	static void DrawCircle(CImg<unsigned char>* image, float x, float y, float r, bool useCImg = false) {
		//float x = 60.0;
		//float y = 60.0;
		//float r = 2.0;
		unsigned char yellow[] = { 255, 255, 0 };

		if (useCImg) {
			image->draw_circle(x, y, r, yellow, 1.0);
		}
		else {
			for (int w = x - r; w < x + r; w++) {
				for (int h = y - r; h < y + r; h++) {
					if (HW1Utils::L2Distance(w, h, x, y) <= r) {
						for (int c = 0; c < 3; c++) {
							image->set_linear_atXY(yellow[c], w, h, 0, c);
						}
					}
				}
			}
		}
	}

	static void DrawLine(CImg<unsigned char>* image, bool useCImg) {
		float length = 100.0;
		float x = 0.0;
		float y = 0.0;
		unsigned char red[] = { 255, 0, 0 };

		if (useCImg) {
			float sideLength = 50.0 * sqrt(2);
			image->draw_line(0, 0, sideLength, sideLength, red, 1.0, 0xFFFFFFFF, false);
		}
		else {
			for (int i = 0; i < 100; i++) {
				float xi = i;
				float yi = 0;
				HW1Utils::RotatePoint(xi, yi, x, y, 45.0);
				for (int c = 0; c < 3; c++) {
					image->set_linear_atXY(red[c], xi, yi, 0, c);
				}
			}
		}
	}

	static CImg<unsigned char>* RotateImage(CImg<unsigned char>* image, float angle, bool useCImg, bool useForward) {
		int width = image->width();
		int height = image->height();

		if (useCImg) {
			image->rotate(angle);
			return image;
		}

		float left = (float)(0x7FFFFFFF);
		float right = -(float)(0x7FFFFFFF);
		float top = (float)(0x7FFFFFFF);
		float bottom = -(float)(0x7FFFFFFF);
		float centerX = width / 2;
		float centerY = height / 2;
		float xs[] = { 0.0, width - 1, 0.0, width - 1 };
		float ys[] = { 0.0, 0.0, height - 1, height - 1 };
		for (int i = 0; i < 4; i++) {
			HW1Utils::RotatePoint(xs[i], ys[i], centerX, centerY, angle);
			left = min(left, xs[i]);
			right = max(right, xs[i]);
			top = min(top, ys[i]);
			bottom = max(bottom, ys[i]);
		}
		float newWidth = right - left + 1.0;
		float newHeight = bottom - top + 1.0;
		float deltaX = (newWidth - width) / 2;
		float deltaY = (newHeight - height) / 2;
		CImg<unsigned char>* answer = new CImg<unsigned char>(ceil(newWidth), ceil(newHeight), 1, 3, 0);
		if (useForward) {
			for (int w = 0; w < width; w++) {
				for (int h = 0; h < height; h++) {
					float x = w;
					float y = h;
					HW1Utils::RotatePoint(x, y, centerX, centerY, angle);
					for (int c = 0; c < 3; c++) {
						answer->set_linear_atXY(image->operator()(w, h, 0, c), x + deltaX, y + deltaY, 0, c);
					}
				}
			}
		}
		else {
			for (int w = 0; w < ceil(newWidth); w++) {
				for (int h = 0; h < ceil(newHeight); h++) {
					float x = w - deltaX;
					float y = h - deltaY;
					HW1Utils::RotatePoint(x, y, centerX, centerY, angle, false);
					x = round(x);
					y = round(y);
					if ((x >= 0) && (x <= width - 1) && (y >= 0) && (y <= height - 1)) {
						for (int c = 0; c < 3; c++) {
							answer->set_linear_atXY(image->operator()((int)x, (int)y, 0, c), w, h, 0, c);
						}
					}

				}
			}
		}
		return answer;
	}
};

void HW1() {
	CImg<unsigned char> image;
	string path = "C:/Users/User/OneDrive/资料/研二/计算机视觉助教/第1次作业/1.bmp";
	image.load(path.c_str());
	unsigned char green[] = { 0, 255, 0 };

	const float opacity = 1.0;

	//2
	//image.draw_triangle(60, 73, 45, 47, 75, 47, green, 1.0);
	//3
	image.atXYZC(60, 60, 0, 0) = 255;
	image.atXYZC(70, 60, 0, 0) = 255;
	//4
	//HW1Utils::DrawLine(&image, false);
	//5
	//CImg<unsigned char>* rotated = HW1Utils::RotateImage(&image, 40.0, false, false);
	//rotated->display();

	image.display();
}