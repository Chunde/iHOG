#include "stdafx.h"


#include "math.h"
#include "Hog.h"


#define PI 3.1415926535897932384626433832795

//inline int round(double v)
//{
//	return (int)(v + 0.5);
//}
inline static unsigned char GetImagePixel(unsigned char* ptr,int w,int h,int x,int y)
{
	return ptr[y * w + x];
}
inline static unsigned char GetInterpolateImagePixel(unsigned char* ptr, int w, int h,double x,double y)
{
	return GetImagePixel(ptr,w,h,round(x),round(y));
}

inline bool IsSimilarRegion(RegionList* r1,RegionList* r2)
{
	static float eps = 0.1f;
	
	int delta = 0;//eps * (min(r1->size.x,r2->size.x)) + (min(r1->size.y,r2->size.y));
	delta += min(r1->size.x,r2->size.x);
	delta += min(r1->size.y,r2->size.y);
	delta /= 10;
	delta +=2;

	return ((abs(r1->location.x - r2->location.x) < delta) && (abs(r1->location.y - r2->location.y) < delta) && (abs(r1->size.x - r2->size.x)< delta ) && (abs(r1->size.y - r2->size.y) < delta));

}
inline bool IsReg1InReg2(RegionList* r1,RegionList* r2)
{
	if(r1->location.x < r2->location.x)
		return false;
	if(r1->location.y < r2->location.y)
		return false;
	if(r1->location.x + r1->size.x > r2->location.x + r2->size.x)
		return false;
	if(r1->location.y + r1->size.y > r2->location.y + r2->size.y)
		return false;
	return true;
}
int LabeRegion(RegionList* list,int* label,int n)
{
	int i, j, N = n;
	RegionList* p1 = list;
	RegionList* p2 = list;

	const int PARENT=0;
	const int RANK=1;

	int* _nodes = new int[2 * n];
	int (*nodes)[2] = (int(*)[2])&_nodes[0];

	// The first O(N) pass: create N single-vertex trees
	for(i = 0; i < N; i++)
	{
		nodes[i][PARENT]=-1;
		nodes[i][RANK] = 0;
	}

	// The main O(N^2) pass: merge connected components
	for( i = 0; i < N; i++ )
	{
		int root = i;

		// find root
		while( nodes[root][PARENT] >= 0 )
			root = nodes[root][PARENT];
		p2 = list;
		for( j = 0; j < N; j++ )
		{
			if( i == j || !IsSimilarRegion(p1,p2))
			{
				p2 = p2->next;
				continue;
			}
			int root2 = j;

			while( nodes[root2][PARENT] >= 0 )
				root2 = nodes[root2][PARENT];

			if( root2 != root )
			{
				// unite both trees
				int rank = nodes[root][RANK], rank2 = nodes[root2][RANK];
				if( rank > rank2 )
					nodes[root2][PARENT] = root;
				else
				{
					nodes[root][PARENT] = root2;
					nodes[root2][RANK] += rank == rank2;
					root = root2;
				}
				//assert( nodes[root][PARENT] < 0 );

				int k = j, parent;

				// compress the path from node2 to root
				while( (parent = nodes[k][PARENT]) >= 0 )
				{
					nodes[k][PARENT] = root;
					k = parent;
				}

				// compress the path from node to root
				k = i;
				while( (parent = nodes[k][PARENT]) >= 0 )
				{
					nodes[k][PARENT] = root;
					k = parent;
				}
			}
			p2 = p2->next;
		}
		p1 = p1->next;
	}
	int nclasses = 0;

	for( i = 0; i < N; i++ )
	{
		int root = i;
		while( nodes[root][PARENT] >= 0 )
			root = nodes[root][PARENT];
		// re-use the rank as the class label
		if( nodes[root][RANK] >= 0 )
			nodes[root][RANK] = ~nclasses++;
		label[i] = ~nodes[root][RANK];
	}
	delete[] _nodes;
	return nclasses;

}
void GroupRectangles(RegionList*& list)
{
	if(list == 0)
		return;
	int n = 0;
	RegionList* p = list;
	while(p)
	{
		n ++;
		p = p->next;
	}
	int* label  = new int[n];
	int m = LabeRegion(list,label,n);
	RegionList* regs = new RegionList[m];
	int* weight = new int[m];
	for(int i = 0; i < m ; i ++)
		weight[i] = 0;
	p = list;
	int id = 0;
	for(int i = 0; i < n; i ++)
	{
		id = label[i];
		regs[id].location.x += p->location.x;
		regs[id].location.y += p->location.y;
		regs[id].size.x += p->size.x;
		regs[id].size.y += p->size.y;
		weight[id] ++;
		p = p->next;
	}
	p = list;
	for(int i = 0; i < m ; i ++)
	{
		regs[i].location.x /= weight[i];
		regs[i].location.y /= weight[i];
		regs[i].size.x /= weight[i];
		regs[i].size.y /=weight[i];
		regs[i].next =p->next;
		*p = regs[i];
		if(i < m - 1)
			p = p->next;
		else
		{
			p->next->Release();
			p->next = 0;
		}
	}
	p = list;
	RegionList* p1 = list;
	for(int i = 0; i < m; i ++)
	{
		p1 = list;
		weight[i] = 1;
		for(int j = 0; j < m ; j ++)
		{
			if(i != j)
			{
				if(IsReg1InReg2(p,p1))
				{
					weight[i] = 0;
					break;
				}
			}
			p1 = p1->next;
		}
		p = p->next;
	}
	p = list;
	p1 = list;
	for(int i = 0; i < m ; i ++)
	{
		if(weight[i] == 0)
		{
			if(p == list)
			{
				list = p->next;
				p->next = 0;
				p->Release();
				p = list;
				p1 = p;
			}
			else
			{
				p1->next = p->next;
				p->next = 0;
				p->Release();
				p = p1->next;
			}
			continue;;
		}
		p1 = p;
		p = p->next;
	}
	p = list;
	
	delete[]regs;
	delete[] weight;
}
inline int AlignSize(int sz,int n)
{
	for(int i = 0; i < n; i ++)
	{
		if(sz % n == 0)
			break;
		sz ++;
	}
	return sz;
}
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
	//////////////////////////////////////////////////////////////////////////
	
	void HGDescriptor::GenerateGradientWorkCache()
	{
		
		if(magnitudePtr)
		{
			delete[] magnitudePtr;
			magnitudePtr = 0;
		}
		if(anglePtr)
		{
			delete[] anglePtr;
			anglePtr = 0;

		}
		int len = maxImageSize.x  * maxImageSize.y  * 2;
		magnitudePtr = new GT[len];

		anglePtr = new unsigned char[len];
	}
	
	void HGDescriptor::CartToPolar(float x,float y, float& m,float& a)
	{
		static float PI2 = 6.283185307179586476925286766559;
		static double scale = 0.01745329251994329576923690768489;

		m = (x * x + y * y);
		m = sqrt(m);

		a = atan2l((long double)y,(long double)x);
		if(a < 0)
			a += PI2;
	}
	void HGDescriptor::CartToPolar(float* x, float* y, float* m,float* a,int len)
	{
		for(int i = 0; i < len; i ++)
		{
			CartToPolar(x[i],y[i],m[i],a[i]);
		}
	}
	HGDescriptor::HGDescriptor()
	{
		binLookUpTable = 0;
		magnitudeLookUpTable = 0;
		magnitudePtr = 0;
		anglePtr = 0;
		hyperPanelVector = 0;
#ifdef _FIX_POINT_SVM_
		floatHyperPanelVector = 0;
#endif
		responseThreshold = 1.0f;
	}
	HGDescriptor::~HGDescriptor()
	{
		if(hyperPanelVector)
		{
			delete[] hyperPanelVector;
			hyperPanelVector = 0;
		}
		
		if(magnitudePtr)
		{
			delete[] magnitudePtr;
			magnitudePtr = 0;
		}
		if(anglePtr)
		{
			delete[] anglePtr;
			anglePtr = 0;
		}
		if(binLookUpTable)
		{
			delete[] binLookUpTable;
			binLookUpTable = 0;
		}
		if(magnitudeLookUpTable)
		{
			delete[] magnitudeLookUpTable;
			magnitudeLookUpTable = 0;
		}

	}
	void HGDescriptor::GenerateLookUpTable()
	{
		binLookUpTable = new unsigned char[511 * 512 * 2];
		magnitudeLookUpTable = new GT[511 * 512 * 2];
		int index = 0;
		float Mag,Ang;
		int hidx;
		GT t0;
		float angleScale = (float)(binCount / PI);
		for (int y = -255; y < 256; y ++)
		{
			for(int x = -255; x < 256; x ++)
			{
				CartToPolar(x,y,Mag,Ang);

				Ang *= angleScale;
				Ang -= 0.5f;
				hidx = floor(Ang);
				Ang -= hidx;
				if(hidx < 0)
					hidx += binCount;
				else if(hidx >= binCount)
					hidx -= binCount;

				index = (((y + 255) << 9) | (x + 255)) << 1;
				binLookUpTable[index] = hidx;
				t0 = (GT)Mag * Ang;
				magnitudeLookUpTable[index] = 1;//(GT)(Mag - t0);

				hidx ++;
				if(hidx >= binCount)
					hidx = 0;
				binLookUpTable[index + 1] = hidx;
				magnitudeLookUpTable[index + 1] = 1;//(GT)t0;
			}
		}
	}
	void HGDescriptor::LookUp(int dx,int dy,int& bin1,int& bin2,float& mag1,float& mag2)
	{
		int index = (((dy + 255) << 9) | (dx + 255)) << 1;
		bin1 = binLookUpTable[index];
		bin2 = binLookUpTable[index + 1];
		mag1 = magnitudeLookUpTable[index];
		mag2 = magnitudeLookUpTable[index + 1];

	}
	bool HGDescriptor::Configure(Vector2D _imageSize,Vector2D _windowSize,Vector2D _windowsStride,Vector2D _blockSize,Vector2D _blockStride,Vector2D _cellSize,int _binCount)
	{
		maxImageSize = _imageSize;
		windowSize = _windowSize;
		windowStride = _windowsStride;
		blockSize = _blockSize;
		blockStride = _blockStride;
		cellSize = _cellSize;
		binCount = _binCount;

		cacheStride.x = GreatCommonDivisor(windowStride.x,blockStride.x);
		cacheStride.y = GreatCommonDivisor(windowStride.y,blockStride.y);
		GenerateGradientWorkCache();

		GenerateLookUpTable();



		iiCacheHelper.Configure(this);

		return true;
	}
	bool HGDescriptor::Configure()
	{
		return false;
	}
	int HGDescriptor::GetDescriptorSize()
	{

		int size = (blockSize.x / cellSize.x) * (blockSize.y / cellSize.y) * ((windowSize.x - blockSize.x)/ blockStride.x + 1) * ((windowSize.y - blockSize.y) / blockStride.y + 1) * binCount;

		return size;
	}
	Vector2D HGDescriptor::WindowsInImage(int w, int h)
	{
		return Vector2D((w  - windowSize.x) / windowStride.x + 1,(h  - windowSize.y) / windowStride.y + 1);
	}
	double HGDescriptor::GetWinSigma()
	{
		return winSigma >= 0 ? winSigma : (blockSize.x + blockSize.y)/8.;
	}
	void HGDescriptor::ComputerGradient(ImageInfo* imagePtr)
	{

		int _paddedImageWidth = imagePtr->width ;
		int _paddedImageHeight = imagePtr->height;
		int x2,x3,y1,y2;

		int cn = imagePtr->colorChannel;
		int step = imagePtr->widthStep;
		int index = 0;
		int x1 = 0;
		
		float angleScale = (float)(binCount / PI);
		int hidx;
		float t0;

		short* binPtr = (short*)anglePtr;
#ifdef _USE_SHORT_
		int* magPtr = (int*)magnitudePtr;
#else
		double* magPtr = (double*)magnitudePtr;
#endif
		int dX,dY;
		if(cn == 1)
		{
			unsigned char* prevPtr = 0;
			unsigned char* nextPtr = 0;
			unsigned char* currPtr = 0;
			for (int y = 0; y < _paddedImageHeight;y ++)
			{
				y1 = y - 1;
				y2 = y + 1; 
				y1 = y1 - y1 * ( y1 < 0);
				y2 = y2 - ((y2 >= _paddedImageHeight) * 2);
				currPtr = imagePtr->dataPtr + step * y;
				prevPtr = imagePtr->dataPtr + step * y1;
				nextPtr = imagePtr->dataPtr + step * y2;
				for(int x = 0; x < _paddedImageWidth; x ++)
				{
					x1 = x;
					x2 = x + 1; 
					x2 = x + 1; 
					x3 = x - 1; 
					x2 =x2 - ((x2 >= _paddedImageWidth) * 2);// x2 * (x2 < _paddedImageWidth) + (_paddedImageWidth - 2) * (x2 >= _paddedImageWidth);// 
					x3 = x3 - x3 * (x3 < 0);
					dX = currPtr[x2] - currPtr[x3];
					dY = nextPtr[x1] - prevPtr[x1];
					index = (((dY + 255) << 9) | (dX + 255)) << 1;
					*binPtr = *((short*)(binLookUpTable + index));
#ifdef _USE_SHORT_
					*magPtr = *((int*)(magnitudeLookUpTable + index));
#else
					*magPtr = *((double*)(magnitudeLookUpTable + index));
#endif
					binPtr++;
					magPtr++;
				}
			}
		}
		else
		{
			unsigned char* dataPtr1 = imagePtr->dataPtr;
			unsigned char* dataPtr2 = dataPtr1 + imagePtr->width * imagePtr->height;
			unsigned char* dataPtr3 = dataPtr2 + imagePtr->width * imagePtr->height;

			unsigned char* currPtr1,*currPtr2,*currPtr3;
			unsigned char* prevPtr1,*prevPtr2,*prevPtr3;
			unsigned char* nextPtr1,*nextPtr2,*nextPtr3;

			int index = 0;
			int x1 = 0;

			float dX,dY,Mag,Ang;
			float angleScale = (float)(binCount / PI);
			int hidx;
			float t0;
			for (int y = 0; y < _paddedImageHeight;y ++)
			{
				y1 = y - 1;
				y2 = y + 1;
				y1 = y1 - y1 * (y1 < 0);
				y2 = y2 * (y2 < _paddedImageHeight) - (_paddedImageHeight - 2) * (y2 >= _paddedImageHeight);

				currPtr1 = dataPtr1 + step * y;
				prevPtr1 = dataPtr1 + step * y1;
				nextPtr1 = dataPtr1 + step * y2;

				currPtr2 = dataPtr2 + step * y;
				prevPtr2 = dataPtr2 + step * y1;
				nextPtr2 = dataPtr2 + step * y2;

				currPtr3 = dataPtr3 + step * y;
				prevPtr3 = dataPtr3 + step * y1;
				nextPtr3 = dataPtr3 + step * y2;


				int dx0,dy0,dx,dy,mg0,mg;

				for (int x = 0; x < _paddedImageWidth; x ++)
				{

					x1 = x ;
					x2 = x + 1;
					x3 = x - 1;
					if(x2 >= _paddedImageWidth)
						x2 = _paddedImageWidth - 2;
					if(x3 < 0)
						x3 = 1;
					unsigned char* p2 = currPtr3 + x2;
					unsigned char* p0 = currPtr3 + x3;

					dx0 = *p2 - *p0;
					dy0 = nextPtr3[x1] - prevPtr3[x1];
					mg0 = dx0 * dx0 + dy0 * dy0;

					p2 = currPtr2 + x2;
					p0 = currPtr2 + x3;

					dx = *p2 - *p0;
					dy = nextPtr2[x1] - prevPtr2[x1];
					mg = dx * dx + dy * dy;

					if(mg0 < mg)
					{
						dx0 = dx;
						dy0 = dy;
						mg0 = mg;
					}

					p2 = currPtr1 + x2;
					p0 = currPtr1 + x3;

					dx = *p2 - *p0;
					dy = nextPtr1[x1] - prevPtr1[x1];
					mg = dx* dx + dy * dy;
					if(mg0 < mg)
					{
						dx0 = dx;
						dy0 = dy;
					}

					dX = dx0;
					dY = dy0;
					int b1,b2;
					float m1,m2;
					LookUp(dX,dY,b1,b2,m1,m2);
					anglePtr[index] = b1;
					anglePtr[index + 1] = b2;
					magnitudePtr[index] = m1;
					magnitudePtr[index + 1] = m2;

					index += 2;

				}

			}
		}
	}
