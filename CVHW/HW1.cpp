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
public:
	static void DrawCircle(CImg<unsigned char>* image, bool useCImg = false) {
		float x = 60.0;
		float y = 60.0;
		float r = 20.0;
		unsigned char yellow[] = { 255, 255, 0 };

		if (useCImg) {
			image->draw_circle(60, 60, 20, yellow, 1.0);
		}
		else {
			for (int w = x - r; w < x + r; w++) {
				for (int h = y - r; h < y + r; h++) {
					if (HW1Utils::L2Distance(w, h, x, y) <= r) {
						for (int c = 0; c < 3; c++) {
							image->set_linear_atXYZ(yellow[c], w, h, 0, c);
						}
					}
				}
			}
		}
	}

	static void DrawLine(CImg<unsigned char>* image) {
		int len;
	}
};

void HW1() {
	CImg<unsigned char> image;
	string path = "C:/Users/User/OneDrive/资料/研二/计算机视觉助教/第1次作业/1.bmp";
	image.load(path.c_str());
	unsigned char green[] = { 0, 255, 0 };

	const float opacity = 1.0;

	HW1Utils::DrawCircle(&image, true);

	//image.draw_triangle(60, 73, 45, 47, 75, 47, green, 1.0);
	image.display();
}