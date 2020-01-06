#include "stdafx.h"
#include "math.h"
#include "Hog.h"
#define _PPL_


inline int GreatCommonDivisor(int a,int b)
{
	int c = 0;
	if(a < b)
	{
		c = a;
		a = b;
		b = c;
	}
	while(b > 0)
	{
		c  = a % b;
		a = b;
		b = c;
	}
	return a;
}

IICacheHelper::IICacheHelper()
{
	ppIICacheData = 0;
	pFullHisCacheData = 0;
	iiWidth = 0;
	iiHeight = 0;
#ifdef _SPEEDUP_TABLE_
	pSpeedUpGrid = 0;
	pStaticGrid = 0;
	bSpeedUpFlag = false;
#endif
}
IICacheHelper::~IICacheHelper()
{
	Release();
}
void IICacheHelper::Release()
{

	if(ppIICacheData)
	{
		delete[] ppIICacheData;
		ppIICacheData = 0;
	}

	if(pFullHisCacheData)
	{
		delete[] pFullHisCacheData;
		pFullHisCacheData = 0;
	}
	
#ifdef _SPEEDUP_TABLE_
	if (pSpeedUpGrid)
	{
		delete[] pSpeedUpGrid;
	}
	if (pStaticGrid)
	{
		delete[] pStaticGrid;
	}
	pSpeedUpGrid = 0;
	pStaticGrid = 0;
#endif
}
void IICacheHelper::Configure(HGDescriptor* _hog)
{
	Release();
	m_pHogDescriptor = _hog;
	cacheMaxImageSize = _hog->maxImageSize;
	nBins = _hog->binCount;
#ifdef _ALIGN_BIN_
	nBinStride = (nBins % 2 == 0)? nBins: nBins + 1;
	ppIICacheData = new GT[cacheMaxImageSize.Area() * nBinStride];
#else
	ppIICacheData = new GT[cacheMaxImageSize.Area() * nBins];
#endif

	blockHistSize = (_hog->blockSize / _hog->cellSize).Area() * nBins;
	//generate static reference geometry table for control
	const float scale = 1.05f ;
	float s = 1.f;
	scaleCount = 0;
	GeometryInfo gi;
	int offset = 0;
	cellGrid = (m_pHogDescriptor->blockSize/m_pHogDescriptor->cellSize);
	for(int i = 0; i < 64; i ++)
	{
		gi.winSize = m_pHogDescriptor->windowSize * s;
		gi.blockSize = m_pHogDescriptor->blockSize * s;
		gi.winStride = m_pHogDescriptor->windowStride * s;
		gi.blockStride = m_pHogDescriptor->blockStride * s;
		gi.cellSize = m_pHogDescriptor->cellSize * s;
		gi.cacheStride = Vector2D(GreatCommonDivisor(gi.winStride.x,gi.blockStride.x),GreatCommonDivisor(gi.winStride.y,gi.blockStride.y));
		gi.histCacheOffset = offset;
		int cx = (iiWidth - gi.blockSize.x) / gi.cacheStride.x + 1;
		int cy = (iiHeight - gi.blockSize.y) / gi.cacheStride.y + 1;
		gi.cacheGrid = Vector2D(cx,cy);
		offset += cx * cy * blockHistSize;
		{
			geometryRefTable[i] = gi;
		}
		s *= scale;

	}

#ifdef _SPEEDUP_TABLE_
	GenerateValidGrid(640,480);
#endif
	
}

void IICacheHelper::GenerateGeometryInfo()
{
	const float scale = 1.05f * 1.05f * 1.05f;
	float s = 1.f;
	scaleCount = 0;
	GeometryInfo gi;
	int offset = 0;
	cellGrid = (m_pHogDescriptor->blockSize/m_pHogDescriptor->cellSize);
	do 
	{
		gi.winSize = m_pHogDescriptor->windowSize * s;
		gi.blockSize = m_pHogDescriptor->blockSize * s;
		gi.winStride = m_pHogDescriptor->windowStride * s;
		gi.blockStride = m_pHogDescriptor->blockStride * s;
		gi.cellSize = m_pHogDescriptor->cellSize * s;
		gi.cacheStride = Vector2D(GreatCommonDivisor(gi.winStride.x,gi.blockStride.x),GreatCommonDivisor(gi.winStride.y,gi.blockStride.y));
		gi.histCacheOffset = offset;
		int cx = (iiWidth - gi.blockSize.x) / gi.cacheStride.x + 1;
		int cy = (iiHeight - gi.blockSize.y) / gi.cacheStride.y + 1;
		gi.cacheGrid = Vector2D(cx,cy);
		offset += cx * cy * blockHistSize;
		{
			geometryData[scaleCount ++] = gi;
		}
		s *= scale;

	} while (gi.winSize.x < iiWidth / 3 && gi.winSize.y < iiHeight / 3 && scaleCount < 63);
	if(iiWidth > histCacheSize.x || iiHeight > histCacheSize.y)
	{
		delete[] pFullHisCacheData;
		pFullHisCacheData = 0;
	}
	if(pFullHisCacheData == 0)
	{
		pFullHisCacheData = new float[offset];
		histCacheSize.x = iiWidth;
		histCacheSize.y = iiHeight;
	}
}
void IICacheHelper::ClearIntegralImages()
{
	int len = iiWidth * iiHeight;
#ifdef _ALIGN_BIN_
	len *= nBinStride;
#else
	len *= nBins;

#endif
	double* pt = (double*)ppIICacheData;
	int len1 = len * sizeof(GT) / sizeof(double);
	int len2 = len1 * sizeof(double)/ sizeof(GT);
	for (int i = 0; i < len1; i ++)
	{
		pt[i] = 0;
	}
	for (int i = len2; i < len; i ++)
	{
		ppIICacheData[i] = 0;
	}

}

