#include "pch.h"
#include "Dib.h"
#include "MainFrm.h" // Add this include to define CMainFrame

CDib::CDib(void):m_pDibBits(NULL),m_pGrayValueCount(NULL)
{
	// initialized variables
	m_nBitCount = 0;
	m_nWidth = 0;
	m_nHeight = 0;
	m_nWidthBytes = 0;
	originalDibBits = nullptr;

}

CDib::CDib( CDib &Dib ):m_pDibBits(NULL),m_pGrayValueCount(NULL)
{
	// initialized variables
	m_nBitCount = 0;
	m_nWidth = 0;
	m_nHeight = 0;
	m_nWidthBytes = 0;
	originalDibBits = nullptr;

	if(&Dib == NULL)
	{
		return;
	}
	if(!IsNull())
	{
		Destroy();
	}
	Create(Dib.GetWidth(),Dib.GetHeight(),Dib.GetBPP(),0);
	m_nWidth = Dib.m_nWidth;
	m_nHeight = Dib.m_nHeight;
	if(IsIndexed())
	{
		int nColors=Dib.GetMaxColorTableEntries();
		if(nColors > 0)
		{
			RGBQUAD* pal{};
			pal = new RGBQUAD[nColors];
			Dib.GetColorTable(0,nColors,pal);
			SetColorTable(0,nColors,pal);
			delete[] pal;
		} 
	}
	m_nWidthBytes =abs(GetPitch()) ;
	m_nBitCount = GetBPP();
	m_pDibBits = (unsigned char*)GetBits()+(m_nHeight-1)*GetPitch();
	memcpy(m_pDibBits,Dib.m_pDibBits,m_nHeight*m_nWidthBytes);
}

CDib::~CDib(void)
{
	m_pDibBits = NULL;
	if (m_pGrayValueCount != NULL)
	{
		delete []m_pGrayValueCount;
		m_pGrayValueCount = NULL;
	}
}

void CDib::LoadFile( LPCTSTR lpszPathName )
{
	Load(lpszPathName);
	m_nWidth = GetWidth();
	m_nHeight = GetHeight();
	m_nWidthBytes =abs(GetPitch()) ;
	m_nBitCount = GetBPP();
	m_pDibBits = (unsigned char*)GetBits()+(m_nHeight-1)*GetPitch();

	// ��ȡ������ָ�벢����״̬��
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	if (pMainFrame)
	{
		pMainFrame->UpdateStatusBarWithImageSize(GetActualSize(), GetBitDepth(), GetPixelCount());
	}
}

void CDib::Invert()
{
	for (int i = 0; i < m_nHeight; i++)
	{
		for (int j = 0; j < m_nWidthBytes; j++)
		{
			*(m_pDibBits + i*m_nWidthBytes + j) = 255 - *(m_pDibBits + i*m_nWidthBytes + j);
		}
	}
}

long* CDib::GrayValueCount()
{
	long nColors = GetMaxColorTableEntries();
	if (nColors == 0)
	{
		return NULL;
	}
	long *pGrayValueCount = new long[nColors];
	memset(pGrayValueCount,0,nColors*sizeof(long));
	for (int i=0;i<m_nHeight;i++)
	{
		for (int j=0;j<m_nWidth;j++)
		{
			pGrayValueCount[*(m_pDibBits + i*m_nWidthBytes +j)]++;
		}
	}
	return pGrayValueCount;
}

CSize CDib::GetActualSize() const
{
    return CSize(GetWidth(), GetHeight());
}

int CDib::GetBitDepth() const
{
    return GetBPP();
}

long CDib::GetPixelCount() const
{
    return (GetWidth() * GetHeight() * GetBPP()) / 8 /1024;
}

void CDib::ShowColorChannel(int channel)
{
    if (!IsIndexed())
    {
        return;
    }

    int nColors = GetMaxColorTableEntries();
    if (nColors <= 0)
    {
        return;
    }

    // ����ԭʼ��ɫ��
	static RGBQUAD* originalPal = nullptr;
	if (originalPal == nullptr)
	{
		originalPal = new RGBQUAD[nColors];
		GetColorTable(0, nColors, originalPal);
	}

    RGBQUAD* pal = new RGBQUAD[nColors];
    memcpy(pal, originalPal, nColors * sizeof(RGBQUAD));

    for (int i = 0; i < nColors; ++i)
    {
        switch (channel)
        {
        case 2: // Red channel
			pal[i].rgbRed = originalPal[i].rgbRed;
			pal[i].rgbGreen = 0;
			pal[i].rgbBlue = 0;
            break;
        case 1: // Green channel
			pal[i].rgbGreen = originalPal[i].rgbGreen;
			pal[i].rgbRed = 0;
			pal[i].rgbBlue = 0;   
            break;
        case 0: // Blue channel
			pal[i].rgbBlue = originalPal[i].rgbBlue;
			pal[i].rgbRed = 0;
			pal[i].rgbGreen = 0;
            break;
        default:
            break;
        }
    }

    SetColorTable(0, nColors, pal);
    delete[] pal;

}

void CDib::LinearTransform()
{
	if (m_pDibBits == NULL)
	{
		return;
	}

	for (int i = 0; i < m_nHeight; i++)
	{
		for (int j = 0; j < m_nWidthBytes; j++)
		{
			unsigned char& pixel = *(m_pDibBits + i * m_nWidthBytes + j);
			if (pixel < 100)
			{
				pixel = 0;
			}
			else  if (pixel <=150)
			{
				pixel = static_cast<unsigned char>(100 + (pixel - 100) * (200 - 100) / (150 - 100));
			}

			else if (pixel > 200)
			{
				pixel = 255;
			}
			
		}
	}
}

void CDib::ShowBitPlane(int bitPlane)
{
	if (m_pDibBits == NULL || bitPlane < 0 || bitPlane >= m_nBitCount)
	{
		return;
	}

	int bytesPerPixel = m_nBitCount / 8;
	int bitPosition = bitPlane % 8;

	for (int i = 0; i < m_nHeight; i++)
	{
		for (int j = 0; j < m_nWidth; j++)
		{
			unsigned char* pixel = m_pDibBits + i * m_nWidthBytes + j * bytesPerPixel;
			for (int k = 0; k < bytesPerPixel; k++)
			{
				pixel[k] = (pixel[k] >> bitPosition) & 0x01;
				pixel[k] *= 255; // ��λƽ��ֵ��չ��0��255
			}
		}
	}
}

void CDib::Restore()
{
	// �ָ�ͼ��ʵ��
	if (m_pDibBits == NULL)
	{
		return;
	}
	if (originalDibBits == nullptr)
	{
		originalDibBits = new unsigned char[static_cast<size_t>(m_nHeight) * m_nWidthBytes];
		memcpy(originalDibBits, m_pDibBits, static_cast<size_t>(m_nHeight) * m_nWidthBytes);
	}
	else
		memcpy(m_pDibBits, originalDibBits, static_cast<size_t>(m_nHeight) * m_nWidthBytes);
}

