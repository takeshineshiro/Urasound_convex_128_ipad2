#include  "DSCor.h"

#include  <string.h>

#include  <math.h>


CDSCor* CDSCor::s_pInst = NULL;

CDSCor::CDSCor(void)
	:m_pDscIndex(NULL)
	,m_pDSCImage(NULL)
	,m_nProbeType(-1)
	,m_nZoomIndex(-1)

{
    
	m_pDscIndex = new DSC_INDEX[DSC_WIDTH/2*DSC_HEIGHT];         //320*480
    
    
	m_pDSCImage = new unsigned char[DSC_WIDTH*DSC_HEIGHT*4];     //640*480*4
    
}


CDSCor::~CDSCor(void)
{
	if (m_pDscIndex) {
        
		delete[]m_pDscIndex;
	}
	if (m_pDSCImage) {
        
		delete[]m_pDSCImage;
	}
   
}



CDSCor* CDSCor::GetInstance() {
    if (s_pInst == NULL) {
        s_pInst = new CDSCor;
    }
    return s_pInst;
}


void CDSCor::ClearInstance() {
    if (s_pInst) {
        delete s_pInst;
        s_pInst = NULL;
    }
}


bool CDSCor::InitGama(float gama)            //灰阶变换
{
    
    if (gama < 0.1 || gama > 4.0)
        
        return false;
    
    if (gama == m_fGama)
        
        return true;
    
    float e = -0.05;
    
    m_fGama = gama;
    
    for (int i=0;i<256;i++) {
        
        float x = (float)i/255.0;
        
        float y = pow(x+e,m_fGama);
        
        if (y<0)
            
            y = 0.0;
        
        if (y>1.0)
            
            y = 1.0;
        
        y *= 255.0*(256.0);
        
        m_unGama[i] = (int)y;
        
    }
    return true;
}





unsigned int* CDSCor::GetGama()
{
    return m_unGama;
}


#define PI	3.14159265359