#ifdef _IIHOG_
	void HGDescriptor::Compute2(ImageInfo* imagePtr,float* descriptor)
	{
		iiCacheHelper.Compute2(imagePtr,descriptor);
	}
#endif
	void HGDescriptor::Compute(ImageInfo* imagePtr,float* descriptor)
	{

		iiCacheHelper.Compute(imagePtr,descriptor);

	}
	
	void HGDescriptor::DetectMultiScale(ImageInfo* image, RegionList*& pList)
	{

		iiCacheHelper.Detect(image,pList);

		GroupRectangles(pList);

	
	}
	void HGDescriptor::LoadHyperPanelVector()
	{

		static float svm[] = {-6.90770000e-002,2.72960997e+000,3.29139686e+000,2.78926802e+000,-5.02711892e-001,-3.70592546e+000,4.91470528e+000,3.15386200e+000,-1.21820319e+000,1.00845253e+000,2.71281981e+000,2.55312967e+000,-1.01351988e+000,-1.92605197e+000,-7.74893224e-001,5.29111445e-001,2.79296428e-001,1.05728936e+000,-9.18800354e-001,4.00303936e+000,-1.48758674e+000,1.27025938e+000,3.50532675e+000,-1.71634281e+000,-1.14530540e+000,-9.18771863e-001,-1.33816040e+000,1.45169997e+000,2.83097243e+000,-2.61314797e+000,5.71471751e-002,1.49020493e+000,-9.97171044e-001,5.10523975e-001,-7.50289857e-001,-1.51558018e+000,2.36912489e+000,1.82026935e+000,-3.68926048e-001,1.11453198e-001,-1.09219122e+000,-6.92850947e-002,2.10780859e+000,8.30666482e-001,6.74020231e-001,-2.26749152e-001,2.15377808e+000,-2.13160366e-001,-2.26781058e+000,-6.92305565e-001,2.69455791e+000,-2.15423179e+000,-7.00852334e-001,-1.59273222e-001,1.04674006e+000,1.50357664e+000,-2.53275931e-001,9.42674220e-001,3.85064769e+000,-1.00159132e+000,8.80300105e-001,2.54457116e+000,1.36475205e+000,-1.72032738e+000,-6.11245632e-001,-7.47283220e-001,-8.95396650e-001,-5.66668093e-001,-2.23271894e+000,-1.73626602e+000,-4.94420290e-001,-1.36074162e+000,-4.76675957e-001,3.60845983e-001,-1.28553939e+000,2.60142803e-001,8.81694928e-002,-9.41443861e-001,1.76196826e+000,3.82271552e+000,9.73633766e-001,8.35428536e-002,7.74187148e-001,-2.25325537e+000,-9.03786480e-001,4.40764964e-001,1.23807974e-001,-7.10349798e-001,1.14105010e+000,8.85064006e-001,-1.47329593e+000,1.69192657e-001,-9.49391842e-001,1.12212107e-001,2.86448932e+000,7.26256371e-001,2.20490503e+000,-1.62948832e-001,1.93198085e-001,2.18344283e+000,-3.47589217e-002,1.81610703e+000,-5.79074085e-001,-3.06936145e+000,-4.64977801e-001,-1.85527205e+000,-2.91030973e-001,-2.09351277e+000,9.09424722e-001,1.90511036e+000,3.80135387e-001,-7.57979035e-001,-3.62559631e-002,4.01703835e+000,2.88809776e+000,2.48955655e+000,-7.10350797e-002,-6.27234697e-001,-8.60535860e-001,-5.04626465e+000,-3.53321743e+000,-2.46779978e-001,2.29269242e+000,1.15963066e+000,2.88355613e+000,-1.74404562e+000,8.42626572e-001,7.12192893e-001,-4.47323740e-001,1.35706985e+000,4.16019297e+000,8.24223906e-002,-3.98173034e-001,1.60516047e+000,9.29945469e-001,-1.26297069e+000,1.79586017e+000,-1.56653953e+000,-1.83861959e+000,-3.72828403e-003,-1.33821905e+000,-1.73799396e-001,-2.69798636e+000,-2.67930770e+000,-2.27727032e+000,-9.82532620e-001,1.79726630e-001,1.86060178e+000,-5.22085369e-001,2.76580036e-001,7.49897882e-002,-6.70614421e-001,-1.31464398e+000,1.13006607e-001,-5.93749106e-001,-2.41408730e+000,1.80214751e+000,-4.68993068e-001,-1.09429348e+000,-1.87309861e+000,-2.13024282e+000,-2.14719319e+000,-1.00737453e-001,-2.64461637e-001,4.32941645e-001,4.14367914e-001,-1.40110636e+000,3.50663155e-001,-8.53755116e-001,-1.95017624e+000,2.94456363e-001,-8.35283160e-001,9.54708681e-002,-4.87992018e-001,1.26403785e+000,-2.09013224e+000,6.60299838e-001,9.70639288e-002,-5.24137080e-001,9.86690283e-001,-1.99722910e+000,-2.07520843e+000,-1.14329398e-001,-2.27893189e-001,-3.29202461e+000,4.77790952e-001,-5.81913173e-001,-1.24332212e-001,-8.72378230e-001,1.89402223e+000,-3.36627722e-001,1.27986765e+000,3.96872312e-001,1.54089272e-001,-2.20080633e-002,-8.65377903e-001,-1.21710110e+000,-1.62723994e+000,-2.49448046e-001,1.38813818e+000,2.04018784e+000,-1.44311047e+000,-2.87761354e+000,2.46233921e-002,-2.65399766e+000,-8.67212415e-001,-4.46869105e-001,2.53012371e+000,1.45468426e+000,8.01360011e-001,-1.08419919e+000,1.16917026e+000,9.98364270e-001,1.34919894e+000,1.54207885e+000,1.95023477e+000,1.82672811e+000,4.70675141e-001,8.27058733e-001,1.17025650e+000,-7.67962754e-001,8.16979051e-001,2.09717989e+000,1.45903671e+000,1.19123137e+000,-3.03490102e-001,1.23280954e+000,1.45063007e+000,1.71021104e+000,2.69538879e+000,1.80147827e+000,-3.15751046e-001,7.45034695e-001,-8.19271058e-002,-2.33272505e+000,-2.21031234e-001,-6.00152671e-001,-1.88940930e+000,-1.54161000e+000,-3.27810675e-001,-5.74634671e-001,-1.84725475e+000,-1.92559791e+000,-2.23816007e-001,-4.06296968e-001,6.59534097e-001,9.67644155e-001,6.03558362e-001,2.62158966e+000,2.52658874e-001,1.03598142e+000,3.73971075e-001,-2.47286701e+000,-1.76853657e+000,-2.61971140e+000,-5.16367853e-001,-6.19115233e-001,3.88118535e-001,4.79369372e-001,1.67230690e+000,8.97012711e-001,-1.45793295e+000,1.56000841e+000,-2.34879661e+000,8.19368124e-001,2.58581400e+000,9.68571544e-001,9.25686061e-001,-1.12440455e+000,-2.54053402e+000,7.53297806e-001,2.10263848e-001,-6.05620265e-001,-9.11881864e-001,1.60447323e+000,-1.58475149e+000,2.95124948e-002,-1.03026605e+000,9.60028827e-001,7.72315919e-001,-7.09001839e-001,-4.20291990e-001,2.73359632e+000,1.62798512e+000,-3.39422226e+000,-4.71320808e-001,-6.82091773e-001,3.44718099e-001,-1.47058964e+000,-1.31643057e+000,-3.95892429e+000,-1.04697061e+000,4.15965986e+000,2.35351801e+000,2.18551683e+000,2.07096398e-001,-1.91394186e+000,-2.06745267e+000,-2.10936093e+000,-4.20320797e+000,2.49470520e+000,4.99485350e+000,2.13437247e+000,2.81129026e+000,1.66383398e+000,-7.90766001e-001,-1.00109100e+000,2.14489055e+000,-6.17520094e+000,-2.23841023e+000,-2.03794813e+000,2.96872544e+000,1.63773000e+000,2.34320903e+000,1.10013092e+000,-8.73240769e-001,-1.27012324e+000,-2.61939144e+000,-4.12619442e-001,-1.80364466e+000,5.30933917e-001,1.19126868e+000,4.99177217e+000,2.98944294e-001,2.26446795e+000,1.07704669e-001,-1.26355302e+000,-2.74341971e-001,2.36116385e+000,2.86167175e-001,-1.61347818e+000,-1.47035569e-002,-1.29165351e+000,9.88643706e-001,4.27847415e-001,-1.69356430e+000,7.26233661e-001,6.56820297e-001,-3.34566188e+000,-9.59658206e-001,-1.68317354e+000,9.14758861e-001,2.53475022e+000,1.56561565e+000,-2.29823858e-001,-9.79166180e-002,2.85842657e+000,2.28481007e+000,-1.59151042e+000,1.16637433e+000,1.55807447e+000,1.51763737e+000,1.32067621e-001,2.38023445e-001,2.19162941e+000,-1.76122114e-001,7.39894331e-001,6.98973835e-002,3.74361515e+000,-5.31368554e-001,-1.51623559e+000,1.14085424e+000,2.55458027e-001,2.59136343e+000,1.55474031e+000,7.27520585e-001,4.94140051e-002,-9.95859802e-001,4.35948372e-001,-4.79117960e-001,-1.50999498e+000,-4.61667299e-001,1.06434859e-001,-1.82619178e+000,1.77771226e-001,-1.24598372e+000,-1.65219164e+000,-2.76685548e+000,1.82278025e+000,8.57105732e-001,-5.26326895e-001,2.87989855e-001,9.62473929e-001,-3.72042954e-001,3.31888825e-001,4.12517041e-001,1.42663121e+000,2.31111193e+000,1.02931452e+000,-3.04222703e+000,8.88136506e-001,9.72454131e-001,-6.01410389e-001,-1.83441865e+000,-5.66867173e-001,-6.57576025e-001,1.16079259e+000,8.76310647e-001,-2.79320747e-001,1.78868043e+000,1.80313730e+000,-9.61822569e-001,1.06445539e+000,-3.30030411e-001,7.04524100e-001,-1.83800268e+000,-1.30743766e+000,-1.15754509e+000,-7.58373588e-002,1.69228911e+000,8.35394621e-001,-5.07948995e-001,-7.67806172e-001,-3.69143224e+000,-7.08467185e-001,1.64312172e+000,3.08462590e-001,2.74384165e+000,-6.26280427e-001,-1.79416621e+000,-1.90451610e+000,2.27989808e-001,1.37762690e+000,2.51909226e-001,4.07284832e+000,-3.31755579e-001,1.89238393e+000,-6.58471406e-001,-1.11511636e+000,-4.35982418e+000,1.53984714e+000,-1.25403002e-001,-1.64391193e+001};

		if(hyperPanelVector)
		{
			delete[] hyperPanelVector;
			hyperPanelVector = 0;

		}
		int len = sizeof(svm) / sizeof(float);
#ifdef _FIX_POINT_SVM_
		hyperPanelVector = new int[len];
		for(int i = 0; i < len; i ++)
		{
			hyperPanelVector[i] = (int)(1024.f * svm[i]);
		}
#ifndef _DISCARD_NORM_
		hyperPanelVector[len - 1] <<= 10;
#endif
#else
		hyperPanelVector = new float[len];
		for(int i = 0; i < len; i ++)
		{
			hyperPanelVector[i] = svm[i];
		}

#endif
	}
	
#ifdef _FIX_POINT_SVM_
	float* HGDescriptor::GetHyperPanelVector()
	{
		
		return floatHyperPanelVector;
	}
#else
	float* HGDescriptor::GetHyperPanelVector()
	{

		return hyperPanelVector;

	}
#endif
	float HGDescriptor::GetResponseThreshold()
	{
		return responseThreshold;
	}
	void HGDescriptor::SetResponseThreshold(float _value)
	{
		responseThreshold = _value;
	}
	double* HGDescriptor::GetEnergyImage()
	{
		return iiCacheHelper.GetEnergyImage();
	}
	
