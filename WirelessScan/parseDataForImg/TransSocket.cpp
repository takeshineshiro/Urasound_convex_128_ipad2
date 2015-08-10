// TransSocket.cpp : implementation file
//

#include "TransSocket.h"
#include <string>
#include <assert.h>

// CTransSocket


CTransSocket ::CTransSocket(){
	
    m_pRawImg = new unsigned char[64 * 2 * 1024];
    
	m_pPackBuf = new unsigned char[4200];
    
	m_nStreamLen = 0;
	m_pStreamBuf = new unsigned char[128*1024];

	m_nLength = 0;
    
    m_nPackIndex = 0;
    
    m_nNeedFrame = 0;
    m_nNeedLine = 0;

}



// CTransSocket member functions





bool CTransSocket::Package(unsigned char* pucPack, unsigned long ulPackLen)
{

	bool bRet = false;
    if (!(ulPackLen+m_nStreamLen<128*1024)) {
        return false;
    }
	assert(ulPackLen + m_nStreamLen < 128*1024);
    
	memcpy(m_pStreamBuf + m_nStreamLen, pucPack, ulPackLen);
	m_nStreamLen += ulPackLen;
    
     


	while (m_nStreamLen >= 525) {


		int head = 0;
		for (head = 0; head <= m_nStreamLen - 8; head++) {
			if ((m_pStreamBuf[head] == 0x5A)
				&& (m_pStreamBuf[head + 1] == 0xA5)
				&& (m_pStreamBuf[head + 2] == 0xFF)
				&& (m_pStreamBuf[head + 3] == 0x00)
				&& (m_pStreamBuf[head + 4] == 0x5A)
				&& (m_pStreamBuf[head + 5] == 0xA5)
				&& (m_pStreamBuf[head + 6] == 0xFF)
				&& (m_pStreamBuf[head + 7] == 0x00)      //查找帧头
				)
			{

				if (head > 0) {
					for (int b = 0; b < m_nStreamLen - head; b++) {
						m_pStreamBuf[b] = m_pStreamBuf[b + head];    //重新组帧
					}

					m_nStreamLen -= head;
				}
				break;
			}
		}


		if (head >= m_nStreamLen - 8) {

			memmove(m_pStreamBuf, m_pStreamBuf + head, m_nStreamLen - head); //重新组帧
			m_nStreamLen -= head;
			break;
		}


		if (m_nStreamLen >= 525) {
           
    

			unsigned char nCurFrame = m_pStreamBuf[8];           //第8帧传的是帧编号
			unsigned char nCurLine = m_pStreamBuf[9];            //第9帧传的是扫描线编号
            
            
            nCurLine |= ( (nCurFrame & 0x01) << 7 ) ;            //
            nCurFrame >>= 1;                                     // ??//
            



			if (	(m_nNeedFrame != nCurFrame)
				||	(m_nNeedLine != nCurLine)
				)
			{



				m_nNeedFrame = nCurFrame;
                
				m_nNeedLine = 0;
                





			}

			if (	(m_nNeedFrame != nCurFrame)
				||	(m_nNeedLine != nCurLine)
				)
			{




				if (m_nStreamLen > 525) {
					memmove(m_pStreamBuf, m_pStreamBuf + 525, m_nStreamLen - 525);       //丢帧
				}
				m_nStreamLen -= 525;
			}
			else {
                


				assert(m_nNeedLine < 256);
					
                memcpy(m_pRawImg + 512 * m_nNeedLine, m_pStreamBuf + 10, 512);         // 10 
 

                
                assert(m_nStreamLen < 128 * 1024);
                
				if (m_nStreamLen > 525)
					memmove(m_pStreamBuf, m_pStreamBuf + 525, m_nStreamLen - 525);
				m_nStreamLen -= 525;

				
				if (m_nNeedLine == 255) {

					m_nNeedFrame++;
					m_nNeedLine = 0;
					bRet = true;
					break;
				}
                else {
                    m_nNeedLine++;
                }
			}
		}
	}

	return bRet;
}

