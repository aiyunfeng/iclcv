#include <iclDemoGrabber.h>
#include <iclThread.h>
#include <iclMathematics.h>

namespace icl{

  namespace{
    template<class T>
    void rect(Img<T> &image, const Color &color, const Rect &r){
      for(int i=0;i<image.getChannels() && i < 3;++i){
        T val = clipped_cast<icl8u,T>(color[i]);
        for(int y=r.y; y<r.y+r.height; ++y){
          T *data = image.getROIData(i,Point(r.x,y));
          std::fill(data,data+r.width,val);
        }
      }
    }
  }

  DemoGrabber::DemoGrabber(float maxFPS){
    m_x = Point32f(0.5,0.5);
    m_v = Point32f(0.01, 0.01);
    m_color = Color(255,100,50);
    m_size = Size32f(0.02,0.02);
    m_maxFPS = maxFPS;
    m_lastTime = Time::now();
  }
  const ImgBase* DemoGrabber::grab(ImgBase **ppoDst){
    ImgBase *image = prepareOutput(ppoDst);
    
    m_v += Point32f(icl::random(-0.001, 0.001),icl::random(-0.001, 0.001));
    m_x += m_v;
    if(m_x.x>1 || m_x.x<0){
      m_v.x *= -1;
      m_x.x += m_v.x;
    }
    if(m_x.y>1 || m_x.y<0){
      m_v.y *= -1;
      m_x.y += m_v.y;
    }
    Size s = image->getSize();
    Rect r((int)((m_x.x-m_size.width)*s.width),
           (int)((m_x.y-m_size.height)*s.height),
           (int)(m_size.width*s.width),
           (int)(m_size.height*s.height));
    r &= image->getImageRect();
    
    switch(image->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: rect(*image->asImg<icl##D>(),m_color,r); break;
      ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
    }
    
    Time now = Time::now();
    Time neededInterval = Time(1000000)/m_maxFPS;
    if((now-m_lastTime) < neededInterval){
      Time restSleepTime = neededInterval-(now-m_lastTime);
      Thread::msleep(restSleepTime.toMilliSeconds());
    }
    return image;
    
  }

}
