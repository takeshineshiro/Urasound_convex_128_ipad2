
// WLBScanDesktopDlg.cpp :  µœ÷Œƒº˛
//

#include "WLBScanDesktopDlg.h"

#include <math.h>

#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ”√”⁄”¶”√≥Ã–Ú°∞πÿ”⁄°±≤Àµ•œÓµƒ CAboutDlg ∂‘ª∞øÚ





// CWLBScanDesktopDlg ∂‘ª∞øÚ







// CWLBScanDesktopDlg œ˚œ¢¥¶¿Ì≥Ã–Ú

CWLBScanDesktopDlg::CWLBScanDesktopDlg()
{

	// Ω´°∞πÿ”⁄...°±≤Àµ•œÓÃÌº”µΩœµÕ≥≤Àµ•÷–°£

	// IDM_ABOUTBOX ±ÿ–Î‘⁄œµÕ≥√¸¡Ó∑∂Œßƒ⁄°£
	

	// …Ë÷√¥À∂‘ª∞øÚµƒÕº±Í°£  µ±”¶”√≥Ã–Ú÷˜¥∞ø⁄≤ª «∂‘ª∞øÚ ±£¨øÚº‹Ω´◊‘∂Ø
	//  ÷¥––¥À≤Ÿ◊˜
			// …Ë÷√–°Õº±Í

	// TODO:  ‘⁄¥ÀÃÌº”∂ÓÕ‚µƒ≥ı ºªØ¥˙¬Î
	m_pImage = new unsigned char[640 * 480 * 4];
    
	memset(m_pImage, 0x00, 640 * 480 * 4);
    
	m_pDscIndex = new DSC_INDEX[640 * 480];
    
	memset(m_pDscIndex, 0, sizeof(DSC_INDEX)* 640 * 480);

	InitDscIndex();

	m_bRunning = false;

}



// »Áπ˚œÚ∂‘ª∞øÚÃÌº”◊Ó–°ªØ∞¥≈•£¨‘Ú–Ë“™œ¬√Êµƒ¥˙¬Î
//  ¿¥ªÊ÷∆∏√Õº±Í°£  ∂‘”⁄ π”√Œƒµµ/ ”Õºƒ£–Õµƒ MFC ”¶”√≥Ã–Ú£¨
//  ’‚Ω´”…øÚº‹◊‘∂ØÕÍ≥…°£



//µ±”√ªßÕœ∂Ø◊Ó–°ªØ¥∞ø⁄ ±œµÕ≥µ˜”√¥À∫Ø ˝»°µ√π‚±Í
//œ‘ æ°£



//	≥ı ºªØDSCœµ ˝

#define PI		(3.14159265359)

#define WIDTH   640

#define HEIGHT  480

void CWLBScanDesktopDlg::InitDscIndex()

