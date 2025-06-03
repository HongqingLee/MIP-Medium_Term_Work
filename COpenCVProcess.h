#pragma once
#include <opencv2/opencv.hpp>
//--------------------------------------------------------------
//COpenCVProcess类,用于OpenCV图像处理
//--------------------------------------------------------------
using namespace cv;
class CDib;

class COpenCVProcess
{
public:
	COpenCVProcess(CDib* pDib);
	~COpenCVProcess(void);

public:
	Mat cvimg, cvimgbuffer;

public:
	void OpenCVGaussianBlur();

public:
	//transfering between CDib and cv::Mat
	void Dib2Mat(CDib& dib);
	void Mat2Dib(CDib& dib);
	void FindContours();
	void DrawContoursOnOriginal();
	void OpenCVWatershed();

	cv::Mat ToGray(const cv::Mat& src);
	cv::Mat RemoveHair();
	cv::Mat ToBinaryInv(const cv::Mat& gray);
	cv::Mat MorphOpen(const cv::Mat& binary);
	cv::Mat DistanceTransformNorm(const cv::Mat& src);
	cv::Mat GetSureForeground(const cv::Mat& dist);
	cv::Mat GetUnknown(const cv::Mat& fillholes_fg, const cv::Mat& sure_fg);
	void DrawWatershedBoundary(cv::Mat& img, const cv::Mat& sure_fg, const cv::Mat& unknown);
};

