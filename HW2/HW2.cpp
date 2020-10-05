#include <iostream>
#include "CImg.h"
#include "CannyEdgeDetector.h"
using namespace std;
using namespace cimg_library;

int main() {
	CImg<unsigned char>* image = new CImg<unsigned char>();
	string path = "C:/Users/User/OneDrive/资料/研二/计算机视觉助教/第二次作业/test_Data/lena.bmp";
	image->load(path.c_str());
	CannyEdgeDetector cannyEdgeDetector;
	CImg<unsigned char>* answer = cannyEdgeDetector.ProcessImage(image, image->width(), image->height());
	path = "C:/Users/User/Desktop/temp/answer.bmp";
	answer->save(path.c_str());
}
