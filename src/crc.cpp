#include "include/common.h"


QByteArray CRC16Calculate(QByteArray buf)
{
    quint16 IX, IY, CRC,Len;
    Len=buf.size();
    CRC = 0xFFFF;
    if(Len <= 0)    return 0;
    for(IX = 0; IX < Len; IX++)
    {
        CRC = CRC ^ (quint8)(buf[IX]);
        for(IY = 0; IY < 8; IY++)
        {
            if((CRC & 1) != 0)
                CRC = (CRC >> 1) ^ 0xA001;
            else
                CRC = CRC >> 1;
        }
    }
    QByteArray r;
    r.append((unsigned char)(CRC>>8));
    r.append((unsigned char)(CRC>>0));
    return r;
}

//modbus crc16
quint16 modbus_crc16(QByteArray Data)
{
  quint32 IX, IY, Len ;
  quint16 CRC;
  Len=Data.size();
  CRC = 0xFFFF;
  if (Len <= 0)    return 0;
  for (IX = 0; IX < Len; IX++)
  {
      CRC = CRC ^ (quint8)(Data.at(IX));
      for (IY = 0; IY < 8; IY++)
      {
        if ((CRC & 1) != 0)
          CRC = (CRC >> 1) ^ 0xA001;
        else
          CRC = CRC >> 1;
      }
  }
  return CRC;
}

//多项是为0x1021的CRC算法
qint16 GetCrc(QByteArray Data)
{
   quint16 mCrc = 0;
   quint16 sLen = Data.size();
   for(quint32 j = 0; j < sLen; j++){
       mCrc = mCrc^(quint16)(Data.at(j)) << 8;
       for (quint32 i=8; i!=0; i--){
           if (mCrc & 0x8000)
               mCrc = mCrc << 1 ^ 0x1021;
           else
               mCrc = mCrc << 1;
       }
   }
   return mCrc;
}
