/**
 * \file      CannyEdgeDetector.cpp
 * \brief     Canny algorithm class file.
 * \details   This file is part of student project. Some parts of code may be
 *            influenced by various examples found on internet.
 * \author    resset <silentdemon@gmail.com>
 * \date      2006-2012
 * \copyright GNU General Public License, http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include <math.h>
#include "CImg.h"
#include "CannyEdgeDetector.h"
using namespace cimg_library;

CannyEdgeDetector::CannyEdgeDetector() {
	width = (unsigned int)0;
	height = (unsigned int)0;
	x = (unsigned int)0;
	y = (unsigned int)0;
	mask_halfsize = (unsigned int)0;
}

CannyEdgeDetector::~CannyEdgeDetector() {
	delete edge_magnitude;
	delete edge_direction;
	delete workspace_bitmap;
}

CImg<unsigned char>* CannyEdgeDetector::ProcessImage(CImg<unsigned char>* source_bitmap, unsigned int width,
	unsigned int height, float sigma,
	uint8_t lowThreshold, uint8_t highThreshold) {
	/*
	 * Setting up image width and height in pixels.
	 */
	this->width = width;
	this->height = height;

	/*
	 * We store image in array of bytes (chars) in BGR(BGRBGRBGR...) order.
	 * Size of the table is width * height * 3 bytes.
	 */
	this->source_bitmap = source_bitmap;

	/*
	 * Conversion to grayscale. Only luminance information remains.
	 */
	this->Luminance();

	/*
	 * "Widening" image. At this step we already need to know the size of
	 * gaussian mask.
	 */
	this->PreProcessImage(sigma);

	/*
	 * Noise reduction - Gaussian filter.
	 */
	this->GaussianBlur(sigma);

	/*
	 * Edge detection - Sobel filter.
	 */
	this->EdgeDetection();

	/*
	 * Suppression of non maximum pixels.
	 */
	this->NonMaxSuppression();

	/*
	 * Hysteresis thresholding.
	 */
	this->Hysteresis(lowThreshold, highThreshold);

	/*
	 * "Shrinking" image.
	 */
	this->PostProcessImage();

	return source_bitmap;
}

inline uint8_t CannyEdgeDetector::GetPixelValue(unsigned int x, unsigned int y) {
	return this->workspace_bitmap->operator()(y, x, 0, 0);
}

inline void CannyEdgeDetector::SetPixelValue(unsigned int x, unsigned int y, uint8_t value) {
	this->workspace_bitmap->operator()(y, x, 0, 0) = value;
}

void CannyEdgeDetector::PreProcessImage(float sigma) {
	// Finding mask size with given sigma.
	mask_size = 2 * round(sqrt(-log(0.3) * 2 * sigma * sigma)) + 1;
	mask_halfsize = mask_size / 2;

	// Enlarging workspace bitmap width and height.
	height += mask_halfsize * 2;
	width += mask_halfsize * 2;
	// Working area.
	this->workspace_bitmap = new CImg<unsigned char>(width, height, 1, 1);

	// Edge information arrays.
	this->edge_magnitude = new CImg<unsigned char>(width, height, 1, 1);
	this->edge_direction = new CImg<unsigned char>(width, height, 1, 1);

	// Zeroing direction array.
	for (x = 0; x < height; x++) {
		for (y = 0; y < width; y++) {
			this->edge_direction->operator()(y, x, 0, 0) = 0;
		}
	}

	// Copying image data into work area.
	for (x = 0; x < height; x++) {
		for (y = 0; y < width; y++) {
			// Upper left corner.
			if (x < mask_halfsize && y < mask_halfsize) {
				SetPixelValue(x, y, this->source_bitmap->operator()(0, 0, 0, 0));
			}
			// Bottom left corner.
			else if (x >= height - mask_halfsize && y < mask_halfsize) {
				int rawIndex = (height - 2 * mask_halfsize - 1) * (width - 2 * mask_halfsize);
				int row = rawIndex / width;
				int column = rawIndex % width;
				SetPixelValue(x, y, this->source_bitmap->operator()(column, row, 0, 0));
			}
			// Upper right corner.
			else if (x < mask_halfsize && y >= width - mask_halfsize) {
				int rawIndex = (width - 2 * mask_halfsize - 1);
				int row = rawIndex / width;
				int column = rawIndex % width;
				SetPixelValue(x, y, this->source_bitmap->operator()(column, row, 0, 0));
			}
			// Bottom right corner.
			else if (x >= height - mask_halfsize && y >= width - mask_halfsize) {
				int rawIndex = (height - 2 * mask_halfsize - 1) * (width - 2 * mask_halfsize) + (width - 2 * mask_halfsize - 1);
				int row = rawIndex / width;
				int column = rawIndex % width;
				SetPixelValue(x, y, this->source_bitmap->operator()(column, row, 0, 0));
			}
			// Upper beam.
			else if (x < mask_halfsize) {
				int rawIndex = (y - mask_halfsize);
				int row = rawIndex / width;
				int column = rawIndex % width;
				SetPixelValue(x, y, this->source_bitmap->operator()(column, row, 0, 0));
			}
			// Bottom beam.
			else if (x >= height - mask_halfsize) {
				int rawIndex = (height - 2 * mask_halfsize - 1) * (width - 2 * mask_halfsize) + (y - mask_halfsize);
				int row = rawIndex / width;
				int column = rawIndex % width;
				SetPixelValue(x, y, this->source_bitmap->operator()(column, row, 0, 0));
			}
			// Left beam.
			else if (y < mask_halfsize) {
				int rawIndex = (x - mask_halfsize) * (width - 2 * mask_halfsize);
				int row = rawIndex / width;
				int column = rawIndex % width;
				SetPixelValue(x, y, this->source_bitmap->operator()(column, row, 0, 0));
			}
			// Right beam.
			else if (y >= width - mask_halfsize) {
				int rawIndex = (x - mask_halfsize) * (width - 2 * mask_halfsize) + (width - 2 * mask_halfsize - 1);
				int row = rawIndex / width;
				int column = rawIndex % width;
				SetPixelValue(x, y, this->source_bitmap->operator()(column, row, 0, 0));
			}
			// The rest of the image.
			else {
				int rawIndex = (x - mask_halfsize) * (width - 2 * mask_halfsize) + (y - mask_halfsize);
				int row = rawIndex / width;
				int column = rawIndex % width;
				SetPixelValue(x, y, this->source_bitmap->operator()(column, row, 0, 0));
			}
		}
	}
}

