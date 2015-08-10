
//

#pragma once

#include "TransSocket.h"

class CWLBScanDesktopDlg
{
   

public:
    
	 CWLBScanDesktopDlg();
    


	
	bool	       m_bRunning;

	CTransSocket  m_TransSocket;


    
	typedef struct _tagDSC_INDEX{
		bool          bIn;
		int           nLine;
		int	          nPixel;
		float         fPartLine;
		float         fPartPixel;
        unsigned long ulPartLine;
        unsigned long ulPartPixel;
        
    } DSC_INDEX;
    
    

	DSC_INDEX* m_pDscIndex;
	
	void            InitDscIndex();

	unsigned char*  m_pImage;
	
	void            DscImage(unsigned char* pRawData);
	
	
};
