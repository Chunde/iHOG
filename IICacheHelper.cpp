#include "stdafx.h"
#include "math.h"
#include "Hog.h"
#include "windows.h"
#define _PPL_
using namespace std;
//inline int min(int a,int b){return a > b ? b:a;}
//inline float min(float a,float b){return a > b ? b : a;}
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
	pEnergyImage = 0;
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
	if (pEnergyImage)
	{
		delete[] pEnergyImage;
		pEnergyImage = 0;
	}
}
void IICacheHelper::Configure(HGDescriptor* _hog)
{
	Release();
	m_pHogDescriptor = _hog;
	cacheMaxImageSize = _hog->maxImageSize;
	nBins = _hog->binCount;
	pEnergyImage = new double[cacheMaxImageSize.Area()];
	nBinStride = (nBins % 2 == 0)? nBins: nBins + 1;
	ppIICacheData = new GT[cacheMaxImageSize.Area() * nBinStride];
	blockHistSize = (_hog->blockSize / _hog->cellSize).Area() * nBins;
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
}
double* IICacheHelper::GetEnergyImage()
{
	return  pEnergyImage;
}
void IICacheHelper::GenerateGeometryInfo()
{
	const float scale = 1.05f;// ;
	float s = 1;// 1.05f * 1.05f * 1.05f;
	scaleCount = 0;
	GeometryInfo gi;
	int offset = 0;
	cellGrid = (m_pHogDescriptor->blockSize/m_pHogDescriptor->cellSize);
	s = 0.65f;
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
	} while (gi.winSize.x < iiWidth / 3 && gi.winSize.y < iiHeight / 3 && scaleCount < 64);
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
void IICacheHelper::ClearEnergyImage()
{
	if (pEnergyImage == 0)
	{
		return;
	}
	for (int i = 0;i < iiWidth * iiHeight; i ++)
	{
		pEnergyImage[i] = 0;
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
void IICacheHelper::Detect(ImageInfo* imagePtr,RegionList*& pList)
{
	CacheImage(imagePtr);
	ClearEnergyImage();
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
void IICacheHelper::FillEnergyImage(int x,int y,int w,int h,double e)
{
	double* pt = pEnergyImage+ iiWidth * y;;
	for (int i = y; i < y + h; i ++)
	{
		for (int j = x; j < x + w; j ++ )
		{
			pt[j] += e;
		}
		pt += iiWidth;
	}
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
#ifdef _DSP_OPT_
				GetDspBlockHist(x,y,gi,pt);
#else
				GetBlockHist(x,y,gi,pt);
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
void IICacheHelper::GenerateIntegralImages()
{
	int index = 0;
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
	GT* pt = ppIICacheData;
	for(i = 0; i < iiWidth * iiHeight; i ++)
	{
		bin1 = bp[0];
		bin2 = bp[1];
		mag1 = mp[0];
		mag2 = mp[1];
		pt[bin1] += mag1;
		pt[bin2] += mag2;
		pt += nBinStride;
		bp += 2;
		mp += 2;
	}
	//Start integration.
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
}
GT IICacheHelper::GetIIValue(int index,int x,int y)
{

	return ppIICacheData[(y * iiWidth + x) * nBinStride + index];

}
GT IICacheHelper::GetIIRectValue(int binI,Vector2D tlPt,Vector2D brPt)
{
	float v11,v12,v21,v22;
	if (tlPt.x > 0)
	{
		tlPt.x --;
	}
	if (tlPt.y > 0)
	{
		tlPt.y --;
	}
	if (brPt.x >= iiWidth)
	{
		brPt.x --;
	}
	if (brPt.y >= iiHeight)
	{
		brPt.y --;
	}
	brPt.x --;
	brPt.y --;
	int w = iiWidth * nBinStride;
	int x1 = tlPt.x * nBinStride + binI;
	int x2 = brPt.x * nBinStride + binI;
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
		pt += nBinStride;
		bp += 2;
#else
		mag1 = mp[0];
		mag2 = mp[1];
		pt[bin1] +=mag1;
		pt[bin2] +=mag2;

		pt += nBinStride;

		bp += 2;
		mp += 2;
#endif
	}
	//Start integration.
	pt = ppIICacheData + nBinStride;
	for(i = 1; i < iiWidth; i ++)
	{
		pt1 = pt - nBinStride;
		_pt = (unsigned int*)pt;
		_pt1 = (unsigned int*)pt1;
		_pt[0] += _pt1[0];
		_pt[1] += _pt1[1];
		_pt[2] += _pt1[2];
		_pt[3] += _pt1[3];
		pt[8] += pt1[8];
		pt += nBinStride;
	}
	index = iiWidth * nBinStride;
	pt = ppIICacheData + index;
	for (i = 1; i < iiHeight; i ++)
	{
		pt1 = pt - index;
		_pt = (unsigned int*)pt;
		_pt1 = (unsigned int*)pt1;
		_pt[0] += _pt1[0];
		_pt[1] += _pt1[1];
		_pt[2] += _pt1[2];
		_pt[3] += _pt1[3];
		pt[8] += pt1[8];
		pt += index;
	}
	pt = ppIICacheData + index + nBinStride;
	for (i = 1; i < iiHeight; i ++)
	{
		for(j = 1; j < iiWidth; j ++)
		{
			pt1 = pt - nBinStride;
			pt2 = pt - index;
			pt3 = pt2 - nBinStride;
			_pt = (unsigned int*)pt;
			_pt1 = (unsigned int*)pt1;
			_pt2 = (unsigned int*)pt2;
			_pt3 = (unsigned int*)pt3;
			_pt[0] +=(_pt1[0] + _pt2[0] - _pt3[0]);
			_pt[1] +=(_pt1[1] + _pt2[1] - _pt3[1]);
			_pt[2] +=(_pt1[2] + _pt2[2] - _pt3[2]);
			_pt[3] +=(_pt1[3] + _pt2[3] - _pt3[3]);
			pt[8] += (pt1[8] + pt2[8] - pt3[8]);
			pt += nBinStride;
		}
		pt += nBinStride;
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
	int xr = x + 2 * gi.cellSize.x -1;
	int yr = y + 2 * gi.cellSize.y -1;
	int xl = x;
	int yl = y;
	xl = (xl > 0) * (xl - 1);
	yl = (yl > 0) * (yl - 1);
	xr = (xr >= iiWidth) * (xr - 1) + (xr < iiWidth) * (xr);
	yr = (yr >= iiHeight) * (yr - 1) + (yr < iiHeight) * (yr);
	int x0,x2;
	int y0,y2;
#ifdef _DISCARD_NORM_
#ifdef _FIX_POINT_SVM_
	int* p = (int*)buffer;
#else
	float* p = buffer;
#endif
#else
	int* p = (int*)buffer;
#endif

	int ww = iiWidth * nBinStride;
	int dx = gi.cellSize.x * nBinStride;
	int dy = gi.cellSize.y * ww;
	int dd = ww + nBinStride;
	x0 = xl * nBinStride;
	y0 = yl * ww;
	x2 = xr * nBinStride;

	y2 = yr * ww;
#ifdef _USE_SHORT_
	unsigned short*p11,*p12,*p13,*p21,*p22,*p23,*p31,*p32,*p33;
	static unsigned short p00[] = {0,0,0,0,0,0,0,0,0,0};
#else
	unsigned int *p11,*p12,*p13,*p21,*p22,*p23,*p31,*p32,*p33;
	static unsigned int p00[] = {0,0,0,0,0,0,0,0,0,0};
#endif
	p11 = ppIICacheData + y0 + x0;
	p12 = p11 + dx;
	p13 = ppIICacheData + y0 + x2;
	p21 = p11 + dy;
	p22 = p12 + dy;
	p23 = p13 + dy;
	p31 = ppIICacheData + y2 + x0;
	p32 = p31 + dx;
	p33 = ppIICacheData + y2 + x2;
	if (xl == 0 )
	{
		p11 = p00;
		p21 = p00;
		p31 = p00;
	}
	if (yl == 0)
	{
		p11 = p00;
		p12 = p00;
		p13 = p00;
	}
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
	static int pt0,pt1,pt2,pt3,/*pt4,*/pt5,pt6,pt7,pt8,/*pt9,*/pt10,pt11,pt12,pt13,/*pt14,*/pt15,pt16,pt17,pt18/*,pt19*/;

	pt0 = _p22[0] + _p11[0] - _p21[0] - _p12[0];
	pt1 = _p22[1] + _p11[1] - _p21[1] - _p12[1];
	pt2 = _p22[2] + _p11[2] - _p21[2] - _p12[2];
	pt3 = _p22[3] + _p11[3] - _p21[3] - _p12[3];
	p[8] = p22[8]  + p11[8] - p21[8] - p12[8];
	pt5 = _p32[0] + _p21[0] - _p31[0] - _p22[0];
	pt6 =  _p32[1] + _p21[1] - _p31[1] - _p22[1];
	pt7 = _p32[2] + _p21[2] - _p31[2] - _p22[2];
	pt8 = _p32[3] + _p21[3] - _p31[3] - _p22[3];
	p[17] = p32[8] + p21[8] - p31[8] - p22[8];

	pt10 = _p23[0] + _p12[0] - _p22[0] - _p13[0];
	pt11 = _p23[1] + _p12[1] - _p22[1] - _p13[1];
	pt12 = _p23[2] + _p12[2] - _p22[2] - _p13[2];
	pt13 = _p23[3] + _p12[3] - _p22[3] - _p13[3];
	p[26] = p23[8] + p12[8] - p22[8] - p13[8];
	pt15 = _p33[0] + _p22[0] - _p32[0] - _p23[0];
	pt16 = _p33[1] + _p22[1] - _p32[1] - _p23[1];
	pt17 = _p33[2] + _p22[2] - _p32[2] - _p23[2];
	pt18 = _p33[3] + _p22[3] - _p32[3] - _p23[3];
	p[35] = p33[8] + p22[8] - p32[8] - p23[8];

	p[0] = pt0 & 0x0000FFFF;
	p[1] = pt0 >> 16;
	p[2] = pt1 & 0x0000FFFF;
	p[3] = pt1 >> 16;
	p[4] = pt2 & 0x0000FFFF;
	p[5] = pt2 >> 16;
	p[6] = pt3 & 0x0000FFFF;
	p[7] = pt3 >> 16;

	p[9] = pt5 & 0x0000FFFF;
	p[10] = pt5 >> 16;
	p[11] = pt6 & 0x0000FFFF;
	p[12] = pt6 >> 16;
	p[13] = pt7 & 0x0000FFFF;
	p[14] = pt7 >> 16;
	p[15] = pt8 & 0x0000FFFF;
	p[16] = pt8 >> 16;

	p[18] = pt10 & 0x0000FFFF;
	p[19] = pt10 >> 16;
	p[20] = pt11 & 0x0000FFFF;
	p[21] = pt11 >> 16;
	p[22] = pt12 & 0x0000FFFF;
	p[23] = pt12 >> 16;
	p[24] = pt13 & 0x0000FFFF;
	p[25] = pt13 >> 16;

	p[27] = pt15 & 0x0000FFFF;
	p[28] = pt15 >> 16;
	p[29] = pt16 & 0x0000FFFF;
	p[30] = pt16 >> 16;
	p[31] = pt17 & 0x0000FFFF;
	p[32] = pt17 >> 16;
	p[33] = pt18 & 0x0000FFFF;
	p[34] = pt18 >> 16;
	//p[35] = pt19;/**/
	

#else
	p[0] = p22[0]  + p11[0] - p21[0] - p12[0];
	p[1] = p22[1]  + p11[1] - p21[1] - p12[1];
	p[2] = p22[2]  + p11[2] - p21[2] - p12[2];
	p[3] = p22[3]  + p11[3] - p21[3] - p12[3];
	p[4] = p22[4]  + p11[4] - p21[4] - p12[4];
	p[5] = p22[5]  + p11[5] - p21[5] - p12[5];
	p[6] = p22[6]  + p11[6] - p21[6] - p12[6];
	p[7] = p22[7]  + p11[7] - p21[7] - p12[7];
	p[8] = p22[8]  + p11[8] - p21[8] - p12[8];
	 p[9] = p32[0] + p21[0] - p31[0] - p22[0];
	p[10] = p32[1] + p21[1] - p31[1] - p22[1];
	p[11] = p32[2] + p21[2] - p31[2] - p22[2];
	p[12] = p32[3] + p21[3] - p31[3] - p22[3];
	p[13] = p32[4] + p21[4] - p31[4] - p22[4];
	p[14] = p32[5] + p21[5] - p31[5] - p22[5];
	p[15] = p32[6] + p21[6] - p31[6] - p22[6];
	p[16] = p32[7] + p21[7] - p31[7] - p22[7];
	p[17] = p32[8] + p21[8] - p31[8] - p22[8];
	p[18] = p23[0] + p12[0] - p22[0] - p13[0];
	p[19] = p23[1] + p12[1] - p22[1] - p13[1];
	p[20] = p23[2] + p12[2] - p22[2] - p13[2];
	p[21] = p23[3] + p12[3] - p22[3] - p13[3];
	p[22] = p23[4] + p12[4] - p22[4] - p13[4];
	p[23] = p23[5] + p12[5] - p22[5] - p13[5];
	p[24] = p23[6] + p12[6] - p22[6] - p13[6];
	p[25] = p23[7] + p12[7] - p22[7] - p13[7];
	p[26] = p23[8] + p12[8] - p22[8] - p13[8];
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
	for (int i = 0; i < 36; i += 6)
	{
		t0 = p[i];
		t1 = p[i + 1];
		t2 = p[i + 2];
		t3 = p[i + 3];
		t4 = p[i + 4];
		t5 = p[i + 5];
		sum += t0 * t0 + t1 * t1 + t2 * t2 + t3 * t3 + t4 * t4 + t5 * t5;
	}
	DspNormalizeHist(p,sum);
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
void IICacheHelper::ClearIntegralImages()
{
	int len = iiWidth * iiHeight;
	len *= nBinStride;
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
				FillEnergyImage(x,y,gi.winSize.x,gi.winSize.y,(double)s);
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
