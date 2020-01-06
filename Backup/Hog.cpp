#include "stdafx.h"
#include "math.h"
#include "Hog.h"


#define PI 3.1415926535897932384626433832795


inline int round(double v)
{
	return (int)(v + 0.5);
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
	int HGDescriptor::BorderInterpolate(int index,int scale)
	{
		if(index < 0)
			return -(index );
		if(index >= scale)
			return scale - 2 - (index - scale  );
		return index;
	}
	void HGDescriptor::CartToPolar(float x,float y, float& m,float& a)
	{
		static float PI2 = 6.283185307179586476925286766559;
		static double scale = 0.01745329251994329576923690768489;
		static float maxM = 0;
		static float minM = 99999999.0f;
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
		responseThreshold = 2.1f;
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
#ifdef _USE_SHORT_
				magnitudeLookUpTable[index] = 1;
#else
				t0 = (GT)Mag * Ang;
				magnitudeLookUpTable[index] = (GT)(Mag - t0);
#endif

				hidx ++;
				if(hidx >= binCount)
					hidx = 0;
				binLookUpTable[index + 1] = hidx;
#ifdef _USE_SHORT_
				magnitudeLookUpTable[index + 1] = 1;
#else
				magnitudeLookUpTable[index + 1] = t0;
#endif
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
#ifdef _IIHOG_
		iiCacheHelper.Compute(imagePtr,descriptor);
#else
		if(descriptor == 0)
			return;
		cacheHelper.CacheImage(imagePtr);
		Vector2D nWindows = WindowsInImage(imagePtr->width,imagePtr->height);
		Vector2D nBlocks = cacheHelper.nBlocks;
		float s = 0;
		Vector2D pt0;
		Vector2D offset;
		float* hist = 0;
		int histLen = cacheHelper.blockHistSize;
		int index = 0;
		for(int y = 0; y < nWindows.y; y ++)//window sliding in vertical direction.
		{

			pt0.x = 0;
			for (int x = 0; x < nWindows.x; x ++)//window sliding in horizontal direction.
			{
				offset = pt0;
				for (int x0 = 0; x0 < nBlocks.x; x0++)//block sliding in horizontal direction.
				{
					offset.y = pt0.y;
					for(int y0 = 0; y0 < nBlocks.y; y0 ++)//block sliding in vertical direction.
					{

						hist = cacheHelper.GetBlockHist(offset);
						if(hist)
						{
							for(int k = 0; k < histLen; k ++)
							{
								descriptor[index ++] = hist[k];
							}
						}
						offset.y += blockStride.y;
					}//block in x
					offset.x += blockStride.x;
				}//block in y
				pt0.x += windowStride.x;
			}//window in x
			pt0.y += windowStride.y;

		}//window in y
#endif
	}

	void HGDescriptor::DetectMultiScale(ImageInfo* image, RegionList*& pList)
	{

		iiCacheHelper.Detect(image,pList);

		GroupRectangles(pList);
#ifdef _SPEEDUP_TABLE_
		if (pList)
		{
			RegionList* p = pList;
			while(p)
			{
				iiCacheHelper.HitGridCell(p->location.x,p->location.y,p->size.x,p->size.y);
				p = p->next;
			}
		}
		
#endif
		
	
	}
	void HGDescriptor::LoadHyperPanelVector()
	{
#ifdef _USE_SHORT_
		static float svm[] = {1.27028811e+000,-9.27009463e-001,2.04578376e+000,-7.13863552e-001,1.01406372e+000,-2.63375700e-001,6.17121041e-001,9.33552384e-001,5.27167499e-001,-7.56186783e-001,1.68680996e-001,-1.67001259e+000,1.96575058e+000,1.13346004e+000,6.49248362e-001,-5.50615847e-001,2.27409005e+000,-3.30919921e-001,1.08973110e+000,2.36165953e+000,1.51770723e+000,2.36531556e-001,-2.14057493e+000,-7.61341989e-001,-1.61152124e-001,3.60876799e-001,8.86197567e-001,7.31908560e-001,6.51807308e-001,9.17717695e-001,1.06744635e+000,-2.73409128e+000,-3.51149154e+000,-6.80111170e-001,1.96323121e+000,4.11174870e+000,3.38168573e+000,4.14611483e+000,1.21506321e+000,2.30029035e+000,-3.76242685e+000,3.15178782e-001,1.74190569e+000,4.60257679e-001,3.02371472e-001,1.59743893e+000,2.38502479e+000,4.59590167e-001,2.85903525e+000,-2.60310292e+000,1.04019463e+000,2.58771122e-001,8.26813579e-001,9.18106198e-001,1.78104782e+000,3.48724437e+000,2.06212148e-001,1.44313738e-001,-2.63629055e+000,2.21799803e+000,4.73360181e-001,1.06987119e+000,1.69986916e+000,1.77530456e+000,1.91281581e+000,3.23679328e-001,1.49383172e-001,3.46231163e-001,-2.32598275e-001,-3.59620035e-001,7.24455357e-001,1.18597484e+000,1.19383705e+000,4.65773296e+000,-1.01987553e+000,1.03933883e+000,-2.78357267e+000,3.96201348e+000,-5.75116873e-001,1.09102571e+000,2.75490195e-001,3.76042366e-001,3.48678684e+000,9.68987107e-001,-6.99365556e-001,-2.27019572e+000,3.60236764e+000,-1.75173080e+000,-3.53473127e-002,1.93627465e+000,-1.19564712e+000,-8.05723548e-001,-6.83569849e-001,-3.27825159e-001,1.19685459e+000,4.31667995e+000,1.61186600e+000,6.46596909e-001,1.45651758e+000,-1.30855536e+000,-4.68426138e-001,1.24069345e+000,2.76743984e+000,1.82056677e+000,1.27593577e+000,6.88361049e-001,-6.14201963e-001,-6.22156262e-001,6.74520195e-001,-2.76104540e-001,-3.35840154e+000,3.18029261e+000,-1.01595843e+000,4.74754959e-001,1.02334368e+000,1.35915184e+000,2.08556509e+000,-1.54734933e+000,5.84797204e-001,-2.05243635e+000,3.38113964e-001,-4.81282330e+000,4.07809496e+000,-4.03928488e-001,3.86297989e+000,2.98203158e+000,3.36473227e+000,-2.15058899e+000,5.17146349e-001,-3.73882860e-001,-4.16267604e-001,1.69892371e+000,2.70218468e+000,-2.21790481e+000,4.79613960e-001,-1.40550756e+000,4.07734513e-001,-1.07263982e+000,1.30383146e+000,5.02960026e-001,2.77657843e+000,1.85041499e+000,-4.19921041e-001,-1.57572579e+000,5.96672893e-001,-2.33625317e+000,-1.27397060e+000,1.69326818e+000,2.40969515e+000,1.23398554e+000,-2.81376648e+000,-2.55139661e+000,3.05528611e-001,-5.38030624e-001,-2.31766582e+000,-1.20226789e+000,7.17977166e-001,2.16222835e+000,5.10186493e-001,-1.57036150e+000,-1.43322098e+000,1.56942606e+000,2.38940582e-001,-5.10326207e-001,1.50156707e-001,5.71163356e-001,1.16104496e+000,7.86030173e-001,-1.41097784e+000,-1.39778864e+000,-1.55154252e+000,9.94565904e-001,-1.96800220e+000,-3.91800284e-001,-3.33942562e-001,2.11975917e-001,1.44292057e+000,4.26697284e-001,-2.37596989e+000,8.25626969e-001,-2.62538409e+000,-4.66292650e-001,-4.73175573e+000,-1.55079329e+000,1.40673745e+000,1.90181983e+000,-1.61176133e+000,-1.85836220e+000,-5.16739750e+000,-1.90257847e+000,1.89773023e+000,-5.80489492e+000,-7.50177026e-001,5.33882022e-001,-1.28239065e-001,-2.22615099e+000,-8.40809882e-001,-2.02627134e+000,-9.47121799e-001,-2.90477300e+000,-2.35774493e+000,-4.76715744e-001,2.96624184e-001,9.85139906e-001,1.01354110e+000,-4.30836391e+000,-2.33463240e+000,-1.45086384e+000,4.32251066e-001,-2.80927157e+000,-1.15620947e+000,8.32576275e-001,-8.54626358e-001,9.79691371e-002,-6.00380659e-001,-2.88071775e+000,-1.33797854e-001,-4.95621234e-001,-2.39746046e+000,3.99294090e+000,-1.10056436e+000,8.43606710e-001,-1.09554684e+000,-1.97931063e+000,1.54607344e+000,-1.14663267e+000,8.86151910e-001,5.60673177e-001,-3.54491830e-001,4.32363674e-002,1.22931562e-001,-1.04659414e+000,1.90055177e-001,8.59086141e-002,8.41601908e-001,-1.31038773e+000,-7.68979728e-001,6.85071945e-001,-2.84390062e-001,3.92046452e-001,3.68547261e-001,-9.11318839e-001,3.52740198e-001,-8.72865558e-001,7.27535009e-001,7.96433568e-001,6.78753912e-001,-1.06849718e+000,1.20298827e+000,-1.21373749e+000,-2.22377941e-001,-6.46896362e-001,1.71151423e+000,-3.40622950e+000,-1.93845952e+000,2.03938946e-001,2.85607785e-001,-1.16805784e-001,1.28079548e-001,-1.86183083e+000,9.20863390e-001,-1.87381372e-001,-1.12444699e+000,-4.16809511e+000,-4.13478613e-001,2.05402303e+000,1.83191752e+000,8.42231661e-002,-1.55662096e+000,5.34228444e-001,-1.15513086e+000,-1.06779885e+000,-3.21058488e+000,2.61981726e-001,-1.11011612e+000,5.30742943e-001,9.38909471e-001,-7.23500133e-001,2.74808383e+000,-1.28339335e-001,-1.36792338e+000,-3.50326180e-001,5.71043551e-001,2.11136952e-001,9.66657549e-002,-5.10051966e-001,-2.69769096e+000,1.61840093e+000,1.44452250e+000,-2.32185388e+000,-7.90466070e-001,-6.25484705e-001,1.80477512e+000,2.49408174e+000,3.37004113e+000,2.69675851e-001,2.32744932e+000,2.01596618e+000,2.60860348e+000,1.77531636e+000,-2.95017362e-001,8.24979320e-002,2.59889245e-001,-1.03572023e+000,-4.09532666e-001,9.15194333e-001,-1.35287797e+000,8.86883080e-001,1.74276841e+000,4.96620893e-001,-8.12569737e-001,2.30452418e+000,8.07655394e-001,1.33133674e+000,3.22195143e-001,-1.16594386e+000,9.61891651e-001,5.11445012e-003,2.70307279e+000,4.76359546e-001,1.08154833e+000,-3.07404757e-001,4.90304679e-001,-5.97369894e-002,3.30358118e-001,9.63006675e-001,-5.09424150e-001,9.34904397e-001,1.79842710e-001,-7.35807538e-001,6.11755192e-001,1.88220632e+000,2.47028017e+000,-4.84772265e-001,2.17353439e+000,5.58616482e-002,9.41235125e-001,-4.27415490e-001,-5.82963824e-001,-1.78407550e+000,2.61457777e+000,2.20278168e+000,-5.29506803e-001,3.18929482e+000,-4.42576855e-001,1.69435367e-001,-3.57498717e+000,8.12065125e-001,1.34598941e-001,1.93912923e+000,2.20961332e+000,-7.35824525e-001,2.54876637e+000,-1.68845809e+000,-8.34939361e-001,-9.21801746e-001,3.30378145e-001,2.29647562e-001,1.91515791e+000,2.44674659e+000,7.48403549e-001,1.02595925e+000,5.27309299e-001,2.65496516e+000,1.07654464e+000,2.28682184e+000,-7.18059838e-001,5.03772974e-001,8.02368343e-001,6.40483379e-001,9.54577208e-001,1.60094762e+000,1.99606168e+000,9.87626672e-001,4.51348662e-001,-1.88395619e+000,-4.52295095e-001,2.23333192e+000,2.02514067e-001,3.30518985e+000,-6.21575534e-001,3.73447269e-001,-4.98123288e-001,9.15019214e-001,-1.77381432e+000,2.95397568e+000,1.46172428e+000,1.04350436e+000,6.02897219e-002,3.60474795e-001,2.22282076e+000,-7.68367425e-002,4.79180813e-001,-9.42336977e-001,1.21790719e+000,3.53096753e-001,4.29160476e-001,1.15390837e+000,-2.13155079e+000,3.09154081e+000,1.56081128e+000,-8.27050626e-001,1.95835590e+000,-3.82160854e+000,7.20547259e-001,-5.72664961e-002,3.51615691e+000,-2.21300745e+000,2.33977652e+000,2.89278388e+000,1.69097912e+000,-1.48358691e+000,-1.82894278e+000,-3.25679088e+000,3.09803200e+000,1.58452177e+000,-1.49440527e+000,1.64087987e+000,-1.97636974e+000,7.73298025e-001,-1.74750280e+000,8.81299734e-001,-1.13639176e+000,3.63438082e+000,5.22131824e+000,7.86275625e-001,-1.64515936e+000,-2.00332046e+000,-4.60207367e+000,-1.27509737e+000,9.78478909e-001,3.38128321e-002,-2.34683533e+001};
#else
		static float svm[] = {-1.32990867e-001,-4.81424153e-001,2.38664597e-001,1.58313382e+000,-1.56896484e+000,-2.81735516e+000,-3.27468109e+000,1.85971707e-001,-1.54719031e+000,-4.37118649e-001,3.49778843e+000,-6.12284958e-001,-1.96825072e-001,-1.67866898e+000,9.69915152e-001,1.62841213e+000,1.84378624e+000,-6.66257262e-001,4.61227149e-001,1.42682970e+000,2.30233335e+000,-3.60811353e-001,1.07450175e+000,-1.46040678e+000,-4.87020433e-001,-1.69606626e+000,-2.88298786e-001,8.24983358e-001,1.26340818e+000,2.25071728e-001,-1.05269253e+000,2.76348329e+000,-3.97113413e-001,-1.10235560e+000,1.29062140e+000,1.18011725e+000,4.11147022e+000,-1.57738686e+000,-1.82573348e-001,7.87858009e-001,-4.46486950e-001,-4.44632620e-001,-2.01848388e+000,1.09817934e+000,5.28965145e-003,-1.36296928e+000,5.40025234e+000,2.11503291e+000,4.53199074e-002,-5.02866328e-001,-1.37541354e-001,3.01813960e-001,8.97261024e-001,-2.87994170e+000,-8.40569735e-001,1.47232974e+000,2.24485445e+000,2.25652719e+000,1.14765847e+000,7.69671142e-001,-3.81510794e-001,5.84256113e-001,1.18974507e+000,8.97642970e-001,1.31545079e+000,7.06390023e-001,-8.26479554e-001,-1.20152140e+000,1.86689508e+000,4.57617730e-001,1.96743524e+000,3.69171262e-001,3.10208511e+000,-2.20658398e+000,-2.24264216e+000,-2.16280723e+000,-2.54165912e+000,-6.11095548e-001,-2.92840768e-002,1.78560674e+000,1.67443967e+000,1.61710978e+000,2.26148629e+000,-6.52237296e-001,-1.76101136e+000,2.87457705e+000,1.19415116e+000,1.39871061e+000,1.60484803e+000,-3.13793421e-001,-7.61821270e-002,-7.94730932e-002,-3.81750911e-001,-2.20487404e+000,1.06942570e+000,2.41864443e+000,-3.06959867e-001,1.97920814e-001,-7.18419626e-002,-2.10871053e+000,-6.81426346e-001,-8.39803293e-002,8.48782957e-002,-2.01369238e+000,-1.51744679e-001,-4.09041852e-001,-1.65454403e-001,-1.66955471e+000,1.08654857e+000,-1.03514385e+000,-9.74100351e-001,-2.05148911e+000,-2.42839146e+000,-7.81897366e-001,-1.26394138e-001,-7.22970426e-001,1.52494395e+000,-2.34864309e-001,-4.51627731e+000,-1.16638839e+000,-3.33067393e+000,-2.62783384e+000,2.10529804e+000,2.81042516e-001,2.02017450e+000,1.73088455e+000,1.99397326e+000,2.71272016e+000,-1.63949579e-002,1.49492677e-002,2.00076628e+000,2.82437205e+000,4.08868462e-001,2.66530228e+000,1.09215510e+000,-1.10955507e-001,7.41684139e-001,2.04838485e-001,2.42360502e-001,1.95267391e+000,9.38198745e-001,1.32418442e+000,8.86911631e-001,1.88546085e+000,-8.82825613e-001,-1.88791156e+000,-1.33408225e+000,1.65799248e+000,1.96860409e+000,-1.08765268e+000,4.22973156e-001,-1.24400032e+000,-1.10887718e+000,7.00375259e-001,-2.49296233e-001,-1.58719265e+000,-3.45663881e+000,1.03615463e+000,-1.11825740e+000,-1.35624814e+000,1.09817362e+000,-4.69098449e-001,-3.55516911e-001,4.23003048e-001,8.95749211e-001,1.83049455e-001,1.59337568e+000,3.63254175e-002,-7.23301947e-001,-2.01683208e-001,1.48674726e-001,-3.58131342e-002,-2.99373567e-001,-5.77452719e-001,-3.18108886e-001,-2.74304420e-001,-8.13846707e-001,1.91600800e+000,8.36254299e-001,7.88672686e-001,-3.30235422e-001,2.05788553e-001,1.72862720e+000,5.45069635e-001,-2.14274979e+000,1.58374524e+000,1.62805244e-001,-4.49638963e-001,6.10224009e-001,1.35559046e+000,-1.68774679e-001,-1.78365135e+000,-3.14457804e-001,1.48712003e+000,-1.99580562e+000,-3.15826154e+000,-1.51707625e+000,-1.11793625e+000,-1.93380487e+000,-6.66035712e-001,2.12378260e-002,2.87882209e+000,8.43553171e-002,-1.25026488e+000,-2.89680600e+000,-2.24760437e+000,-1.27550018e+000,-1.55537057e+000,-1.32546210e+000,-7.49230206e-001,3.78630638e-001,7.02944279e-001,-1.14427674e+000,-1.41296124e+000,4.53377664e-002,7.11983383e-001,-1.40262496e+000,8.95571709e-001,1.31214499e+000,3.42987865e-001,-3.11353624e-001,1.29516768e+000,-2.08709860e+000,8.56539488e-001,-1.69263184e+000,3.95719558e-001,-6.25234663e-001,1.45587429e-001,5.90677202e-001,6.49189591e-001,-3.84735256e-001,4.14639056e-001,-8.14183578e-002,6.09996498e-001,-2.88587749e-001,-1.00319207e+000,-7.37109601e-001,-1.79314822e-001,-1.43686116e-001,4.62578565e-001,-1.51430988e+000,-3.15800220e-001,-1.61767066e+000,-1.15052044e+000,-3.48172694e-001,2.34851763e-001,-3.40833127e-001,-1.91715539e+000,1.88586497e+000,1.92973733e-001,5.80637038e-001,3.48380119e-001,-4.74159420e-001,-9.45430696e-001,-2.16807890e+000,-1.06644261e+000,-2.69382238e-001,1.30835843e+000,-8.71597826e-001,1.79690421e-001,-1.45142078e+000,-1.79278457e+000,3.44158351e-001,-1.74179590e+000,-2.13190794e+000,9.54409420e-001,2.98293090e+000,-2.17595434e+000,2.24303603e-002,-1.38041461e+000,-2.02194762e+000,-9.33432102e-001,3.40192944e-001,-1.12549901e+000,-2.52940267e-001,-1.47020257e+000,-1.48578346e-001,3.14905852e-001,-1.27095103e+000,-1.28554118e+000,-1.44140625e+000,-1.29304260e-001,-1.74622476e+000,9.32865918e-001,1.85983145e+000,-5.76363504e-001,1.78450108e-001,3.38967919e-001,-6.31776929e-001,-2.10575056e+000,-5.51315188e-001,-1.89468396e+000,5.69546938e-001,1.49487638e+000,-8.36608648e-001,-2.69527137e-001,-7.73664117e-001,1.60509551e+000,8.99506330e-001,-3.81087601e-001,-2.38569808e+000,2.00008273e+000,2.55857158e+000,1.84014547e+000,2.19793463e+000,1.71710312e+000,-2.18243861e+000,-9.13647234e-001,-1.69929707e+000,-3.11037636e+000,-1.62086928e+000,1.06574786e+000,6.79403365e-001,2.10816669e+000,-4.05579209e-001,-9.24659669e-001,1.78302860e+000,-3.40525180e-001,-2.58251333e+000,-1.15814865e+000,1.06189978e+000,1.34416723e+000,4.62097645e+000,2.26786876e+000,-6.33341908e-001,-2.39377305e-001,2.54220873e-001,-6.32786930e-001,-3.48151207e-001,-1.25257164e-001,-2.15803933e+000,-2.49646235e+000,-9.20649588e-001,1.03806309e-001,4.60425943e-001,4.00239080e-001,1.70731042e-002,5.03841460e-001,6.67627156e-001,-3.19259346e-001,5.64944565e-001,-5.53839147e-001,2.84504533e+000,1.22946799e+000,2.44760084e+000,1.69161344e+000,7.48831749e-001,1.21913922e+000,-1.79305756e+000,-1.64021397e+000,1.93121210e-001,-2.46651816e+000,1.19505167e+000,-5.88807046e-001,-1.23097874e-001,-1.29011190e+000,-1.33314121e+000,1.06254257e-001,2.49773598e+000,4.13088512e+000,2.35111117e+000,9.73395586e-001,1.77064225e-001,-4.59545059e-003,2.24978709e+000,6.99213922e-001,-8.00015092e-001,7.88275361e-001,2.36508157e-002,2.86151737e-001,3.74357224e-001,-5.58705747e-001,1.28411663e+000,2.93792343e+000,2.90826154e+000,1.57820895e-001,-8.19901347e-001,-2.65059978e-001,3.42395234e+000,1.16964173e+000,-1.34824917e-001,9.75577533e-002,2.16238689e+000,1.49678445e+000,-1.65828502e+000,1.01654458e+000,-1.89345300e+000,1.47923243e+000,2.50079846e+000,1.35680592e+000,2.22109437e+000,1.69995832e+000,1.52041078e+000,-2.37670827e+000,3.33255434e+000,-1.04374182e+000,2.31242490e+000,1.95366395e+000,3.00753069e+000,1.82594371e+000,2.41174531e+000,-2.22498846e+000,1.01218486e+000,8.92067790e-001,1.15547776e+000,-4.11040872e-001,1.59264362e+000,8.86393666e-001,6.05259120e-001,1.85613620e+000,-2.47808290e+000,-1.46734464e+000,-1.23948789e+000,-1.68811190e+000,1.21344006e+000,2.33979300e-001,-1.88926077e+000,-6.57541931e-001,-1.51127651e-001,-4.32649994e+000,-2.28403664e+000,5.72175741e-001,3.25410366e-001,2.94183064e+000,1.89359391e+000,1.11161053e-001,1.28470671e+000,-2.32890034e+000,-1.19119012e+000,-4.95595407e+000,-1.59809673e+000,-2.27197814e+000,-1.37233849e+001};
#endif
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
		hyperPanelVector[len - 1] <<= 10;
#else
		hyperPanelVector = new float[len];
		for(int i = 0; i < len; i ++)
		{
			hyperPanelVector[i] = svm[i];
		}

#endif
	}

	float* HGDescriptor::GetHyperPanelVector()
	{
#ifdef _FIX_POINT_SVM_
		return 0;
#else
		return hyperPanelVector;
#endif
	}
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
	
