
// TermProjectDoc.cpp: CTermProjectDoc 类的实现
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "TermProject.h"
#endif

#include "TermProjectDoc.h"
#include "COpenCVProcess.h"
#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CTermProjectDoc

IMPLEMENT_DYNCREATE(CTermProjectDoc, CDocument)

BEGIN_MESSAGE_MAP(CTermProjectDoc, CDocument)
	ON_COMMAND(ID_ORIGINAL, &CTermProjectDoc::OnOriginal)
	ON_COMMAND(ID_REMOVEHAIR, &CTermProjectDoc::OnRemoveHair)
	ON_COMMAND(ID_DRAWCONTOURSONORIGINAL, &CTermProjectDoc::OnDrawContoursOnOriginal)
	ON_COMMAND(ID_GRAY, &CTermProjectDoc::OnGray)
	ON_COMMAND(ID_BINARYINV, &CTermProjectDoc::OnBinaryInv)
	ON_COMMAND(ID_MORPHOPEN, &CTermProjectDoc::OnMorphOpen)
	ON_COMMAND(ID_DISTANCETRANSFORM, &CTermProjectDoc::OnDistanceTransform)
	ON_COMMAND(ID_SUREFOREGROUND, &CTermProjectDoc::OnSureForeground)
	ON_COMMAND(ID_UNKNOWN, &CTermProjectDoc::OnUnknown)
	ON_COMMAND(ID_WATERSHEDBOUNDARY, &CTermProjectDoc::OnDrawWatershedBoundary)
	ON_COMMAND(ID_WATERSHED, &CTermProjectDoc::OnWatershed)
END_MESSAGE_MAP()


// CTermProjectDoc 构造/析构

CTermProjectDoc::CTermProjectDoc() noexcept
{
	// TODO: 在此添加一次性构造代码
	m_pDib = nullptr; // 初始化Dib指针为nullptr
	m_pDibBackup = nullptr; // 初始化Dib备份指针为nullptr
}

CTermProjectDoc::~CTermProjectDoc()
{
	if (m_pDib != nullptr) // 如果Dib指针不为空
		delete m_pDib; // 释放Dib指针
	if (m_pDibBackup != nullptr)
		delete m_pDibBackup;
}

BOOL CTermProjectDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: 在此添加重新初始化代码
	// (SDI 文档将重用该文档)

	return TRUE;
}




// CTermProjectDoc 序列化

void CTermProjectDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 在此添加存储代码
	}
	else
	{
		// TODO: 在此添加加载代码
	}
}

#ifdef SHARED_HANDLERS

// 缩略图的支持
void CTermProjectDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// 修改此代码以绘制文档数据
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// 搜索处理程序的支持
void CTermProjectDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// 从文档数据设置搜索内容。
	// 内容部分应由“;”分隔

	// 例如:     strSearchContent = _T("point;rectangle;circle;ole object;")；
	SetSearchContent(strSearchContent);
}

void CTermProjectDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = nullptr;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != nullptr)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CTermProjectDoc 诊断

#ifdef _DEBUG
void CTermProjectDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CTermProjectDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CTermProjectDoc 命令

BOOL CTermProjectDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	// TODO:  在此添加您专用的创建代码
	if (m_pDib != nullptr) // 如果Dib指针不为空
	{
		delete m_pDib; // 释放Dib指针
		m_pDib = nullptr; // 初始化Dib指针为nullptr
	}
	m_pDib = new CDib(); // 创建Dib指针
	m_pDib->LoadFile(lpszPathName); // 加载文件
	//创建备份
	m_pDibBackup = new CDib(*m_pDib);

	return TRUE;
}

void CTermProjectDoc::OnOriginal()
{
	// TODO: 在此添加命令处理程序代码
	if (m_pDib != nullptr && m_pDibBackup != nullptr)
	{
		delete m_pDib; // 释放当前指针
		m_pDib = new CDib(*m_pDibBackup); // 从备份重新创建
		UpdateAllViews(NULL);
	}
}

