#pragma once
#include "afxdialogex.h"
#include "Dib.h"

// 前向声明CTermProjectDoc类
class CTermProjectDoc; // 添加这一行进行前向声明

// CImageDialog 对话框

class CImageDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CImageDialog)

public:
	CImageDialog(CDib* pDibImage, CTermProjectDoc* pDoc, CWnd* pParent = nullptr);
	virtual ~CImageDialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_IMAGE_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	DECLARE_MESSAGE_MAP()
	CTermProjectDoc* m_pDocument; // 保存文档指针
	CDib* m_pDibImage;

public:
	afx_msg void OnPaint();
	virtual void PostNcDestroy();
	afx_msg void OnClose();
};