void CDib::Equalize()
{
	if (m_pDibBits == NULL)
	{
		return;
	}

	// ����ֱ��ͼ
	long nColors = GetMaxColorTableEntries();

	long* histogram = new long[nColors];
	memset(histogram, 0, nColors * sizeof(long));

	for (int i = 0; i < m_nHeight; i++)
	{
		for (int j = 0; j < m_nWidthBytes; j++)
		{
			histogram[*(m_pDibBits + i * m_nWidthBytes + j)]++;
		}
	}

	// �����ۻ��ֲ����� (CDF)
	long* cdf = new long[nColors];
	cdf[0] = histogram[0];
	for (int i = 1; i < nColors; i++)
	{
		cdf[i] = cdf[i - 1] + histogram[i];
	}

	// ��һ�� CDF
	long cdf_min = cdf[0];
	for (int i = 0; i < nColors; i++)
	{
		cdf[i] = ((cdf[i] - cdf_min) * (nColors - 1)) / (m_nHeight * m_nWidth);
	}

	// Ӧ�þ��⻯
	for (int i = 0; i < m_nHeight; i++)
	{
		for (int j = 0; j < m_nWidthBytes; j++)
		{
			*(m_pDibBits + i * m_nWidthBytes + j) = static_cast<unsigned char>(cdf[*(m_pDibBits + i * m_nWidthBytes + j)]);
		}
	}

	delete[] histogram;
	delete[] cdf;
}

void CDib::Smooth()
{
	if (m_pDibBits == NULL)
	{
		return;
	}

	int kernel[3][3] = {
		{1, 1, 1},
		{1, 1, 1},
		{1, 1, 1}
	};
	int kernelSize = 3;
	int kernelSum = 9;

	unsigned char* tempBits = new unsigned char[m_nHeight * m_nWidthBytes];
	memcpy(tempBits, m_pDibBits, m_nHeight * m_nWidthBytes);

	for (int i = 1; i < m_nHeight - 1; i++) // �ӵڶ��п�ʼ
	{
		for (int j = 1; j < m_nWidth - 1; j++) // �ӵڶ��п�ʼ
		{
			int sum = 0;
			for (int ki = 0; ki < kernelSize; ki++)
			{
				for (int kj = 0; kj < kernelSize; kj++)
				{
					int pixel = *(tempBits + (i + ki - 1) * m_nWidthBytes + (j + kj - 1));
					sum += pixel * kernel[ki][kj];
				}
			}
			*(m_pDibBits + i * m_nWidthBytes + j) = sum / kernelSum;
		}
	}

	delete[] tempBits;
}

void CDib::Sharp()
{
	if (m_pDibBits == NULL)
	{
		return;
	}

	int kernel[3][3] = {
		{ 0, -1,  0},
		{-1,  5, -1},
		{ 0, -1,  0}
	};
	int kernelSize = 3;

	unsigned char* tempBits = new unsigned char[m_nHeight * m_nWidthBytes];
	memcpy(tempBits, m_pDibBits, m_nHeight * m_nWidthBytes);

	for (int i = 1; i < m_nHeight - 1; i++)
	{
		for (int j = 1; j < m_nWidth - 1; j++)
		{
			int sum = 0;
			for (int ki = 0; ki < kernelSize; ki++)
			{
				for (int kj = 0; kj < kernelSize; kj++)
				{
					int pixel = *(tempBits + (i + ki - 1) * m_nWidthBytes + (j + kj - 1));
					sum += pixel * kernel[ki][kj];
				}
			}
			*(m_pDibBits + i * m_nWidthBytes + j) = min(max(sum, 0), 255);
		}
	}

	delete[] tempBits;
}

void CDib::LaplaceTransform()
{
	if (m_nBitCount == 8) { // 8λ�Ҷ�ͼ��
		unsigned char* pNewDibBits = new unsigned char[m_nHeight * m_nWidthBytes];
		memcpy(pNewDibBits, m_pDibBits, m_nHeight * m_nWidthBytes);
		int kernel[3][3] = {
		{ 0, -1,  0},
		{-1,  4, -1},
		{ 0, -1,  0}
		};

		int minVal = 255, maxVal = 0;
		for (int y = 1; y < m_nHeight - 1; y++) {
			for (int x = 1; x < m_nWidth - 1; x++) {
				int sum = 0;
				for (int i = -1; i <= 1; i++) {
					for (int j = -1; j <= 1; j++) {
						sum += pNewDibBits[(y + i) * m_nWidthBytes + x + j] * kernel[i + 1][j + 1];
					}
				}
				minVal = min(minVal, sum);
				maxVal = max(maxVal, sum);
			}
		}
		for (int y = 1; y < m_nHeight - 1; y++) {
			for (int x = 1; x < m_nWidth - 1; x++) {
				int sum = 0;
				for (int i = -1; i <= 1; i++) {
					for (int j = -1; j <= 1; j++) {
						sum += pNewDibBits[(y + i) * m_nWidthBytes + x + j] * kernel[i + 1][j + 1];
					}
				}
				m_pDibBits[y * m_nWidthBytes + x] = (unsigned char)((sum - minVal) * 255 / (maxVal - minVal));
			}
		}
		delete[] pNewDibBits;
	}
}
	
void CDib::EnhanceLaplace()  
{  
   if (m_pDibBits == NULL)  
   {  
       return;  
   }  

   if (m_nBitCount == 8) { // 8λ�Ҷ�ͼ��  
       unsigned char* pNewDibBits = new unsigned char[m_nHeight * m_nWidthBytes];  
       memcpy(pNewDibBits, m_pDibBits, m_nHeight * m_nWidthBytes);  
       int kernel[3][3] = {  
       { 0, -1,  0},  
       {-1,  4, -1},  
       { 0, -1,  0}  
       };  

       for (int y = 1; y < m_nHeight - 1; y++) {  
           for (int x = 1; x < m_nWidth - 1; x++) {  
               int sum = 0;  
               for (int i = -1; i <= 1; i++) {  
                   for (int j = -1; j <= 1; j++) {  
                       sum += pNewDibBits[(y + i) * m_nWidthBytes + x + j] * kernel[i + 1][j + 1];  
                   }  
               }  
               int newValue = m_pDibBits[y * m_nWidthBytes + x] + sum;  
               m_pDibBits[y * m_nWidthBytes + x] = min(max(newValue, 0), 255);  
           }  
       }  
       delete[] pNewDibBits;  
   }  
}

void CDib::CreateColorImageWithCircles(int width, int height)
{
	// ����24λ��ɫͼ��
	Create(width, height, 24, 0);

	// ���ó�Ա����
	m_nWidth = width;
	m_nHeight = height;
	m_nWidthBytes = abs(GetPitch());
	m_nBitCount = 24;  // 24λͼ��
	m_pDibBits = (unsigned char*)GetBits() + (m_nHeight - 1) * GetPitch();

	// ����ͼ���ڴ����ڻָ�
	if (originalDibBits != nullptr)
	{
		delete[] originalDibBits;
	}
	originalDibBits = new unsigned char[static_cast<size_t>(m_nHeight) * m_nWidthBytes];

	// ���ú�ɫ����
	memset(m_pDibBits, 0, static_cast<size_t>(m_nHeight) * m_nWidthBytes);

	int radius = width / 6; // Բ�İ뾶
	int centerX1 = width / 2 - radius / 2; // ��ɫԲ������X����
	int centerY1 = height / 2 - radius / 2; // ��ɫԲ������Y����
	int centerX2 = width / 2 + radius / 2; // ��ɫԲ������X����
	int centerY2 = height / 2 - radius / 2; // ��ɫԲ������Y����
	int centerX3 = width / 2; // ��ɫԲ������X����
	int centerY3 = height / 2 + radius / 2; // ��ɫԲ������Y����

	// ����ͼ�����أ�����Բ��
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			// ����ÿ��Բ�ľ���
			int dist1 = (x - centerX1) * (x - centerX1) + (y - centerY1) * (y - centerY1);
			int dist2 = (x - centerX2) * (x - centerX2) + (y - centerY2) * (y - centerY2);
			int dist3 = (x - centerX3) * (x - centerX3) + (y - centerY3) * (y - centerY3);

			// ��ȡ��ǰ����ָ��
			unsigned char* pixel = m_pDibBits + y * m_nWidthBytes + x * 3;

			// ����ں�ɫԲ��
			if (dist1 <= radius * radius)
			{
				pixel[2] = 255; // ��ɫͨ��
			}

			// �������ɫԲ��
			if (dist2 <= radius * radius)
			{
				pixel[1] = 255; // ��ɫͨ��
			}

			// �������ɫԲ��
			if (dist3 <= radius * radius)
			{
				pixel[0] = 255; // ��ɫͨ��
			}
		}
	}
}

