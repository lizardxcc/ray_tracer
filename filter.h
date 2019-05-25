#ifndef FILTER_H
#define FILTER_H

class Filter {
	public:
		Filter(double *img, int width, int height);
		virtual void FilterImage(void) = 0;
		double *result = nullptr;
	protected:
		double *orig_img;
		int img_width;
		int img_height;
};

class BiliteralFilter : public Filter {
	public:
		BiliteralFilter(double *img, int width, int height);
		virtual void FilterImage(void);
	private:
		double gaussian(double x, double c) const;
};

#endif
