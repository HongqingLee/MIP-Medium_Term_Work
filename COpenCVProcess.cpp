#include "pch.h"
#include "COpenCVProcess.h"
#include "Dib.h"

COpenCVProcess::COpenCVProcess(CDib* pDib)
{
	Dib2Mat(*pDib);
	cvimgbuffer = cvimg.clone();
}

COpenCVProcess::~COpenCVProcess(void)
{
	cvimg.release();
	cvimgbuffer.release();
}

void COpenCVProcess::OpenCVGaussianBlur()
{
	//Mat tmp;
	GaussianBlur(cvimg, cvimg, Size(7, 7), 0, 0, BORDER_DEFAULT);
	//cvimg = tmp;
}

void COpenCVProcess::Dib2Mat(CDib& dib)
{
	if (true == dib.IsNull())
	{
		return;
	}
	int nChannels = dib.GetBPP() / 8;
	if ((1 != nChannels) && (3 != nChannels))
	{
		return;
	}
	int nWidth = dib.GetWidth();
	int nHeight = dib.GetHeight();


	//重建mat
	if (1 == nChannels)
	{
		cvimg.create(nHeight, nWidth, CV_8UC1);
	}
	else if (3 == nChannels)
	{
		cvimg.create(nHeight, nWidth, CV_8UC3);
	}

	//拷贝数据
	uchar* pucRow;									//指向数据区的行指针
	uchar* pucImage = (uchar*)dib.GetBits();		//指向数据区的指针
	int nStep = dib.GetPitch();					//每行的字节数,注意这个返回值有正有负

	for (int nRow = 0; nRow < nHeight; nRow++)
	{
		pucRow = (cvimg.ptr<uchar>(nRow));
		for (int nCol = 0; nCol < nWidth; nCol++)
		{
			if (1 == nChannels)
			{
				pucRow[nCol] = *(pucImage + nRow * nStep + nCol);
			}
			else if (3 == nChannels)
			{
				for (int nCha = 0; nCha < 3; nCha++)
				{
					pucRow[nCol * 3 + nCha] = *(pucImage + nRow * nStep + nCol * 3 + nCha);
				}
			}
		}
	}
}

void COpenCVProcess::Mat2Dib(CDib& dib)
{
	if (0 == cvimg.total())
	{
		return;
	}
	int nChannels = cvimg.channels();
	if ((1 != nChannels) && (3 != nChannels))
	{
		return;
	}
	int nWidth = cvimg.cols;
	int nHeight = cvimg.rows;

	//重建CDib
	dib.Destroy();
	dib.Create(nWidth, nHeight, 8 * nChannels);

	//拷贝数据
	uchar* pucRow;									//指向数据区的行指针
	uchar* pucImage = (uchar*)dib.GetBits();		//指向数据区的指针
	int nStep = dib.GetPitch();					//每行的字节数,注意这个返回值有正有负
	dib.SetWidth(nWidth);
	dib.SetHeight(nHeight);
	dib.SetWidthBytes(abs(nStep));
	dib.SetBitCount(8 * nChannels);
	dib.SetDibBits((unsigned char*)dib.GetBits() + (nHeight - 1) * dib.GetPitch());

	if (1 == nChannels)								//对于单通道的图像需要初始化调色板
	{
		RGBQUAD* rgbquadColorTable;
		int nMaxColors = 256;
		rgbquadColorTable = new RGBQUAD[nMaxColors];
		dib.GetColorTable(0, nMaxColors, rgbquadColorTable);
		for (int nColor = 0; nColor < nMaxColors; nColor++)
		{
			rgbquadColorTable[nColor].rgbBlue = (uchar)nColor;
			rgbquadColorTable[nColor].rgbGreen = (uchar)nColor;
			rgbquadColorTable[nColor].rgbRed = (uchar)nColor;
		}
		dib.SetColorTable(0, nMaxColors, rgbquadColorTable);
		delete[]rgbquadColorTable;
	}

	for (int nRow = 0; nRow < nHeight; nRow++)
	{
		pucRow = (cvimg.ptr<uchar>(nRow));
		for (int nCol = 0; nCol < nWidth; nCol++)
		{
			if (1 == nChannels)
			{
				*(pucImage + nRow * nStep + nCol) = pucRow[nCol];
			}
			else if (3 == nChannels)
			{
				for (int nCha = 0; nCha < 3; nCha++)
				{
					*(pucImage + nRow * nStep + nCol * 3 + nCha) = pucRow[nCol * 3 + nCha];
				}
			}
		}
	}
}