void CDib::GenerateHueImage()
{
   if (m_pDibBits == NULL || m_nBitCount != 24) // ȷ����24λ��ɫͼ��
   {
       return;
   }

   unsigned char* tempBits = new unsigned char[m_nHeight * m_nWidthBytes];
   memcpy(tempBits, m_pDibBits, m_nHeight * m_nWidthBytes);

   for (int y = 0; y < m_nHeight; y++)
   {
       for (int x = 0; x < m_nWidth; x++)
       {
           unsigned char* pixel = tempBits + y * m_nWidthBytes + x * 3;
           unsigned char blue = pixel[0];
           unsigned char green = pixel[1];
           unsigned char red = pixel[2];

		   // ת��ΪHSVɫ������һ����0-1��Χ
           float r = red / 255.0f;
           float g = green / 255.0f;
           float b = blue / 255.0f;

           float maxVal = max(r, max(g, b));
           float minVal = min(r, min(g, b));
           float delta = maxVal - minVal;

           float hue = 0.0f;
           if (delta > 0.0f)
           {
               if (maxVal == r)
               {
                   hue = 60.0f * fmod(((g - b) / delta), 6.0f);
               }
               else if (maxVal == g)
               {
                   hue = 60.0f * (((b - r) / delta) + 2.0f);
               }
               else if (maxVal == b)
               {
                   hue = 60.0f * (((r - g) / delta) + 4.0f);
               }
           }
           if (hue < 0.0f)
           {
               hue += 360.0f;
           }

           // ��ɫ��ӳ�䵽0-255��Χ
           unsigned char hueValue = static_cast<unsigned char>((hue / 360.0f) * 255.0f);

           // ��������Ϊ�Ҷ�ɫ��ֵ
           unsigned char* targetPixel = m_pDibBits + y * m_nWidthBytes + x * 3;
           targetPixel[0] = hueValue; // ��ɫͨ��
           targetPixel[1] = hueValue; // ��ɫͨ��
           targetPixel[2] = hueValue; // ��ɫͨ��
       }
   }

   delete[] tempBits;
}

void CDib::GenerateSaturationImage()  
{  
   if (m_pDibBits == NULL || m_nBitCount != 24) // ȷ����24λ��ɫͼ��  
   {  
       return;  
   }  

   unsigned char* tempBits = new unsigned char[m_nHeight * m_nWidthBytes];  
   memcpy(tempBits, m_pDibBits, m_nHeight * m_nWidthBytes);  

   for (int y = 0; y < m_nHeight; y++)  
   {  
       for (int x = 0; x < m_nWidth; x++)  
       {  
           unsigned char* pixel = tempBits + y * m_nWidthBytes + x * 3;  
           unsigned char blue = pixel[0];  
           unsigned char green = pixel[1];  
           unsigned char red = pixel[2];  

           // ת��ΪHSV���Ͷ�  
           float r = red / 255.0f;  
           float g = green / 255.0f;  
           float b = blue / 255.0f;  

           float maxVal = max(r, max(g, b));  
           float minVal = min(r, min(g, b));  

           float saturation = 0.0f;  
           if (maxVal > 0.0f)  
           {  
               saturation = (maxVal - minVal) / maxVal;  
           }  

           // �����Ͷ�ӳ�䵽0-255��Χ  
           unsigned char saturationValue = static_cast<unsigned char>(saturation * 255.0f);  

           // ��������Ϊ�Ҷȱ��Ͷ�ֵ  
           unsigned char* targetPixel = m_pDibBits + y * m_nWidthBytes + x * 3;  
           targetPixel[0] = saturationValue; // ��ɫͨ��  
           targetPixel[1] = saturationValue; // ��ɫͨ��  
           targetPixel[2] = saturationValue; // ��ɫͨ��  
       }  
   }  

   delete[] tempBits;  
}

void CDib::GenerateBrightnessImage()  
{  
   if (m_pDibBits == NULL || m_nBitCount != 24) // ȷ����24λ��ɫͼ��  
   {  
       return;  
   }  

   unsigned char* tempBits = new unsigned char[m_nHeight * m_nWidthBytes];  
   memcpy(tempBits, m_pDibBits, m_nHeight * m_nWidthBytes);  

   for (int y = 0; y < m_nHeight; y++)  
   {  
       for (int x = 0; x < m_nWidth; x++)  
       {  
           unsigned char* pixel = tempBits + y * m_nWidthBytes + x * 3;  
           unsigned char blue = pixel[0];  
           unsigned char green = pixel[1];  
           unsigned char red = pixel[2];  

           // �������� (ʹ�ü�Ȩ��ʽ)  
           unsigned char brightness = static_cast<unsigned char>((red + green + blue)/3);  

           // ��������Ϊ�Ҷ�����ֵ  
           unsigned char* targetPixel = m_pDibBits + y * m_nWidthBytes + x * 3;  
           targetPixel[0] = brightness; // ��ɫͨ��  
           targetPixel[1] = brightness; // ��ɫͨ��  
           targetPixel[2] = brightness; // ��ɫͨ��  
       }  
   }  

   delete[] tempBits;  
}

void CDib::ConvertRGBToHSI()
{
   if (m_pDibBits == NULL || m_nBitCount != 24) // ȷ����24λ��ɫͼ��
   {
       return;
   }

   unsigned char* tempBits = new unsigned char[m_nHeight * m_nWidthBytes];
   memcpy(tempBits, m_pDibBits, m_nHeight * m_nWidthBytes);

   for (int y = 0; y < m_nHeight; y++)
   {
       for (int x = 0; x < m_nWidth; x++)
       {
           unsigned char* pixel = tempBits + y * m_nWidthBytes + x * 3;
           unsigned char blue = pixel[0];
           unsigned char green = pixel[1];
           unsigned char red = pixel[2];

           // ת��ΪHSI
           float r = red / 255.0f;
           float g = green / 255.0f;
           float b = blue / 255.0f;

           float intensity = (r + g + b) / 3.0f;

           float minVal = min(r, min(g, b));
           float saturation = 0.0f;
           if (intensity > 0.0f)
           {
               saturation = 1.0f -(minVal / intensity);
           }

           float hue = 0.0f;
           if (saturation > 0.0f)
           {
               float numerator = 0.5f * ((r - g) + (r - b));
               float denominator = sqrt((r - g) * (r - g) + (r - b) * (g - b));
               hue = acos(numerator / (denominator + 1e-6f)); // ��ֹ����
               if (b > g)
               {
                   hue = 2.0f * Pi - hue;
               }
           }

           // ��HSIֵӳ�䵽0-255��Χ
           unsigned char hValue = static_cast<unsigned char>((hue / (2.0f * Pi)) * 255.0f);
           unsigned char sValue = static_cast<unsigned char>(saturation * 255.0f);
           unsigned char iValue = static_cast<unsigned char>(intensity * 255.0f);

           // ��������ΪHSIֵ
           unsigned char* targetPixel = m_pDibBits + y * m_nWidthBytes + x * 3;
           targetPixel[0] = hValue; // Hͨ��
           targetPixel[1] = sValue; // Sͨ��
           targetPixel[2] = iValue; // Iͨ��
       }
   }

   delete[] tempBits;
}

