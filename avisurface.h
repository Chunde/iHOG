/*
 *      AVI interface for the demo of Speed Up Integral Image Based HOG .
 *
 *
 *	
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS(CHUNDE HUANG)
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 *
*/

#ifndef __AVISURFACE_H
#define __AVISURFACE_H
#include <vfw.h>
//#include "ddsound.h"
class CAviSurface 
{
protected:
	HRESULT DrawBuffer(void);
	BOOL m_bRunning;                     // Video is playing
	LONG m_lIndex,						 // actual Frame - Index
		m_lFrames;                       // Video-Frames in AVI-File
	BOOL m_bLoop;                        // Looping of Video on? 
	DWORD m_dwFps;                       // Freames per Second
	int m_iTimerID;				         // TimerID of timeEvents
	int m_iTimeTick;                     // Time between two frames 
private:
	RECT m_rcSrc;
	CRITICAL_SECTION    m_csAccessBuffer;
	PAVISTREAM hVideo;               // the Handle to the AVI-Videostream
	AVISTREAMINFO m_asiMovie;            // The Video-Information
	LPBYTE m_lpInput,					 // Decompressor Input -Buffer
		   m_lpOutput;                   // Decompressor Output-Buffer
	HIC  m_hicDecompressor;			     // Handle to the AVI-Decompressor
	LONG m_nFrameStreamLenght,						 // Input-Bufferlength
		 m_lLinePitch;                   // Bitmap-Linepitch
	LPBITMAPINFOHEADER m_lpScrFmt;       // Format of VideoInput
	LPBITMAPV4HEADER m_lpb4hTargetFmt;   // Format of Surface
	DWORD m_dwColorKeyFlag,              // Flag for Colorkeying in Blt
		  m_dwColorKeyFlagFast;          // Flag for Colorkeying in BltFast 

public:
	CAviSurface(char *lpszFilename,BOOL bLoop=FALSE);

	virtual ~CAviSurface();


public:
	BOOL ReadFrame(LONG iFrame);
	BOOL ReadFrame(LONG iFrame, BYTE*& buffer);


private:
	BOOL Init();
	void Close();

public:
	BYTE* GetDecodedBuffer(byte* destBuffer);
	int ImageWidth();
	int ImageHeight();
	int FrameNumber();

};


#pragma comment(lib,"vfw32.lib")

#endif
