
// TermProjectView.h: CTermProjectView 类的接口
//

#pragma once


class CTermProjectView : public CView
{
protected: // 仅从序列化创建
	CTermProjectView() noexcept;
	DECLARE_DYNCREATE(CTermProjectView)

// 特性
public:
	CTermProjectDoc* GetDocument() const;

// 操作
public:

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 实现
public:
	virtual ~CTermProjectView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // TermProjectView.cpp 中的调试版本
inline CTermProjectDoc* CTermProjectView::GetDocument() const
   { return reinterpret_cast<CTermProjectDoc*>(m_pDocument); }
#endif