void CannyEdgeDetector::PostProcessImage() {
	// Decreasing width and height.
	height -= 2 * mask_halfsize;
	width -= 2 * mask_halfsize;

	// Shrinking image.
	for (x = 0; x < height; x++) {
		for (y = 0; y < width; y++) {
			this->source_bitmap->operator()(y, x, 0, 0) = this->workspace_bitmap->operator()(y + mask_halfsize, x + mask_halfsize, 0, 0);
			this->source_bitmap->operator()(y, x, 0, 1) = this->workspace_bitmap->operator()(y + mask_halfsize, x + mask_halfsize, 0, 0);
			this->source_bitmap->operator()(y, x, 0, 2) = this->workspace_bitmap->operator()(y + mask_halfsize, x + mask_halfsize, 0, 0);
		}
	}
}

void CannyEdgeDetector::Luminance() {
	unsigned long i;
	float gray_value, blue_value, green_value, red_value;

	for (x = 0; x < height; x++) {
		for (y = 0; y < width; y++) {
			// The order of bytes is RGB.
			red_value = this->source_bitmap->operator()(y, x, 0, 0);
			green_value = this->source_bitmap->operator()(y, x, 0, 1);
			blue_value = this->source_bitmap->operator()(y, x, 0, 2);

			// Standard equation from RGB to grayscale.
			gray_value = (uint8_t)(0.299 * red_value + 0.587 * green_value + 0.114 * blue_value);

			// Ultimately making picture grayscale.
			this->source_bitmap->operator()(y, x, 0, 0) = gray_value;
			this->source_bitmap->operator()(y, x, 0, 1) = gray_value;
			this->source_bitmap->operator()(y, x, 0, 2) = gray_value;
		}
	}
}

void CannyEdgeDetector::GaussianBlur(float sigma) {
	// We already calculated mask size in PreProcessImage.
	long signed_mask_halfsize = this->mask_halfsize;

	CImg<unsigned char>* gaussianMask = new CImg<unsigned char>(mask_size, mask_size, 1, 1);

	for (int i = -signed_mask_halfsize; i <= signed_mask_halfsize; i++) {
		for (int j = -signed_mask_halfsize; j <= signed_mask_halfsize; j++) {
			gaussianMask->operator()(j + signed_mask_halfsize, i + signed_mask_halfsize, 0, 0)
				= (1 / (2 * PI * sigma * sigma)) * exp(-(i * i + j * j) / (2 * sigma * sigma));
		}
	}

	unsigned long i;
	unsigned long i_offset;
	int row_offset;
	int col_offset;
	float new_pixel;

	for (x = signed_mask_halfsize; x < height - signed_mask_halfsize; x++) {
		for (y = signed_mask_halfsize; y < width - signed_mask_halfsize; y++) {
			new_pixel = 0;
			for (row_offset = -signed_mask_halfsize; row_offset <= signed_mask_halfsize; row_offset++) {
				for (col_offset = -signed_mask_halfsize; col_offset <= signed_mask_halfsize; col_offset++) {
					new_pixel += (float)(this->workspace_bitmap->operator()(y + col_offset, x + row_offset, 0, 0))
						* gaussianMask->operator()(signed_mask_halfsize + col_offset, signed_mask_halfsize + row_offset, 0, 0);
				}
			}
			i = (unsigned long)(x * width + y);
			this->workspace_bitmap->operator()(y, x, 0, 0);
		}
	}
	delete gaussianMask;
}