void IICacheHelper::GenerateIntegralImages()
{
	int index = 0;
	//short bins;
	unsigned char bin1,bin2;
	GT mag1,mag2;

	unsigned char* binPtr = m_pHogDescriptor->anglePtr;
	GT* magPtr = m_pHogDescriptor->magnitudePtr;
	ClearIntegralImages();
	unsigned char* bp = binPtr;
	GT* mp = magPtr;
	int i;
	int j;

	int k;
	int k1,k2,k3;
	int index1,index2,index3;

	GT* pt = ppIICacheData;
	for(i = 0; i < iiWidth * iiHeight; i ++)
	{
		bin1 = bp[0];
		bin2 = bp[1];
		mag1 = mp[0];
		mag2 = mp[1];
		pt[bin1] += mag1;
		pt[bin2] += mag2;
#ifdef _ALIGN_BIN_
		pt += nBinStride;
#else
		pt += nBins;
#endif
		bp += 2;
		mp += 2;
	}
	//Start integration.
#ifdef _ALIGN_BIN_
	pt = ppIICacheData + nBinStride;
	for(i = 1; i < iiWidth; i ++)
	{

		for(j = 0; j < nBins;j ++)
		{
			pt[j] += pt[j - nBinStride];
		}
		pt += nBinStride;


	}

	index = iiWidth * nBinStride;
	pt = ppIICacheData + index;
	for (i = 1; i < iiHeight; i ++)
	{

		for(j = 0; j < nBins;j ++)
		{
			pt[j] += pt[j - index];
		}

		pt += index;
	}

	pt = ppIICacheData + index + nBinStride;
	for (i = 1; i < iiHeight; i ++)
	{
		for(j = 1; j < iiWidth; j ++)
		{

			for (k = 0; k < nBins;k ++)
			{
				pt[k] += (pt[k - nBinStride] + pt[k - index] - pt[k - index - nBinStride]);
			}

			pt += nBinStride;
		}
		pt += nBinStride;
	}
#else
	pt = ppIICacheData + nBins;
	for(i = 1; i < iiWidth; i ++)
	{
		for(j = 0; j < nBins;j ++)
		{
			pt[j] += pt[j - nBins];
		}
		pt += nBins;

	}
	index = iiWidth * nBins;
	pt = ppIICacheData + index;
	for (i = 1; i < iiHeight; i ++)
	{

		for(j = 0; j < nBins;j ++)
		{
			pt[j] += pt[j - index];
		}

		pt += index;
	}

	pt = ppIICacheData + index + nBins;
	for (i = 1; i < iiHeight; i ++)
	{
		for(j = 1; j < iiWidth; j ++)
		{

			for (k = 0; k < nBins;k ++)
			{
				pt[k] += (pt[k - nBins] + pt[k - index] - pt[k - index - nBins]);
			}

			pt += nBins;
		}
		pt += nBins;
	}
#endif

}
GT IICacheHelper::GetIIValue(int index,int x,int y)
{
#ifdef _ALIGN_BIN_
	return ppIICacheData[(y * iiWidth + x) * nBinStride + index];
#else
	return ppIICacheData[(y * iiWidth + x) * nBins + index];
#endif
}
GT IICacheHelper::GetIIRectValue(int binI,Vector2D tlPt,Vector2D brPt)
{
	float v11,v12,v21,v22;
	
	brPt.x --;
	brPt.y --;
#ifdef _ALIGN_BIN_
	int w = iiWidth * nBinStride;
	int x1 = tlPt.x * nBinStride + binI;
	int x2 = brPt.x * nBinStride + binI;
#else
	int w = iiWidth * nBins;
	int x1 = tlPt.x * nBins + binI;
	int x2 = brPt.x * nBins + binI;
#endif
	int y1 = tlPt.y * w;
	int y2 = brPt.y* w;
	v11 = ppIICacheData[y1 + x1];
	v22 = ppIICacheData[y2 + x2];
	v12 = ppIICacheData[y1 + x2];
	v21 = ppIICacheData[y2 + x1];

	return v22 + v11 - v21 - v12;
}
void IICacheHelper::GetRectHist(float* buffer,Vector2D tlPt,Vector2D brPt)
{
	for(int i = 0; i < nBins;i ++)
	{
		buffer[i] = GetIIRectValue(i,tlPt,brPt);
	}
}
void IICacheHelper::CacheImage(ImageInfo* imagePtr)
{
	m_pHogDescriptor->ComputerGradient(imagePtr);
	iiWidth = imagePtr->width;
	iiHeight = imagePtr->height;
	GenerateGeometryInfo();
#ifdef _DSP_OPT_
	DspIntegralImages();
#else
	GenerateIntegralImages();
#endif
	CacheFullHist();
}
void IICacheHelper::GetBlockHist(int x,int y,GeometryInfo& gi,float* buffer)
{
	float* pt = buffer;

	/////////////////////////////////////////////////////////////////////////////////////////
	//As Comprehensive Code Testing, PCA can be Applied to Reduce Histogram Size and Improve 
	//Processing Speed and without lose any significant discriminative power.Since following 
	//code can be optimized via discarding certain dimensions of HOG feature Histogram, 
	//It depends on your decision to make trade off between speed and discriminative power
	//No further Idea available, Please Do as your wish and wisdom in practical application to
	//improve performance.
	//---IIHOG Coded by Halley---2011-9-11-10:41---ShiTong Opt Software L.T.D.
	/////////////////////////////////////////////////////////////////////////////////////////
	/*for(int i = x; i <= x + gi.blockSize.x - gi.cellSize.x;i += gi.cellSize.x)
	{
		for (int j = y; j <= y + gi.blockSize.y - gi.cellSize.y; j += gi.cellSize.y)
		{
			GetRectHist(pt,Vector2D(i,j),Vector2D(i + gi.cellSize.x,j + gi.cellSize.y));
			pt += nBins;

		}
	}*/
	int x0 = x;
	int y0;
	
	for(int i = 0; i < cellGrid.x;i ++)
	{
		y0 = y;
		for (int j = 0; j < cellGrid.y;j ++)
		{
			GetRectHist(pt,Vector2D(x0,y0),Vector2D(x0 + gi.cellSize.x,y0 + gi.cellSize.y));
			pt += nBins;
			y0 += gi.cellSize.y;
		}
		x0 += gi.cellSize.x;
	}
	NormalizeHist(buffer);
}
#ifdef _DSP_OPT_
void IICacheHelper::DspIntegralImages()
{
	int index = 0;
	//short bins;
	unsigned char bin1,bin2;
	GT mag1,mag2;

	unsigned char* binPtr = m_pHogDescriptor->anglePtr;
#ifndef _DISCARD_NORM_
	GT* magPtr = m_pHogDescriptor->magnitudePtr;
	GT* mp = magPtr;
#endif
	ClearIntegralImages();
	unsigned char* bp = binPtr;
	int i;
	int j;

	int k;
	int k1,k2,k3;
	int index1,index2,index3;

#ifdef _USE_SHORT_
	GT* pt = ppIICacheData;
	GT* pt1,*pt2,*pt3;
	unsigned int* _pt,*_pt1,* _pt2,*_pt3;
	for(i = 0; i < iiWidth * iiHeight; i ++)
	{
		bin1 = bp[0];
		bin2 = bp[1];
#ifdef _DISCARD_NORM_

		pt[bin1] +=1;
		pt[bin2] +=1;
#ifdef _ALIGN_BIN_
		pt += nBinStride;
#else
		pt += nBins;
#endif
		bp += 2;

#else
		mag1 = mp[0];
		mag2 = mp[1];
		pt[bin1] +=mag1;
		pt[bin2] +=mag2;
#ifdef _ALIGN_BIN_
		pt += nBinStride;
#else
		pt += nBins;
#endif
		bp += 2;
		mp += 2;
#endif
	}
	//Start integration.
#ifdef _ALIGN_BIN_
	pt = ppIICacheData + nBinStride;
#else
	pt = ppIICacheData + nBins;
#endif
	for(i = 1; i < iiWidth; i ++)
	{
#ifdef _ALIGN_BIN_
		pt1 = pt - nBinStride;
#else
		pt1 = pt - nBins;
#endif
		_pt = (unsigned int*)pt;
		_pt1 = (unsigned int*)pt1;
		//pt[0] += pt1[0];
		//pt[1] += pt1[1];
		_pt[0] += _pt1[0];
		//pt[2] += pt1[2];
		//pt[3] += pt1[3];
		_pt[1] += _pt1[1];
		//pt[4] += pt1[4];
		//pt[5] += pt1[5];
		_pt[2] += _pt1[2];
		//pt[6] += pt1[6];
		//pt[7] += pt1[7];
		_pt[3] += _pt1[3];
		pt[8] += pt1[8];
#ifdef _ALIGN_BIN_
		pt += nBinStride;
#else
		pt += nBins;
#endif

	}
#ifdef _ALIGN_BIN_
	index = iiWidth * nBinStride;
#else
	index = iiWidth * nBins;
#endif
	pt = ppIICacheData + index;
	for (i = 1; i < iiHeight; i ++)
	{
		pt1 = pt - index;
		_pt = (unsigned int*)pt;
		_pt1 = (unsigned int*)pt1;
		//pt[0] += pt1[0];
		//pt[1] += pt1[1];
		_pt[0] += _pt1[0];
		//pt[2] += pt1[2];
		//pt[3] += pt1[3];
		_pt[1] += _pt1[1];
		//pt[4] += pt1[4];
		//pt[5] += pt1[5];
		_pt[2] += _pt1[2];
		//pt[6] += pt1[6];
		//pt[7] += pt1[7];
		_pt[3] += _pt1[3];
		pt[8] += pt1[8];
		pt += index;
	}
#ifdef _ALIGN_BIN_
	pt = ppIICacheData + index + nBinStride;
#else
	pt = ppIICacheData + index + nBins;
#endif

	for (i = 1; i < iiHeight; i ++)
	{
		for(j = 1; j < iiWidth; j ++)
		{
#ifdef _ALIGN_BIN_
			pt1 = pt - nBinStride;
			pt2 = pt - index;
			pt3 = pt2 - nBinStride;
#else
			pt1 = pt - nBins;
			pt2 = pt - index;
			pt3 = pt2 - nBins;
#endif
			_pt = (unsigned int*)pt;
			_pt1 = (unsigned int*)pt1;
			_pt2 = (unsigned int*)pt2;
			_pt3 = (unsigned int*)pt3;
			//pt[0] += (pt1[0] + pt2[0] - pt3[0]);
			//pt[1] += (pt1[1] + pt2[1] - pt3[1]);
			_pt[0] +=(_pt1[0] + _pt2[0] - _pt3[0]);
			//pt[2] += (pt1[2] + pt2[2] - pt3[2]);
			//pt[3] += (pt1[3] + pt2[3] - pt3[3]);
			_pt[1] +=(_pt1[1] + _pt2[1] - _pt3[1]);
			//pt[4] += (pt1[4] + pt2[4] - pt3[4]);
			//pt[5] += (pt1[5] + pt2[5] - pt3[5]);
			_pt[2] +=(_pt1[2] + _pt2[2] - _pt3[2]);
			//pt[6] += (pt1[6] + pt2[6] - pt3[6]);
			//pt[7] += (pt1[7] + pt2[7] - pt3[7]);
			_pt[3] +=(_pt1[3] + _pt2[3] - _pt3[3]);
			pt[8] += (pt1[8] + pt2[8] - pt3[8]);
#ifdef _ALIGN_BIN_
			pt += nBinStride;
		}
		pt += nBinStride;
#else
			pt += nBins;
		}
		pt += nBins;
#endif
	}