void CDib::ConvertHSIToRGB()
{
	if (m_pDibBits == NULL || m_nBitCount != 24) // ȷ����24λ��ɫͼ��  
	{
		return;
	}

	unsigned char* tempBits = new unsigned char[m_nHeight * m_nWidthBytes];
	memcpy(tempBits, m_pDibBits, m_nHeight * m_nWidthBytes);

	for (int y = 0; y < m_nHeight; y++)
	{
		for (int x = 0; x < m_nWidth; x++)
		{
			unsigned char* pixel = tempBits + y * m_nWidthBytes + x * 3;
			unsigned char hValue = pixel[0];
			unsigned char sValue = pixel[1];
			unsigned char iValue = pixel[2];

			// ��HSIֵת����0-1��Χ  
			float h = (hValue / 255.0f) * (2.0f * Pi);
			float s = sValue / 255.0f;
			float i = iValue / 255.0f;

			float r = 0.0f, g = 0.0f, b = 0.0f;

			// ����Hֵ�ķ�Χʹ�ò�ͬ��ת����ʽ  
			if (h >= 0 && h < 2.0f * Pi / 3.0f) // 0�� �� 120��  
			{
				b = i * (1.0f - s);
				r = i * (1.0f + s * cos(h) / cos(Pi / 3.0f - h));
				g = 3.0f * i - (r + b);
			}
			else if (h >= 2.0f * Pi / 3.0f && h < 4.0f * Pi / 3.0f) // 120�� �� 240��  
			{
				h = h - 2.0f * Pi / 3.0f;
				r = i * (1.0f - s);
				g = i * (1.0f + s * cos(h) / cos(Pi / 3.0f - h));
				b = 3.0f * i - (r + g);
			}
			else // 240�� �� 360��  
			{
				h = h - 4.0f * Pi / 3.0f;
				g = i * (1.0f - s);
				b = i * (1.0f + s * cos(h) / cos(Pi / 3.0f - h));
				r = 3.0f * i - (g + b);
			}

			// ȷ��RGBֵ��0-1��Χ��  
			r = min(1.0f, max(0.0f, r));
			g = min(1.0f, max(0.0f, g));
			b = min(1.0f, max(0.0f, b));

			// ��RGBֵӳ�䵽0-255��Χ  
			unsigned char redValue = static_cast<unsigned char>(r * 255.0f);
			unsigned char greenValue = static_cast<unsigned char>(g * 255.0f);
			unsigned char blueValue = static_cast<unsigned char>(b * 255.0f);

			// ��������ΪRGBֵ  
			unsigned char* targetPixel = m_pDibBits + y * m_nWidthBytes + x * 3;
			targetPixel[0] = blueValue;  // ��ɫͨ��  
			targetPixel[1] = greenValue; // ��ɫͨ��  
			targetPixel[2] = redValue;   // ��ɫͨ��  
		}
	}

	delete[] tempBits;
}

void CDib::EqualizeColorImage()
{
	if (m_pDibBits == NULL || m_nBitCount != 24) // ȷ����24λ��ɫͼ��
	{
		return;
	}

	// ������ʱ������������ԭʼ��������
	unsigned char* tempBits = new unsigned char[m_nHeight * m_nWidthBytes];
	memcpy(tempBits, m_pDibBits, m_nHeight * m_nWidthBytes);

	// ����HSIֵ����
	float* hValues = new float[m_nHeight * m_nWidth];
	float* sValues = new float[m_nHeight * m_nWidth];
	float* iValues = new float[m_nHeight * m_nWidth];

	// ��һ������RGBת��ΪHSI
	for (int y = 0; y < m_nHeight; y++)
	{
		for (int x = 0; x < m_nWidth; x++)
		{
			unsigned char* pixel = tempBits + y * m_nWidthBytes + x * 3;
			unsigned char blue = pixel[0];
			unsigned char green = pixel[1];
			unsigned char red = pixel[2];

			// ת��ΪHSI
			float r = red / 255.0f;
			float g = green / 255.0f;
			float b = blue / 255.0f;

			float intensity = (r + g + b) / 3.0f;

			float minVal = min(r, min(g, b));
			float saturation = 0.0f;
			if (intensity > 0.0f)
			{
				saturation = 1.0f - (minVal / intensity);
			}

			float hue = 0.0f;
			if (saturation > 0.0f)
			{
				float numerator = 0.5f * ((r - g) + (r - b));
				float denominator = sqrt((r - g) * (r - g) + (r - b) * (g - b));
				hue = acos(numerator / (denominator + 1e-6f)); // ��ֹ����
				if (b > g)
				{
					hue = 2.0f * Pi - hue;
				}
			}

			int index = y * m_nWidth + x;
			hValues[index] = hue;
			sValues[index] = saturation;
			iValues[index] = intensity;
		}
	}

	// �ڶ�����������ͨ������ֱ��ͼ���⻯  
	const int INTENSITY_LEVELS = 256;
	long histogram[INTENSITY_LEVELS] = { 0 };
	for (int i = 0; i < m_nHeight * m_nWidth; i++)
	{
		int intensityLevel = static_cast<int>(iValues[i] * (INTENSITY_LEVELS - 1) + 0.5f);
		histogram[intensityLevel]++;
	}

	long cdf[INTENSITY_LEVELS] = { 0 };
	cdf[0] = histogram[0];
	for (int i = 1; i < INTENSITY_LEVELS; i++)
	{
		cdf[i] = cdf[i - 1] + histogram[i];
	}

	float normalizedCdf[INTENSITY_LEVELS] = { 0.0f };
	long cdfMin = cdf[0];
	for (int i = 0; i < INTENSITY_LEVELS; i++)
	{
		normalizedCdf[i] = static_cast<float>(cdf[i] - cdfMin) / (m_nHeight * m_nWidth - cdfMin);
	}

	// ���������Ա��Ͷ�ͨ�������ʶ�̧��  
	for (int i = 0; i < m_nHeight * m_nWidth; i++)
	{
		sValues[i] = sValues[i] + 0.09f; // ���Ͷ�����
	}
	// ���Ʊ��Ͷ�ֵ��0-1��Χ��
	for (int i = 0; i < m_nHeight * m_nWidth; i++)
	{
		sValues[i] = min(1.0f, max(0.0f, sValues[i]));
	}

	// ���Ĳ�����HSIת����RGB  
	for (int y = 0; y < m_nHeight; y++)
	{
		for (int x = 0; x < m_nWidth; x++)
		{
			int index = y * m_nWidth + x;
			float h = hValues[index];
			float s = sValues[index];
			float i = iValues[index];

			// ���⻯����ֵ
			int intensityLevel = static_cast<int>(i * (INTENSITY_LEVELS - 1) + 0.5f);
			float newI = normalizedCdf[intensityLevel];

			// ��HSIת����RGB
			float r = 0.0f, g = 0.0f, b = 0.0f;

			if (s <= 0.0f)
			{
				r = g = b = newI;
			}
			else
			{
				// ����Hֵ�ķ�Χʹ�ò�ͬ��ת����ʽ
				if (h >= 0 && h < 2.0f * Pi / 3.0f) // 0�� �� 120��
				{
					b = newI * (1.0f - s);
					r = newI * (1.0f + s * cos(h) / cos(Pi / 3.0f - h));
					g = 3.0f * newI - (r + b);
				}
				else if (h >= 2.0f * Pi / 3.0f && h < 4.0f * Pi / 3.0f) // 120�� �� 240��
				{
					h = h - 2.0f * Pi / 3.0f;
					r = newI * (1.0f - s);
					g = newI * (1.0f + s * cos(h) / cos(Pi / 3.0f - h));
					b = 3.0f * newI - (r + g);
				}
				else // 240�� �� 360��
				{
					h = h - 4.0f * Pi / 3.0f;
					g = newI * (1.0f - s);
					b = newI * (1.0f + s * cos(h) / cos(Pi / 3.0f - h));
					r = 3.0f * newI - (g + b);
				}
			}

			// ȷ��RGBֵ��0-1��Χ��
			r = min(1.0f, max(0.0f, r));
			g = min(1.0f, max(0.0f, g));
			b = min(1.0f, max(0.0f, b));

			// ��RGBֵӳ�䵽0-255��Χ
			unsigned char redValue = static_cast<unsigned char>(r * 255.0f);
			unsigned char greenValue = static_cast<unsigned char>(g * 255.0f);
			unsigned char blueValue = static_cast<unsigned char>(b * 255.0f);

			// ��������ΪRGBֵ
			unsigned char* targetPixel = m_pDibBits + y * m_nWidthBytes + x * 3;
			targetPixel[0] = blueValue;  // ��ɫͨ��
			targetPixel[1] = greenValue; // ��ɫͨ��
			targetPixel[2] = redValue;   // ��ɫͨ��
		}
	}

	// ����
	delete[] hValues;
	delete[] sValues;
	delete[] iValues;
	delete[] tempBits;
}

