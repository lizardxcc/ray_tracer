#ifndef FILTER_H
#define FILTER_H

#include <vector>

class Filter {
	public:
		Filter(const std::vector<double>& img, int width, int height);
		virtual void FilterImage(void) = 0;
		std::vector<double> result;
	protected:
		std::vector<double> orig_img;
		int img_width;
		int img_height;
};

class BiliteralFilter : public Filter {
	public:
		BiliteralFilter(const std::vector<double>& img, int width, int height);
		void FilterImage(void) override;
		double sigma_d;
		double sigma_r;
		int window;
	private:
		double gaussian(double x, double c) const;
};



#endif
