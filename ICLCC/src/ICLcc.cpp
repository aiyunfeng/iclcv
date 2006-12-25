#ifndef ICLIMAGE_COLOR_CONVERSION_CPP
#define ICLIMAGE_COLOR_CONVERSION_CPP

#include <ICLcc.h>
#include <math.h>

namespace icl{

#ifdef WITH_IPP_OPTIxxMIZATION
// {{{ convert,set, and copy channels using IPP#define CONVERT_8U32F(S,SC,D,DC) ippiConvert_8u32f_C1R(S->asImg<icl8u>()->getData(SC),S->getLineStep(),D->asImg<icl32f>()->getData(DC),D->getLineStep(),D->getSize())#define CONVERT_32F8U(S,SC,D,DC) ippiConvert_32f8u_C1R(S->asImg<icl32f>()->getData(SC),S->getLineStep(),D->asImg<icl8u>()->getData(DC),D->getLineStep(),D->getSize(),ippRndZero)   #define SET_32F(V,D,DC) ippiSet_32f_C1R(V,D->asImg<icl32f>()->getData(DC),D->getLineStep(),D->getSize());#define SET_8U(V,D,DC) ippiSet_8u_C1R(V,D->asImg<icl8u>()->getData(DC),D->getLineStep(),D->getSize());#define COPY_8U(S,SC,D,DC) ippiCopy_8u_C1R(S->asImg<icl8u>()->getData(SC),S->getLineStep(),D->asImg<icl8u>()->getData(DC),D->getLineStep(),D->getSize())  #define COPY_32F(S,SC,D,DC) ippiCopy_32f_C1R(S->asImg<icl32f>()->getData(SC),S->getLineStep(),D->asImg<icl32f>()->getData(DC),D->getLineStep(),D->getSize())  // }}}
#else
// {{{ convert,set, and copy channels using C++#define CONVERT_8U32F(S,SC,D,DC)                                                        \{                                                                                       \  icl8u *pucSrc = S->asImg<icl8u>()->getData(SC);                                       \  icl32f *pfDst = D->asImg<icl32f>()->getData(DC);                                      \  icl32f *pfDstEnd = pfDst+D->getDim();                                                 \  while(pfDst < pfDstEnd) *pfDst++ = Cast<icl8u,icl32f>::cast(*pucSrc++);               \}#define CONVERT_32F8U(S,SC,D,DC)                                                        \{                                                                                       \  icl32f *fSrc = S->asImg<icl32f>()->getData(SC);                                       \  icl8u *pucDst = D->asImg<icl8u>()->getData(DC);                                       \  icl8u *pucDstEnd = pucDst+D->getDim();                                                \  while(pucDst < pucDstEnd) *pucDst++ = Cast<icl32f,icl8u>::cast(*fSrc++);              \}#define SET_32F(V,D,DC) D->asImg<icl32f>()->clear(DC,V)#define SET_8U(V,D,DC) D->asImg<icl8u>()->clear(DC,V)#define COPY_8U(S,SC,D,DC) memcpy(D->getDataPtr(DC),S->getDataPtr(SC),D->getDim()*sizeof(icl8u))#define COPY_32F(S,SC,D,DC) memcpy(D->getDataPtr(DC),S->getDataPtr(SC),D->getDim()*sizeof(icl32f))// }}}
#endif

static const icl8u ucDefaultHue = 0;
static const icl8u ucDefaultSaturation = 0;
static const icl32f fDefaultHue = ucDefaultHue;
static const icl32f fDefaultSaturation = ucDefaultSaturation;
  
template<class S, class D>
inline void planarToInterleaved_Generic(const Img<S> *src, D* dst){
  // {{{ open  FUNCTION_LOG("");  ICLASSERT_RETURN(src);  ICLASSERT_RETURN(dst);  int c=src->getChannels();  if(c==1){    copy<S,D>(src->getData(0),src->getData(0)+src->getDim(),dst);    return;  }  int dim=src->getDim();  S** pp=new S* [c];  S** ppEnd=pp+c;    for (int i=0;i<c;i++){    pp[i]=src->getData(i);  }    D* dstEnd=dst+c*dim;  while (dst<dstEnd){    for (S** p=pp;p<ppEnd;++(*p),++p,++dst ){      *dst=Cast<S,D>::cast(*(*p));    }  }  delete [] pp;}    // }}}

template<class S, class D>
inline void interleavedToPlanar_Generic(const S *src,const Size &srcSize, int c,  Img<D> *dst){
  // {{{ open    FUNCTION_LOG("");  ICLASSERT_RETURN(src);  ICLASSERT_RETURN(dst);  dst->setChannels(c);  dst->setSize(srcSize);  if(c==1){    copy<S,D>(src,src+srcSize.getDim(),dst->getData(0));    return;  }      D** pp=new D* [c];  D** ppEnd=pp+c;  for (int i=0;i<c;i++){    pp[i]=dst->getData(i);  }  const S* srcEnd=src+srcSize.getDim()*c;  while (src<srcEnd){    for (D** p=pp;p<ppEnd;++(*p),++p,++src){      *(*p)= Cast<S,D>::cast(*src);    }  }  delete [] pp;}    // }}}

template<class S,class D>
void planarToInterleaved(const Img<S> *src, D* dst){
  planarToInterleaved_Generic(src,dst);
}
  
template<class S, class D>
void interleavedToPlanar(const S *src,const Size &srcSize, int c,  Img<D> *dst){
  interleavedToPlanar_Generic(src,srcSize,c,dst);
}
  
#ifdef WITH_IPP_OPTIMIZATION

  // {{{ PLANAR_2_INTERLEAVED_IPP#define PLANAR_2_INTERLEAVED_IPP(DEPTH)                                                                       \  template<> void planarToInterleaved(const Img<icl##DEPTH>*src, icl##DEPTH *dst){                            \    ICLASSERT_RETURN( src );                                                                                  \    ICLASSERT_RETURN( dst );                                                                                  \    ICLASSERT_RETURN( src->getChannels() );                                                                   \    switch(src->getChannels()){                                                                               \      case 3: {                                                                                               \        icl##DEPTH* apucChannels[3]={src->getData(0),src->getData(1),src->getData(2)};                        \        ippiCopy_##DEPTH##_P3C3R(apucChannels,src->getLineStep(),dst,src->getLineStep()*3,src->getSize());    \        break;                                                                                                \      }                                                                                                       \      case 4: {                                                                                               \        icl##DEPTH* apucChannels[4]={src->getData(0),src->getData(1),src->getData(2),src->getData(3)};        \        ippiCopy_##DEPTH##_P4C4R(apucChannels,src->getLineStep(),dst,src->getLineStep()*4,src->getSize());    \        break;                                                                                                \      }                                                                                                       \      default:                                                                                                \        planarToInterleaved_Generic(src,dst);                                                                 \        break;                                                                                                \    }                                                                                                         \  }  PLANAR_2_INTERLEAVED_IPP(8u)  PLANAR_2_INTERLEAVED_IPP(16s)  PLANAR_2_INTERLEAVED_IPP(32s)  PLANAR_2_INTERLEAVED_IPP(32f)#undef PLANAR_2_INTERLEAVED_IPP  // }}}

  // {{{ INTERLEAVED_2_PLANAR_IPP#define INTERLEAVED_2_PLANAR_IPP(DEPTH)                                                                                         \  template<> void interleavedToPlanar(const icl##DEPTH *src, const Size &srcSize, int srcChannels,  Img<icl##DEPTH> *dst){      \    ICLASSERT_RETURN( src );                                                                                                    \    ICLASSERT_RETURN( dst );                                                                                                    \    ICLASSERT_RETURN( srcChannels );                                                                                            \    dst->setChannels( srcChannels );                                                                                            \    switch(srcChannels){                                                                                                        \      case 3: {                                                                                                                 \        icl##DEPTH* apucChannels[3]={dst->getData(0),dst->getData(1),dst->getData(2)};                                          \        ippiCopy_##DEPTH##_C3P3R(src,srcSize.width*srcChannels,apucChannels,srcSize.width,srcSize);                             \        break;                                                                                                                  \      }                                                                                                                         \      case 4: {                                                                                                                 \        icl##DEPTH* apucChannels[4]={dst->getData(0),dst->getData(1),dst->getData(2),dst->getData(3)};                          \        ippiCopy_##DEPTH##_C4P4R(src,srcSize.width*srcChannels,apucChannels,srcSize.width,srcSize);                             \        break;                                                                                                                  \      }                                                                                                                         \      default:                                                                                                                  \        interleavedToPlanar_Generic(src,srcSize,srcChannels,dst);                                                               \        break;                                                                                                                  \    }                                                                                                                           \  }  INTERLEAVED_2_PLANAR_IPP(8u)  INTERLEAVED_2_PLANAR_IPP(16s)  INTERLEAVED_2_PLANAR_IPP(32s)  INTERLEAVED_2_PLANAR_IPP(32f)#undef INTERLEAVED_2_PLANAR_IPP  // }}}

#endif // WITH_IPP_OPTINIZATION

  // {{{ explicit template instatiations for interleavedToPlanar and planarToInterleaved#define EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(TYPEA,TYPEB)                            \  template void planarToInterleaved<TYPEA,TYPEB>(const Img<TYPEA>*,TYPEB*);                 \  template void interleavedToPlanar<TYPEA,TYPEB>(const TYPEA*,const Size&,int,Img<TYPEB>*)#define EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION_FOR_ALL_TYPEB(TYPEA)  \  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(TYPEA,icl8u);               \  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(TYPEA,icl16s);              \  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(TYPEA,icl32s);              \  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(TYPEA,icl32f);              \  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION(TYPEA,icl64f)    EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION_FOR_ALL_TYPEB(icl8u);  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION_FOR_ALL_TYPEB(icl16s);  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION_FOR_ALL_TYPEB(icl32s);  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION_FOR_ALL_TYPEB(icl32f);  EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION_FOR_ALL_TYPEB(icl64f);#undef EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION#undef EXPLICIT_I2P_AND_P2I_TEMPLATE_INSTANTIATION_FOR_ALL_TYPEB  // }}}



void iclcc(ImgBase *poDst, ImgBase *poSrc){
  // {{{ open  ICLASSERT_RETURN (poDst->getSize() == poSrc->getSize());  if(poSrc->getFormat()==formatMatrix || poDst->getFormat()==formatMatrix){    printf("error in iclcc: formatMatrix is not allowed here!\n");    return;  }  if(poSrc->getDepth()==depth8u){    switch(poSrc->getFormat()){      case formatGray: convertFromGray8(poDst,poSrc); break;      case formatRGB: convertFromRGB8(poDst,poSrc); break;      case formatHLS: convertFromHLS8(poDst,poSrc); break;      case formatLAB: convertFromLAB8(poDst,poSrc); break;      case formatYUV: convertFromYUV8(poDst,poSrc); break;      default: return;    }  }else{    switch(poSrc->getFormat()){      case formatGray: convertFromGray32(poDst,poSrc); break;      case formatRGB: convertFromRGB32(poDst,poSrc); break;      case formatHLS: convertFromHLS32(poDst,poSrc); break;      case formatLAB: convertFromLAB32(poDst,poSrc); break;      case formatYUV: convertFromYUV32(poDst,poSrc); break;      default: return;    }  }  poDst->setROI (poSrc->getROI());}// }}}

void convertFromGray8(ImgBase *poDst, ImgBase *poSrc){
  // {{{ open  if(poDst->getDepth()==depth8u){    switch(poDst->getFormat()){      case formatGray:        COPY_8U(poSrc,0,poDst,0);        break;      case formatRGB:        COPY_8U(poSrc,0,poDst,0);        COPY_8U(poSrc,0,poDst,1);        COPY_8U(poSrc,0,poDst,2);        break;      case formatHLS:        SET_8U(ucDefaultHue,poDst,0);        COPY_8U(poSrc,1,poDst,1);        SET_8U(ucDefaultSaturation,poDst,2);        break;      case formatLAB:      case formatYUV:      default:        ERROR_LOG("unsupported format!");    }  }else{    switch(poDst->getFormat()){     case formatGray:        CONVERT_8U32F(poSrc,0,poDst,0);        break;      case formatRGB:        CONVERT_8U32F(poSrc,0,poDst,0);        CONVERT_8U32F(poSrc,0,poDst,1);        CONVERT_8U32F(poSrc,0,poDst,2);        break;      case formatHLS:        SET_32F(fDefaultHue,poDst,0);        CONVERT_8U32F(poSrc,1,poDst,1);        SET_32F(fDefaultSaturation,poDst,1);        break;        case formatLAB:      case formatYUV:      default:        ERROR_LOG("unsupported format!");        break;    }  }}  // }}}
  
void convertFromGray32(ImgBase *poDst, ImgBase *poSrc){
  // {{{ open if(poDst->getDepth()==depth8u){    switch(poDst->getFormat()){      case formatGray:        CONVERT_32F8U(poSrc,0,poDst,0);        break;      case formatRGB:        CONVERT_32F8U(poSrc,0,poDst,0);        CONVERT_32F8U(poSrc,0,poDst,1);        CONVERT_32F8U(poSrc,0,poDst,2);        break;      case formatHLS:         SET_8U(ucDefaultHue,poDst,0);        CONVERT_32F8U(poSrc,1,poDst,1);        SET_8U(ucDefaultSaturation,poDst,2);        break;      case formatLAB:      case formatYUV:      default:        ERROR_LOG("unsupported format!");    }  }else{    switch(poDst->getFormat()){     case formatGray:       COPY_32F(poSrc,0,poDst,0);        break;      case formatRGB:        COPY_32F(poSrc,0,poDst,0);        COPY_32F(poSrc,0,poDst,1);        COPY_32F(poSrc,0,poDst,2);        break;      case formatHLS:        SET_32F(fDefaultHue,poDst,0);        COPY_32F(poSrc,1,poDst,1);        SET_32F(fDefaultSaturation,poDst,1);        break;        case formatLAB:      case formatYUV:      default:        ERROR_LOG("unsupported format!");        break;    }  }}   // }}}
 
void convertFromRGB8(ImgBase *poDstImage, ImgBase *poSrcImage){
  // {{{ open

    icl8u *pR = poSrcImage->asImg<icl8u>()->getData(0);
    icl8u *pG = poSrcImage->asImg<icl8u>()->getData(1);
    icl8u *pB = poSrcImage->asImg<icl8u>()->getData(2);
    
    if(poDstImage->getDepth() == depth8u){
      switch(poDstImage->getFormat()){
        case formatGray:{ //no ROI
          register icl8u *poDst = poDstImage->asImg<icl8u>()->getData(0);
          register icl8u *poDstEnd = poDst+poDstImage->getDim();
          while(poDst!=poDstEnd){
            *poDst++=((*pR++)+(*pG++)+(*pB++))/3; 
          }
          break;
        }
        case formatRGB:
          COPY_8U(poSrcImage,0,poDstImage,0);
          COPY_8U(poSrcImage,1,poDstImage,1);
          COPY_8U(poSrcImage,2,poDstImage,2);
          break;
        case formatHLS:
          {
            init_table();
            register icl8u *pH = poDstImage->asImg<icl8u>()->getData(0);
            register icl8u *pL = poDstImage->asImg<icl8u>()->getData(1);
            register icl8u *pS = poDstImage->asImg<icl8u>()->getData(2);
            register icl8u *pHEnd = pH+poDstImage->getDim();
            while(pH!=pHEnd){
              rgb_to_hls(*pR++,*pG++,*pB++,*pH++,*pL++,*pS++);
            }
            break;
          }
          break;
        case formatChroma:
        {
          register int sum = 0;
          register icl8u *pChromaR = poDstImage->asImg<icl8u>()->getData(0);
          register icl8u *pChromaG = poDstImage->asImg<icl8u>()->getData(1);
          register icl8u *pChromaREnd = pChromaR+poDstImage->getDim();
          
          while(pChromaR != pChromaREnd){
            sum = (*pR + *pG + *pB);
            sum+=!sum; //avoid division by zero
            *pChromaR++=((*pR * 255) / sum);
            *pChromaG++=((*pG * 255) / sum);
            
            pR++; pG++; pB++;
            }        
          break;
        }
        case formatYUV:
        case formatLAB:
        default:
          ERROR_LOG("unsupported format!");
      }
    }else{//depth32f
      switch(poDstImage->getFormat()){
        case formatGray:{
          register icl32f *poDst = poDstImage->asImg<icl32f>()->getData(0);
          register icl32f *poDstEnd = poDst+poDstImage->getDim();
          while(poDst!=poDstEnd){
            *poDst++=(icl32f)((*pR++)+(*pG++)+(*pB++))/3.0;
          }
          break;
        }
        case formatRGB:
          CONVERT_8U32F(poSrcImage,0,poDstImage,0);
          CONVERT_8U32F(poSrcImage,1,poDstImage,1);
          CONVERT_8U32F(poSrcImage,2,poDstImage,2);
          break;
        case formatHLS:
          {
            init_table();
            register icl32f *pH = poDstImage->asImg<icl32f>()->getData(0);
            register icl32f *pL = poDstImage->asImg<icl32f>()->getData(1);
            register icl32f *pS = poDstImage->asImg<icl32f>()->getData(2);
            register icl32f *pHEnd = pH+poDstImage->getDim();
            
            while(pH!=pHEnd){
              rgb_to_hls(*pR++,*pG++,*pB++,*pH++,*pS++,*pL++);
            }
          }
          break;
        case formatChroma:
        {
          register int sum = 0;
          register icl32f *pChromaR = poDstImage->asImg<icl32f>()->getData(0);
          register icl32f *pChromaG = poDstImage->asImg<icl32f>()->getData(1);
          register icl32f *pChromaREnd = pChromaR+poDstImage->getDim();
          
          while(pChromaR != pChromaREnd){
            sum = (*pR + *pG + *pB);
            sum+=!sum; //avoid division by zero
            *pChromaR++=((*pR * 255) / sum);
            *pChromaG++=((*pG * 255) / sum);
            
            pR++; pG++; pB++;
          }
          break;
        }
        case formatYUV:
        case formatLAB:
        default:
          ERROR_LOG("unsupported format!");
      }
    }
}

  // }}}

void convertFromRGB32(ImgBase *poDstImage, ImgBase *poSrcImage){
  // {{{ open    icl32f *pR = poSrcImage->asImg<icl32f>()->getData(0);    icl32f *pG = poSrcImage->asImg<icl32f>()->getData(1);    icl32f *pB = poSrcImage->asImg<icl32f>()->getData(2);    if(poDstImage->getDepth() == depth8u){          switch(poDstImage->getFormat()){        case formatGray:{ //no ROI          register icl32f *poDst = poDstImage->asImg<icl32f>()->getData(0);          register icl32f *poDstEnd = poDst+poDstImage->getDim();          while(poDst!=poDstEnd){            *poDst++=(icl8u)(((*pR++)+(*pG++)+(*pB++))/3);          }          break;        }        case formatRGB:          CONVERT_32F8U(poSrcImage,0,poDstImage,0);          CONVERT_32F8U(poSrcImage,1,poDstImage,1);          CONVERT_32F8U(poSrcImage,2,poDstImage,2);          break;        case formatHLS:          {            init_table();            register icl8u *pH = poDstImage->asImg<icl8u>()->getData(0);            register icl8u *pL = poDstImage->asImg<icl8u>()->getData(1);            register icl8u *pS = poDstImage->asImg<icl8u>()->getData(2);            register icl8u *pHEnd = pH+poDstImage->getDim();            while(pH!=pHEnd){              rgb_to_hls(*pR++,*pG++,*pB++,*pH++,*pL++,*pS++);            }            break;          }        case formatChroma:        {          register float sum = 0;          register icl8u *pChromaR = poDstImage->asImg<icl8u>()->getData(0);          register icl8u *pChromaG = poDstImage->asImg<icl8u>()->getData(1);          register icl8u *pChromaREnd = pChromaR+poDstImage->getDim();                    while(pChromaR != pChromaREnd){            sum = (*pR + *pG + *pB);            sum+=!sum; //avoid division by zero            *pChromaR++=Cast<icl32f,icl8u>::cast((*pR * 255) / sum);            *pChromaG++=Cast<icl32f,icl8u>::cast((*pG * 255) / sum);            pR++; pG++; pB++;          }                    break;        }        case formatLAB:        case formatYUV:        default:          ERROR_LOG("unsupported format !");      }    }else{      switch(poDstImage->getFormat()){        case formatGray:          {            register icl32f *poDst = poDstImage->asImg<icl32f>()->getData(0);            register icl32f *poDstEnd = poDst+poDstImage->getDim();            while(poDst!=poDstEnd){              *poDst++=((*pR++)+(*pG++)+(*pB++))/3.0;            }            break;          }        case formatRGB:          COPY_32F(poSrcImage,0,poDstImage,0);          COPY_32F(poSrcImage,1,poDstImage,1);          COPY_32F(poSrcImage,2,poDstImage,2);          break;        case formatHLS:          {            init_table();            register icl32f *pH = poDstImage->asImg<icl32f>()->getData(0);            register icl32f *pL = poDstImage->asImg<icl32f>()->getData(1);            register icl32f *pS = poDstImage->asImg<icl32f>()->getData(2);            register icl32f *pHEnd = pH+poDstImage->getDim();            while(pH!=pHEnd){              rgb_to_hls(*pR++,*pG++,*pB++,*pH++,*pL++,*pS++);            }            break;          }        case formatChroma:        {          register float sum = 0;          register icl32f *pChromaR = poDstImage->asImg<icl32f>()->getData(0);          register icl32f *pChromaG = poDstImage->asImg<icl32f>()->getData(1);          register icl32f *pChromaREnd = pChromaR+poDstImage->getDim();                    while(pChromaR != pChromaREnd){            sum = (*pR + *pG + *pB);            sum+=!sum; //avoid division by zero            *pChromaR++=((*pR * 255) / sum);            *pChromaG++=((*pG * 255) / sum);                        pR++; pG++; pB++;          }          break;        }        case formatLAB:        case formatYUV:        default:          ERROR_LOG("unsupported format!");          break;      }    }}  // }}}

void convertFromRGBA8(ImgBase *poDst, ImgBase *poSrc){
  // {{{ open

  convertFromRGB8(poDst,poSrc);
  }

  // }}}

void convertFromRGBA32(ImgBase *poDst, ImgBase *poSrc){
  // {{{ open    convertFromRGB32(poDst,poSrc);  }  // }}}

void convertFromLAB8(ImgBase *poDst, ImgBase *poSrc){
  // {{{ open  (void)poDst;  (void)poSrc;  ERROR_LOG("converting from lab is not yet supported!");}// }}}

void convertFromLAB32(ImgBase *poDst, ImgBase *poSrc){
  // {{{ open  (void)poDst;  (void)poSrc;  ERROR_LOG("converting from lab is not yet supported!");}// }}}

void convertFromYUV8(ImgBase *poDst, ImgBase *poSrc){
  // {{{ open  (void)poDst;  (void)poSrc;  ERROR_LOG("converting from yuv is not yet supported!");}// }}}

void convertFromYUV32(ImgBase *poDst, ImgBase *poSrc){
  // {{{ open  (void)poDst;  (void)poSrc;  ERROR_LOG("converting from yuv is not yet supported!");}// }}}

void convertFromHLS8(ImgBase *poDst, ImgBase *poSrc){
  // {{{ open#ifdef WITH_IPP_OPTIMIZATION  if(poDst->getFormat() == formatRGB){    icl8u *buffer = new icl8u[poDst->getDim()*3];    icl8u *buffer2 = new icl8u[poDst->getDim()*3];    planarToInterleaved(poSrc->asImg<icl8u>(),buffer);    ippiHLSToRGB_8u_C3R(buffer,poSrc->getLineStep()*3, buffer2,poSrc->getLineStep()*3,poSrc->getSize());    interleavedToPlanar(buffer2,poDst->getSize(), 3, poDst->asImg<icl8u>());    delete [] buffer;    delete [] buffer2;  }else{    ERROR_LOG("converting from hls is not yet supported!");  }#else  (void)poDst;  (void)poSrc;  ERROR_LOG("converting from hls is not yet supported!");#endif}  // }}}

void convertFromHLS32(ImgBase *poDst, ImgBase *poSrc){
  // {{{ open#ifdef WITH_IPP_OPTIMIZATION  if(poDst->getFormat() == formatRGB){    icl32f *buffer = new icl32f[poDst->getDim()*3];    icl32f *buffer2 = new icl32f[poDst->getDim()*3];    planarToInterleaved(poSrc->asImg<icl32f>(),buffer);    ippiHLSToRGB_32f_C3R(buffer,poSrc->getLineStep()*3, buffer2,poSrc->getLineStep()*3,poSrc->getSize());    interleavedToPlanar(buffer2,poDst->getSize(), 3, poDst->asImg<icl32f>());    delete [] buffer;    delete [] buffer2;  }else{    ERROR_LOG("converting from hls is not yet supported!");  }#else  (void)poDst;  (void)poSrc;  ERROR_LOG("converting from hls is not yet supported!");#endif}  // }}}
    
void convertYUV420ToRGB8(Img8u *poDst, unsigned char *pucSrc,const Size &s){
  // {{{ open#ifdef WITH_IPP_OPTIMIZATION  icl8u *apucSrc[] = {pucSrc,pucSrc+s.getDim(), pucSrc+s.getDim()+s.getDim()/4};  icl8u *apucDst[] = {poDst->getData(0),poDst->getData(1),poDst->getData(2)};  ippiYUV420ToRGB_8u_P3(apucSrc,apucDst,s); #else  // allocate memory for lookup tables  static float fy_lut[256];  static float fu_lut[256];  static float fv_lut[256];  static int r_lut[65536];  static int b_lut[65536];  static float g_lut1[65536];  static float g_lut2[256];  static int iInitedFlag=0;  // initialize to lookup tables  if(!iInitedFlag){    float fy,fu,fv;    for(int i=0;i<256;i++){      fy_lut[i] = (255* (i - 16)) / 219;      fu_lut[i] = (127 * (i - 128)) / 112;      fv_lut[i] = (127 * (i - 128)) / 112;    }        for(int v=0;v<256;v++){      g_lut2[v] = 0.714 * fv_lut[v];    }        for(int y=0;y<256;y++){      fy = fy_lut[y];      for(int u=0;u<256;u++){	g_lut1[y+256*u] = fy - 0.344 * fu_lut[u];      }    }          for(int y=0;y<256;y++){      fy = fy_lut[y];      for(int v=0;v<256;v++){	fv = fv_lut[v];	r_lut[y+256*v]= (int)( fy + (1.402 * fv) );	fu = fu_lut[v];	b_lut[y+256*v]= (int)( fy + (1.772 * fu) );       }    }        iInitedFlag = 1;  }  // creating temporary pointer for fast data access  int iW = s.width;  int iH = s.height;      icl8u *pucR = poDst->getData(0);  icl8u *pucG = poDst->getData(1);  icl8u *pucB = poDst->getData(2);  icl8u *ptY = pucSrc;  icl8u *ptU = ptY+iW*iH;  icl8u *ptV = ptU+(iW*iH)/4;    register int r,g,b,y,u,v;    register int Xflag=0;  register int Yflag=1;  register int w2 = iW/2;    // converting the image (ptY,ptU,ptV)----->(pucR,pucG,pucB)  for(int yy=iH-1; yy >=0 ; yy--){    for(int xx=0; xx < iW; xx++){      u=*ptU;      v=*ptV;      y=*ptY;            r = r_lut[y+256*v];      g = (int) ( g_lut1[y+256*u] - g_lut2[v]);      b = b_lut[y+256*u];            #define LIMIT(x) (x)>255?255:(x)<0?0:(x);      *pucR++=LIMIT(r);      *pucG++=LIMIT(g);      *pucB++=LIMIT(b);      #undef LIMIT            if(Xflag++){        ptV++;        ptU++;        Xflag=0;      }      ptY++;    }    if(Yflag++){      ptU -= w2;      ptV -= w2;      Yflag = 0;    }  }#endif}    // }}}
  
void convertToARGB32Interleaved(unsigned char *pucDst, Img8u *poSrc){
  // {{{ open  if(!poSrc || poSrc->getChannels() != 4 ) return;#ifdef WITH_IPP_OPTIMIZATION  icl8u* apucChannels[4]={poSrc->getData(0),poSrc->getData(1),poSrc->getData(2),poSrc->getData(3)};  ippiCopy_8u_P4C4R(apucChannels,poSrc->getLineStep(),pucDst,poSrc->getLineStep()*4,poSrc->getSize());#else  printf("c++ fallback for convertToRGB8Interleaved(unsigned char *pucDst, Img8u *poSrc) not yet implemented \n");#endif}   // }}}

void convertToARGB32Interleaved(unsigned char *pucDst, Img32f *poSrc, Img8u *poBuffer){
  // {{{ open  if(!poSrc || poSrc->getChannels() != 4 ) return;#ifdef WITH_IPP_OPTIMIZATION  poSrc->convertTo<icl8u>(poBuffer);  icl8u* apucChannels[4]={poBuffer->getData(0),poBuffer->getData(1),poBuffer->getData(2),poBuffer->getData(3)};  ippiCopy_8u_P4C4R(apucChannels,poBuffer->getLineStep(),pucDst,poBuffer->getLineStep()*4,poBuffer->getSize());#else  printf("c++ fallback for convertToRGB8Interleaved(unsigned char *pucDst,"         " Img32f *poSrc, Img8u* poBuffer) not yet implemented \n");#endif}   // }}}


static unsigned char aucHlsTable[257];
static float afHlsTable[257];
  
/// intern utility function
void get_min_and_max(const unsigned char &r,const unsigned char &g,const unsigned char&b,unsigned char &min,unsigned char &max){
  // {{{ open    if(r<g) {      min=r;      max=g;    }    else {      min=g;      max=r;	    }    if(b<min)      min=b;    else {      max = b>max ? b : max;    }  }    // }}}



void rgb_to_hls(const unsigned char &r,const unsigned char &g,const unsigned char &b, unsigned char &h, unsigned char &l, unsigned char &s){
  // {{{ open    static int RG,RB,nom,denom, cos2Hx256;    static unsigned char min;    get_min_and_max(r, g, b,min,l);    s = l>0 ? 255-((255*min)/l) : 0;      //Hue    RG = r - g;    RB = r - b;    nom = (RG+RB)*(RG+RB);    denom = (RG*RG+RB*(g-b))<<2;      cos2Hx256 = denom>0 ? (nom<<8)/denom : 0;    h=aucHlsTable[cos2Hx256];    if ((RG+RB)<0) h=127-h;    if (b>g) h=255-h;  }  // }}}

void rgb_to_hls(const float &r,const float &g,const float &b, unsigned char &h, unsigned char &l, unsigned char &s){
  // {{{ open    rgb_to_hls((unsigned char)(r),(unsigned char)(g),(unsigned char)(b),h,l,s);    }  // }}}
void rgb_to_hls(const float &r,const float &g,const float &b, float &h, float &l, float &s){
  // {{{ open    static unsigned char H,L,S;    rgb_to_hls((unsigned char)(r),(unsigned char)(g),(unsigned char)(b),H,L,S);    h=(float)H;    l=(float)L;    s=(float)S;  }  // }}}
void rgb_to_hls(const unsigned char &r,const unsigned char &g,const unsigned char &b, float &h, float &l, float &s){
  // {{{ open    static unsigned char H,L,S;    rgb_to_hls(r,g,b,H,L,S);    h=(float)H;    l=(float)L;    s=(float)S;  }  // }}}

void init_table() {
  // {{{ open    static int inited = 0;    if(inited) return;    for(int i=0 ; i<256 ; i++){      aucHlsTable[i] = static_cast<unsigned char>(acos(sqrt((float)i/256.0)) * 2 * 63 / M_PI);      afHlsTable[i] = (float)aucHlsTable[i];//(255.0*360.0); or scale to 360° ?    }    aucHlsTable[256]=aucHlsTable[255];    afHlsTable[256]=afHlsTable[255];    inited = 1;  }  // }}}


}//namespace icl

#endif