void CDib::AddGaussianNoise(double mean, double stddev)  
{  
   if (m_pDibBits == NULL || m_nBitCount != 24) // ȷ����24λ��ɫͼ��  
   {  
       return;  
   }  

   std::default_random_engine generator;  
   std::normal_distribution<double> distribution(mean, stddev);  

   for (int y = 0; y < m_nHeight; y++)  
   {  
       for (int x = 0; x < m_nWidth; x++)  
       {  
           unsigned char* pixel = m_pDibBits + y * m_nWidthBytes + x * 3;  

           for (int channel = 0; channel < 3; channel++) // ����RGBͨ��  
           {  
               int noise = static_cast<int>(distribution(generator));  
               int newValue = pixel[channel] + noise;  
               pixel[channel] = static_cast<unsigned char>(min(255, max(0, newValue)));  
           }  
       }  
   }  
}

void CDib::GuassianFilter() 
{
	// ����Ч��ͦ�õľ�����
	// �����嶼��ƽ����Ү����ʹ����
	if (m_pDibBits == NULL)
	{
		return;
	}

	// �ж��ǲ�ɫͼ���ǻҶ�ͼ��
	bool isColorImage = (m_nBitCount == 24);
	int bytesPerPixel = isColorImage ? 3 : 1;

	// ������ʱ������
	unsigned char* tempBits = new unsigned char[m_nHeight * m_nWidthBytes];
	memcpy(tempBits, m_pDibBits, m_nHeight * m_nWidthBytes);

	// �Ǿֲ���ֵ�˲�����
	const int searchWindowSize = 11; // �������ڴ�С
	const int halfSearchSize = searchWindowSize / 2;
	const int patchSize = 5;         // ���С
	const int halfPatchSize = patchSize / 2;
	const double h = 15.0;           // �˲�ǿ�Ȳ�������Ҫ��������ˮƽ����
	const double h2 = h * h;

	// ��ÿ�����ؽ��д���
	for (int y = halfPatchSize; y < m_nHeight - halfPatchSize; y++)
	{
		for (int x = halfPatchSize; x < m_nWidth - halfPatchSize; x++)
		{
			for (int c = 0; c < bytesPerPixel; c++) // ����ÿ��ͨ��
			{
				double sum = 0.0;
				double weightSum = 0.0;

				// ������������Ѱ�����ƿ�
				for (int sy = max(halfPatchSize, y - halfSearchSize);
					sy <= min(m_nHeight - halfPatchSize - 1, y + halfSearchSize); sy++)
				{
					for (int sx = max(halfPatchSize, x - halfSearchSize);
						sx <= min(m_nWidth - halfPatchSize - 1, x + halfSearchSize); sx++)
					{
						// ����������ľ���
						double dist = 0.0;
						for (int py = -halfPatchSize; py <= halfPatchSize; py++)
						{
							for (int px = -halfPatchSize; px <= halfPatchSize; px++)
							{
								unsigned char p1 = tempBits[(y + py) * m_nWidthBytes + (x + px) * bytesPerPixel + c];
								unsigned char p2 = tempBits[(sy + py) * m_nWidthBytes + (sx + px) * bytesPerPixel + c];
								double diff = p1 - p2;
								dist += diff * diff;
							}
						}

						// ����Ȩ��
						dist /= (patchSize * patchSize);
						double w = exp(-dist / h2);
						weightSum += w;
						sum += w * tempBits[sy * m_nWidthBytes + sx * bytesPerPixel + c];
					}
				}

				// �����Ȩƽ��ֵ
				unsigned char* pixel = m_pDibBits + y * m_nWidthBytes + x * bytesPerPixel;
				if (weightSum > 0)
				{
					pixel[c] = static_cast<unsigned char>(sum / weightSum);
				}
			}
		}
	}

	// �ͷ���ʱ�ڴ�
	delete[] tempBits;
}

void CDib::AddSaltAndPepperNoise(double noiseRatio)  
{  
   if (m_pDibBits == NULL || m_nBitCount != 24) // ȷ����24λ��ɫͼ��  
   {  
       return;  
   }  

   std::default_random_engine generator;  
   std::uniform_real_distribution<double> distribution(0.0, 1.0);  

   for (int y = 0; y < m_nHeight; y++)  
   {  
       for (int x = 0; x < m_nWidth; x++)  
       {  
           unsigned char* pixel = m_pDibBits + y * m_nWidthBytes + x * 3;  

           for (int channel = 0; channel < 3; channel++) // ����RGBͨ��  
           {  
               double randomValue = distribution(generator);  
               if (randomValue < noiseRatio / 2)  
               {  
                   pixel[channel] = 0; // ������  
               }  
               else if (randomValue < noiseRatio)  
               {  
                   pixel[channel] = 255; // ������  
               }  
           }  
       }  
   }  
}

