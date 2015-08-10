#pragma once

// CTransSocket command target
#define WM_RAWIMG_NOTICE		(WM_USER+0x82)

 class CTransSocket
{
    
public:

    CTransSocket();
    
	
	unsigned int   m_nPackIndex;
public:
	unsigned char *m_pRawImg;
public:
	unsigned char* m_pPackBuf;

	
	unsigned long  m_nStreamLen;
    
	unsigned char* m_pStreamBuf;
    
	unsigned char m_nNeedFrame;
    
	unsigned char m_nNeedLine;
    
	bool Package(unsigned char* pucPack, unsigned long ulPackLen);


	int m_nLength;
	

	void SendRawImage();
    
    
};


