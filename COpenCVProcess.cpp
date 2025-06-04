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

cv::Mat COpenCVProcess::DrawWatershedBoundary(cv::Mat& img, const cv::Mat& sure_fg, const cv::Mat& unknown)
{
	cv::Mat markers;
	cv::connectedComponents(sure_fg, markers);
	markers = markers + 1;
	for (int i = 0; i < unknown.rows; ++i)
		for (int j = 0; j < unknown.cols; ++j)
			if (unknown.at<uchar>(i, j) == 255)
				markers.at<int>(i, j) = 0;

    cv::watershed(img, markers);
    // ���Ʒ�ˮ��߽�Ϊ��ɫ
    cv::Mat result;
    if (img.channels() == 1)
		cv::cvtColor(img, result, cv::COLOR_GRAY2BGR);
    else
		result = img.clone();
    for (int i = 1; i < markers.rows - 1; ++i)
    {
		for (int j = 1; j < markers.cols - 1; ++j)
		{
			if (markers.at<int>(i, j) == -1)
			{
				result.at<cv::Vec3b>(i, j) = cv::Vec3b(0, 255, 0);
			}
		}
    }
	// ��ȡ��ˮ��߽��ڵ�ͼ�����򣨼��ָ����ǰ������
	cv::Mat foregroundMask = (markers > 1);
	foregroundMask.convertTo(foregroundMask, CV_8U, 255); // תΪ0/255����
	cv::Mat foreground;
	result.copyTo(foreground, foregroundMask);
	return foreground;
}

void COpenCVProcess::OpenCVWatershed()
{
	// �������ͼ��
	if (cvimg.empty())
		return;

	// ����ԭͼ�Ա��������
	cv::Mat original = cvimg.clone();

	// ����ѯ���Ƿ����ë������
	int ret = MessageBox(NULL, _T("�Ƿ����ë��ȥ������"), _T("��ʾ"), MB_YESNO | MB_ICONQUESTION);
	if (ret == IDYES)
	{
		cv::Mat hairRemoved = RemoveHair();
		if (!hairRemoved.empty())
			cvimg = hairRemoved.clone();
	}

	// ��ͼ��תΪ�Ҷ�
	cv::Mat gray;
	if (cvimg.channels() == 3)
		gray = ToGray(cvimg);
	else
		gray = cvimg.clone();

	// ��ֵ������ת��
	cv::Mat binary = ToBinaryInv(gray);

	// ��̬ѧ��������ȥ������
	cv::Mat opening = MorphOpen(binary);

	// ȷ���ı�������
	cv::Mat sure_bg = opening.clone();

	// ����任��ȡǱ�ڵ�ǰ�����
	cv::Mat dist_transform = DistanceTransformNorm(opening);

	// ��ȡȷ����ǰ������
	cv::Mat sure_fg = GetSureForeground(dist_transform);

	// ��ȡδ֪���򣨱�����ǰ��֮�������
	cv::Mat unknown = GetUnknown(sure_bg, sure_fg);

	// ��ԭͼ����Ϊ��ɫ�Ա������ɫ�߽�
	cv::Mat result;
	if (original.channels() == 1)
		cv::cvtColor(original, result, cv::COLOR_GRAY2BGR);
	else
		result = original.clone();

	// �������ͼ��
	cv::Mat markers;
	cv::connectedComponents(sure_fg, markers);
	// �����ͼ���ֵ��1��ȷ������Ϊ0
	markers = markers + 1;
	// ��δ֪������Ϊ0
	for (int i = 0; i < unknown.rows; ++i)
	{
		for (int j = 0; j < unknown.cols; ++j)
		{
			if (unknown.at<uchar>(i, j) == 255)
			{
				markers.at<int>(i, j) = 0;
			}
		}
	}
	// ִ�з�ˮ���㷨
	cv::watershed(result, markers);
	// ���Ʒ�ˮ��߽磨������ͼ��߽��ϣ�
	for (int i = 1; i < markers.rows - 1; ++i)
	{
		for (int j = 1; j < markers.cols - 1; ++j)
		{
			if (markers.at<int>(i, j) == -1) // ��ˮ��߽�
			{
				result.at<cv::Vec3b>(i, j) = cv::Vec3b(0, 255, 0); // ��ɫ�߽�
			}
		}
	}

	// ��ȡ��ˮ��߽��ڵ�ͼ�����򣨼��ָ����ǰ������
	// ǰ���������룺markers > 1������Ϊ1��δ֪Ϊ0��ǰ��Ϊ2�����ϣ�
	cv::Mat foregroundMask = (markers > 1);
	foregroundMask.convertTo(foregroundMask, CV_8U, 255); // תΪ0/255����

	// ��ȡǰ������
	cv::Mat foreground;
	result.copyTo(foreground, foregroundMask);

	// ��ǰ�����򱣴浽cvimg
	cvimg = foreground.clone();
}

