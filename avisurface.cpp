#include "stdafx.h"
#include "avisurface.h"
#include "standartmacros.h"
#include "dderrmsg.h"

CAviSurface::CAviSurface(char *lpszFilename,BOOL bLoop)
{
	AVIFileInit();  
	m_bLoop = bLoop;               
	m_lIndex = 0;
	hVideo = 0;
	ZeroMemory(&m_asiMovie,sizeof(m_asiMovie));
	m_lIndex = m_lFrames = 0;
	m_hicDecompressor = 0;
	m_lpInput=m_lpOutput = 0;
	if (AVIStreamOpenFromFile(&hVideo,(LPCWSTR)lpszFilename,streamtypeVIDEO,0,OF_READ,NULL))
	{
		AVIFileExit();

		return;
	}

	if (!Init())
	{
		Close();
	}
	if (!InitializeCriticalSectionAndSpinCount(&m_csAccessBuffer, 0x00000400) ) 
        return;
}

CAviSurface::~CAviSurface()
{
	DeleteCriticalSection(&m_csAccessBuffer);
	Close();
	AVIFileExit();
}

BOOL CAviSurface::Init()
{
	LONG lFmtLenght;
	
	int ddrval;
	
	AVIStreamFormatSize(hVideo,0,&lFmtLenght);
	m_lpScrFmt = (LPBITMAPINFOHEADER)malloc(lFmtLenght);
	m_lpb4hTargetFmt = (LPBITMAPV4HEADER)malloc(max(lFmtLenght,
		    sizeof(BITMAPV4HEADER)));
	ZeroMemory(m_lpb4hTargetFmt,sizeof(BITMAPV4HEADER));
	AVIStreamReadFormat(hVideo,0,m_lpScrFmt,&lFmtLenght);
	m_lFrames = AVIStreamLength(hVideo);
	AVIStreamInfo(hVideo,&m_asiMovie,sizeof(AVISTREAMINFO));
	
	memcpy(m_lpb4hTargetFmt,m_lpScrFmt,lFmtLenght);

	m_lpb4hTargetFmt->bV4Size = max(lFmtLenght,sizeof(BITMAPV4HEADER));
	m_lpb4hTargetFmt->bV4BitCount = 24;
	m_lpb4hTargetFmt->bV4V4Compression = BI_BITFIELDS;
    if (m_lpb4hTargetFmt->bV4BitCount==24)
		 m_lpb4hTargetFmt->bV4V4Compression = BI_RGB;

    m_lpb4hTargetFmt->bV4ClrUsed = 0;

	/*m_lpb4hTargetFmt->bV4RedMask   = ddsd.ddpfPixelFormat.dwRBitMask;
    m_lpb4hTargetFmt->bV4GreenMask = ddsd.ddpfPixelFormat.dwGBitMask;
    m_lpb4hTargetFmt->bV4BlueMask  = ddsd.ddpfPixelFormat.dwBBitMask;
    m_lpb4hTargetFmt->bV4AlphaMask = ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;*/
	m_lpb4hTargetFmt->bV4SizeImage = 
		   ((m_lpb4hTargetFmt->bV4Width +3)&0xFFFFFFFC) *
		     m_lpb4hTargetFmt->bV4Height *
			   (m_lpb4hTargetFmt->bV4BitCount>>3);
   
	m_nFrameStreamLenght = m_lpScrFmt->biWidth *
		        m_lpScrFmt->biHeight *
				(m_lpScrFmt->biBitCount >> 3);

	if (m_asiMovie.dwSuggestedBufferSize)
		 m_nFrameStreamLenght = (LONG)m_asiMovie.dwSuggestedBufferSize;
	
	m_hicDecompressor = ICDecompressOpen(ICTYPE_VIDEO,
		      m_asiMovie.fccHandler,m_lpScrFmt,
			        (LPBITMAPINFOHEADER)m_lpb4hTargetFmt);

	m_lpInput = (BYTE *)calloc(m_nFrameStreamLenght,1);
	ZeroMemory(m_lpInput,m_nFrameStreamLenght);
	m_lpOutput = (BYTE *)calloc(m_lpb4hTargetFmt->bV4SizeImage,1);
	ZeroMemory(m_lpOutput,m_lpb4hTargetFmt->bV4SizeImage);
	
	if (!m_hicDecompressor)	{

		return FALSE;
	}
	m_lLinePitch = m_lpb4hTargetFmt->bV4Width *
		     		   (m_lpb4hTargetFmt->bV4BitCount>>3);
	ICDecompressBegin(m_hicDecompressor,m_lpScrFmt,
		          (LPBITMAPINFOHEADER)m_lpb4hTargetFmt);
    m_dwFps = m_asiMovie.dwRate/m_asiMovie.dwScale;

	m_iTimeTick = (1000*m_asiMovie.dwScale + 
		              (m_asiMovie.dwRate>>1)) /m_asiMovie.dwRate;

	m_rcSrc.left= 0;
	m_rcSrc.top = 0;
	m_rcSrc.right = m_lpb4hTargetFmt->bV4Width;
	m_rcSrc.bottom = m_lpb4hTargetFmt->bV4Height;

	return TRUE;
}