bool CDSCor::InitDSC(int nProbeType,int nZoom)
{
	//	0. »∑∂®DSC∑Ω Ω
	if (nProbeType == m_nProbeType && nZoom == m_nZoomIndex)
		return true;
    
	m_nProbeType = nProbeType;
    
	m_nZoomIndex = nZoom;

	//
	//	1. ∏˘æ›ÃΩÕ∑¿‡–Õ”ÎÀı∑≈≤Œ ˝£¨»∑∂®ª˘±æ≤Œ ˝
	//
	double dbSectorAngle;	//	…®√ËΩ«∂»£¨µ•Œª£∫Ω«∂»
    
	double dbDeadRgn;		//	À¿«¯≥§∂»£¨µ•Œª£∫mm
    
	double dbSampleRate;	//	‘≠ º ˝æ›µ„∂‘”¶µƒ≤…—˘¬ £¨µ•ŒªHz
    
	int    nUseSampleInLine = RAWDATA_SAMPLE_IN_LINE;	//	DSC÷–£¨ µº  π”√µƒ√ø∏˘œﬂ…œµƒ≤…—˘µ„ ˝( <= RAWDATA_SAMPLE_IN_LINE )
    
	if (PROBE_SECTORSCAN == nProbeType) {	//	Œﬁœﬂª˙…®ÃΩÕ∑
        
		dbSectorAngle = 80.0;		//	…®√ËΩ«∂»80°„
        
		dbDeadRgn = 15.0;			//	À¿«¯≥§∂»Œ™15mm
        
		dbSampleRate = 2222222.0;	//	≤…—˘¬ Œ™2MHz
        
		switch (nZoom) {
		case 0:
                
			nUseSampleInLine = 280; //  102mm
                
			break;
		case 1:
			nUseSampleInLine = 392; //  141mm
			break;
		case 2:
			nUseSampleInLine = 450; //  161mm
			break;
		case 3:
			nUseSampleInLine = 512;	//	183mm RAWDATA_SAMPLE_IN_LINE
			break;
		}
	}
	else if (PROBE_SECTORARRAY == nProbeType) {	//	ŒﬁœﬂÕπ’ÛÃΩÕ∑
		//dbSectorAngle = 78.0;	//	Õπ’Û£∫78°„
		//dbDeadRgn = 41.0;		//	Õπ’Û∞Îæ∂£∫41.0mm
        dbSectorAngle = 60.0;   //  Angle:60度
        
        dbDeadRgn = 60.0;       //  R:60mm
        
		switch (nZoom)
		{
            case 0:
                dbSampleRate = 4545454.0;   //  50MHz div 10+1
                break;
            case 1:
                dbSampleRate = 3125000.0;   //  50MHz div 15+1
                break;
            case 2:
                dbSampleRate = 2500000.0;   //  50MHz div 19+1
                break;
            case 3:
                dbSampleRate = 2000000.0;   //  50MHz div 24+1
                break;
            default:
                return false;
                break;
		}
	}
	else {
		return false;
	}

	//
	//	2. º∆À„∏®÷˙≤Œ ˝
	//

	//	…®√ËΩ«∂»£∫ª°∂»
    
	double dbAngle = dbSectorAngle / 180.0 * PI;
    
    
	//	√ø∏ˆ≤…—˘µ„∂‘”¶µƒŒÔ¿Ìæ‡¿Î£¨µ•Œª£∫ mm
    
	double dbScaleSample = 1560.0 / 2.0 / dbSampleRate * 1000.0;
    
	//	DSC∫Ûµƒ√ø∏ˆœÒÀÿµ„∂‘”¶µƒŒÔ¿Ìæ‡¿Î£¨µ•Œª£∫mm
    
	double dbScalePixel = ((1.0 - cos(dbAngle / 2.0))*dbDeadRgn + nUseSampleInLine*dbScaleSample) / (double)DSC_HEIGHT;
    
	//	…®√Ë…»–Œ÷––ƒµ„◊¯±Í£¨µ•ŒªŒ™œÒÀÿµ„
	double dbCenterY = -cos(dbAngle / 2.0)*dbDeadRgn/dbScalePixel;
    
	double dbCenterX = (double)(DSC_WIDTH-1) / 2.0;
    
	//	œ‡¡⁄…®√ËœﬂµƒΩ«∂»º‰∏Ù£¨µ•Œª£∫ ª°∂»
    
	//double dbAngleGrid = dbAngle / 127.0;
    
    double dbAngleGrid = dbAngle / 255.0;

	//
	//	3. º∆À„œµ ˝
	//
	int nIndexPos = 0;
    
	for (int y=0; y<DSC_HEIGHT;y++){
        
		for (int x=0; x<DSC_WIDTH/2;x++){
            
			double dbPosX = x - dbCenterX;
            
			double dbPosY = y - dbCenterY;

			//	Ω«∂»∑¥œÚœﬁ÷∆
            
			double angle = atan(dbPosX / dbPosY);
            
			if (angle <= -dbAngle/2.0) {        //左边弧度一半
                
				m_pDscIndex[nIndexPos].bIn = false;
                
				nIndexPos++;
                
				continue;
			}
            
			m_pDscIndex[nIndexPos].bIn = true;
            
			double dbLine = (angle - (-dbAngle / 2.0)) / dbAngleGrid;
            
			m_pDscIndex[nIndexPos].ulLine = (unsigned long)dbLine;
            
			m_pDscIndex[nIndexPos].ulPartLine = (unsigned long)((1.0 - (dbLine - (double)((int)dbLine)))*255.99);


			//	≥§∂»£®∞Îæ∂£©∑ΩœÚœﬁ÷∆
			double len = sqrt(dbPosX*dbPosX + dbPosY*dbPosY) * dbScalePixel;
            
			double samp = (len-dbDeadRgn) / dbScaleSample;
            
			if (samp <= 0.0 || samp >= 511.0) {
                
				m_pDscIndex[nIndexPos].bIn = false;
                
				nIndexPos++;
                
				continue;
			}
			m_pDscIndex[nIndexPos].ulPixel     = (unsigned long)samp;
            
			m_pDscIndex[nIndexPos].ulPartPixel = (unsigned long)((1.0 - (samp - (double)((int)samp)))*255.99);
            
			
			//	∂ÓÕ‚µƒœﬁ÷∆Ãıº˛
			if (m_pDscIndex[nIndexPos].ulLine >= 255)
                
				m_pDscIndex[nIndexPos].bIn = false;
            
			if (m_pDscIndex[nIndexPos].ulPixel >= 511)
                
				m_pDscIndex[nIndexPos].bIn = false;

			nIndexPos++;
		}
	}
	return true;
}





