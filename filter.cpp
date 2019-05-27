#include <cmath>
#include "filter.h"

Filter::Filter(double *img, int width, int height) : orig_img(img),
	img_width(width), img_height(height)
{
}

BiliteralFilter::BiliteralFilter(double *img, int width, int height) : Filter(img, width, height)
{
}

void BiliteralFilter::FilterImage(void)
{
	result = new double[img_width*img_height*4];

	for (int x = 0; x < img_width; x++) {
		for (int y = 0; y < img_height; y++) {
			double sum[3] = {0.0, 0.0, 0.0};
			double W[3] = {0.0, 0.0, 0.0};
			for (int i = -window; i <= window; i++) {
				for (int j = -window; j <= window; j++) {
					for (int c = 0; c < 3; c++) {
						int wx = x+i;
						int wy = y+j;
						if (wx < 0)
							wx = 0;
						if (wx >= img_width)
							wx = img_width-1;
						if (wy < 0)
							wy = 0;
						if (wy >= img_height)
							wy = img_height-1;
						double orig_value = orig_img[((img_height-y-1)*img_width+x)*4+c];
						double orig_wvalue = orig_img[((img_height-wy-1)*img_width+wx)*4+c];
						double power_index = -static_cast<double>((wx-x)*(wx-x) + (wy-y)*(wy-y))/(2.0*sigma_d*sigma_d);
						power_index += -(std::pow(orig_value - orig_wvalue, 2.0) / (2.0*sigma_r*sigma_r));
						double w = std::exp(power_index);
						sum[c] += orig_wvalue * w;
						W[c] += w;
					}
				}
			}
			for (int c = 0; c < 3; c++) {
				result[((img_height-y-1)*img_width+x)*4+c] = sum[c] / W[c];
			}
			result[((img_height-y-1)*img_width+x)*4+3] = 1.0;

		}
	}
}

double BiliteralFilter::gaussian(double x, double c) const
{
	return exp(-x*x/(c*c));
}