void CDib::ApplyMedianFilter()
{
	if (m_pDibBits == NULL || m_nBitCount != 24) // ȷ����24λ��ɫͼ��  
	{
		return;
	}
	unsigned char* tempBits = new unsigned char[m_nHeight * m_nWidthBytes];
	memcpy(tempBits, m_pDibBits, m_nHeight * m_nWidthBytes);
	for (int y = 1; y < m_nHeight - 1; y++)
	{
		for (int x = 1; x < m_nWidth - 1; x++)
		{
			std::vector<int> rValues, gValues, bValues;
			for (int ky = -1; ky <= 1; ky++)
			{
				for (int kx = -1; kx <= 1; kx++)
				{
					int pixelX = x + kx;
					int pixelY = y + ky;
					unsigned char* pixel = tempBits + pixelY * m_nWidthBytes + pixelX * 3;
					bValues.push_back(pixel[0]); // ��ɫͨ��  
					gValues.push_back(pixel[1]); // ��ɫͨ��  
					rValues.push_back(pixel[2]); // ��ɫͨ��  
				}
			}
			std::sort(rValues.begin(), rValues.end());
			std::sort(gValues.begin(), gValues.end());
			std::sort(bValues.begin(), bValues.end());
			unsigned char* targetPixel = m_pDibBits + y * m_nWidthBytes + x * 3;
			targetPixel[0] = bValues[4]; // ��ֵ��ɫͨ��  
			targetPixel[1] = gValues[4]; // ��ֵ��ɫͨ��  
			targetPixel[2] = rValues[4]; // ��ֵ��ɫͨ��  
		}
	}
	delete[] tempBits;
}

void CDib::EnhanceLaplaceColorImage()  
{  
   if (m_pDibBits == NULL || m_nBitCount != 24) // ȷ����24λ��ɫͼ��  
   {  
       return;  
   }  

   // ����������˹����  
   int kernel[3][3] = {  
       { 0, -1,  0},  
       {-1,  4, -1},  
       { 0, -1,  0}  
   };  
   int kernelSize = 3;  

   // ������ʱ������  
   unsigned char* tempBits = new unsigned char[m_nHeight * m_nWidthBytes];  
   memcpy(tempBits, m_pDibBits, m_nHeight * m_nWidthBytes);  

   for (int y = 1; y < m_nHeight - 1; y++)  
   {  
       for (int x = 1; x < m_nWidth - 1; x++)  
       {  
           int rSum = 0, gSum = 0, bSum = 0;  

           // ���������  
           for (int ky = 0; ky < kernelSize; ky++)  
           {  
               for (int kx = 0; kx < kernelSize; kx++)  
               {  
                   int pixelX = x + kx - 1;  
                   int pixelY = y + ky - 1;  
                   unsigned char* pixel = tempBits + pixelY * m_nWidthBytes + pixelX * 3;  

                   bSum += pixel[0] * kernel[ky][kx]; // ��ɫͨ��  
                   gSum += pixel[1] * kernel[ky][kx]; // ��ɫͨ��  
                   rSum += pixel[2] * kernel[ky][kx]; // ��ɫͨ��  
               }  
           }  

           // ������˹��ǿ  
           unsigned char* targetPixel = m_pDibBits + y * m_nWidthBytes + x * 3;  
           targetPixel[0] = static_cast<unsigned char>(min(max(targetPixel[0] + bSum, 0), 255)); // ��ɫͨ��  
           targetPixel[1] = static_cast<unsigned char>(min(max(targetPixel[1] + gSum, 0), 255)); // ��ɫͨ��  
           targetPixel[2] = static_cast<unsigned char>(min(max(targetPixel[2] + rSum, 0), 255)); // ��ɫͨ��  
       }  
   }  

   delete[] tempBits;  
}

void CDib::ApplyPseudoColor()
{
   if (m_pDibBits == NULL || m_nBitCount != 8) // ȷ����8λ�Ҷ�ͼ��
   {
       return;
   }
   // ��ʾ�û���������
   AfxMessageBox(_T("��ѡ���ɫ����ʼ��ɫ��"));
   // ������ɫ��Ի��򣬹��û�ѡ����ʼ��ɫ����ֹ��ɫ
   CColorDialog startColorDialog;
   if (startColorDialog.DoModal() != IDOK)
   {
       return; // �û�ȡ������
   }

   // ��ʾ�û���������
   AfxMessageBox(_T("��ѡ���ɫ����ֹ��ɫ��"));
   CColorDialog endColorDialog;
   if (endColorDialog.DoModal() != IDOK)
   {
	   return; // �û�ȡ������
   }

   COLORREF startColor = startColorDialog.GetColor();
   COLORREF endColor = endColorDialog.GetColor();

   // ��ȡ��ʼ����ֹ��ɫ��RGB����
   BYTE startR = GetRValue(startColor);
   BYTE startG = GetGValue(startColor);
   BYTE startB = GetBValue(startColor);

   BYTE endR = GetRValue(endColor);
   BYTE endG = GetGValue(endColor);
   BYTE endB = GetBValue(endColor);

   // ����α��ɫ��ɫ��
   RGBQUAD palette[256];
   for (int i = 0; i < 256; i++)
   {
       float ratio = i / 255.0f;
       palette[i].rgbRed = static_cast<BYTE>(startR + ratio * (endR - startR));
       palette[i].rgbGreen = static_cast<BYTE>(startG + ratio * (endG - startG));
       palette[i].rgbBlue = static_cast<BYTE>(startB + ratio * (endB - startB));
       palette[i].rgbReserved = 0;
   }

   // Ӧ��α��ɫ��ɫ��
   SetColorTable(0, 256, palette);
}

void CDib::FFT2D()
{
	// Get the size of the image
	int nWidth = GetWidth();
	int nHeight = GetHeight();
	// Create a complex array to store the image
	complex<double>* pComplexImage = new complex<double>[nWidth * nHeight];
	for (int i = 0; i < nHeight; i++)
	{
		for (int j = 0; j < nWidth; j++)
		{
			pComplexImage[i * nWidth + j] = complex<double>(*(m_pDibBits + i * m_nWidthBytes + j), 0);
		}
	}
	// Perform 1D FFT on each row
	for (int i = 0; i < nHeight; i++)
	{
		FFT(pComplexImage + i * nWidth, nWidth);
	}
	// Perform 1D FFT on each column
	complex<double>* pColumn = new complex<double>[nHeight];
	for (int i = 0; i < nWidth; i++)
	{
		for (int j = 0; j < nHeight; j++)
		{
			pColumn[j] = pComplexImage[j * nWidth + i];
		}
		FFT(pColumn, nHeight);
		for (int j = 0; j < nHeight; j++)
		{
			pComplexImage[j * nWidth + i] = pColumn[j];
		}
	}
	// Normalize the magnitude of the FFT
	double maxMagnitude = 0;
	for (int i = 0; i < nHeight; i++)
	{
		for (int j = 0; j < nWidth; j++)
		{
			double magnitude = abs(pComplexImage[i * nWidth + j]);
			if (magnitude > maxMagnitude)
			{
				maxMagnitude = magnitude;
			}
		}
	}
	for (int i = 0; i < nHeight; i++)
	{
		for (int j = 0; j < nWidth; j++)
		{
			double magnitude = abs(pComplexImage[i * nWidth + j]);
			*(m_pDibBits + i * m_nWidthBytes + j) = (unsigned char)(255.0 * magnitude / maxMagnitude);
		}
	}
	// Clean up
	delete[] pComplexImage;
	delete[] pColumn;

}

