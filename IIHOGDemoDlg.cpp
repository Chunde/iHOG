
// IIHOGDemoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IIHOGDemo.h"
#include "IIHOGDemoDlg.h"
#include "afxdialogex.h"
#include "standartmacros.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CIIHOGDemoDlg::CIIHOGDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CIIHOGDemoDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bThreadExitFlag = FALSE;
	m_bIsRunning = FALSE;
#ifdef _OPEN_CV_
//	m_cvCapture = NULL;
#endif
}
CIIHOGDemoDlg::~CIIHOGDemoDlg()
{
	if (m_pAviDecoder)
	{
		delete m_pAviDecoder;
		m_pAviDecoder = NULL;
	}
	DeleteCriticalSection(&m_csDecodeBuffer);
	delete m_ImageReadyEvent;
	delete m_NextFrameEvent;
}
void CIIHOGDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CIIHOGDemoDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOPEN, &CIIHOGDemoDlg::OnBnClickedOpen)
	ON_BN_CLICKED(IDNEXT, &CIIHOGDemoDlg::OnBnClickedNext)
	ON_BN_CLICKED(IDSTART, &CIIHOGDemoDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDPREVIOUS, &CIIHOGDemoDlg::OnBnClickedPrevious)
	ON_MESSAGE(MSG_UPDATE_VIEW,OnUpdateViewMsg)
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CIIHOGDemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetIcon(m_hIcon, TRUE);	
	SetIcon(m_hIcon, FALSE);

	m_hBitmap = NULL;
	m_pDecoderThread = NULL;
	m_pProcessThread = NULL;
	m_pAviDecoder = NULL;
	m_pImageBuffer = NULL;
	m_ImageReadyEvent = new CEvent(0,TRUE);
	m_NextFrameEvent = new CEvent(0,TRUE);
	m_ImageReadyExitEvent = new CEvent(0,TRUE);;
	m_NextFrameExitEvent = new CEvent(0,TRUE);
	InitializeCriticalSection(&m_csDecodeBuffer);

	
	GetDlgItem(IDNEXT)->EnableWindow(FALSE);
	GetDlgItem(IDPREVIOUS)->EnableWindow(FALSE);
	GetDlgItem(IDSTART)->EnableWindow(FALSE);
	return TRUE;
}
void CIIHOGDemoDlg::OnPaint()
{
	CPaintDC dc(this); 
	if (IsIconic())
	{
		
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		dc.DrawIcon(x, y, m_hIcon);
		
	}
	else
	{
		CDialogEx::OnPaint();
	}
	if (m_hBitmap)
	{
		CDC mdc;
		mdc.CreateCompatibleDC(&dc);
		mdc.SelectObject(m_hBitmap);
		dc.BitBlt(0,0,m_nWidht,m_nHeight,&mdc,0,0,SRCCOPY);
	}
}

HCURSOR CIIHOGDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
void CIIHOGDemoDlg::OnBnClickedOpen()
{
	CFileDialog dlg(TRUE,NULL,0,6UL,_T("Avi file(*.avi)|*.avi||"));
	if (dlg.DoModal() == IDOK)
	{
		m_VideoPath = dlg.GetPathName();
#ifdef _OPEN_CV_
		CvvImage m_Img;
	
		SetWindowTextW(dlg.GetPathName());
		char str[256];
		wchar_t* wc =  m_VideoPath.GetBuffer();
		WideCharToMultiByte(CP_ACP,  WC_COMPOSITECHECK,wc,-1,str,255,NULL,NULL);
		CvCapture* cvCapture = cvCaptureFromAVI( str);
		if(cvCapture == NULL)
			return ;
		IplImage* frame = cvQueryFrame(cvCapture);
		frame = cvQueryFrame(cvCapture);
		if(frame = NULL)
			return;
		frame = cvQueryFrame(cvCapture);
		
		m_nWidht = frame->width;
		m_nHeight = frame->height;
		CreateImage();
		m_Img.CopyOf(frame);
		CClientDC dc(this);
		
		CRect rect(0,0,m_nWidht,m_nHeight);
		CDC mdc;
		mdc.CreateCompatibleDC(&dc);
		mdc.SelectObject(m_hBitmap);
		m_Img.DrawToHDC(mdc.GetSafeHdc(),&rect);
		this->Invalidate();
		cvReleaseCapture(&cvCapture);
		//
#else
		if (m_pAviDecoder)
		{
			delete m_pAviDecoder;
			m_pAviDecoder = NULL;
		}
		m_pAviDecoder = new CAviSurface((char*)m_VideoPath.GetBuffer());
		m_nWidht = m_pAviDecoder->ImageWidth();
		m_nHeight = m_pAviDecoder->ImageHeight();
		m_nFrameIndex = 0;
		CreateImage();
		NextFrame();
		NextFrame();
#endif
		m_Detector.Configure(Vector2D(640,480),Vector2D(32,40),Vector2D(8,8),Vector2D(16,16),Vector2D(8,8),Vector2D(8,8),9);
		m_Detector.LoadHyperPanelVector();
		GetDlgItem(IDOPEN)->EnableWindow(TRUE);
		GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
		GetDlgItem(IDNEXT)->EnableWindow(TRUE);
		GetDlgItem(IDPREVIOUS)->EnableWindow(TRUE);
		GetDlgItem(IDSTART)->EnableWindow(TRUE);/*i*/

	}
}