void CannyEdgeDetector::EdgeDetection() {
	// Sobel masks.
	float Gx[9];
	Gx[0] = 1.0; Gx[1] = 0.0; Gx[2] = -1.0;
	Gx[3] = 2.0; Gx[4] = 0.0; Gx[5] = -2.0;
	Gx[6] = 1.0; Gx[7] = 0.0; Gx[8] = -1.0;
	float Gy[9];
	Gy[0] = -1.0; Gy[1] = -2.0; Gy[2] = -1.0;
	Gy[3] = 0.0; Gy[4] = 0.0; Gy[5] = 0.0;
	Gy[6] = 1.0; Gy[7] = 2.0; Gy[8] = 1.0;

	float value_gx, value_gy;

	float max = 0.0;
	float angle = 0.0;

	// Convolution.
	for (x = 0; x < height; x++) {
		for (y = 0; y < width; y++) {
			value_gx = 0.0;
			value_gy = 0.0;

			for (int k = 0; k < 3; k++) {
				for (int l = 0; l < 3; l++) {
					value_gx += Gx[l * 3 + k] * GetPixelValue((x + 1) + (1 - k),
						(y + 1) + (1 - l));
					value_gy += Gy[l * 3 + k] * GetPixelValue((x + 1) + (1 - k),
						(y + 1) + (1 - l));
				}
			}
			this->edge_magnitude->operator()(y, x, 0 ,0) = sqrt(value_gx * value_gx + value_gy * value_gy) / 4.0;

			// Maximum magnitude.
			max = this->edge_magnitude->operator()(y, x, 0, 0) > max ? this->edge_magnitude->operator()(y, x, 0, 0) : max;

			// Angle calculation.
			if ((value_gx != 0.0) || (value_gy != 0.0)) {
				angle = atan2(value_gy, value_gx) * 180.0 / PI;
			}
			else {
				angle = 0.0;
			}
			if (((angle > -22.5) && (angle <= 22.5)) ||
				((angle > 157.5) && (angle <= -157.5))) {
				this->edge_direction->operator()(y, x, 0, 0) = 0;
			}
			else if (((angle > 22.5) && (angle <= 67.5)) ||
				((angle > -157.5) && (angle <= -112.5))) {
				this->edge_direction->operator()(y, x, 0, 0) = 45;
			}
			else if (((angle > 67.5) && (angle <= 112.5)) ||
				((angle > -112.5) && (angle <= -67.5))) {
				this->edge_direction->operator()(y, x, 0, 0) = 90;
			}
			else if (((angle > 112.5) && (angle <= 157.5)) ||
				((angle > -67.5) && (angle <= -22.5))) {
				this->edge_direction->operator()(y, x, 0, 0) = 135;
			}
		}
	}

	for (x = 0; x < height; x++) {
		for (y = 0; y < width; y++) {
			this->edge_magnitude->operator()(y, x, 0, 0) = 255.0f * this->edge_magnitude->operator()(y, x, 0, 0) / max;
			SetPixelValue(x, y, this->edge_magnitude->operator()(y, x, 0, 0));
		}
	}
}