#else
	GT* pt = ppIICacheData;
	GT* pt1,*pt2,*pt3;
	for(i = 0; i < iiWidth * iiHeight; i ++)
	{
		bin1 = bp[0];
		bin2 = bp[1];
		mag1 = mp[0];
		mag2 = mp[1];
		pt[bin1] += mag1;
		pt[bin2] += mag2;
		pt += nBins;
		bp += 2;
		mp += 2;
	}
	//Start integration.
	pt = ppIICacheData + nBins;
	for(i = 1; i < iiWidth; i ++)
	{
		pt1 = pt - nBins;
		pt[0] += pt1[0];
		pt[1] += pt1[1];
		pt[2] += pt1[2];
		pt[3] += pt1[3];
		pt[4] += pt1[4];
		pt[5] += pt1[5];
		pt[6] += pt1[6];
		pt[7] += pt1[7];
		pt[8] += pt1[8];
		pt += nBins;

	}
	index = iiWidth * nBins;
	pt = ppIICacheData + index;
	for (i = 1; i < iiHeight; i ++)
	{
		pt1 = pt - index;
		pt[0] += pt1[0];
		pt[1] += pt1[1];
		pt[2] += pt1[2];
		pt[3] += pt1[3];
		pt[4] += pt1[4];
		pt[5] += pt1[5];
		pt[6] += pt1[6];
		pt[7] += pt1[7];
		pt[8] += pt1[8];
		pt += index;
	}
	pt = ppIICacheData + index + nBins;

	for (i = 1; i < iiHeight; i ++)
	{
		for(j = 1; j < iiWidth; j ++)
		{
			pt1 = pt - nBins;
			pt2 = pt - index;
			pt3 = pt2 - nBins;
			pt[0] += (pt1[0] + pt2[0] - pt3[0]);
			pt[1] += (pt1[1] + pt2[1] - pt3[1]);
			pt[2] += (pt1[2] + pt2[2] - pt3[2]);
			pt[3] += (pt1[3] + pt2[3] - pt3[3]);
			pt[4] += (pt1[4] + pt2[4] - pt3[4]);
			pt[5] += (pt1[5] + pt2[5] - pt3[5]);
			pt[6] += (pt1[6] + pt2[6] - pt3[6]);
			pt[7] += (pt1[7] + pt2[7] - pt3[7]);
			pt[8] += (pt1[8] + pt2[8] - pt3[8]);
			pt += nBins;
		}
		pt += nBins;
	}