void CDib::FFT(complex<double>* pComplex, int nLength)
{
	// Base case
	if (nLength <= 1)
	{
		return;
	}
	// Split the array into even and odd parts
	complex<double>* pEven = new complex<double>[nLength / 2];
	complex<double>* pOdd = new complex<double>[nLength / 2];
	for (int i = 0; i < nLength / 2; i++)
	{
		pEven[i] = pComplex[2 * i];
		pOdd[i] = pComplex[2 * i + 1];
	}
	// Recursively compute FFT on even and odd parts
	FFT(pEven, nLength / 2);
	FFT(pOdd, nLength / 2);
	// Combine the results
	for (int i = 0; i < nLength / 2; i++)
	{
		double theta = -2 * Pi * i / nLength;
		complex<double> twiddle = complex<double>(cos(theta), sin(theta)) * pOdd[i];
		pComplex[i] = pEven[i] + twiddle;
		pComplex[i + nLength / 2] = pEven[i] - twiddle;
	}
	// Clean up
	delete[] pEven;
	delete[] pOdd;
}

void CDib::IFFT2D()
{
	// Get the size of the image
	int nWidth = GetWidth();
	int nHeight = GetHeight();
	// Create a complex array to store the image
	complex<double>* pComplexImage = new complex<double>[nWidth * nHeight];
	for (int i = 0; i < nHeight; i++)
	{
		for (int j = 0; j < nWidth; j++)
		{
			pComplexImage[i * nWidth + j] = complex<double>(*(m_pDibBits + i * m_nWidthBytes + j), 0);
		}
	}
	// Perform 1D IFFT on each row
	for (int i = 0; i < nHeight; i++)
	{
		IFFT(pComplexImage + i * nWidth, nWidth);
	}
	// Perform 1D IFFT on each column
	complex<double>* pColumn = new complex<double>[nHeight];
	for (int i = 0; i < nWidth; i++)
	{
		for (int j = 0; j < nHeight; j++)
		{
			pColumn[j] = pComplexImage[j * nWidth + i];
		}
		IFFT(pColumn, nHeight);
		for (int j = 0; j < nHeight; j++)
		{
			pComplexImage[j * nWidth + i] = pColumn[j];
		}
	}
	// Normalize the magnitude of the IFFT
	double maxMagnitude = 0;
	for (int i = 0; i < nHeight; i++)
	{
		for (int j = 0; j < nWidth; j++)
		{
			double magnitude = abs(pComplexImage[i * nWidth + j]);
			if (magnitude > maxMagnitude)
			{
				maxMagnitude = magnitude;
			}
		}
	}
	for (int i = 0; i < nHeight; i++)
	{
		for (int j = 0; j < nWidth; j++)
		{
			double magnitude = abs(pComplexImage[i * nWidth + j]);
			*(m_pDibBits + i * m_nWidthBytes + j) = (unsigned char)(255.0 * magnitude / maxMagnitude);
		}
	}
	// Clean up

	delete[] pComplexImage;
	delete[] pColumn;
}

void CDib::IFFT(complex<double>* pComplex, int nLength)
{
	// Base case
	if (nLength <= 1)
	{
		return;
	}
	// Split the array into even and odd parts
	complex<double>* pEven = new complex<double>[nLength / 2];
	complex<double>* pOdd = new complex<double>[nLength / 2];
	for (int i = 0; i < nLength / 2; i++)
	{
		pEven[i] = pComplex[2 * i];
		pOdd[i] = pComplex[2 * i + 1];
	}
	// Recursively compute IFFT on even and odd parts
	IFFT(pEven, nLength / 2);
	IFFT(pOdd, nLength / 2);
	// Combine the results
	for (int i = 0; i < nLength / 2; i++)
	{
		double theta = 2 * Pi * i / nLength;
		complex<double> twiddle = complex<double>(cos(theta), sin(theta)) * pOdd[i];
		pComplex[i] = pEven[i] + twiddle;
		pComplex[i + nLength / 2] = pEven[i] - twiddle;
	}
	// Clean up
	delete[] pEven;
	delete[] pOdd;
}

void CDib::DegradeImage(double k)
{
	if (m_pDibBits == NULL)
	{
		return;
	}

	// ��ȡͼ���Ⱥ͸߶�  
	int nWidth = GetWidth();
	int nHeight = GetHeight();

	// ������������洢ͼ��  
	complex<double>* pComplexImage = new complex<double>[nWidth * nHeight];
	for (int i = 0; i < nHeight; i++)
	{
		for (int j = 0; j < nWidth; j++)
		{
			// Center the FFT by multiplying by (-1)^(x+y)
			pComplexImage[i * nWidth + j] = complex<double>((*(m_pDibBits + i * m_nWidthBytes + j)) * pow(-1, i + j), 0);
		}
	}
	// Perform 1D FFT on each row
	for (int i = 0; i < nHeight; i++)
	{
		FFT(pComplexImage + i * nWidth, nWidth);
	}
	// Perform 1D FFT on each column
	complex<double>* pColumn = new complex<double>[nHeight];
	for (int i = 0; i < nWidth; i++)
	{
		for (int j = 0; j < nHeight; j++)
		{
			pColumn[j] = pComplexImage[j * nWidth + i];
		}
		FFT(pColumn, nHeight);
		for (int j = 0; j < nHeight; j++)
		{
			pComplexImage[j * nWidth + i] = pColumn[j];
		}
	}

	// Ӧ���˻����� h(u, v) = exp(-k * (u^2 + v^2)^(5/6))  
	for (int i = 0; i < nHeight; i++)
	{
		for (int j = 0; j < nWidth; j++)
		{
			double u = i - nHeight / 2;
			double v = j - nWidth / 2;
			double distance = u * u + v * v;
			double h = exp(-k * pow(distance, 5.0 / 6.0));
			pComplexImage[i * nWidth + j] *= h;
		}
	}

	// Perform 1D IFFT on each column
	for (int i = 0; i < nWidth; i++)
	{
		for (int j = 0; j < nHeight; j++)
		{
			pColumn[j] = pComplexImage[j * nWidth + i];
		}
		IFFT(pColumn, nHeight);
		for (int j = 0; j < nHeight; j++)
		{
			pComplexImage[j * nWidth + i] = pColumn[j];
		}
	}
	// Perform 1D IFFT on each row
	for (int i = 0; i < nHeight; i++)
	{
		IFFT(pComplexImage + i * nWidth, nWidth);
	}

	// Normalize the magnitude of the IFFT
	double maxMagnitude = 0;
	for (int i = 0; i < nHeight; i++)
	{
		for (int j = 0; j < nWidth; j++)
		{
			double magnitude = abs(pComplexImage[i * nWidth + j]);
			if (magnitude > maxMagnitude)
			{
				maxMagnitude = magnitude;
			}
		}
	}
	for (int i = 0; i < nHeight; i++)
	{
		for (int j = 0; j < nWidth; j++)
		{
			double magnitude = abs(pComplexImage[i * nWidth + j]);
			*(m_pDibBits + i * m_nWidthBytes + j) = static_cast<unsigned char>(255.0 * magnitude / maxMagnitude);
		}
	}

	// Clean up
	delete[] pComplexImage;
	delete[] pColumn;
}