void CannyEdgeDetector::NonMaxSuppression() {
	float pixel_1 = 0;
	float pixel_2 = 0;
	float pixel;

	for (x = 1; x < height - 1; x++) {
		for (y = 1; y < width - 1; y++) {
			if (this->edge_direction->operator()(y, x, 0, 0) == 0) {
				pixel_1 = this->edge_magnitude->operator()(y, x + 1, 0, 0);
				pixel_2 = this->edge_magnitude->operator()(y, x - 1, 0, 0);
			}
			else if (this->edge_direction->operator()(y, x, 0, 0) == 45) {
				pixel_1 = this->edge_magnitude->operator()(y - 1, x + 1, 0, 0);
				pixel_2 = this->edge_magnitude->operator()(y + 1, x - 1, 0, 0);
			}
			else if (this->edge_direction->operator()(y, x, 0, 0) == 90) {
				pixel_1 = this->edge_magnitude->operator()(y - 1, x, 0, 0);
				pixel_2 = this->edge_magnitude->operator()(y + 1, x, 0, 0);
			}
			else if (this->edge_direction->operator()(y, x, 0, 0) == 135) {
				pixel_1 = this->edge_magnitude->operator()(y + 1, x + 1, 0, 0);
				pixel_2 = this->edge_magnitude->operator()(y - 1, x - 1, 0, 0);
			}
			pixel = this->edge_magnitude->operator()(y, x, 0, 0);
			if ((pixel >= pixel_1) && (pixel >= pixel_2)) {
				SetPixelValue(x, y, pixel);
			}
			else {
				SetPixelValue(x, y, 0);
			}
		}
	}

	bool change = true;
	while (change) {
		change = false;
		for (x = 1; x < height - 1; x++) {
			for (y = 1; y < width - 1; y++) {
				if (GetPixelValue(x, y) == 255) {
					if (GetPixelValue(x + 1, y) == 128) {
						change = true;
						SetPixelValue(x + 1, y, 255);
					}
					if (GetPixelValue(x - 1, y) == 128) {
						change = true;
						SetPixelValue(x - 1, y, 255);
					}
					if (GetPixelValue(x, y + 1) == 128) {
						change = true;
						SetPixelValue(x, y + 1, 255);
					}
					if (GetPixelValue(x, y - 1) == 128) {
						change = true;
						SetPixelValue(x, y - 1, 255);
					}
					if (GetPixelValue(x + 1, y + 1) == 128) {
						change = true;
						SetPixelValue(x + 1, y + 1, 255);
					}
					if (GetPixelValue(x - 1, y - 1) == 128) {
						change = true;
						SetPixelValue(x - 1, y - 1, 255);
					}
					if (GetPixelValue(x - 1, y + 1) == 128) {
						change = true;
						SetPixelValue(x - 1, y + 1, 255);
					}
					if (GetPixelValue(x + 1, y - 1) == 128) {
						change = true;
						SetPixelValue(x + 1, y - 1, 255);
					}
				}
			}
		}
		if (change) {
			for (x = height - 2; x > 0; x--) {
				for (y = width - 2; y > 0; y--) {
					if (GetPixelValue(x, y) == 255) {
						if (GetPixelValue(x + 1, y) == 128) {
							change = true;
							SetPixelValue(x + 1, y, 255);
						}
						if (GetPixelValue(x - 1, y) == 128) {
							change = true;
							SetPixelValue(x - 1, y, 255);
						}
						if (GetPixelValue(x, y + 1) == 128) {
							change = true;
							SetPixelValue(x, y + 1, 255);
						}
						if (GetPixelValue(x, y - 1) == 128) {
							change = true;
							SetPixelValue(x, y - 1, 255);
						}
						if (GetPixelValue(x + 1, y + 1) == 128) {
							change = true;
							SetPixelValue(x + 1, y + 1, 255);
						}
						if (GetPixelValue(x - 1, y - 1) == 128) {
							change = true;
							SetPixelValue(x - 1, y - 1, 255);
						}
						if (GetPixelValue(x - 1, y + 1) == 128) {
							change = true;
							SetPixelValue(x - 1, y + 1, 255);
						}
						if (GetPixelValue(x + 1, y - 1) == 128) {
							change = true;
							SetPixelValue(x + 1, y - 1, 255);
						}
					}
				}
			}
		}
	}

	// Suppression
	for (x = 0; x < height; x++) {
		for (y = 0; y < width; y++) {
			if (GetPixelValue(x, y) == 128) {
				SetPixelValue(x, y, 0);
			}
		}
	}
}

void CannyEdgeDetector::Hysteresis(uint8_t lowThreshold, uint8_t highThreshold) {
	for (x = 0; x < height; x++) {
		for (y = 0; y < width; y++) {
			if (GetPixelValue(x, y) >= highThreshold) {
				SetPixelValue(x, y, 255);
				this->HysteresisRecursion(x, y, lowThreshold);
			}
		}
	}

	for (x = 0; x < height; x++) {
		for (y = 0; y < width; y++) {
			if (GetPixelValue(x, y) != 255) {
				SetPixelValue(x, y, 0);
			}
		}
	}
}

void CannyEdgeDetector::HysteresisRecursion(long x, long y, uint8_t lowThreshold) {
	uint8_t value = 0;

	for (long x1 = x - 1; x1 <= x + 1; x1++) {
		for (long y1 = y - 1; y1 <= y + 1; y1++) {
			if ((x1 < height) & (y1 < width) & (x1 >= 0) & (y1 >= 0)
				& (x1 != x) & (y1 != y)) {

				value = GetPixelValue(x1, y1);
				if (value != 255) {
					if (value >= lowThreshold) {
						SetPixelValue(x1, y1, 255);
						this->HysteresisRecursion(x1, y1, lowThreshold);
					}
					else {
						SetPixelValue(x1, y1, 0);
					}
				}
			}
		}
	}
}
