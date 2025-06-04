#pragma once
#include <opencv2/opencv.hpp>
//--------------------------------------------------------------
//COpenCVProcess��,����OpenCVͼ����
//--------------------------------------------------------------
using namespace cv;
class CDib;

// ���� LesionFeatures �ṹ��
struct LesionFeatures {
	// ��ɫ����
	double colorMean[3];        // RGB��ֵ
	double colorStd[3];         // RGB��׼��
	double colorAsymmetry;      // ��ɫ���Գ���

	// ��״����
	double area;                // ���
	double perimeter;           // �ܳ�
	double circularity;         // Բ�ζ�
	double irregularity;        // �߽粻�����
	double asymmetryRatio;      // ��״���ԳƱ�

	// ��������
	double contrast;            // �Աȶ�
	double correlation;         // �����
	double energy;              // ����
	double homogeneity;         // ͬ����
	double entropy;             // ��

	// ��Ե����
	double edgeSharpness;       // ��Ե������
	double edgeGradientMean;    // ��Ե�ݶȾ�ֵ
	double edgeRegularity;      // ��Ե������
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