LesionFeatures COpenCVProcess::ExtractLesionFeatures(const cv::Mat& src, const cv::Mat& mask)
{
	LesionFeatures features;

	// 1. ��ɫ������ȡ
	cv::Mat hsv;
	cv::cvtColor(src, hsv, cv::COLOR_BGR2HSV);

	cv::Scalar mean, stddev;
	cv::meanStdDev(src, mean, stddev, mask);

	for (int i = 0; i < 3; i++) {
		features.colorMean[i] = mean[i];
		features.colorStd[i] = stddev[i];
	}

	// 2. ��״������ȡ
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(mask.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	if (!contours.empty()) {
		// ���
		features.area = cv::contourArea(contours[0]);

		// �ܳ�
		features.perimeter = cv::arcLength(contours[0], true);

		// Բ�ζ� = 4��*���/�ܳ�^2
		features.circularity = 4 * CV_PI * features.area / (features.perimeter * features.perimeter);

		// �������
		cv::RotatedRect minRect = cv::minAreaRect(contours[0]);
		double rectangleArea = minRect.size.width * minRect.size.height;
		features.irregularity = rectangleArea / features.area;
	}

	// 3. ����������ȡ��ʹ��GLCM - �Ҷȹ�������
	cv::Mat gray;
	cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);

	cv::Mat glcm = calculateGLCM(gray, mask, 1, 0); // Provide default values for distance and angle

	calculateGLCMFeatures(glcm, features);

	// 4. ��Ե������ȡ
	cv::Mat edges;
	cv::Sobel(gray, edges, CV_32F, 1, 1);

	cv::Mat edgeMagnitude;
	cv::magnitude(edges, edges, edgeMagnitude);

	cv::Scalar edgeMean = cv::mean(edgeMagnitude, mask);
	features.edgeGradientMean = edgeMean[0];

	return features;
}

// ��������������GLCM
cv::Mat COpenCVProcess::calculateGLCM(const cv::Mat& gray, const cv::Mat& mask, int distance = 1, double angle = 0)
{
	cv::Mat glcm = cv::Mat::zeros(256, 256, CV_32F);
	double radian = angle * CV_PI / 180.0;
	int stepX = static_cast<int>(std::cos(radian) * distance);
	int stepY = static_cast<int>(std::sin(radian) * distance);

	for (int i = 0; i < gray.rows; i++) {
		for (int j = 0; j < gray.cols; j++) {
			if (mask.at<uchar>(i, j) == 0) continue;

			int nextI = i + stepY;
			int nextJ = j + stepX;

			if (nextI >= 0 && nextI < gray.rows && nextJ >= 0 && nextJ < gray.cols) {
				if (mask.at<uchar>(nextI, nextJ) > 0) {
					int intensity1 = gray.at<uchar>(i, j);
					int intensity2 = gray.at<uchar>(nextI, nextJ);
					glcm.at<float>(intensity1, intensity2)++;
				}
			}
		}
	}

	// ��һ��GLCM
	glcm = glcm / cv::sum(glcm)[0];

	return glcm;
}

// ��������������GLCM����
void COpenCVProcess::calculateGLCMFeatures(const cv::Mat& glcm, LesionFeatures& features)
{
	features.contrast = 0;
	features.correlation = 0;
	features.energy = 0;
	features.homogeneity = 0;
	features.entropy = 0;

	double meanI = 0, meanJ = 0;
	double stdI = 0, stdJ = 0;

	// �����ֵ
	for (int i = 0; i < glcm.rows; i++) {
		for (int j = 0; j < glcm.cols; j++) {
			float Pij = glcm.at<float>(i, j);
			meanI += i * Pij;
			meanJ += j * Pij;
			features.energy += Pij * Pij;
		}
	}

	// �����׼��
	for (int i = 0; i < glcm.rows; i++) {
		for (int j = 0; j < glcm.cols; j++) {
			float Pij = glcm.at<float>(i, j);
			stdI += (i - meanI) * (i - meanI) * Pij;
			stdJ += (j - meanJ) * (j - meanJ) * Pij;

			features.contrast += (i - j) * (i - j) * Pij;
			features.homogeneity += Pij / (1 + abs(i - j));
			if (Pij > 0)
				features.entropy -= Pij * std::log(Pij);
		}
	}

	stdI = std::sqrt(stdI);
	stdJ = std::sqrt(stdJ);

	// ���������
	for (int i = 0; i < glcm.rows; i++) {
		for (int j = 0; j < glcm.cols; j++) {
			float Pij = glcm.at<float>(i, j);
			features.correlation += ((i - meanI) * (j - meanJ) * Pij) / (stdI * stdJ);
		}
	}
}
