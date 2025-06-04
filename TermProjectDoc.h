
// TermProjectDoc.h: CTermProjectDoc 类的接口
//


#pragma once
#include "Dib.h"
#include<opencv2/opencv.hpp>

class CTermProjectDoc : public CDocument
{
protected: // 仅从序列化创建
	CTermProjectDoc() noexcept;
	DECLARE_DYNCREATE(CTermProjectDoc)

// 特性
public:
	CDib* m_pDib; // 图像数据
	CDib* m_pDibBackup; // 图像备份数据

// 操作
public:

// 重写
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// 实现
public:
	virtual ~CTermProjectDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	cv::Mat m_grayMat;
	cv::Mat m_hairRemovedMat;
	cv::Mat m_binaryInvMat;
	cv::Mat m_morphOpenMat;
	cv::Mat m_distTransMat;
	cv::Mat m_sureFgMat;
	cv::Mat m_unknownMat;
	cv::Mat m_markersMat;
	cv::Mat m_watershedInput;
	cv::Mat m_watershedBoundaryMat;

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// 用于为搜索处理程序设置搜索内容的 Helper 函数
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	afx_msg void OnOriginal();
	afx_msg void OnRemoveHair();
	afx_msg void OnDrawContoursOnOriginal();

public:
	afx_msg void OnGray();
	afx_msg void OnBinaryInv();
	afx_msg void OnMorphOpen();
	afx_msg void OnDistanceTransform();
	afx_msg void OnSureForeground();
	afx_msg void OnUnknown();
	afx_msg void OnDrawWatershedBoundary();
	afx_msg void OnWatershed();
	afx_msg void OnExtractLesionFeatures();
};
