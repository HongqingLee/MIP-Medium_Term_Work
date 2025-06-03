// CImageDialog.cpp: 实现文件
//

#include "pch.h"
#include "TermProject.h"
#include "afxdialogex.h"
#include "CImageDialog.h"
#include "TermProjectDoc.h"


// CImageDialog 对话框

IMPLEMENT_DYNAMIC(CImageDialog, CDialogEx)

CImageDialog::CImageDialog(CDib* pDibImage, CTermProjectDoc* pDoc, CWnd* pParent /*=nullptr*/)
: CDialogEx(IDD_IMAGE_DIALOG, pParent), m_pDibImage(pDibImage), m_pDocument(pDoc)
{
}

CImageDialog::~CImageDialog()
{
}

void CImageDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CImageDialog, CDialogEx)
	ON_WM_PAINT()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CImageDialog 消息处理程序
void CImageDialog::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CDialogEx::OnPaint()
	if (m_pDibImage != nullptr && !m_pDibImage->IsNull())
	{
		// 绘制图像
		m_pDibImage->Draw(dc.m_hDC, 0, 0, m_pDibImage->GetWidth(), m_pDibImage->GetHeight());
	}
}

void CImageDialog::PostNcDestroy()
{
	// 通知文档对象，对话框已关闭
	if (m_pDocument != nullptr)
	{
		m_pDocument->SetImageDialogPtr(nullptr);
	}

	CDialogEx::PostNcDestroy();
	delete this;
}

void CImageDialog::OnClose()
{
	DestroyWindow();
}

BOOL CImageDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetWindowText(_T("Image Viewer"));
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}
