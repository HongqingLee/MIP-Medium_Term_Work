#pragma once
#include <opencv2/opencv.hpp>
//--------------------------------------------------------------
//COpenCVProcess类,用于OpenCV图像处理
//--------------------------------------------------------------
using namespace cv;
class CDib;

// 定义 LesionFeatures 结构体
struct LesionFeatures {
	// 颜色特征
	double colorMean[3];        // RGB均值
	double colorStd[3];         // RGB标准差
	double colorAsymmetry;      // 颜色不对称性

	// 形状特征
	double area;                // 面积
	double perimeter;           // 周长
	double circularity;         // 圆形度
	double irregularity;        // 边界不规则度
	double asymmetryRatio;      // 形状不对称比

	// 纹理特征
	double contrast;            // 对比度
	double correlation;         // 相关性
	double energy;              // 能量
	double homogeneity;         // 同质性
	double entropy;             // 熵

	// 边缘特征
	double edgeSharpness;       // 边缘锐利度
	double edgeGradientMean;    // 边缘梯度均值
	double edgeRegularity;      // 边缘规则性
};

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

	LesionFeatures ExtractLesionFeatures(const cv::Mat& src, const cv::Mat& mask);

	cv::Mat calculateGLCM(const cv::Mat& gray, const cv::Mat& mask, int distance, double angle);

	void calculateGLCMFeatures(const cv::Mat& glcm, LesionFeatures& features);

	cv::Mat ToGray(const cv::Mat& src);
	cv::Mat RemoveHair();
	cv::Mat ToBinaryInv(const cv::Mat& gray);
	cv::Mat MorphOpen(const cv::Mat& binary);
	cv::Mat DistanceTransformNorm(const cv::Mat& src);
	cv::Mat GetSureForeground(const cv::Mat& dist);
	cv::Mat GetUnknown(const cv::Mat& fillholes_fg, const cv::Mat& sure_fg);
	cv::Mat DrawWatershedBoundary(cv::Mat& img, const cv::Mat& sure_fg, const cv::Mat& unknown);
};