//	ªÒ»°DSC∫ÛÕºœÒµƒ±»¿˝≥ﬂ£¨µ•Œª£∫ mm/pixel£¨∑µªÿ∏∫ ˝±Ì æº∆À„ ß∞‹
double CDSCor::GetScale(int nProbeType,int nZoom)
{
	//
	//	1. ∏˘æ›ÃΩÕ∑¿‡–Õ”ÎÀı∑≈≤Œ ˝£¨»∑∂®ª˘±æ≤Œ ˝
	//
	double dbSectorAngle;	//	…®√ËΩ«∂»£¨µ•Œª£∫Ω«∂»
    
	double dbDeadRgn;		//	À¿«¯≥§∂»£¨µ•Œª£∫mm
    
	double dbSampleRate;	//	‘≠ º ˝æ›µ„∂‘”¶µƒ≤…—˘¬ £¨µ•ŒªHz
    
	int    nUseSampleInLine = RAWDATA_SAMPLE_IN_LINE;	//	DSC÷–£¨ µº  π”√µƒ√ø∏˘œﬂ…œµƒ≤…—˘µ„ ˝( <= RAWDATA_SAMPLE_IN_LINE )
    
    
	if (PROBE_SECTORSCAN == nProbeType) {	//	Œﬁœﬂª˙…®ÃΩÕ∑
        
		dbSectorAngle = 80.0;		//	…®√ËΩ«∂»80°„
        
		dbDeadRgn = 15.0;			//	À¿«¯≥§∂»Œ™15mm
        
		dbSampleRate = 2222222.0;	//	≤…—˘¬ Œ™2MHz
        
		switch (nZoom) {
		case 0:
			nUseSampleInLine = 280; //  102mm
			break;
		case 1:
			nUseSampleInLine = 392; //  141mm
			break;
		case 2:
			nUseSampleInLine = 450; //  161mm
			break;
		case 3:
			nUseSampleInLine = 512;	//	183mm RAWDATA_SAMPLE_IN_LINE
			break;
		}
	}
	else if (PROBE_SECTORARRAY == nProbeType) {	//	ŒﬁœﬂÕπ’ÛÃΩÕ∑
        
		dbSectorAngle = 78.0;	//	Õπ’Û£∫78°„
        
		dbDeadRgn = 41.0;		//	Õπ’Û∞Îæ∂£∫41.0mm
        
		switch (nZoom)
		{
		case 0:
			dbSampleRate = 4545454.0;   //  50MHz div 10+1
			break;
		case 1:
			dbSampleRate = 3125000.0;   //  50MHz div 15+1
			break;
		case 2:
			dbSampleRate = 2500000.0;   //  50MHz div 19+1
			break;
		case 3:
			dbSampleRate = 2000000.0;   //  50MHz div 24+1
			break;
		default:
			return -1;
			break;
		}
	}
	else {
		return -1;
	}

	//
	//	2. º∆À„∏®÷˙≤Œ ˝
	//

	//	…®√ËΩ«∂»£∫ª°∂»
	double dbAngle = dbSectorAngle / 180.0 * PI;
    
	//	√ø∏ˆ≤…—˘µ„∂‘”¶µƒŒÔ¿Ìæ‡¿Î£¨µ•Œª£∫ mm
	double dbScaleSample = 1560.0 / 2.0 / dbSampleRate * 1000.0;
    
	//	DSC∫Ûµƒ√ø∏ˆœÒÀÿµ„∂‘”¶µƒŒÔ¿Ìæ‡¿Î£¨µ•Œª£∫mm
	double dbScalePixel = ((1.0 - cos(dbAngle / 2.0))*dbDeadRgn + nUseSampleInLine*dbScaleSample) / (double)DSC_HEIGHT;
    
	//	…®√Ë…»–Œ÷––ƒµ„◊¯±Í£¨µ•ŒªŒ™œÒÀÿµ„
	//double dbCenterY = -cos(dbAngle / 2.0)*dbDeadRgn/dbScalePixel;
	//double dbCenterX = (double)(DSC_WIDTH-1) / 2.0;
	//	œ‡¡⁄…®√ËœﬂµƒΩ«∂»º‰∏Ù£¨µ•Œª£∫ ª°∂»
	//double dbAngleGrid = dbAngle / 127.0;

	return dbScalePixel;
    
}