void CDib::DegradeColorImage(double k)
{
	if (m_pDibBits == NULL || m_nBitCount != 24) // ȷ����24λ��ɫͼ��  
	{
		return;
	}

	// ��ȡͼ���Ⱥ͸߶�  
	int nWidth = GetWidth();
	int nHeight = GetHeight();

	// ��ÿ����ɫͨ���ֱ���  
	for (int channel = 0; channel < 3; channel++)
	{
		// ������������洢��ǰͨ����ͼ������  
		complex<double>* pComplexImage = new complex<double>[nWidth * nHeight];

		// ��临�����飬ע�����Ļ�����  
		for (int i = 0; i < nHeight; i++)
		{
			for (int j = 0; j < nWidth; j++)
			{
				// ���Ļ�FFT��������(-1)^(i+j)  
				double value = *(m_pDibBits + i * m_nWidthBytes + j * 3 + channel);
				pComplexImage[i * nWidth + j] = complex<double>(value * pow(-1, i + j), 0);
			}
		}

		// ���н���FFT  
		for (int i = 0; i < nHeight; i++)
		{
			FFT(pComplexImage + i * nWidth, nWidth);
		}

		// ���н���FFT  
		complex<double>* pColumn = new complex<double>[nHeight];
		for (int i = 0; i < nWidth; i++)
		{
			for (int j = 0; j < nHeight; j++)
			{
				pColumn[j] = pComplexImage[j * nWidth + i];
			}
			FFT(pColumn, nHeight);
			for (int j = 0; j < nHeight; j++)
			{
				pComplexImage[j * nWidth + i] = pColumn[j];
			}
		}

		// Ӧ���˻����� h(u, v) = exp(-k * (u^2 + v^2)^(5/6))  
		for (int i = 0; i < nHeight; i++)
		{
			for (int j = 0; j < nWidth; j++)
			{
				double u = i - nHeight / 2;
				double v = j - nWidth / 2;
				double distance = u * u + v * v;
				double h = exp(-k * pow(distance, 5.0 / 6.0));
				pComplexImage[i * nWidth + j] *= h;
			}
		}

		// ���н���IFFT  
		for (int i = 0; i < nWidth; i++)
		{
			for (int j = 0; j < nHeight; j++)
			{
				pColumn[j] = pComplexImage[j * nWidth + i];
			}
			IFFT(pColumn, nHeight);
			for (int j = 0; j < nHeight; j++)
			{
				pComplexImage[j * nWidth + i] = pColumn[j];
			}
		}

		// ���н���IFFT  
		for (int i = 0; i < nHeight; i++)
		{
			IFFT(pComplexImage + i * nWidth, nWidth);
		}

		// �ҳ���������ֵ���ڹ�һ��  
		double maxMagnitude = 0;
		for (int i = 0; i < nHeight; i++)
		{
			for (int j = 0; j < nWidth; j++)
			{
				double magnitude = abs(pComplexImage[i * nWidth + j]);
				if (magnitude > maxMagnitude)
				{
					maxMagnitude = magnitude;
				}
			}
		}

		// �������һ����0-255��Χ��д��ԭͼ��  
		for (int i = 0; i < nHeight; i++)
		{
			for (int j = 0; j < nWidth; j++)
			{
				double magnitude = abs(pComplexImage[i * nWidth + j]);
				*(m_pDibBits + i * m_nWidthBytes + j * 3 + channel) = static_cast<unsigned char>(255.0 * magnitude / maxMagnitude);
			}
		}

		// �����ڴ�  
		delete[] pComplexImage;
		delete[] pColumn;
	}
}

void CDib::WienerFilter(double k, double NSR)
{
	if (m_pDibBits == NULL)
	{
		return;
	}

	// ��ȡͼ���Ⱥ͸߶�  
	int nWidth = GetWidth();
	int nHeight = GetHeight();

	// �ж��ǲ�ɫͼ���ǻҶ�ͼ��
	bool isColorImage = (m_nBitCount == 24);
	int bytesPerPixel = isColorImage ? 3 : 1;
	int channels = isColorImage ? 3 : 1;

	// ��ÿ����ɫͨ���ֱ���
	for (int channel = 0; channel < channels; channel++)
	{
		// ������������洢ͼ��
		complex<double>* pComplexImage = new complex<double>[nWidth * nHeight];

		// ��临�����鲢ִ�����Ļ�
		for (int i = 0; i < nHeight; i++)
		{
			for (int j = 0; j < nWidth; j++)
			{
				double value = 0;
				if (isColorImage)
					value = *(m_pDibBits + i * m_nWidthBytes + j * bytesPerPixel + channel);
				else
					value = *(m_pDibBits + i * m_nWidthBytes + j);

				// ���Ļ�FFT��������(-1)^(i+j)
				pComplexImage[i * nWidth + j] = complex<double>(value * pow(-1, i + j), 0);
			}
		}

		// ���н���FFT
		for (int i = 0; i < nHeight; i++)
		{
			FFT(pComplexImage + i * nWidth, nWidth);
		}

		// ���н���FFT
		complex<double>* pColumn = new complex<double>[nHeight];
		for (int i = 0; i < nWidth; i++)
		{
			for (int j = 0; j < nHeight; j++)
			{
				pColumn[j] = pComplexImage[j * nWidth + i];
			}
			FFT(pColumn, nHeight);
			for (int j = 0; j < nHeight; j++)
			{
				pComplexImage[j * nWidth + i] = pColumn[j];
			}
		}

		// Ӧ��ά���˲�
		// �����˻�����������ʹ�ø�˹ģ��ģ��
		for (int i = 0; i < nHeight; i++)
		{
			for (int j = 0; j < nWidth; j++)
			{
				double u = i - nHeight / 2;
				double v = j - nWidth / 2;
				double distance = u * u + v * v;

				// �˻����� H(u,v) = exp(-k * (u^2 + v^2)^(5/6))  
				double h = exp(-k * pow(distance, 5.0 / 6.0));

				// ά���˲���ʽ��G(u,v) = (H*(u,v) / (|H(u,v)|^2 + Sn/Si)) * F(u,v)  
				double hMagnitudeSquared = abs(h) * abs(h);
				double wienerFactor = hMagnitudeSquared / (hMagnitudeSquared + NSR);
				pComplexImage[i * nWidth + j] *= wienerFactor / h;
			}
		}

		// ���н���IFFT
		for (int i = 0; i < nWidth; i++)
		{
			for (int j = 0; j < nHeight; j++)
			{
				pColumn[j] = pComplexImage[j * nWidth + i];
			}
			IFFT(pColumn, nHeight);
			for (int j = 0; j < nHeight; j++)
			{
				pComplexImage[j * nWidth + i] = pColumn[j];
			}
		}

		// ���н���IFFT
		for (int i = 0; i < nHeight; i++)
		{
			IFFT(pComplexImage + i * nWidth, nWidth);
		}

		// ��һ����ȡ��ֵ
		double maxMagnitude = 0;
		for (int i = 0; i < nHeight; i++)
		{
			for (int j = 0; j < nWidth; j++)
			{
				// ȥ�����Ļ�Ч��������(-1)^(i+j)
				pComplexImage[i * nWidth + j] *= pow(-1, i + j);
				double magnitude = abs(pComplexImage[i * nWidth + j]);
				if (magnitude > maxMagnitude)
				{
					maxMagnitude = magnitude;
				}
			}
		}

		// �����д��ԭͼ��
		for (int i = 0; i < nHeight; i++)
		{
			for (int j = 0; j < nWidth; j++)
			{
				double magnitude = abs(pComplexImage[i * nWidth + j]);
				unsigned char value = static_cast<unsigned char>(min(255.0, max(0.0, 255.0 * magnitude / maxMagnitude)));

				if (isColorImage)
					*(m_pDibBits + i * m_nWidthBytes + j * bytesPerPixel + channel) = value;
				else
					*(m_pDibBits + i * m_nWidthBytes + j) = value;
			}
		}

		// �����ڴ�
		delete[] pComplexImage;
		delete[] pColumn;
	}
}