#endif

}
void IICacheHelper::GetDspBlockHist(int x,int y,GeometryInfo& gi,float* buffer)
{
	//float* pt = buffer;

	/////////////////////////////////////////////////////////////////////////////////////////
	//As Comprehensive Code Testing, PCA can be Applied to Reduce Histogram Size and Improve 
	//Processing Speed and without lose any significant discriminative power.Since following 
	//code can be optimized via discarding certain dimensions of HOG feature Histogram, 
	//It depends on your decision to make trade off between speed and discriminative power
	//No further Idea available, Please Do as your wish and wisdom in practical application to
	//improve performance.
	//---IIHOG Coded by Halley---2011-9-01-10:41---ShiTong Opt Software L.T.D.
	/////////////////////////////////////////////////////////////////////////////////////////
	int xr = x + 2 * gi.cellSize.x;
	int yr = y + 2 * gi.cellSize.y;
	int xl = x;
	int yl = y;
	xl = (xl > 0) * (xl - 1);
	yl = (yl > 0) * (yl - 1);
	xr = (xr >= iiWidth) * (xr - 2) + (xr < iiWidth) * (xr - 1);
	yr = (yr >= iiHeight) * (yr - 2) + (yr < iiHeight) * (yr -1);


	int x0,x2;
	int y0,y2;
	int* p = (int*)buffer;
#ifdef _ALIGN_BIN_
	int ww = iiWidth * nBinStride;
	int dx = gi.cellSize.x * nBinStride;
	int dy = gi.cellSize.y * ww;
	int dd = ww + nBinStride;
	x0 = xl * nBinStride;
	y0 = yl * ww;
	x2 = xr * nBinStride;
#else
	int ww = iiWidth * nBins;
	int dx = gi.cellSize.x * nBins;
	int dy = gi.cellSize.y * ww;
	int dd = ww + nBins;
	x0 = xl * nBins;
	y0 = yl * ww;
	x2 = xr * nBins;
#endif
	y2 = yr * ww;
#ifdef _USE_SHORT_
	unsigned short*p11,*p12,*p13,*p21,*p22,*p23,*p31,*p32,*p33;
#else
	int *p11,*p12,*p13,*p21,*p22,*p23,*p31,*p32,*p33;
#endif
		//* p11,*p12,*p13,*p21,*p22,*p23,*p31,*p32,*p33;
	p11 = ppIICacheData + y0 + x0;
	p12 = p11 + dx;
	p13 = ppIICacheData + y0 + x2;
	p21 = p11 + dy;
	p22 = p12 + dy;
	p23 = p13 + dy;
	p31 = ppIICacheData + y2 + x0;
	p32 = p31 + dx;
	p33 = ppIICacheData + y2 + x2;

#ifdef _USE_SHORT_
	int* _p11 = (int*)p11;
	int* _p12 = (int*)p12;
	int* _p13 = (int*)p13;
	int* _p21 = (int*)p21;
	int* _p22 = (int*)p22;
	int* _p23 = (int*)p23;
	int* _p31 = (int*)p31;
	int* _p32 = (int*)p32;
	int* _p33 = (int*)p33;
	//unsigned short pt[36];
	//int* _p = (int*)pt;
	static int pt0,pt1,pt2,pt3,pt4,pt5,pt6,pt7,pt8,pt9,pt10,pt11,pt12,pt13,pt14,pt15,pt16,pt17,pt18,pt19;

	//p[0] = p22[0]  + p11[0] - p21[0] - p12[0];
	//p[1] = p22[1]  + p11[1] - p21[1] - p12[1];
	pt0 = _p22[0] + _p11[0] - _p21[0] - _p12[0];
	//p[2] = p22[2]  + p11[2] - p21[2] - p12[2];
	//p[3] = p22[3]  + p11[3] - p21[3] - p12[3];
	pt1 = _p22[1] + _p11[1] - _p21[1] - _p12[1];
	//p[4] = p22[4]  + p11[4] - p21[4] - p12[4];
	//p[5] = p22[5]  + p11[5] - p21[5] - p12[5];
	pt2 = _p22[2] + _p11[2] - _p21[2] - _p12[2];
	//p[6] = p22[6]  + p11[6] - p21[6] - p12[6];
	//p[7] = p22[7]  + p11[7] - p21[7] - p12[7];
	pt3 = _p22[3] + _p11[3] - _p21[3] - _p12[3];
	//p[8] = p22[8]  + p11[8] - p21[8] - p12[8];
	pt4 = p22[8]  + p11[8] - p21[8] - p12[8];


	//cell 21
	//p[9] = p32[0] + p21[0] - p31[0] - p22[0];
	//p[10] = p32[1] + p21[1] - p31[1] - p22[1];
	pt5 = _p32[0] + _p21[0] - _p31[0] - _p22[0];
	//p[11] = p32[2] + p21[2] - p31[2] - p22[2];
	//p[12] = p32[3] + p21[3] - p31[3] - p22[3];
	pt6 =  _p32[1] + _p21[1] - _p31[1] - _p22[1];
	//p[13] = p32[4] + p21[4] - p31[4] - p22[4];
	//p[14] = p32[5] + p21[5] - p31[5] - p22[5];
	pt7 = _p32[2] + _p21[2] - _p31[2] - _p22[2];
	//p[15] = p32[6] + p21[6] - p31[6] - p22[6];
	//p[16] = p32[7] + p21[7] - p31[7] - p22[7];
	pt8 = _p32[3] + _p21[3] - _p31[3] - _p22[3];
	//p[17] = p32[8] + p21[8] - p31[8] - p22[8];
	pt9 = p32[8] + p21[8] - p31[8] - p22[8];


	//cell 12
	//p[18] = p23[0] + p12[0] - p22[0] - p13[0];
	//p[19] = p23[1] + p12[1] - p22[1] - p13[1];
	pt10 = _p23[0] + _p12[0] - _p22[0] - _p13[0];
	//p[20] = p23[2] + p12[2] - p22[2] - p13[2];
	//p[21] = p23[3] + p12[3] - p22[3] - p13[3];
	pt11 = _p23[1] + _p12[1] - _p22[1] - _p13[1];
	//p[22] = p23[4] + p12[4] - p22[4] - p13[4];
	//p[23] = p23[5] + p12[5] - p22[5] - p13[5];
	pt12 = _p23[2] + _p12[2] - _p22[2] - _p13[2];
	//p[24] = p23[6] + p12[6] - p22[6] - p13[6];
	//p[25] = p23[7] + p12[7] - p22[7] - p13[7];
	pt13 = _p23[3] + _p12[3] - _p22[3] - _p13[3];
	//p[26] = p23[8] + p12[8] - p22[8] - p13[8];
	pt14 = p23[8] + p12[8] - p22[8] - p13[8];


	//cell 22
	//p[27] = p33[0] + p22[0] - p32[0] - p23[0];
	//p[28] = p33[1] + p22[1] - p32[1] - p23[1];
	pt15 = _p33[0] + _p22[0] - _p32[0] - _p23[0];
	//p[29] = p33[2] + p22[2] - p32[2] - p23[2];
	//p[30] = p33[3] + p22[3] - p32[3] - p23[3];
	pt16 = _p33[1] + _p22[1] - _p32[1] - _p23[1];
	//p[31] = p33[4] + p22[4] - p32[4] - p23[4];
	//p[32] = p33[5] + p22[5] - p32[5] - p23[5];
	pt17 = _p33[2] + _p22[2] - _p32[2] - _p23[2];
	//p[33] = p33[6] + p22[6] - p32[6] - p23[6];
	//p[34] = p33[7] + p22[7] - p32[7] - p23[7];
	pt18 = _p33[3] + _p22[3] - _p32[3] - _p23[3];
	//p[35] = p33[8] + p22[8] - p32[8] - p23[8];
	pt19 = p33[8] + p22[8] - p32[8] - p23[8];

	p[0] = pt0 & 0x0000FFFF;
	p[1] = pt0 >> 16;
	p[2] = pt1 & 0x0000FFFF;
	p[3] = pt1 >> 16;
	p[4] = pt2 & 0x0000FFFF;
	p[5] = pt2 >> 16;
	p[6] = pt3 & 0x0000FFFF;
	p[7] = pt3 >> 16;
	p[8] = pt4;

	p[9] = pt5 & 0x0000FFFF;
	p[10] = pt5 >> 16;
	p[11] = pt6 & 0x0000FFFF;
	p[12] = pt6 >> 16;
	p[13] = pt7 & 0x0000FFFF;
	p[14] = pt7 >> 16;
	p[15] = pt8 & 0x0000FFFF;
	p[16] = pt8 >> 16;
	p[17] = pt9;

	p[18] = pt10 & 0x0000FFFF;
	p[19] = pt10 >> 16;
	p[20] = pt11 & 0x0000FFFF;
	p[21] = pt11 >> 16;
	p[22] = pt12 & 0x0000FFFF;
	p[23] = pt12 >> 16;
	p[24] = pt13 & 0x0000FFFF;
	p[25] = pt13 >> 16;
	p[26] = pt14;

	p[27] = pt15 & 0x0000FFFF;
	p[28] = pt15 >> 16;
	p[29] = pt16 & 0x0000FFFF;
	p[30] = pt16 >> 16;
	p[31] = pt17 & 0x0000FFFF;
	p[32] = pt17 >> 16;
	p[33] = pt18 & 0x0000FFFF;
	p[34] = pt18 >> 16;
	p[35] = pt19;/**/


#else
	/*int x0 ;
	int y0;
	int* p = (int*)buffer;
	int ww = iiWidth * nBins;
	int dx = gi.cellSize.x * nBins;
	int dy = gi.cellSize.y * ww;
	int dd = ww + nBins;
	x0 = x * nBins;
	y0 = y * ww;
	int* p11,*p12,*p13,*p21,*p22,*p23,*p31,*p32,*p33;
	p11 = ppIICacheData + y0 + x0;// ppIICacheData + (y * iiWidth + x) * nBins;//
	p12 = p11 + dx;//ppIICacheData + (y * iiWidth + x + gi.cellSize.x) * nBins;//
	p13 = p12 + dx;//ppIICacheData + (y * iiWidth + x + 2 * gi.cellSize.x) * nBins;//
	p21 = p11 + dy;//ppIICacheData + ((y + gi.cellSize.y) * iiWidth + x) * nBins;//
	p22 = p12 + dy;//ppIICacheData + ((y + gi.cellSize.y) * iiWidth + x + gi.cellSize.x) * nBins;//
	p23 = p13 + dy;//ppIICacheData + ((y + gi.cellSize.y) * iiWidth + x + 2 * gi.cellSize.x) * nBins;//
	p31 = p21 + dy;//ppIICacheData + ((y + 2 * gi.cellSize.y) * iiWidth + x) * nBins;//
	p32 = p22 + dy;//ppIICacheData + ((y + 2 * gi.cellSize.y) * iiWidth + x + gi.cellSize.x) * nBins;//
	p33 = p23 + dy;//ppIICacheData + ((y + 2 * gi.cellSize.y) * iiWidth + x + 2 * gi.cellSize.x) * nBins;//
	*/
	/*
	//c11
	p[0] = p22[0 - dd] - p21[0 - ww] - p12[0 - nBins] + p11[0];
	p[1] = p22[1 - dd] - p21[1 - ww] - p12[1 - nBins] + p11[1];
	p[2] = p22[2 - dd] - p21[2 - ww] - p12[2 - nBins] + p11[2];
	p[3] = p22[3 - dd] - p21[3 - ww] - p12[3 - nBins] + p11[3];
	p[4] = p22[4 - dd] - p21[4 - ww] - p12[4 - nBins] + p11[4];
	p[5] = p22[5 - dd] - p21[5 - ww] - p12[5 - nBins] + p11[5];
	p[6] = p22[6 - dd] - p21[6 - ww] - p12[6 - nBins] + p11[6];
	p[7] = p22[7 - dd] - p21[7 - ww] - p12[7 - nBins] + p11[7];
	p[8] = p22[8 - dd] - p21[8 - ww] - p12[8 - nBins] + p11[8];
	//cell 21
	p[9] = p32[0 - dd] - p31[0 - ww] - p22[0 - nBins] + p21[0];
	p[10] = p32[1 - dd] - p31[1 - ww] - p22[1 - nBins] + p21[1];
	p[11] = p32[2 - dd] - p31[2 - ww] - p22[2 - nBins] + p21[2];
	p[12] = p32[3 - dd] - p31[3 - ww] - p22[3 - nBins] + p21[3];
	p[13] = p32[4 - dd] - p31[4 - ww] - p22[4 - nBins] + p21[4];
	p[14] = p32[5 - dd] - p31[5 - ww] - p22[5 - nBins] + p21[5];
	p[15] = p32[6 - dd] - p31[6 - ww] - p22[6 - nBins] + p21[6];
	p[16] = p32[7 - dd] - p31[7 - ww] - p22[7 - nBins] + p21[7];
	p[17] = p32[8 - dd] - p31[8 - ww] - p22[8 - nBins] + p21[8];
	//cell 12
	p[18] = p23[0 - dd] - p22[0 - ww] - p13[0 - nBins] + p12[0];
	p[19] = p23[1 - dd] - p22[1 - ww] - p13[1 - nBins] + p12[1];
	p[20] = p23[2 - dd] - p22[2 - ww] - p13[2 - nBins] + p12[2];
	p[21] = p23[3 - dd] - p22[3 - ww] - p13[3 - nBins] + p12[3];
	p[22] = p23[4 - dd] - p22[4 - ww] - p13[4 - nBins] + p12[4];
	p[23] = p23[5 - dd] - p22[5 - ww] - p13[5 - nBins] + p12[5];
	p[24] = p23[6 - dd] - p22[6 - ww] - p13[6 - nBins] + p12[6];
	p[25] = p23[7 - dd] - p22[7 - ww] - p13[7 - nBins] + p12[7];
	p[26] = p23[8 - dd] - p22[8 - ww] - p13[8 - nBins] + p12[8];
	//cell 22
	p[27] = p33[0 - dd] - p32[0 - ww] - p23[0 - nBins] + p22[0];
	p[28] = p33[1 - dd] - p32[1 - ww] - p23[1 - nBins] + p22[1];
	p[29] = p33[2 - dd] - p32[2 - ww] - p23[2 - nBins] + p22[2];
	p[30] = p33[3 - dd] - p32[3 - ww] - p23[3 - nBins] + p22[3];
	p[31] = p33[4 - dd] - p32[4 - ww] - p23[4 - nBins] + p22[4];
	p[32] = p33[5 - dd] - p32[5 - ww] - p23[5 - nBins] + p22[5];
	p[33] = p33[6 - dd] - p32[6 - ww] - p23[6 - nBins] + p22[6];
	p[34] = p33[7 - dd] - p32[7 - ww] - p23[7 - nBins] + p22[7];
	p[35] = p33[8 - dd] - p32[8 - ww] - p23[8 - nBins] + p22[8];
*/
	//c11
	p[0] = p22[0]  + p11[0] - p21[0] - p12[0];
	p[1] = p22[1]  + p11[1] - p21[1] - p12[1];
	p[2] = p22[2]  + p11[2] - p21[2] - p12[2];
	p[3] = p22[3]  + p11[3] - p21[3] - p12[3];
	p[4] = p22[4]  + p11[4] - p21[4] - p12[4];
	p[5] = p22[5]  + p11[5] - p21[5] - p12[5];
	p[6] = p22[6]  + p11[6] - p21[6] - p12[6];
	p[7] = p22[7]  + p11[7] - p21[7] - p12[7];
	p[8] = p22[8]  + p11[8] - p21[8] - p12[8];
	//cell 21
	 p[9] = p32[0] + p21[0] - p31[0] - p22[0];
	p[10] = p32[1] + p21[1] - p31[1] - p22[1];
	p[11] = p32[2] + p21[2] - p31[2] - p22[2];
	p[12] = p32[3] + p21[3] - p31[3] - p22[3];
	p[13] = p32[4] + p21[4] - p31[4] - p22[4];
	p[14] = p32[5] + p21[5] - p31[5] - p22[5];
	p[15] = p32[6] + p21[6] - p31[6] - p22[6];
	p[16] = p32[7] + p21[7] - p31[7] - p22[7];
	p[17] = p32[8] + p21[8] - p31[8] - p22[8];
	//cell 12
	p[18] = p23[0] + p12[0] - p22[0] - p13[0];
	p[19] = p23[1] + p12[1] - p22[1] - p13[1];
	p[20] = p23[2] + p12[2] - p22[2] - p13[2];
	p[21] = p23[3] + p12[3] - p22[3] - p13[3];
	p[22] = p23[4] + p12[4] - p22[4] - p13[4];
	p[23] = p23[5] + p12[5] - p22[5] - p13[5];
	p[24] = p23[6] + p12[6] - p22[6] - p13[6];
	p[25] = p23[7] + p12[7] - p22[7] - p13[7];
	p[26] = p23[8] + p12[8] - p22[8] - p13[8];
	//cell 22
	p[27] = p33[0] + p22[0] - p32[0] - p23[0];
	p[28] = p33[1] + p22[1] - p32[1] - p23[1];
	p[29] = p33[2] + p22[2] - p32[2] - p23[2];
	p[30] = p33[3] + p22[3] - p32[3] - p23[3];
	p[31] = p33[4] + p22[4] - p32[4] - p23[4];
	p[32] = p33[5] + p22[5] - p32[5] - p23[5];
	p[33] = p33[6] + p22[6] - p32[6] - p23[6];
	p[34] = p33[7] + p22[7] - p32[7] - p23[7];
	p[35] = p33[8] + p22[8] - p32[8] - p23[8];
	
#endif
#ifndef _DISCARD_NORM_
	int sum = 0;
	int t0,t1,t2,t3,t4,t5;
	//double tt1,tt2,tt3;
	for (int i = 0; i < 36; i += 6)
	{
		t0 = p[i];
		t1 = p[i + 1];
		t2 = p[i + 2];
		t3 = p[i + 3];
		t4 = p[i + 4];
		t5 = p[i + 5];
		//	printf("%d %d %d %d %d %d \r\n",t0,t1,t2,t3,t4,t5);
		sum += t0 * t0 + t1 * t1 + t2 * t2 + t3 * t3 + t4 * t4 + t5 * t5;
		//sum += t0 + t1 + t2 + t3 + t4 + t5;
	}
	//printf("%d\r\n",sum);
	DspNormalizeHist(p,sum);
	//NormalizeHist(buffer);
#endif
}
void IICacheHelper::DspNormalizeHist(int* buffer, float scale)
{
#ifdef _FIX_POINT_SVM_
	scale = sqrt(scale + blockHistSize / 10);
	int s = (int)scale;
	int i;
	for(i = 0; i < blockHistSize - 4; i += 4)
	{
		buffer[i] = (buffer[i] << 10) / s;
		buffer[i + 1] = (buffer[i + 1] << 10) / s;
		buffer[i + 2] = (buffer[i + 2] << 10) / s;
		buffer[i + 3] =  (buffer[i + 3] << 10) / s;
	}
	for(; i < blockHistSize; i ++)
	{
		buffer[i] = (buffer[i] << 10) / s;
	}
#else
	float* hist = (float*)buffer;
	scale = 1.f / (sqrt(scale + blockHistSize / 10));

	int i;
	for(i = 0; i < blockHistSize - 4; i += 4)
	{
		hist[i]= (float)buffer[i] * scale;
		hist[i + 1]= (float)buffer[i + 1] * scale;
		hist[i + 2] = (float)buffer[i + 2] * scale;
		hist[i + 3] =  (float)buffer[i + 3] *scale;
	}
	for(; i < blockHistSize; i ++)
	{
		hist[i] = (float)buffer[i] * scale;
	}
#endif
}
#endif
void IICacheHelper::NormalizeHist(float* hist)
{
	float sum = 0;
	float t;
	float t0,t1,t2,t3;
	int i;
	for(i = 0; i < blockHistSize - 4;i += 4)
	{
		t0 = hist[i];
		t1 = hist[i +1];
		t2 = hist[i +2];
		t3 = hist[i +3];
		sum += t0 * t0 + t1 * t1 + t2 * t2 + t3 * t3;
	}
	for(;i < blockHistSize; i ++)
	{
		t = hist[i];
		sum += t * t;
	}
	float scale = 1.f / (sqrt(sum + blockHistSize * 0.1f));
	/*static float thresh = 0.2;
	sum = 0;
	for(i = 0; i < blockHistSize - 4;i += 4)
	{
		hist[i] = min(hist[i] * scale,thresh);
		hist[i + 1] = min(hist[i + 1] * scale,thresh);
		hist[i + 2] = min(hist[i + 2] * scale,thresh);
		hist[i + 3] = min(hist[i + 3] * scale,thresh);

		sum += hist[i] * hist[i] + hist[i + 1] * hist[i + 1] +hist[i + 2] * hist[i + 2] +hist[i + 3] * hist[i + 3];
	}
	for(; i < blockHistSize; i ++)
	{
		hist[i] = min(hist[i] * scale,thresh);
		sum += hist[i] * hist[i];
	}


	scale = 1.f / (sqrt(sum) + 1e-3f);*/

	for(i = 0; i < blockHistSize - 4; i += 4)
	{
		hist[i] *= scale;
		hist[i + 1] *= scale;
		hist[i + 2] *= scale;
		hist[i + 3] *= scale;
	}
	for(; i < blockHistSize; i ++)
	{
		hist[i] *= scale;
	}
}
float IICacheHelper::EvaluateWindowSignal(int x,int y,GeometryInfo& gi)
{
#ifdef _FIX_POINT_SVM_
	const int* ptr = 0;
	int s = 0;
	int* svmVector = m_pHogDescriptor->hyperPanelVector;
	int rho = svmVector[m_pHogDescriptor->GetDescriptorSize()];
#ifdef _DISCARD_NORM_
	rho =  rho * gi.blockStride.Area() / 64;
#endif
	s = rho;// * 1000;
	int index = 0;
	int k;
	for(int i = x; i <= x + gi.winSize.x - gi.blockSize.x; i += gi.blockStride.x)
	{
		for (int j = y; j <= y + gi.winSize.y - gi.blockSize.y; j += gi.blockStride.y)
		{
			ptr = (int*)GetBlockHist(gi,i,j);

			for(k = 0; k < blockHistSize - 4; k += 4)
			{
				s += (svmVector[index] * ptr[k] + svmVector[index +1] * ptr[k + 1] + svmVector[index + 2] * ptr[k + 2] + svmVector[index + 3] * ptr[k + 3]);
				index += 4;
			}
			for (;k < blockHistSize; k ++)
			{
				s += svmVector[index ++] * ptr[k];
			}

		}
	}
	return s;
#else
	const float* ptr = 0;
	float s = 0;

	float* svmVector = m_pHogDescriptor->hyperPanelVector;
	float rho = svmVector[m_pHogDescriptor->GetDescriptorSize()];
#ifdef _DISCARD_NORM_
	rho = ((float)rho * (float)gi.blockStride.Area() / 64.f);
#endif
	s = rho;
	int index = 0;
	int k;
	for(int i = x; i <= x + gi.winSize.x - gi.blockSize.x; i += gi.blockStride.x)
	{
		for (int j = y; j <= y + gi.winSize.y - gi.blockSize.y; j += gi.blockStride.y)
		{
			ptr = GetBlockHist(gi,i,j);

			for(k = 0; k < blockHistSize - 4; k += 4)
			{
				s += (svmVector[index] * ptr[k] + svmVector[index +1] * ptr[k + 1] + svmVector[index + 2] * ptr[k + 2] + svmVector[index + 3] * ptr[k + 3]);
				index += 4;
			}
			for (;k < blockHistSize; k ++)
			{
				s += svmVector[index ++] * ptr[k];
			}

		}
	}
	return s;
#endif
}
void IICacheHelper::Detect(int scale,RegionList*& pList)
{
	GeometryInfo gi = geometryData[scale];
	float s = 0;
	Vector2D offset;
	Vector2D size;
	for(int y = 0; y < iiHeight - gi.winSize.y; y += gi.winStride.y)
	{
		for (int x = 0; x < iiWidth - gi.winSize.x; x += gi.winStride.x)
		{
#ifdef _SPEEDUP_TABLE_
#ifdef _SPEEDUP_FLAG_
			if (!IsValidGridCell(x,y,gi) && bSpeedUpFlag)
			{
				continue;
			}
#else
			if (!IsValidGridCell(x,y,gi))
			{
				continue;
			}
#endif
#endif
			s = EvaluateWindowSignal(x,y,gi);
#ifdef _FIX_POINT_SVM_

			
#ifdef _DISCARD_NORM_
			int t = (int)(m_pHogDescriptor->responseThreshold * 1024.f);

			if(s > t * gi.blockStride.Area() / 64) 
#else
			int t = (int)(m_pHogDescriptor->responseThreshold * 1024.f * 1024.f);
			//static const int t = (1 << 21);
			if (s > t)
			
#endif
				
#else
			
#ifdef _DISCARD_NORM_
			if(s > m_pHogDescriptor->responseThreshold* gi.blockStride.Area() / 64)
#else
			if(s > m_pHogDescriptor->responseThreshold)
#endif
				
#endif
			{
				
				offset.x = x;
				offset.y = y;
				if (pList == 0)
				{
					pList = new RegionList();
					pList->location = offset;
					pList->size = gi.winSize;
				}
				else
					pList->AppendItemToVector2D(offset,gi.winSize);
				x += (gi.winSize.x / gi.winStride.x) * gi.winStride.x;
			}
		}
	}
}
void IICacheHelper::Compute(ImageInfo* imagePtr,float* descriptor)
{
	if(imagePtr->width != m_pHogDescriptor->windowSize.x || imagePtr->height != m_pHogDescriptor->windowSize.y)
		return;
	CacheImage(imagePtr);
	GeometryInfo gi = geometryData[0];
	const float* ptr = 0;
	
	int index = 0;
	int k;
	for(int i = 0; i <= gi.winSize.x - gi.blockSize.x; i += gi.blockStride.x)
	{
		for (int j = 0; j <= gi.winSize.y - gi.blockSize.y; j += gi.blockStride.y)
		{
			ptr = GetBlockHist(gi,i,j);
			for (int k = 0; k < blockHistSize; k ++)
			{
				descriptor[index ++] = ptr[k];
			}
		}
	}
//	ASSERT(index == m_pHogDescriptor->GetDescriptorSize());
	
}
int IICacheHelper::FindBestGI(int x,int y)
{
	for(int i = 0; i < scaleCount; i ++)
	{
		if(geometryData[i].winSize.x <= x && geometryData[i].winSize.y <= y)
		{
			if((x + y -geometryData[i].winSize.x - geometryData[i].winSize.y ) <= 1)
				return i;
		}
	}
	return -1;
}
int IICacheHelper::FindBestControlGI(int x,int y)
{
	for(int i = 0; i < 64; i ++)
	{
		if(geometryRefTable[i].winSize.x <= x && geometryRefTable[i].winSize.y <= y)
		{
			if((x + y -geometryRefTable[i].winSize.x - geometryRefTable[i].winSize.y ) <= geometryRefTable[i].blockStride.x / 2)
				return i;
		}
	}
	return -1;
}
void IICacheHelper::CacheImage(ImageInfo* imagePtr,GeometryInfo& gi)
{
	m_pHogDescriptor->ComputerGradient(imagePtr);
	iiWidth = imagePtr->width;
	iiHeight = imagePtr->height;
#ifdef _DSP_OPT_
	DspIntegralImages();
#else
	GenerateIntegralImages();
#endif
	CacheSpecialHist(gi);
}
void IICacheHelper::Compute2(ImageInfo* imagePtr,float* descriptor)
{
	int id = FindBestControlGI(imagePtr->width,imagePtr->height);
	if(id == -1)
		return;
	GeometryInfo gi = geometryRefTable[id];

	CacheImage(imagePtr,gi);
	const float* ptr = 0;

	for (int i = 0; i < m_pHogDescriptor->GetDescriptorSize();i ++)
	{
		descriptor[i] = pFullHisCacheData[i];
	}

}
void IICacheHelper::Detect(ImageInfo* imagePtr,RegionList*& pList)
{

#ifdef _SPEEDUP_TABLE_
#ifdef _SPEEDUP_FLAG_
	if (imagePtr->width == speedUpWidth && imagePtr->height == speedUpHeight)
	{
		bSpeedUpFlag = true;
	}
	else
	{
		bSpeedUpFlag = false;
	}
	if (bSpeedUpFlag)
	{
		DimGridStrenght();
	}
#else
	DimGridStrenght();//decrease grdi strenght before detection.
#endif
#endif
	CacheImage(imagePtr);
	for (int s = 0; s < scaleCount; s ++)
	{
		RegionList* hitList = 0;
		Detect(s,hitList);
		if(hitList)
		{
			if(pList == 0)
				pList = hitList;
			else
			{
				RegionList* p = pList;
				while(p->next)
					p = p->next;
				p->next = hitList;

			}
		}
	}
}
void IICacheHelper::CacheFullHist()
{
	float* pt = pFullHisCacheData;
	GeometryInfo gi;
	int cx = 0;
	int cy = 0;


	for(int i = 0; i < scaleCount; i ++)
	{
		gi = geometryData[i];


		cx = 0;
		for (int x = 0; x <= iiWidth - gi.blockSize.x; x += gi.cacheStride.x)
		{
			cx ++;
			cy = 0;
			for (int y = 0; y <= iiHeight - gi.blockSize.y; y += gi.cacheStride.y)
			{

#ifdef _SPEEDUP_TABLE_
#ifdef _SPEEDUP_FLAG_
				if (bSpeedUpFlag)
				{
					if (IsValidGridCell(x,y,gi.blockSize.x,gi.blockSize.y))

					{

#ifdef _DSP_OPT_
						GetDspBlockHist(x,y,gi,pt);
#else
						GetBlockHist(x,y,gi,pt);
#endif
					}
				}
				else
				{
					

					{

#ifdef _DSP_OPT_
						GetDspBlockHist(x,y,gi,pt);
#else
						GetBlockHist(x,y,gi,pt);
#endif
					}
				}

#else
				if (IsValidGridCell(x,y,gi.blockSize.x,gi.blockSize.y))

				{

#ifdef _DSP_OPT_
					GetDspBlockHist(x,y,gi,pt);
#else
					GetBlockHist(x,y,gi,pt);
#endif
				}
#endif
#else
#ifdef _DSP_OPT_
				GetDspBlockHist(x,y,gi,pt);
#else
				GetBlockHist(x,y,gi,pt);
#endif
#endif
				pt += blockHistSize;
			cy ++;
			}
		
		}
	
	}
}
void IICacheHelper::CacheSpecialHist(GeometryInfo& gi)
{
	float* pt = pFullHisCacheData;
	int cx = 0;
	int cy = 0;

	cx = 0;
	for (int x = 0; x <= iiWidth - gi.blockSize.x; x += gi.cacheStride.x)
	{
		cx ++;
		cy = 0;
		for (int y = 0; y <= iiHeight - gi.blockSize.y; y += gi.cacheStride.y)
		{
			GetBlockHist(x,y,gi,pt);
			pt += blockHistSize;
			cy ++;
		}
	}
}
const float* IICacheHelper::GetBlockHist(GeometryInfo& gi,int x,int y)
{
	float* pt = pFullHisCacheData + gi.histCacheOffset;
	int cx = x / gi.cacheStride.x;
	int cy = y / gi.cacheStride.y;

	return pt + (cx * gi.cacheGrid.y + cy) * blockHistSize;
}

