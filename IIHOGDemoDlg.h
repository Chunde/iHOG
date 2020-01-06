
//#define _OPEN_CV_
#pragma once
#include "avisurface.h"
#include "Hog.h"
#ifdef _OPEN_CV_
#include "cv.h"
#include "cvaux.h"
#include "cxcore.h"
#include "highgui.h"

#endif
#define MSG_UPDATE_VIEW 10000
//#define _DIRECTX_VIEW_
#ifdef _DIRECTX_VIEW_
#include <d3d9.h>
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 )
#endif
class CIIHOGDemoDlg : public CDialogEx
{
public:
	CIIHOGDemoDlg(CWnd* pParent = NULL);
	~CIIHOGDemoDlg();
	enum { IDD = IDD_IIHOGDEMO_DIALOG };
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
protected:
	HICON m_hIcon;
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOpen();
public:
	CString m_VideoPath;
	HBITMAP m_hBitmap;
	BYTE* m_pImageBuffer;
	int m_nWidht;
	int m_nHeight;
	int m_nFrameIndex;
	CAviSurface* m_pAviDecoder;
	BOOL m_bIsRunning;
	CRITICAL_SECTION    m_csDecodeBuffer;
	BOOL m_bThreadExitFlag;
	HGDescriptor m_Detector;
	
protected:
	CWinThread* m_pDecoderThread;
	CWinThread* m_pProcessThread;
	CEvent* m_ImageReadyEvent;
	CEvent* m_NextFrameEvent;
	CEvent* m_ImageReadyExitEvent;
	CEvent* m_NextFrameExitEvent;
	static UINT DecoderImageThread(LPVOID para);
	static UINT ProcessImageThread(LPVOID para);
#ifdef _OPEN_CV_
	static UINT cvProcessImageThread(LPVOID para);
#endif
	static UINT TestSpeedThread(LPVOID para);
	void CreateImage();
	void NextFrame();
	void PreviousFrame();
	void TryIt();
	void UpdateUI();
#ifdef _OPEN_CV_
	//CvCapture* m_cvCapture;
#endif
public:
	afx_msg void OnBnClickedNext();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedPrevious();
	LRESULT OnUpdateViewMsg(WPARAM wParam, LPARAM lParam);
	void UpdateView();
	afx_msg void OnBnClickedTest();

	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