void CAviSurface::Close()
{
	if (m_hicDecompressor)	{
	  ICDecompressEnd(m_hicDecompressor);
	  ICClose(m_hicDecompressor);
	}
	
	if (m_lpScrFmt)       free(m_lpScrFmt);
	if (m_lpb4hTargetFmt) free(m_lpb4hTargetFmt);
	if (m_lpInput)        free(m_lpInput);
	if (m_lpOutput)       free(m_lpOutput);
	AVIStreamRelease(hVideo);
}

BOOL CAviSurface::ReadFrame(LONG iFrame)
{
	if (iFrame < 0 || iFrame >= m_lFrames)
	{
		return FALSE;
	}
	if (iFrame < m_lFrames) 
	{
	  AVIStreamRead(hVideo, iFrame, 1, m_lpInput, m_nFrameStreamLenght, NULL, NULL);
	  EnterCriticalSection(&m_csAccessBuffer);	
	  ICDecompress(m_hicDecompressor, 0, m_lpScrFmt, m_lpInput,(LPBITMAPINFOHEADER)m_lpb4hTargetFmt, m_lpOutput);
	  LeaveCriticalSection(&m_csAccessBuffer);
	}
	return TRUE;
}
BOOL CAviSurface::ReadFrame(LONG iFrame,BYTE*& buffer)
{
	if (iFrame < 0 || iFrame >= m_lFrames)
	{
		return FALSE;
	}
	if (iFrame < m_lFrames) 
	{
		AVIStreamRead(hVideo, iFrame, 1, m_lpInput, m_nFrameStreamLenght, NULL, NULL);
		EnterCriticalSection(&m_csAccessBuffer);	
		ICDecompress(m_hicDecompressor, 0, m_lpScrFmt, m_lpInput, 
			(LPBITMAPINFOHEADER)m_lpb4hTargetFmt, buffer);
		LeaveCriticalSection(&m_csAccessBuffer);
	}
	return TRUE;
}

BYTE* CAviSurface::GetDecodedBuffer(byte* destBuffer)
{
	EnterCriticalSection(&m_csAccessBuffer);	
	memcpy(destBuffer,m_lpOutput,m_lpScrFmt->biHeight * m_lpScrFmt->biWidth *3);
	LeaveCriticalSection(&m_csAccessBuffer);
	return destBuffer;
}
int CAviSurface::ImageHeight()
{
	if (m_lpScrFmt == NULL)
	{
		return 0;
	}
	return m_lpScrFmt->biHeight;
}
int CAviSurface::ImageWidth()
{
	if (m_lpScrFmt == NULL)
	{
		return 0;
	}
	return m_lpScrFmt->biWidth;
}
int CAviSurface::FrameNumber()
{
	if (m_lpScrFmt == NULL)
	{
		return 0;
	}
	return m_lFrames;
}