void COpenCVProcess::FindContours()
{
	if (cvimg.empty())
		return;

	cv::Mat gray;
	if (cvimg.channels() == 1) {
		gray = cvimg.clone();
	}
	else if (cvimg.channels() == 3) {
		cv::cvtColor(cvimg, gray, cv::COLOR_BGR2GRAY);
	}
	else {
		// 只支持1通道或3通道
		return;
	}

	// 确保是二值图像
	cv::Mat binImg;
	if (cv::countNonZero(gray != 0 & gray != 255) > 0) {
		cv::threshold(gray, binImg, 128, 255, cv::THRESH_BINARY);
	}
	else {
		binImg = gray;
	}

	// 查找轮廓
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(binImg.clone(), contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	// 创建空白图像用于绘制轮廓
	cv::Mat contourImg = cv::Mat::zeros(binImg.size(), CV_8UC1);
	cv::drawContours(contourImg, contours, -1, cv::Scalar(255), 1);

	// 输出到cvimg，保持原通道数
	if (cvimg.channels() == 1) {
		contourImg.copyTo(cvimg);
	}
	else if (cvimg.channels() == 3) {
		cv::Mat colorContour;
		cv::cvtColor(contourImg, colorContour, cv::COLOR_GRAY2BGR);
		colorContour.copyTo(cvimg);
	}
}

void COpenCVProcess::DrawContoursOnOriginal()
{
	// 检查图像是否为空
	if (cvimg.empty())
		return;

	// 创建轮廓图像的副本
	cv::Mat contourImg = cvimg.clone();

	// 在Doc.cpp中处理，使用m_pDibBackup代替cvimgbuffer
	// 但这里先保持使用cvimgbuffer作为原图示例
	cvimg = cvimgbuffer.clone();

	// 处理不同通道的图像
	if (contourImg.channels() == 1) // 灰度图像
	{
		// 查找轮廓
		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;

		// 将二值轮廓图像转换为二值图像（确保是二值的）
		cv::Mat binaryContour;
		cv::threshold(contourImg, binaryContour, 128, 255, cv::THRESH_BINARY);

		// 查找轮廓
		cv::findContours(binaryContour, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		// 如果原图是灰度图，需要转为彩色以绘制彩色轮廓
		if (cvimg.channels() == 1)
			cv::cvtColor(cvimg, cvimg, cv::COLOR_GRAY2BGR);

		// 在原图上绘制绿色轮廓
		cv::drawContours(cvimg, contours, -1, cv::Scalar(0, 255, 0), 1);
	}
	else if (contourImg.channels() == 3) // 彩色图像
	{
		// 转换为灰度图以处理轮廓
		cv::Mat gray;
		cv::cvtColor(contourImg, gray, cv::COLOR_BGR2GRAY);

		// 确保是二值图像
		cv::Mat binaryContour;
		cv::threshold(gray, binaryContour, 128, 255, cv::THRESH_BINARY);

		// 查找轮廓
		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(binaryContour, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		// 在原图上绘制绿色轮廓
		cv::drawContours(cvimg, contours, -1, cv::Scalar(0, 255, 0), 1);
	}
}



cv::Mat COpenCVProcess::ToGray(const cv::Mat& src)
{
	cv::Mat gray;
	cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
	return gray;
}

cv::Mat COpenCVProcess::RemoveHair()
{
	if (cvimg.empty())
		return cv::Mat();

	cv::Mat gray, mask, result;

	// 转为灰度图
	if (cvimg.channels() == 3)
		cv::cvtColor(cvimg, gray, cv::COLOR_BGR2GRAY);
	else
		gray = cvimg.clone();

	// 黑色毛发检测：黑色细线，使用黑帽操作突出
	cv::Mat blackhat;
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(9, 9));
	cv::morphologyEx(gray, blackhat, cv::MORPH_BLACKHAT, kernel);

	// 二值化，提取毛发区域
	cv::threshold(blackhat, mask, 10, 255, cv::THRESH_BINARY);

	// 形态学操作去除小噪声
	cv::Mat kernel2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel2);

	// 修复（去除毛发）
	if (cvimg.channels() == 3)
		cv::inpaint(cvimg, mask, result, 1, cv::INPAINT_TELEA);
	else
		cv::inpaint(gray, mask, result, 1, cv::INPAINT_TELEA);

	return result;
}

cv::Mat COpenCVProcess::ToBinaryInv(const cv::Mat& gray)
{
	if (gray.channels() == 3)
		return cv::Mat();

	cv::Mat binary;
	cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
	return binary;
}

cv::Mat COpenCVProcess::MorphOpen(const cv::Mat& binary)
{
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
	cv::Mat opening;
	cv::morphologyEx(binary, opening, cv::MORPH_OPEN, kernel, cv::Point(-1, -1), 2);
	return opening;
}

cv::Mat COpenCVProcess::DistanceTransformNorm(const cv::Mat& src)
{
	cv::Mat dist, dist_norm;
	cv::distanceTransform(src, dist, cv::DIST_L2, 5);
	cv::normalize(dist, dist_norm, 0, 255, cv::NORM_MINMAX);
	dist_norm.convertTo(dist_norm, CV_8U);
	return dist_norm;
}

cv::Mat COpenCVProcess::GetSureForeground(const cv::Mat& dist)
{
	cv::Mat dist_norm_01;
	cv::normalize(dist, dist_norm_01, 0, 1.0, cv::NORM_MINMAX);
	double thresh = 0.1;
	cv::Mat sure_fg;
	cv::threshold(dist_norm_01, sure_fg, thresh, 1.0, cv::THRESH_BINARY);
	sure_fg.convertTo(sure_fg, CV_8U, 255);
	return sure_fg;
}

cv::Mat COpenCVProcess::GetUnknown(const cv::Mat&sure_bg , const cv::Mat& sure_fg)
{
	cv::Mat unknown;
	cv::subtract(sure_bg, sure_fg, unknown);
	return unknown;
}

void COpenCVProcess::DrawWatershedBoundary(cv::Mat& img, const cv::Mat& sure_fg, const cv::Mat& unknown)
{
	cv::Mat markers;
	cv::connectedComponents(sure_fg, markers);
	markers = markers + 1;
	for (int i = 0; i < unknown.rows; ++i)
		for (int j = 0; j < unknown.cols; ++j)
			if (unknown.at<uchar>(i, j) == 255)
				markers.at<int>(i, j) = 0;

	// 分水岭
	cv::watershed(img, markers);

	// 绘制分水岭边界
	int rows = markers.rows;
	int cols = markers.cols;
	for (int i = 1; i < rows - 1; i++)
	{
		for (int j = 1; j < cols - 1; j++)
		{
			if (markers.at<int>(i, j) == -1)
			{
				img.at<cv::Vec3b>(i, j) = cv::Vec3b(0, 255, 0);
			}
		}
	}
}

void COpenCVProcess::OpenCVWatershed()
{
	if (cvimg.empty() || cvimg.channels() != 3)
		return;

	cv::Mat gray = ToGray(cvimg);
	cv::Mat binary = ToBinaryInv(gray);
	cv::Mat opening = MorphOpen(binary);

	cv::Mat fillholes_fg = opening.clone();

	cv::Mat dist_norm = DistanceTransformNorm(fillholes_fg);
	cv::Mat sure_fg = GetSureForeground(dist_norm);
	cv::Mat unknown = GetUnknown(fillholes_fg, sure_fg);

}
