#include <iclMouseEvent.h>

namespace icl{
  
  MouseEvent::MouseEvent(const Point &widgetPos,
                         const Point &imagePos,
                         const Point32f &relImagePos,
                         const bool downMask[3],
                         const std::vector<double> &color,
                         MouseEventType type,
                         ICLWidget* widget):
    m_widgetPos(widgetPos),m_imagePos(imagePos),
    m_relImagePos(relImagePos),
    m_color(color),m_type(type),m_widget(widget){
    
    std::copy(downMask,downMask+3,m_downMask);
  }

  MouseEvent::MouseEvent():
    m_widgetPos(Point::null),m_imagePos(Point::null),
    m_relImagePos(Point32f::null),
    m_type(MouseLeaveEvent),m_widget(0){
    
    std::fill(m_downMask,m_downMask+3,false);
  }
}