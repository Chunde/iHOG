
#pragma once
	//////////////////////////////////////////////////////////////////////////
	//Class HOGHelper is used for auxiliary functions
#include "stdio.h"
#include "string.h"
#define _YUV_
#define _SHRINK_ROI_
//#define _CACHE_SCALE_IMAGE_INDEX_
#define  _IIHOG_
#define _EXE_OPT_
#define _COMPACT_DATA_
#ifdef _COMPACT_DATA_
#define _USE_INT_
#define _USE_SHORT_
#endif
#ifdef _IIHOG_
#ifdef _EXE_OPT_
#define _DSP_OPT_
#endif
#endif
#ifdef _DSP_OPT_
#define _SPEEDUP_TABLE_
#define _SPEEDUP_FLAG_
#endif
#ifdef _USE_INT_
#ifdef _USE_SHORT_//short should not be the value type of gradient magnitude 
#define GT unsigned short
#else
#define GT int
#endif
#define _FIX_POINT_SVM_
#else
#define GT float
#endif
#define _DISCARD_NORM_
#define _ALIGN_BIN_
class HGDescriptor;
struct ImageInfo
{
	unsigned char* dataPtr;
	int colorChannel;
	int width;
	int height;
	int widthStep;
	ImageInfo()
	{
		memset(this,0,sizeof(ImageInfo));
	}
};
 struct Vector2D
 {
	 int x;
	 int y;
	 Vector2D()
	 {
		 x = 0;
		 y = 0;
	 }
	 Vector2D(int _x,int _y)
	 {
		 x = _x;
		 y = _y;
	 }
	 int Area()
	 {
		 return x * y;
	 }
	 Vector2D operator *(float s)
	 {
		 int _x = x * s;
		 int _y = y * s;
		 return Vector2D(_x,_y);
	 }
	 Vector2D operator *= (float s)
	 {
		 return *this * s;
	 }
	 Vector2D operator +(Vector2D v)
	 {
		int _x =  x + v.x;
		int _y =  y + v.y;
		 return Vector2D(_x,_y);
	 }
	 Vector2D operator /(float s)
	 {
		 int _x = x/ s;
		 int _y = y/ s;
		 return Vector2D(_x,_y);
	 }
	 Vector2D operator /(Vector2D v)
	 {
		 int _x = x / v.x;
		 int _y = y / v.y;
		 return Vector2D(_x,_y);
	 }
 };
 struct RegionList 
 {
	 Vector2D location;
	 Vector2D size;
	 RegionList* next;
	 RegionList()
	 {
		 next = 0;
	 }
	 void Release()
	 {
		 ReleaseVector2DList(this);
	 }
	 static void ReleaseVector2DList(RegionList* list)
	 {
		 RegionList* pNext;
		 while(list)
		 {
			 pNext = list->next;
			 delete list;
			 list = pNext;
		 }
	 }
	 void AppendItemToVector2D(Vector2D pt,Vector2D _s = Vector2D())
	 {
		 RegionList* item = new RegionList();
		 item->location = pt;
		 item->size = _s;
		 RegionList* p = this;
		 while(p->next)
		 {
			 p = p->next;
		 }
		 p->next = item;
	 }
	 static void AppendItemToVector2DList(RegionList*& list,RegionList* item)
	 {
		 if(list == NULL)
		 {
			 list = item;
			 return;
		 }
		 RegionList* p = list;
		 while(p->next)
		 {
			 p = p->next;
		 }
		 p->next = item;

	 }
 };
 struct PixLutData
 {

	 float weight;
	 float histWeight[4];

	 int offset;
	 int histOffset[4];
	 PixLutData()
	 {
		 memset(this,0,sizeof(PixLutData));
	 }
 };
 struct GeometryInfo
 {
	 Vector2D winSize;
	 Vector2D blockSize;
	 Vector2D winStride;
	 Vector2D blockStride;
	 Vector2D cellSize;
	 int histCacheOffset;
	 Vector2D cacheStride;
	 Vector2D cacheGrid;
 };
 class IICacheHelper
 {
private:
	 HGDescriptor* m_pHogDescriptor;
	 Vector2D cacheMaxImageSize;
	 int iiWidth;
	 int iiHeight;
	 int nBins;
	 int nBinStride;
	 double* pEnergyImage;
	 GT* ppIICacheData;
	 float* pFullHisCacheData;
	 Vector2D histCacheSize;
	 int scaleCount;
	 int blockHistSize;
	 Vector2D cellGrid;
	 GeometryInfo geometryRefTable[64];
	 GeometryInfo geometryData[64];
 private:
	 inline GT GetIIValue(int index,int x,int y);
	 inline GT GetIIRectValue(int binI,Vector2D tlPt,Vector2D brPt);
	 inline void GetRectHist(float* buffer,Vector2D tlPt,Vector2D brPt);
 protected:
	 void GenerateIntegralImages();
	 void DspIntegralImages();
	 void ClearIntegralImages();
	 void Release();
	 void GenerateGeometryInfo();
 	 void Detect(int scale,RegionList*& pList);
	 float EvaluateWindowSignal(int x,int y,GeometryInfo& gi);
	 void GetBlockHist(int x,int y,GeometryInfo& gi,float* buffer);
	 void NormalizeHist(float* buffer);
	 void CacheFullHist();
	 void CacheSpecialHist(GeometryInfo& gi);
	 const float* GetBlockHist(GeometryInfo& gi,int x,int y);
#ifdef _DSP_OPT_
	 void DspNormalizeHist(int* buffer, float scale);
	 void GetDspBlockHist(int x,int y,GeometryInfo& gi, float* buffer);
#endif
#ifdef _SPEEDUP_TABLE_
	 Vector2D gridSize;
	 int* pSpeedUpGrid;
	 int* pStaticGrid;
#ifdef _SPEEDUP_FLAG_
	 bool bSpeedUpFlag;
	 int speedUpWidth;
	 int speedUpHeight;
#endif
 public:
	 inline bool IsValidStaticCell(int x,int y);
	 inline bool IsValidGridCell(int x,int y,int w,int h);
	 inline bool IsValidGridCell(int x,int y,GeometryInfo& gi);
	 inline bool IsValidGridCell(int x,int y);
	 void GenerateValidGrid(int w,int h);
	 void ResetGrid();
	 inline void HitGridCell(int x,int y,GeometryInfo& gi);
	 void HitGridCell(int x,int y,int w,int h);
	 inline void OverlayStaticGrid();
	 void LoadStaticGrid();
	 void DimGridStrenght();
	 int* GetGridImageData(){return pSpeedUpGrid;}
	 //void ResetGrid();
#endif
	 int FindBestGI(int x,int y);
	 int FindBestControlGI(int x,int y);
public:
	 IICacheHelper();
	 virtual ~IICacheHelper();
	 void Configure(HGDescriptor* _hog);
	 void CacheImage(ImageInfo* imagePtr);
	 void CacheImage(ImageInfo* imagePtr,GeometryInfo& gi);

	 void Detect(ImageInfo* imagePtr,RegionList*& pList);
	 void Compute(ImageInfo* imagePtr,float* descriptor);
	 void Compute2(ImageInfo* imagePtr,float* descriptor);
 };
	//////////////////////////////////////////////////////////////////////////
 class HGDescriptor
	{
		//Insert structure for cache utility
		friend class CacheHelper;
		friend class IICacheHelper;
	public:
		
		//Basic class members
		Vector2D maxImageSize;
		Vector2D windowSize;
		Vector2D windowStride;
		Vector2D blockSize;
		Vector2D blockStride;
		Vector2D cellSize;
		Vector2D cacheStride;

		unsigned char* binLookUpTable;
		GT* magnitudeLookUpTable;
		inline void LookUp(int dx,int dy,int& bin1,int& bin2,float& mag1,float& mag2);
		void GenerateLookUpTable();


		int binCount;
		double winSigma;

		bool bSpeedUpFlag;
		int speedWidth;
		int speedHeight;
		//Buffer members

	private:
		//cache members
		

#ifdef _IIHOG_
		IICacheHelper iiCacheHelper;
	public:
		void Compute2(ImageInfo* imagePtr,float* descriptor);
#endif
	private:
		float lookUpTable[256];
#ifdef _FIX_POINT_SVM_
		int* hyperPanelVector;
#else
		float* hyperPanelVector;
#endif
		float responseThreshold;
		GT* magnitudePtr;

		unsigned char* anglePtr;
	protected:
		bool CheckConfigure();

		void GenerateGradientWorkCache();
		inline int BorderInterpolate(int index,int scale);
		inline void CartToPolar(float x,float y, float& m,float& a);
		inline void CartToPolar(float* x, float* y, float* m,float* a,int len);
		bool Configure();
		
	public:
		HGDescriptor();
		~HGDescriptor();
		bool Configure(Vector2D _imageSize,Vector2D _windowSize,Vector2D _windowsStride,Vector2D _blockSize,Vector2D _blockStride,Vector2D _cellSize,int _binCount);
		int GetDescriptorSize();
		Vector2D WindowsInImage(int,int);
		double GetWinSigma();
		void ComputerGradient(ImageInfo* imagePtr);
		void Compute(ImageInfo* imagePtr,float* descriptor);
		
		void Detect(ImageInfo* imagePtr,RegionList*& pList);
		void DetectMultiScale(ImageInfo* image, RegionList*& pList);
		void LoadHyperPanelVector();
		void LoadHyperPanelVector(const char* file);
		float* GetHyperPanelVector();
		float GetResponseThreshold();
		void SetResponseThreshold(float value);
#ifdef _SPEEDUP_TABLE_
		int* GetGridImage(){return iiCacheHelper.GetGridImageData();}
		void ResetGrid(){iiCacheHelper.ResetGrid();}
#endif
	};
