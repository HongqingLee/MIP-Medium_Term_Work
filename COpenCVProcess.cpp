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
    // 绘制分水岭边界为绿色
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
	// 提取分水岭边界内的图像区域（即分割出的前景区域）
	cv::Mat foregroundMask = (markers > 1);
	foregroundMask.convertTo(foregroundMask, CV_8U, 255); // 转为0/255掩码
	cv::Mat foreground;
	result.copyTo(foreground, foregroundMask);
	return foreground;
}

void COpenCVProcess::OpenCVWatershed()
{
	// 检查输入图像
	if (cvimg.empty())
		return;

	// 保存原图以便后续绘制
	cv::Mat original = cvimg.clone();

	// 弹窗询问是否进行毛发处理
	int ret = MessageBox(NULL, _T("是否进行毛发去除处理？"), _T("提示"), MB_YESNO | MB_ICONQUESTION);
	if (ret == IDYES)
	{
		cv::Mat hairRemoved = RemoveHair();
		if (!hairRemoved.empty())
			cvimg = hairRemoved.clone();
	}

	// 将图像转为灰度
	cv::Mat gray;
	if (cvimg.channels() == 3)
		gray = ToGray(cvimg);
	else
		gray = cvimg.clone();

	// 二值化（反转）
	cv::Mat binary = ToBinaryInv(gray);

	// 形态学开操作以去除噪声
	cv::Mat opening = MorphOpen(binary);

	// 确定的背景区域
	cv::Mat sure_bg = opening.clone();

	// 距离变换获取潜在的前景标记
	cv::Mat dist_transform = DistanceTransformNorm(opening);

	// 获取确定的前景区域
	cv::Mat sure_fg = GetSureForeground(dist_transform);

	// 获取未知区域（背景与前景之间的区域）
	cv::Mat unknown = GetUnknown(sure_bg, sure_fg);

	// 将原图设置为彩色以便绘制有色边界
	cv::Mat result;
	if (original.channels() == 1)
		cv::cvtColor(original, result, cv::COLOR_GRAY2BGR);
	else
		result = original.clone();

	// 创建标记图像
	cv::Mat markers;
	cv::connectedComponents(sure_fg, markers);
	// 将标记图像的值加1，确保背景为0
	markers = markers + 1;
	// 将未知区域标记为0
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
	// 执行分水岭算法
	cv::watershed(result, markers);
	// 绘制分水岭边界（不画在图像边界上）
	for (int i = 1; i < markers.rows - 1; ++i)
	{
		for (int j = 1; j < markers.cols - 1; ++j)
		{
			if (markers.at<int>(i, j) == -1) // 分水岭边界
			{
				result.at<cv::Vec3b>(i, j) = cv::Vec3b(0, 255, 0); // 绿色边界
			}
		}
	}

	// 提取分水岭边界内的图像区域（即分割出的前景区域）
	// 前景区域掩码：markers > 1（背景为1，未知为0，前景为2及以上）
	cv::Mat foregroundMask = (markers > 1);
	foregroundMask.convertTo(foregroundMask, CV_8U, 255); // 转为0/255掩码

	// 提取前景区域
	cv::Mat foreground;
	result.copyTo(foreground, foregroundMask);

	// 将前景区域保存到cvimg
	cvimg = foreground.clone();
}

LesionFeatures COpenCVProcess::ExtractLesionFeatures(const cv::Mat& src, const cv::Mat& mask)
{
	LesionFeatures features;

	// 1. 颜色特征提取
	cv::Mat hsv;
	cv::cvtColor(src, hsv, cv::COLOR_BGR2HSV);

	cv::Scalar mean, stddev;
	cv::meanStdDev(src, mean, stddev, mask);

	for (int i = 0; i < 3; i++) {
		features.colorMean[i] = mean[i];
		features.colorStd[i] = stddev[i];
	}

	// 2. 形状特征提取
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(mask.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	if (!contours.empty()) {
		// 面积
		features.area = cv::contourArea(contours[0]);

		// 周长
		features.perimeter = cv::arcLength(contours[0], true);

		// 圆形度 = 4π*面积/周长^2
		features.circularity = 4 * CV_PI * features.area / (features.perimeter * features.perimeter);

		// 不规则度
		cv::RotatedRect minRect = cv::minAreaRect(contours[0]);
		double rectangleArea = minRect.size.width * minRect.size.height;
		features.irregularity = rectangleArea / features.area;
	}

	// 3. 纹理特征提取（使用GLCM - 灰度共生矩阵）
	cv::Mat gray;
	cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);

	cv::Mat glcm = calculateGLCM(gray, mask, 1, 0); // Provide default values for distance and angle

	calculateGLCMFeatures(glcm, features);

	// 4. 边缘特征提取
	cv::Mat edges;
	cv::Sobel(gray, edges, CV_32F, 1, 1);

	cv::Mat edgeMagnitude;
	cv::magnitude(edges, edges, edgeMagnitude);

	cv::Scalar edgeMean = cv::mean(edgeMagnitude, mask);
	features.edgeGradientMean = edgeMean[0];

	return features;
}

// 辅助函数：计算GLCM
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

	// 归一化GLCM
	glcm = glcm / cv::sum(glcm)[0];

	return glcm;
}

// 辅助函数：计算GLCM特征
void COpenCVProcess::calculateGLCMFeatures(const cv::Mat& glcm, LesionFeatures& features)
{
	features.contrast = 0;
	features.correlation = 0;
	features.energy = 0;
	features.homogeneity = 0;
	features.entropy = 0;

	double meanI = 0, meanJ = 0;
	double stdI = 0, stdJ = 0;

	// 计算均值
	for (int i = 0; i < glcm.rows; i++) {
		for (int j = 0; j < glcm.cols; j++) {
			float Pij = glcm.at<float>(i, j);
			meanI += i * Pij;
			meanJ += j * Pij;
			features.energy += Pij * Pij;
		}
	}

	// 计算标准差
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

	// 计算相关性
	for (int i = 0; i < glcm.rows; i++) {
		for (int j = 0; j < glcm.cols; j++) {
			float Pij = glcm.at<float>(i, j);
			features.correlation += ((i - meanI) * (j - meanJ) * Pij) / (stdI * stdJ);
		}
	}
}