void CIIHOGDemoDlg::CreateImage()
{
	if (m_hBitmap)
	{
		DeleteObject(m_hBitmap);
		m_hBitmap = NULL;
	}
	BITMAPINFO info;
	::ZeroMemory(&info,sizeof(info));
	info.bmiHeader.biSize = sizeof(info);
	info.bmiHeader.biBitCount = 24;
	info.bmiHeader.biWidth = m_nWidht;
	info.bmiHeader.biHeight = m_nHeight;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biCompression = BI_RGB;
	info.bmiHeader.biSizeImage = 0;
	HDC hdc = ::CreateCompatibleDC(NULL);
	m_hBitmap = CreateDIBSection(hdc,&info,DIB_RGB_COLORS,(void**)&m_pImageBuffer,NULL,NULL);
}
void CIIHOGDemoDlg::NextFrame()
{
	if (m_pAviDecoder == NULL)
	{
		return;
	}
	m_pAviDecoder->ReadFrame(m_nFrameIndex);
	m_pAviDecoder->GetDecodedBuffer(m_pImageBuffer);
	m_nFrameIndex ++;
	this->Invalidate();
}
void CIIHOGDemoDlg::PreviousFrame()
{
	if (m_pAviDecoder == NULL)
	{
		return;
	}
	m_nFrameIndex --;
	if (m_nFrameIndex <= 0)
	{
		m_nFrameIndex = 0;
	}
	m_pAviDecoder->ReadFrame(m_nFrameIndex);
	m_pAviDecoder->GetDecodedBuffer(m_pImageBuffer);
	this->Invalidate();
}
void CIIHOGDemoDlg::OnBnClickedNext()
{
	NextFrame();
}
void CIIHOGDemoDlg::OnBnClickedPrevious()
{
	PreviousFrame();
}
void CIIHOGDemoDlg::OnBnClickedStart()
{
	if (m_bIsRunning)
	{
		m_bThreadExitFlag = TRUE;
		Sleep(500);
		WaitForSingleObject(m_ImageReadyExitEvent->m_hObject,200);
		WaitForSingleObject(m_NextFrameExitEvent->m_hObject,200);
		delete m_pAviDecoder;
		m_pAviDecoder = NULL;
		m_bIsRunning = FALSE;
		UpdateUI();
		return;
	}
#ifdef _OPEN_CV_
	if(m_VideoPath.GetLength() == 0)

#else
	if (m_pAviDecoder == NULL)
#endif
	{
		MessageBox(_T("Please load an AVI file..."));
		return;
	}
	m_bThreadExitFlag = FALSE;

	m_pDecoderThread = AfxBeginThread(DecoderImageThread,this);
	m_pProcessThread = AfxBeginThread(ProcessImageThread,this);
	m_ImageReadyEvent->ResetEvent();
	m_NextFrameEvent->SetEvent();
	m_ImageReadyExitEvent->ResetEvent();
	m_NextFrameExitEvent->ResetEvent();

//#endif
	m_bIsRunning = TRUE;
	UpdateUI();

}
void CIIHOGDemoDlg::UpdateUI()
{
	if (m_bIsRunning)
	{
		SetDlgItemText(IDSTART,_T("Stop"));
		GetDlgItem(IDOPEN)->EnableWindow(FALSE);
		GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
		GetDlgItem(IDNEXT)->EnableWindow(FALSE);
		GetDlgItem(IDPREVIOUS)->EnableWindow(FALSE);
	}
	else
	{

		SetDlgItemText(IDSTART,_T("Start"));
		GetDlgItem(IDSTART)->EnableWindow(FALSE);
		GetDlgItem(IDOPEN)->EnableWindow(TRUE);
		GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
	}
}
UINT CIIHOGDemoDlg::DecoderImageThread(LPVOID para)
{
	CIIHOGDemoDlg* pWnd = (CIIHOGDemoDlg*)para;


#ifdef _OPEN_CV_
	CvvImage m_Img;
	
	char str[256];
	wchar_t* wc = pWnd->m_VideoPath.GetBuffer();
	WideCharToMultiByte(CP_ACP,  WC_COMPOSITECHECK,wc,-1,str,255,NULL,NULL);
	CvCapture* m_cvCapture = cvCaptureFromAVI( str);
	IplImage* tempImage = cvQueryFrame(m_cvCapture);
	for (;tempImage;tempImage = cvQueryFrame(m_cvCapture))
#else
	if (pWnd->m_pAviDecoder == NULL)
	{
		return 0L;
	}
	for (pWnd->m_nFrameIndex = 0; pWnd->m_nFrameIndex < pWnd->m_pAviDecoder->FrameNumber(); pWnd->m_nFrameIndex ++)
#endif
	
	{
		if (pWnd->m_bThreadExitFlag)
		{
			pWnd->m_ImageReadyEvent->SetEvent();
			pWnd->m_ImageReadyExitEvent->SetEvent();
			break;;
		}
		TRACE("decoder wait to enter next frame event\n");
		WaitForSingleObject(pWnd->m_NextFrameEvent->m_hObject,INFINITE);
		if (pWnd->m_bThreadExitFlag)
		{
			pWnd->m_ImageReadyEvent->SetEvent();
			pWnd->m_ImageReadyExitEvent->SetEvent();
			break;
		}
		TRACE("decoder enter next frame event\n");
		pWnd->m_NextFrameEvent->ResetEvent();
		TRACE("decoder reset next frame event\n");
		EnterCriticalSection(&pWnd->m_csDecodeBuffer);
		TRACE("decoder enter cs\n");
#ifdef _OPEN_CV_
		m_Img.CopyOf(tempImage);
		CDC mdc;
		CRect rect(0,0,tempImage->width,tempImage->height);
		mdc.CreateCompatibleDC(pWnd->GetDC());
		mdc.SelectObject(pWnd->m_hBitmap);
		m_Img.DrawToHDC(mdc.GetSafeHdc(),&rect);
		//this->Invalidate();
		//cvReleaseImage(&tempImage);
#else

		pWnd->m_pAviDecoder->ReadFrame(pWnd->m_nFrameIndex);
		pWnd->m_pAviDecoder->GetDecodedBuffer(pWnd->m_pImageBuffer);
#endif
		LeaveCriticalSection(&pWnd->m_csDecodeBuffer);
		TRACE("decoder exit cs\n");
		TRACE("decoder set image ready event\n");
		pWnd->m_ImageReadyEvent->SetEvent();
		//pWnd->SendMessage(MSG_UPDATE_VIEW);

	}
#ifdef _OPEN_CV_
	cvReleaseCapture(&m_cvCapture);
#endif
	pWnd->m_bThreadExitFlag = TRUE;
	pWnd->m_ImageReadyExitEvent->SetEvent();
	TRACE("decoder exit...\n");
	return 0L;
}
UINT CIIHOGDemoDlg::ProcessImageThread(LPVOID para)
{
	CIIHOGDemoDlg* pWnd = (CIIHOGDemoDlg*)para;
	ImageInfo info;
	info.colorChannel = 1;
	info.width = pWnd->m_nWidht;
	info.height = pWnd->m_nHeight;
	info.widthStep = info.width;
	BYTE* dataBuffer = new BYTE[info.width * info.height];
	BYTE* imageBuffer = pWnd->m_pImageBuffer;
	BYTE* ptr;
	BYTE* ptr1;
	info.dataPtr = dataBuffer;
	int v = 0;
	RegionList* regions = 0;
	int pID = 0;
	while(1)
	{
		if (pWnd->m_bThreadExitFlag)
		{
			delete[] dataBuffer;
			pWnd->m_NextFrameEvent->SetEvent();
			pWnd->m_NextFrameExitEvent->SetEvent();
			break;
		}
		TRACE("processor wait image ready event\n");
		WaitForSingleObject(pWnd->m_ImageReadyEvent->m_hObject,INFINITE);
		if (pWnd->m_bThreadExitFlag)
		{
			delete[] dataBuffer;
			pWnd->m_NextFrameEvent->SetEvent();
			pWnd->m_NextFrameExitEvent->SetEvent();
			break;
		}
		TRACE("processor enter image ready event\n");
		pWnd->m_ImageReadyEvent->ResetEvent();
		TRACE("processor reset image ready event\n");

		EnterCriticalSection(&pWnd->m_csDecodeBuffer);
		TRACE("processor enter cs\n");
		ptr = imageBuffer;
		ptr1 = dataBuffer + (info.height - 1) * info.width;
		for (int y = 0; y <info.height;y ++)
		{
			for (int x = 0; x < info.width;x ++)
			{
				v = ptr[0] + 2 * ptr[1] + ptr[2];
				v >>= 2;
				ptr1[x] = (BYTE)v;
				ptr += 3;
			}
			ptr1 -= info.width;
		}

		LARGE_INTEGER ltime0, ltime1, ltimef; //测试线程所用的时间
		double inttime = 0;

		QueryPerformanceFrequency(&ltimef);
		LeaveCriticalSection(&pWnd->m_csDecodeBuffer);
		pWnd->m_NextFrameEvent->SetEvent();

		QueryPerformanceCounter(&ltime0);

		int startTick = GetTickCount();

		pWnd->m_Detector.DetectMultiScale(&info,regions);

		int endTick = GetTickCount();

		double circle = endTick - startTick;

		QueryPerformanceCounter(&ltime1); 
		inttime = (double)(ltime1.QuadPart - ltime0.QuadPart)  * 1000.0/ (ltimef.QuadPart);	
		circle = inttime;
		int n = 0;
		if(regions)
		{
			TRACE("object detected...\n");
			RegionList* p = regions;
			CDC dc;
			

			dc.CreateCompatibleDC(NULL);
			dc.SetBkMode(TRANSPARENT);
			EnterCriticalSection(&pWnd->m_csDecodeBuffer);
			TRACE("object detected enter CS\n");
			HGDIOBJ obj = dc.SelectObject(pWnd->m_hBitmap);
			CBrush brush;
			brush.CreateStockObject(NULL_BRUSH );
			dc.SelectObject(&brush);
			CPen pen;
			pen.CreatePen(0,2,RGB(255,0,0));
			dc.SelectObject(&pen);
			while(p)
			{
				dc.Rectangle(p->location.x,p->location.y,p->location.x + p->size.x,p->location.y +p->size.y);
				p = p->next;
				n ++;
			}
			dc.SelectObject(obj);
			//pWnd->SendMessage(MSG_UPDATE_VIEW);
			pWnd->UpdateView();
			LeaveCriticalSection(&pWnd->m_csDecodeBuffer);
			TRACE("object detected  exit cs\n");
			regions->Release();
			regions = 0;
		}
		else
			pWnd->UpdateView();
		if(pID ++ % 5 == 0)
		{
			CString str;
			str.Format(_T("Regions: %d Frame Processing Time: %.2fms FPS: %.2lf"),n,circle, 1000.0 /(circle == 0 ? 1: circle));
			pWnd->SetWindowTextW(str);
		}
		//pWnd->SendMessage(MSG_UPDATE_VIEW);
	}
	
	pWnd->SetWindowTextW(_T("Speed-Up Integral Image HOG Demo"));
   return 1L;
}
#ifdef _OPEN_CV_
UINT CIIHOGDemoDlg::cvProcessImageThread(LPVOID para)
{

	CIIHOGDemoDlg* pWnd = (CIIHOGDemoDlg*)para;

	CvvImage m_Img;

	char str[256];
	wchar_t* wc =  pWnd->m_VideoPath.GetBuffer();
	WideCharToMultiByte(CP_ACP,  WC_COMPOSITECHECK,wc,-1,str,255,NULL,NULL);
	CvCapture* cvCapture = cvCaptureFromAVI( str);
	if(cvCapture == NULL)
		return 0L ;
	IplImage* frame = cvQueryFrame(cvCapture);
	frame = cvQueryFrame(cvCapture);
	if(frame = NULL)
		return 0;
	frame = cvQueryFrame(cvCapture);
	//cvReleaseCapture(&cvCapture);
	pWnd->m_nWidht = frame->width;
	pWnd->m_nHeight = frame->height;
	pWnd->CreateImage();
	CRect rect(0,0,frame->width,frame->height);
	CDC mdc;
	mdc.CreateCompatibleDC(pWnd->GetDC());


	ImageInfo info;
	info.colorChannel = 1;
	info.width = pWnd->m_nWidht;
	info.height = pWnd->m_nHeight;
	info.widthStep = info.width;
	BYTE* dataBuffer = new BYTE[info.width * info.height];
	BYTE* imageBuffer = pWnd->m_pImageBuffer;
	BYTE* ptr;
	BYTE* ptr1;
	info.dataPtr = dataBuffer;
	int v = 0;
	RegionList* regions = 0;
	int pID = 0;
	
	
	while(frame)
	{
		m_Img.CopyOf(frame);
		
		//mdc.SelectObject(m_hBitmap);
		HGDIOBJ obj = mdc.SelectObject(pWnd->m_hBitmap);
		m_Img.DrawToHDC(mdc.GetSafeHdc(),&rect);
		mdc.SelectObject(obj);
		//cvReleaseImage(&frame);
		frame = NULL;
		if (pWnd->m_bThreadExitFlag)
		{
			delete[] dataBuffer;
			pWnd->m_NextFrameEvent->SetEvent();
			pWnd->m_NextFrameExitEvent->SetEvent();
			return 0L;
		}

		if (pWnd->m_bThreadExitFlag)
		{
			delete[] dataBuffer;
			pWnd->m_NextFrameEvent->SetEvent();
			pWnd->m_NextFrameExitEvent->SetEvent();
			return 0L;
		}

		ptr = imageBuffer;
		ptr1 = dataBuffer + (info.height - 1) * info.width;
		for (int y = 0; y <info.height;y ++)
		{
			for (int x = 0; x < info.width;x ++)
			{
				v = ptr[0] + 2 * ptr[1] + ptr[2];
				v >>= 2;
				ptr1[x] = (BYTE)v;
				ptr += 3;
			}
			ptr1 -= info.width;
		}


		int startTick = GetTickCount();
		pWnd->m_Detector.DetectMultiScale(&info,regions);
		int endTick = GetTickCount();
		int circle = endTick - startTick;

		int n = 0;
		if(regions)
		{
			TRACE("object detected...\n");
			RegionList* p = regions;
			CDC dc;


			dc.CreateCompatibleDC(NULL);
			dc.SetBkMode(TRANSPARENT);
			HGDIOBJ obj = dc.SelectObject(pWnd->m_hBitmap);
			CBrush brush;
			brush.CreateStockObject(NULL_BRUSH );
			dc.SelectObject(&brush);
			CPen pen;
			pen.CreatePen(0,2,RGB(255,0,0));
			dc.SelectObject(&pen);
			while(p)
			{
				dc.Rectangle(p->location.x,p->location.y,p->location.x + p->size.x,p->location.y +p->size.y);
				p = p->next;
				n ++;
			}
			dc.SelectObject(obj);
			pWnd->UpdateView();
			regions->Release();
			regions = 0;
		}
		else
			pWnd->UpdateView();
		if(pID ++ % 5 == 0)
		{
			CString str;
			str.Format(_T("Regions: %d Time: %d FPS: %f"),n,circle, 1000.0f / (circle + 1));
			pWnd->SetWindowTextW(str);
		}/**/
		//pWnd->SendMessage(MSG_UPDATE_VIEW);
		frame = cvQueryFrame(cvCapture);
	}

	return 0L;
}
#endif
LRESULT CIIHOGDemoDlg::OnUpdateViewMsg(WPARAM wParam, LPARAM lParam)
{
	UpdateView();
	return 1L;
}
void CIIHOGDemoDlg::UpdateView()
{

	if (m_hBitmap == NULL)
	{
		return;
	}
	CClientDC dc(this);
	CDC mdc;
	mdc.CreateCompatibleDC(&dc);
	//EnterCriticalSection(&m_csDecodeBuffer);
	HGDIOBJ obj = mdc.SelectObject(m_hBitmap);
	dc.BitBlt(0,0,m_nWidht,m_nHeight,&mdc,0,0,SRCCOPY);
	mdc.SelectObject(obj);
	//CRect rect(0,0,m_nWidht,m_nHeight);
	//InvalidateRect(&rect);
}


