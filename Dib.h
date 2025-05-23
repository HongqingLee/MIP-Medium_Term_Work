#pragma once
#include "atlimage.h"

#include <complex>
#include <cmath>
#include <iostream>
#include <random>
#include <afxdlgs.h> // For CColorDialog
using namespace std;

#define  MAX_SIZE 1000
#define Pi 3.1415926535897932354626
#define Ei 2.71828

class CDib :
	public CImage
{
public:
	CDib(void);
	CDib(CDib &Dib);
	~CDib(void);

public:
	enum chan_color {chan_blue=0,chan_green, chan_red};
	enum HSIChannel{HUE=0,SATURATION,INTENSITY};

public:
	void LoadFile(LPCTSTR lpszPathName);
	CSize GetActualSize() const;
	int GetBitDepth() const;
	long GetPixelCount() const;
	void Invert();
	long* GrayValueCount();
	void ShowColorChannel(int channel);
	void LinearTransform();
	void ShowBitPlane(int bitPlane);
	void Restore();
	void Equalize();
    void Smooth();
	void Sharp();
	void LaplaceTransform();
	void EnhanceLaplace();
	void CreateColorImageWithCircles(int width, int height);
	void GenerateHueImage();
	void GenerateSaturationImage();
	void GenerateBrightnessImage();
	void ConvertRGBToHSI();
	void ConvertHSIToRGB();
	void EqualizeColorImage();
	void AddGaussianNoise(double mean, double stddev);
	void GuassianFilter();
	void AddSaltAndPepperNoise(double noiseRatio);
	void ApplyMedianFilter();
	void EnhanceLaplaceColorImage();
	void ApplyPseudoColor();
	void FFT2D();
	void FFT(complex<double>* pComplex, int nLength);
	void IFFT2D();
	void IFFT(complex<double>* pComplex, int nLength);
	void DegradeImage(double k);
	void DegradeColorImage(double k);
	void WienerFilter(double k, double NSR);
	
private:
	long m_nWidth;
	long m_nHeight;
	int m_nWidthBytes;
	int m_nBitCount;
	unsigned char *m_pDibBits;
	unsigned char* originalDibBits;
	long *m_pGrayValueCount;
	
};

