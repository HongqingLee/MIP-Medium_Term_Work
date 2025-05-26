
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
