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

void COpenCVProcess::OpenCVBinarize()
{
	// 检查图像是否为空
	if (cvimg.empty())
	{
		return;
	}

	// 动态计算阈值（使用大津法）
	double threshold = 0;
	if (cvimg.channels() == 1) // 灰度图像
	{
		threshold = cv::threshold(cvimg, cvimg, 0, 255, THRESH_BINARY | THRESH_OTSU);
	}
	else if (cvimg.channels() == 3) // 彩色图像
	{
		// 先将BGR转为灰度
		cv::Mat gray;
		cv::cvtColor(cvimg, gray, cv::COLOR_BGR2GRAY);
		threshold = cv::threshold(gray, gray, 0, 255, THRESH_BINARY | THRESH_OTSU);
		// 再将二值化结果映射回彩色图像
		for (int i = 0; i < cvimg.rows; i++)
		{
			for (int j = 0; j < cvimg.cols; j++)
			{
				unsigned char value = gray.at<uchar>(i, j);
				cv::Vec3b& pixel = cvimg.at<cv::Vec3b>(i, j);
				pixel[0] = value;
				pixel[1] = value;
				pixel[2] = value;
			}
		}
	}
}