{
	double dbAngle       = 80.0 / 180.0*PI;	//	…®√ËΩ«∂»Œ™80∂»
    
	double dbDeadRgn     = 15.0;			//	À¿«¯≥§∂»Œ™15mm
    
	double dbScaleSample = 780.0 / 2000000.0*1000.0;	//	√ø∏ˆ≤…—˘µ„µƒ≥§∂»£¨µ•Œª£∫ mm£¨≤…—˘¬ Œ™2MHz
    
	double dbScalePixel  = ((1.0 - cos(dbAngle / 2.0))*dbDeadRgn + 512.0*dbScaleSample) / (double)HEIGHT;
    
	double dbCenterY     = -cos(dbAngle / 2.0)*dbDeadRgn/dbScalePixel;
    
	double dbCenterX     = (double)WIDTH / 2.0;

	/*
	//	debug
	for (int x = 0; x < 640; x++) {
		if (x >= 256) {
			for (int y = 0; y < 480; y++) {
				m_pDscIndex[y * 640 + x].bIn = false;
			}
		}
		else {
			for (int y = 0; y < 480; y++) {
				m_pDscIndex[y * 640 + x].bIn = true;
				m_pDscIndex[y * 640 + x].nLine = x / 2;
				m_pDscIndex[y * 640 + x].nPixel = (int)(y*(480.0/512.0));
				m_pDscIndex[y * 640 + x].fPartLine = 1.0;
				m_pDscIndex[y * 640 + x].fPartPixel = 1.0;

				if (x / 2 >= 127)
					m_pDscIndex[y * 640 + x].bIn = false;
				if (y >= 511)
					m_pDscIndex[y * 640 + x].bIn = false;
			}
		}
	}
	*/

	
	double dbAngleGrid = dbAngle / 127.0;
    
    
	for (int x = 0; x < WIDTH; x++ )
         
    {
		for (int y = 0; y < HEIGHT; y++)
        {
			double dbPosX = x - dbCenterX;
            
			double dbPosY = y - dbCenterY;
            
			double angle  = atan(dbPosX / dbPosY);
            
			if ((angle <= -dbAngle/2.0)
                
				|| (angle >= dbAngle/2.0)
				)
			{
				m_pDscIndex[y * WIDTH + x].bIn = false;
				continue;
			}

			m_pDscIndex[y * WIDTH + x].bIn = true;
			double dbLine = (angle - (-dbAngle / 2.0)) / dbAngleGrid;
			m_pDscIndex[y * WIDTH + x].nLine = (int)dbLine;
			m_pDscIndex[y * WIDTH + x].fPartLine = 1.0 - (dbLine - (double)((int)dbLine));

			double len = sqrt(dbPosX*dbPosX + dbPosY*dbPosY) * dbScalePixel;
			double samp = (len-dbDeadRgn) / dbScaleSample;
			if (samp <= 0.0 || samp >= 511.0) {
				m_pDscIndex[y * WIDTH + x].bIn = false;
				continue;
			}
			m_pDscIndex[y * WIDTH + x].nPixel = (int)samp;
			m_pDscIndex[y * WIDTH + x].fPartPixel =1.0 - (samp - (double)((int)samp));
			if (m_pDscIndex[y * WIDTH + x].nLine >= 127)
				m_pDscIndex[y * WIDTH + x].bIn = false;
			if (m_pDscIndex[y*WIDTH+x].nPixel >= 511)
				m_pDscIndex[y * WIDTH + x].bIn = false;
            
            int nPos = y*WIDTH+x;
            if (m_pDscIndex[nPos].bIn) {
                m_pDscIndex[nPos].ulPartLine = (unsigned long)(m_pDscIndex[nPos].fPartLine*256.0);
                m_pDscIndex[nPos].ulPartPixel = (unsigned long)(m_pDscIndex[nPos].fPartPixel*256.0);
            }
		}
	}
}

//	º∆À„DSC£¨∆‰Ω·π˚ ‰≥ˆµΩ m_pImage ÷–
void CWLBScanDesktopDlg::DscImage(unsigned char* pRawData)
{
    static unsigned char g = 0; g += 32;

    unsigned long cmp_line,row_line;
    unsigned long gray;
	//for (int x = 0; x < WIDTH; x++) {
	//	for (int y = 0; y < HEIGHT; y++) {
	//		int nPos = y * WIDTH + x;
    memset(m_pImage,0,WIDTH*HEIGHT*4);
    for (int nPos=0;nPos<WIDTH*HEIGHT;++nPos)
    {
        DSC_INDEX dscindex = m_pDscIndex[nPos];
			if (dscindex.bIn) {
				unsigned long tl, tr;
                unsigned long npp = (dscindex.nLine << 9) + dscindex.nPixel;
                tl = pRawData[npp];

                npp += 512;
                tr = pRawData[npp];
                
				
                row_line = dscindex.ulPartLine;
                //cmp_pixel = (256 - dscindex.ulPartPixel);
                cmp_line = (256 - dscindex.ulPartLine);
                //l = tl * row_pixel + bl * cmp_pixel;
                //r = tr * row_pixel + br * cmp_pixel;
                //gray = l * row_line + r * cmp_line;
                gray = tl * row_line + tr * cmp_line;
                unsigned char rgb = (gray >> 8) & 0x00FF;
                
                int nPixelPos = nPos << 2;
                m_pImage[nPixelPos] = m_pImage[nPixelPos+1] = m_pImage[nPixelPos+2] = rgb;
                m_pImage[nPixelPos+3] = 255;
                
                //gray >>= 16;
                //if (gray > 255) {
                //gray = 255;
                //}
                //if (gray < 0.0)
                //gray = 0.0;
                //else if (gray > 255.0)
                //    gray = 255.0;
                
				//m_pImage[nPos] = (gray >> 16) & 0x00FF;
			}
			else {
                m_pImage[nPos*4+3] = 255;
                
                //
				//m_pImage[nPos] = 0x5A;
			}
    }
    //	}
	//}
	
	//memcpy(m_pImage, pRawData, 512 * 128);
}

//	ªÊ÷∆ÕºœÒ