void CTermProjectDoc::OnDrawContoursOnOriginal()
{
	// TODO: 在此添加命令处理程序代码
	if (m_pDib != nullptr && m_pDibBackup != nullptr)
	{
		// 直接获取当前的轮廓图像
		COpenCVProcess contourProcess(m_pDib);
		cv::Mat contourImage = contourProcess.cvimg.clone(); // 轮廓图像

		// 获取原始图像
		COpenCVProcess originalProcess(m_pDibBackup);
		cv::Mat originalImage = originalProcess.cvimg.clone(); // 原始图像

		// 根据通道数处理
		if (originalImage.channels() == 1)
			cv::cvtColor(originalImage, originalImage, cv::COLOR_GRAY2BGR);

		// 直接绘制轮廓
		// 通过二值化确保轮廓图像是二值的
		cv::Mat binaryContour;
		if (contourImage.channels() == 3)
		{
			cv::Mat grayContour;
			cv::cvtColor(contourImage, grayContour, cv::COLOR_BGR2GRAY);
			cv::threshold(grayContour, binaryContour, 128, 255, cv::THRESH_BINARY);
		}
		else
		{
			cv::threshold(contourImage, binaryContour, 128, 255, cv::THRESH_BINARY);
		}

		// 查找轮廓
		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(binaryContour.clone(), contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		// 在原图上绘制绿色轮廓
		cv::drawContours(originalImage, contours, -1, cv::Scalar(0, 255, 0), 1);

		// 将结果复制回当前DIB
		contourProcess.cvimg = originalImage;
		contourProcess.Mat2Dib(*m_pDib);

		UpdateAllViews(NULL); // 更新视图
	}
}

void CTermProjectDoc::OnGray()
{
	// 使用OpenCV显示灰度图像
	if (m_pDib != nullptr)
	{
		COpenCVProcess cvProcess(m_pDib);
		m_grayMat = cvProcess.ToGray(cvProcess.cvimg);
		cvProcess.cvimg = m_grayMat;
		cvProcess.Mat2Dib(*m_pDib);
		cv::namedWindow("Gray Image", cv::WINDOW_NORMAL);
		cv::setWindowProperty("Gray Image", cv::WND_PROP_TOPMOST, 1);
		cv::imshow("Gray Image", m_grayMat);
		cv::waitKey(0);
		UpdateAllViews(NULL);
	}
}

void CTermProjectDoc::OnRemoveHair()
{
	// TODO: 在此添加命令处理程序代码
	if (m_pDib != nullptr)
	{
		COpenCVProcess cvProcess(m_pDib);
		m_hairRemovedMat = cvProcess.RemoveHair();
		cvProcess.cvimg = m_hairRemovedMat;
		cvProcess.Mat2Dib(*m_pDib);
		cv::namedWindow("Hair Removed Image", cv::WINDOW_NORMAL);
		cv::setWindowProperty("Hair Removed Image", cv::WND_PROP_TOPMOST, 1);
		cv::imshow("Hair Removed Image", m_hairRemovedMat);
		cv::waitKey(0);
		UpdateAllViews(NULL);
	}
}

void CTermProjectDoc::OnBinaryInv()
{
	// TODO: 在此添加命令处理程序代码
	if (m_pDib != nullptr)
	{
		COpenCVProcess cvProcess(m_pDib);
		m_binaryInvMat = cvProcess.ToBinaryInv(cvProcess.cvimg);
		cvProcess.cvimg = m_binaryInvMat;
		cvProcess.Mat2Dib(*m_pDib);
		cv::namedWindow("Binary Inverted Image", cv::WINDOW_NORMAL);
		cv::setWindowProperty("Binary Inverted Image", cv::WND_PROP_TOPMOST, 1);
		cv::imshow("Binary Inverted Image", m_binaryInvMat);
		cv::waitKey(0);
		UpdateAllViews(NULL);
	}
}

void CTermProjectDoc::OnMorphOpen()
{
	// TODO: 在此添加命令处理程序代码
	if (m_pDib != nullptr)
	{
		COpenCVProcess cvProcess(m_pDib);
		m_morphOpenMat = cvProcess.MorphOpen(cvProcess.cvimg);
		cvProcess.cvimg = m_morphOpenMat;
		cvProcess.Mat2Dib(*m_pDib);
		cv::namedWindow("Morphological Opening Image", cv::WINDOW_NORMAL);
		cv::setWindowProperty("Morphological Opening Image", cv::WND_PROP_TOPMOST, 1);
		cv::imshow("Morphological Opening Image", m_morphOpenMat);
		cv::waitKey(0);
		UpdateAllViews(NULL);
	}
}

void CTermProjectDoc::OnDistanceTransform()
{
	// TODO: 在此添加命令处理程序代码
	if (m_pDib != nullptr)
	{
		COpenCVProcess cvProcess(m_pDib);
		m_distTransMat = cvProcess.DistanceTransformNorm(cvProcess.cvimg);
		cvProcess.cvimg = m_distTransMat;
		cvProcess.Mat2Dib(*m_pDib);
		cv::namedWindow("Distance Transform Image", cv::WINDOW_NORMAL);
		cv::setWindowProperty("Distance Transform Image", cv::WND_PROP_TOPMOST, 1);
		cv::imshow("Distance Transform Image", m_distTransMat);
		cv::waitKey(0);
		UpdateAllViews(NULL);
	}
}

void CTermProjectDoc::OnSureForeground()
{
	// TODO: 在此添加命令处理程序代码
	if (m_pDib != nullptr)
	{
		COpenCVProcess cvProcess(m_pDib);
		m_sureFgMat = cvProcess.GetSureForeground(m_distTransMat);
		cvProcess.cvimg = m_sureFgMat;
		cvProcess.Mat2Dib(*m_pDib);
		cv::namedWindow("Sure Foreground Image", cv::WINDOW_NORMAL);
		cv::setWindowProperty("Sure Foreground Image", cv::WND_PROP_TOPMOST, 1);
		cv::imshow("Sure Foreground Image", m_sureFgMat);
		cv::waitKey(0);
		UpdateAllViews(NULL);
	}
}

void CTermProjectDoc::OnUnknown()
{
	// TODO: 在此添加命令处理程序代码
	if (m_pDib != nullptr)
	{
		COpenCVProcess cvProcess(m_pDib);
		m_unknownMat = cvProcess.GetUnknown(m_morphOpenMat, m_sureFgMat);
		cvProcess.cvimg = m_unknownMat;
		cvProcess.Mat2Dib(*m_pDib);
		cv::namedWindow("Unknown Image", cv::WINDOW_NORMAL);
		cv::setWindowProperty("Unknown Image", cv::WND_PROP_TOPMOST, 1);
		cv::imshow("Unknown Image", m_unknownMat);
		cv::waitKey(0);
		UpdateAllViews(NULL);
	}
}

void CTermProjectDoc::OnDrawWatershedBoundary()
{
	// TODO: 在此添加命令处理程序代码
	if (m_pDib != nullptr && !m_sureFgMat.empty() && !m_unknownMat.empty())
	{
		COpenCVProcess cvProcess(m_pDibBackup);
		cvProcess.DrawWatershedBoundary(cvProcess.cvimg, m_sureFgMat, m_unknownMat);
		m_watershedInput = cvProcess.cvimg.clone(); // 获取分水岭处理后的图像
		cvProcess.Mat2Dib(*m_pDib);
		UpdateAllViews(NULL);
		cv::namedWindow("Watershed Boundary Image", cv::WINDOW_NORMAL);
		cv::setWindowProperty("Watershed Boundary Image", cv::WND_PROP_TOPMOST, 1);
		cv::imshow("Watershed Boundary Image", m_watershedInput);
		cv::waitKey(0);
	}
}

void CTermProjectDoc::OnWatershed()
{
	// 使用OpenCV进行分水岭分割
	if (m_pDib != nullptr)
	{
		COpenCVProcess cvProcess(m_pDib);
		cvProcess.OpenCVWatershed(); // 执行分水岭算法，内部应处理m_sureFgMat和m_unknownMat
		m_markersMat = cvProcess.cvimg.clone(); // 获取分水岭分割结果
		cvProcess.Mat2Dib(*m_pDib);
		UpdateAllViews(NULL);
		cv::namedWindow("Watershed Result Image", cv::WINDOW_NORMAL);
		cv::setWindowProperty("Watershed Result Image", cv::WND_PROP_TOPMOST, 1);
		cv::imshow("Watershed Result Image", m_markersMat);
		cv::waitKey(0);
	}
}