#ifdef _SPEEDUP_TABLE_
bool IICacheHelper::IsValidGridCell(int x,int y,int w,int h)
{
	int dx = m_pHogDescriptor->windowStride.x;
	int dy = m_pHogDescriptor->windowStride.y;
	int x0 = x / dx;
	int y0 = y / dy;
	int x1 = (x +w) / dx;
	int y1 = (y + h) / dy;
	return IsValidGridCell(x0,y0) & IsValidGridCell(x1,y1);
}
bool IICacheHelper::IsValidGridCell(int x,int y)
{
	return pSpeedUpGrid[x + y * gridSize.x];
}
bool IICacheHelper::IsValidStaticCell(int x,int y)
{
	return pStaticGrid[x + y * gridSize.x];
}
bool IICacheHelper::IsValidGridCell(int x,int y,GeometryInfo& gi)
{
	int dx = m_pHogDescriptor->windowStride.x;
	int dy = m_pHogDescriptor->windowStride.y;
	int x0 = x / dx;
	int y0 = y / dy;
	int x1 = (x + gi.winSize.x) / dx;
	int y1 = (y + gi.winSize.y) / dy;
	return IsValidGridCell(x0,y0) & IsValidGridCell(x1,y1);
}

void IICacheHelper::GenerateValidGrid(int w,int h)
{
#ifdef _SPEEDUP_FLAG_
	speedUpWidth = w;
	speedUpHeight = h;
#endif
	if (pSpeedUpGrid)
	{
		delete[] pSpeedUpGrid;
		pSpeedUpGrid = 0;
	}
	int dx = m_pHogDescriptor->windowStride.x;
	int dy = m_pHogDescriptor->windowStride.y;
	gridSize.x = w / dx;
	gridSize.y = h / dy;
	pSpeedUpGrid = new int[gridSize.Area()];
	for (int i = 0; i < gridSize.Area(); i ++)
	{
		pSpeedUpGrid[i] = 0;
	}
	LoadStaticGrid();
}
void IICacheHelper::ResetGrid()
{
	int len = gridSize.Area();
	for (int i = 0; i < len;i ++)
	{
		pSpeedUpGrid[i] = pStaticGrid[i];
	}
}
void IICacheHelper::OverlayStaticGrid()
{
	int len = gridSize.Area();
	for (int i = 0; i < len;i ++)
	{
		pSpeedUpGrid[i] |= pStaticGrid[i];
	}
}
void IICacheHelper::HitGridCell(int x,int y,GeometryInfo& gi)
{
	int dx = m_pHogDescriptor->windowStride.x;
	int dy = m_pHogDescriptor->windowStride.y;
	int x0 = x / dx;
	int y0 = y / dy;
	int x1 = (x + gi.winSize.x) / dx;
	int y1 = (y + gi.winSize.y) / dy;
	x0 --;
	y0 --;
	x1 ++;
	y1 ++;
	x0 = (x0 > 0) * x0;
	y0 = (y0 > 0) * y0;
	x1 = (x1 < gridSize.x) * x1 + (x1 >- gridSize.x) * (x1 - 1);
	y1 = (y1 < gridSize.y) * y1 + (y1 >= gridSize.y) * (y1 - 1);
	int* pt = pSpeedUpGrid + gridSize.x * y0;
	for (int i = y0; i < y1; i ++)
	{
		for (int j = x0; j < x1; j ++)
		{
			pt[j] ++;
		}
		pt += gridSize.x;
	}
}
void IICacheHelper::HitGridCell(int x,int y,int w,int h)
{
	int dx = m_pHogDescriptor->windowStride.x;
	int dy = m_pHogDescriptor->windowStride.y;
	int x0 = x / dx;
	int y0 = y / dy;
	int x1 = (x + w) / dx;
	int y1 = (y + h) / dy;
	x0 -= 4;
	y0 -= 4;
	x1 += 4;
	y1 += 4;
	x0 = (x0 > 0) * x0;
	y0 = (y0 > 0) * y0;
	x1 = (x1 < gridSize.x) * x1 + (x1 >= gridSize.x) * (gridSize.x - 1);
	y1 = (y1 < gridSize.y) * y1 + (y1 >= gridSize.y) * (gridSize.y - 1);
	int* pt = pSpeedUpGrid + gridSize.x * y0;//
	for (int i = y0; i < y1; i ++)
	{
		for (int j = x0; j < x1; j ++)
		{
			pt[j] += (pt[j] > 0 ? 1 : 2);
		}
		pt += gridSize.x;
	}
}
void IICacheHelper::DimGridStrenght()
{
	int len = gridSize.Area();
	int v;
	for (int i = 0; i < len;i ++)
	{
		v = pSpeedUpGrid[i];
		pSpeedUpGrid[i] = (v > 0) * (v - 1);
	}
	OverlayStaticGrid();
}
void IICacheHelper::LoadStaticGrid()
{
	static byte roiData[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
	int len = sizeof(roiData) / sizeof(roiData[0]);
	if (len != gridSize.Area())
	{
		return;
	}
	if (pStaticGrid != 0)
	{
		delete[] pStaticGrid;
		pStaticGrid =0;
	}
	pStaticGrid = new int[len];
	for (int i = 0; i < len; i ++)
	{
		pStaticGrid[i] = roiData[i];
	}

}
#endif
