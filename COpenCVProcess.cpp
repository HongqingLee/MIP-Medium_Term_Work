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


	//�ؽ�mat
	if (1 == nChannels)
	{
		cvimg.create(nHeight, nWidth, CV_8UC1);
	}
	else if (3 == nChannels)
	{
		cvimg.create(nHeight, nWidth, CV_8UC3);
	}

	//��������
	uchar* pucRow;									//ָ������������ָ��
	uchar* pucImage = (uchar*)dib.GetBits();		//ָ����������ָ��
	int nStep = dib.GetPitch();					//ÿ�е��ֽ���,ע���������ֵ�����и�

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

	//�ؽ�CDib
	dib.Destroy();
	dib.Create(nWidth, nHeight, 8 * nChannels);

	//��������
	uchar* pucRow;									//ָ������������ָ��
	uchar* pucImage = (uchar*)dib.GetBits();		//ָ����������ָ��
	int nStep = dib.GetPitch();					//ÿ�е��ֽ���,ע���������ֵ�����и�
	dib.SetWidth(nWidth);
	dib.SetHeight(nHeight);
	dib.SetWidthBytes(abs(nStep));
	dib.SetBitCount(8 * nChannels);
	dib.SetDibBits((unsigned char*)dib.GetBits() + (nHeight - 1) * dib.GetPitch());

	if (1 == nChannels)								//���ڵ�ͨ����ͼ����Ҫ��ʼ����ɫ��
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
		// ֻ֧��1ͨ����3ͨ��
		return;
	}

	// ȷ���Ƕ�ֵͼ��
	cv::Mat binImg;
	if (cv::countNonZero(gray != 0 & gray != 255) > 0) {
		cv::threshold(gray, binImg, 128, 255, cv::THRESH_BINARY);
	}
	else {
		binImg = gray;
	}

	// ��������
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(binImg.clone(), contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	// �����հ�ͼ�����ڻ�������
	cv::Mat contourImg = cv::Mat::zeros(binImg.size(), CV_8UC1);
	cv::drawContours(contourImg, contours, -1, cv::Scalar(255), 1);

	// �����cvimg������ԭͨ����
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
	// ���ͼ���Ƿ�Ϊ��
	if (cvimg.empty())
		return;

	// ��������ͼ��ĸ���
	cv::Mat contourImg = cvimg.clone();

	// ��Doc.cpp�д���ʹ��m_pDibBackup����cvimgbuffer
	// �������ȱ���ʹ��cvimgbuffer��Ϊԭͼʾ��
	cvimg = cvimgbuffer.clone();

	// ����ͬͨ����ͼ��
	if (contourImg.channels() == 1) // �Ҷ�ͼ��
	{
		// ��������
		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;

		// ����ֵ����ͼ��ת��Ϊ��ֵͼ��ȷ���Ƕ�ֵ�ģ�
		cv::Mat binaryContour;
		cv::threshold(contourImg, binaryContour, 128, 255, cv::THRESH_BINARY);

		// ��������
		cv::findContours(binaryContour, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		// ���ԭͼ�ǻҶ�ͼ����ҪתΪ��ɫ�Ի��Ʋ�ɫ����
		if (cvimg.channels() == 1)
			cv::cvtColor(cvimg, cvimg, cv::COLOR_GRAY2BGR);

		// ��ԭͼ�ϻ�����ɫ����
		cv::drawContours(cvimg, contours, -1, cv::Scalar(0, 255, 0), 1);
	}
	else if (contourImg.channels() == 3) // ��ɫͼ��
	{
		// ת��Ϊ�Ҷ�ͼ�Դ�������
		cv::Mat gray;
		cv::cvtColor(contourImg, gray, cv::COLOR_BGR2GRAY);

		// ȷ���Ƕ�ֵͼ��
		cv::Mat binaryContour;
		cv::threshold(gray, binaryContour, 128, 255, cv::THRESH_BINARY);

		// ��������
		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(binaryContour, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		// ��ԭͼ�ϻ�����ɫ����
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

	// תΪ�Ҷ�ͼ
	if (cvimg.channels() == 3)
		cv::cvtColor(cvimg, gray, cv::COLOR_BGR2GRAY);
	else
		gray = cvimg.clone();

	// ��ɫë����⣺��ɫϸ�ߣ�ʹ�ú�ñ����ͻ��
	cv::Mat blackhat;
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(9, 9));
	cv::morphologyEx(gray, blackhat, cv::MORPH_BLACKHAT, kernel);

	// ��ֵ������ȡë������
	cv::threshold(blackhat, mask, 10, 255, cv::THRESH_BINARY);

	// ��̬ѧ����ȥ��С����
	cv::Mat kernel2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel2);

	// �޸���ȥ��ë����
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

	// ��ˮ��
	cv::watershed(img, markers);

	// ���Ʒ�ˮ��߽�
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