unsigned char* CDSCor::DSCImage(unsigned char* pRawData)
{
	
    
    memset(m_pDSCImage,0,DSC_WIDTH*DSC_HEIGHT*4);
    
    
//    int npos=0;
//    for (int i=0;i<DSC_WIDTH*DSC_HEIGHT;i++) {
//        m_pDSCImage[npos] = 0xFF;
//        npos += 4;
//    }

	int nDSCIndexPos = 0;
    
    
	int nDSCImagePos = 0;
    
    
    
	//int nRawDataPos = 0;
    
    
	for (int y=0;y<DSC_HEIGHT;y++)
     {
        
		
         nDSCImagePos = y * DSC_WIDTH * 4;                //一行数据量
         
        
		for (int x=0;x<DSC_WIDTH/2;x++) {                 //对称
			//	≤ª‘⁄ÕºœÒ«¯”Úƒ⁄
            
            
			DSC_INDEX* pDscIndex = m_pDscIndex + nDSCIndexPos;
            
            
            
			if (pDscIndex->bIn) {	//	‘⁄ÕºœÒ«¯”Úƒ⁄
                
				int           nImagePos;
                
				unsigned long tl,tr,bl,br;
                
				unsigned long l,r;
                
				unsigned long gray;
                
				//	◊Û≤‡∞Î÷°
				{                                          //左半部
					
                    
                    nImagePos = nDSCImagePos + (x<<2);    //一次移动4个字节
                    
                    
					tl = (pDscIndex->ulLine << 9) + pDscIndex->ulPixel;    //???// top left
                    
                    
					tr = tl + RAWDATA_SAMPLE_IN_LINE;                      //top  right
                    
                    
					bl = tl + 1;                                         // buttom    left
                    
                    
					br = tr + 1;                                        // button   right
                    
					
					tl = pRawData[tl];	tr = pRawData[tr];            //映射？？//
                    
                    
					bl = pRawData[bl];	br = pRawData[br];
                    
                    
                    tl = m_unGama[tl];  tr = m_unGama[tr];          //映射？？//
                    
                    
                    bl = m_unGama[bl];  br = m_unGama[br];
                    

					l = (tl*pDscIndex->ulPartPixel + bl*(255-pDscIndex->ulPartPixel)) >> 8;
                    
                    
					r = (tr*pDscIndex->ulPartPixel + br*(255-pDscIndex->ulPartPixel)) >> 8;
                    
					
					gray = (l*pDscIndex->ulPartLine + r*(255 - pDscIndex->ulPartLine)) >> 16;
                    
                    
					if (gray > 255)
                        
						gray = 255;
                    
                    
					m_pDSCImage[nImagePos] = m_pDSCImage[nImagePos+1] = m_pDSCImage[nImagePos+2] = (unsigned char)gray;
                    
                    
					m_pDSCImage[nImagePos+3] = 255;
                    
                    
				}
				
				{                                                    //右半部
					
                    nImagePos = nDSCImagePos + ((DSC_WIDTH-1-x)<<2);
                    
                    
					tr = ((RAWDATA_LINE_IN_FRAME-1-pDscIndex->ulLine) << 9) + pDscIndex->ulPixel;
                    
                    
					tl = tr - RAWDATA_SAMPLE_IN_LINE;
                    
                    
					bl = tl + 1;
                    
                    
					br = tr + 1;
                    

					tl = pRawData[tl];	tr = pRawData[tr];
                    
                    
					bl = pRawData[bl];	br = pRawData[br];
                    
                    
                    
                    tl = m_unGama[tl];  tr = m_unGama[tr];
                    
                    
                    bl = m_unGama[bl];  br = m_unGama[br];
                    
	
					l = (tl*pDscIndex->ulPartPixel + bl*(255-pDscIndex->ulPartPixel)) >> 8;
                    
                    
					r = (tr*pDscIndex->ulPartPixel + br*(255-pDscIndex->ulPartPixel)) >> 8;
                    

					gray = (l*(255 - pDscIndex->ulPartLine) + r*pDscIndex->ulPartLine) >> 16;
                    
                    
					m_pDSCImage[nImagePos] = m_pDSCImage[nImagePos+1] = m_pDSCImage[nImagePos+2] = (unsigned char)gray;
                    
                    
					m_pDSCImage[nImagePos+3] = 255;
                    
				}
            
			}
            else {
                
                
                int nImagePos ;
                nImagePos = nDSCImagePos + (x<<2);          //一次移动4个字节  左边
                
                
                m_pDSCImage[nImagePos+3] = 255;
                
                
                nImagePos = nDSCImagePos + ((DSC_WIDTH-1-x)<<2);  //一次移动4个字节，右边
                
                
                m_pDSCImage[nImagePos+3] = 255;
                
            }
			nDSCIndexPos++;
		}
	}
    
    /*
    //  灰度条
    for (int y=0;y<DSC_HEIGHT;y++) {
        int gray = (int) ( ((float)(DSC_HEIGHT - y-1) / (float)(DSC_HEIGHT-1) ) * 255.0 );
        if (gray < 0)
            gray = 0;
        if (gray > 255)
            gray = 255;
        gray = m_unGama[gray] >> 8;
        
        int nPos = (y*DSC_WIDTH + 10) * 4;
        for (int nc=0;nc<8;nc++) {
            m_pDSCImage[nPos] = gray;
            m_pDSCImage[nPos+1] = gray;
            m_pDSCImage[nPos+2] = gray;
            m_pDSCImage[nPos+3] = 255;
            nPos += 4;
        }
    }
    */
    
    
	return m_pDSCImage;
}