void CIIHOGDemoDlg::OnBnClickedTest()
{
	//TryIt();
	 AfxBeginThread(TestSpeedThread,this);
}
UINT CIIHOGDemoDlg::TestSpeedThread(LPVOID para)
{
	CIIHOGDemoDlg* pWnd = (CIIHOGDemoDlg*)para;
	if (pWnd->m_pAviDecoder == NULL)
	{
		return 0L;
	}
	ImageInfo info;
	info.colorChannel = 1;
	info.width = pWnd->m_nWidht;
	info.height = pWnd->m_nHeight;
	info.widthStep = info.width;
	BYTE* dataBuffer = new BYTE[info.width * info.height];
	BYTE* imageBuffer = pWnd->m_pImageBuffer;
	BYTE* ptr;
	info.dataPtr = dataBuffer;
	int v = 0;
	RegionList* regions = 0;
	for (pWnd->m_nFrameIndex = 0; pWnd->m_nFrameIndex < pWnd->m_pAviDecoder->FrameNumber(); pWnd->m_nFrameIndex ++)
	{
		pWnd->m_pAviDecoder->ReadFrame(pWnd->m_nFrameIndex);
		pWnd->m_pAviDecoder->GetDecodedBuffer(pWnd->m_pImageBuffer);

		ptr = imageBuffer;
		for (int i = 0; i < info.width * info.height; i ++)
		{
			v = ptr[0] + 2 * ptr[1] + ptr[2];
			v >>= 2;
			dataBuffer[i] = (BYTE)v;
			ptr += 3;
		}
		__int64 startTick = GetTickCount();
		pWnd->m_Detector.DetectMultiScale(&info,regions);
		__int64 endTick = GetTickCount();
		__int64 circle = endTick - startTick;
		CString str;
		str.Format(_T("Time: %d"),circle);
		pWnd->SetWindowTextW(str);
	}
	
	return 0L;
}
void CIIHOGDemoDlg::TryIt()
{
	if (m_pAviDecoder == NULL)
	{
		return;
	}
	ImageInfo info;
	info.colorChannel = 1;
	info.width = m_nWidht;
	info.height =m_nHeight;
	info.widthStep = info.width;
	BYTE* dataBuffer = new BYTE[info.width * info.height];
	BYTE* imageBuffer = m_pImageBuffer;
	BYTE* ptr;
	info.dataPtr = dataBuffer;
	int v = 0;
	RegionList* regions = 0;
	CDC* dc =GetDC();
			CDC mdc;
			mdc.CreateCompatibleDC(dc);
			mdc.SelectObject(m_hBitmap);
	for (m_nFrameIndex = 0; m_nFrameIndex < m_pAviDecoder->FrameNumber(); m_nFrameIndex ++)
	{
		m_pAviDecoder->ReadFrame(m_nFrameIndex);
		m_pAviDecoder->GetDecodedBuffer(m_pImageBuffer);

		ptr = imageBuffer;
		for (int i = 0; i < info.width * info.height; i ++)
		{
			v = ptr[0] + 2 * ptr[1] + ptr[2];
			v >>= 2;
			dataBuffer[i] = (BYTE)v;
			ptr += 3;
		}
		__int64 startTick = GetTickCount64();
		m_Detector.DetectMultiScale(&info,regions);
		__int64 endTick = GetTickCount64();
		__int64 circle = endTick - startTick;
		CString str;
		str.Format(_T("Frame Index: %d Time: %d"),m_nFrameIndex,circle);
		SetWindowTextW(str);
		if(regions)
		{
			
			RegionList* p = regions;
			
			dc->BitBlt(0,0,info.width,info.height,&mdc,0,0,SRCCOPY);
			
			while(p)
			{
				dc->Rectangle(p->location.x,p->location.y,p->location.x + p->size.x,p->location.y +p->size.y);
				p = p->next;
			}
			regions->Release();
			regions = 0;
		}
		//UpdateWindow();

	}
}

void CIIHOGDemoDlg::OnTimer(UINT_PTR nIDEvent)
{
	CDialogEx::OnTimer(nIDEvent);
}
