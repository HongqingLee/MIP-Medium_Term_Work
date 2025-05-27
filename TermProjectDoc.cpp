
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
	ON_COMMAND(ID_BINARIZE, &CTermProjectDoc::OnBinarize)
	ON_COMMAND(ID_REMOVEHAIR, &CTermProjectDoc::OnRemoveHair)
	ON_COMMAND(ID_FINDCONTOURS, &CTermProjectDoc::OnFindContours)
	ON_COMMAND(ID_DRAWCONTOURSONORIGINAL, &CTermProjectDoc::OnDrawContoursOnOriginal)
	ON_COMMAND(ID_INVERTBINARY, &CTermProjectDoc::OnInvertBinary)
	ON_COMMAND(ID_SHOWIMAGE, &CTermProjectDoc::OnShowSecondImage)
END_MESSAGE_MAP()


// CTermProjectDoc 构造/析构

CTermProjectDoc::CTermProjectDoc() noexcept
{
	// TODO: 在此添加一次性构造代码
	m_pDib = nullptr; // 初始化Dib指针为nullptr
	m_pDibBackup = nullptr; // 初始化Dib备份指针为nullptr
	m_pImageDlg = nullptr; // 初始化非模态对话框指针
}

CTermProjectDoc::~CTermProjectDoc()
{
	if (m_pDib != nullptr) // 如果Dib指针不为空
		delete m_pDib; // 释放Dib指针
	if (m_pDibBackup != nullptr)
		delete m_pDibBackup;

	// 关闭并删除非模态对话框
	if (m_pImageDlg != nullptr)
	{
		m_pImageDlg->DestroyWindow(); // 这会触发PostNcDestroy
		m_pImageDlg = nullptr;
	}
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

void CTermProjectDoc::OnBinarize()
{
	// TODO: 在此添加命令处理程序代码
	if (m_pDib != nullptr)
	{
		COpenCVProcess cvProcess(m_pDib);
		cvProcess.OpenCVBinarize(); // 调用OpenCV二值化处理
		cvProcess.Mat2Dib(*m_pDib); // 将处理后的Mat转换回Dib
		UpdateAllViews(NULL); // 更新所有视图
	}
}

void CTermProjectDoc::OnRemoveHair()
{
	// TODO: 在此添加命令处理程序代码
	if (m_pDib != nullptr)
	{
		COpenCVProcess cvProcess(m_pDib);

		cvProcess.RemoveHair(); // 调用OpenCV去除头发处理
		cvProcess.Mat2Dib(*m_pDib); // 将处理后的Mat转换回Dib
		if (m_pDibBackup != nullptr)
		{
			delete m_pDibBackup; // 释放备份指针
			m_pDibBackup = new CDib(*m_pDib); // 创建新的备份
		}
		UpdateAllViews(NULL); // 更新所有视图
	}
}

void CTermProjectDoc::OnFindContours()
{
	// TODO: 在此添加命令处理程序代码
	if (m_pDib != nullptr) // 如果Dib指针不为空
	{
		COpenCVProcess opencvProcess(m_pDib); // 创建OpenCV处理对象
		opencvProcess.FindContours(); // 调用OpenCV添加轮廓函数
		opencvProcess.Mat2Dib(*m_pDib); // 将处理后的Mat转换回Dib
		UpdateAllViews(NULL); // 更新视图
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

void CTermProjectDoc::OnInvertBinary()
{
	// TODO: 在此添加命令处理程序代码
	if (m_pDib != nullptr)
	{
		COpenCVProcess cvProcess(m_pDib);
		cvProcess.InvertBinary(); // 调用OpenCV二值化反转处理
		cvProcess.Mat2Dib(*m_pDib); // 将处理后的Mat转换回Dib
		UpdateAllViews(NULL); // 更新所有视图
	}
}

void CTermProjectDoc::OnShowSecondImage()
{
	// 创建新的非模态对话框
	if (m_pDib != nullptr) {
		m_pImageDlg = new CImageDialog(m_pDib, this);
		if (!m_pImageDlg->Create(IDD_IMAGE_DIALOG, AfxGetMainWnd())) {
			delete m_pImageDlg;
			m_pImageDlg = nullptr;
			AfxMessageBox(_T("创建对话框失败！"));
			return;
		}
		m_pImageDlg->ShowWindow(SW_SHOW);
	}
}

void CTermProjectDoc::SetImageDialogPtr(CImageDialog* pDlg)
{
	m_pImageDlg = pDlg; // 设置对话框指针
}
