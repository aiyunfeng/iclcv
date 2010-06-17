/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/SceneObject.cpp                            **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLGeom/SceneObject.h>

namespace icl{
  const std::vector<Vec> &SceneObject::getVertices() const { 
    return m_vertices; 
  }
  std::vector<Vec> &SceneObject::getVertices() { 
    return m_vertices; 
  }

  const std::vector<Primitive> &SceneObject::getPrimitives() const { 
    return m_primitives; 
  }
  std::vector<Primitive> &SceneObject::getPrimitives() { 
    return m_primitives; 
  }
  
  void SceneObject::setVisible(Primitive::Type t, bool visible) {
    m_visible[t] = visible;
  }
    
  bool SceneObject::isVisible(Primitive::Type t) const {
    return m_visible[t];
  }
  
  SceneObject::SceneObject():
    m_lineColorsFromVertices(false),
    m_triangleColorsFromVertices(false),
    m_quadColorsFromVertices(false),
    m_pointSize(1),
    m_lineWidth(1)
  {

    std::fill(m_visible,m_visible+Primitive::PRIMITIVE_TYPE_COUNT,true);
  }
  
  void SceneObject::addVertex(const Vec &p, const GeomColor &color){
    m_vertices.push_back(p);
    m_vertexColors.push_back(color);
  }
  
  void SceneObject::addLine(int a, int b, const GeomColor &color){
    m_primitives.push_back(Primitive(a,b,color));
  }
    
  void SceneObject::addTriangle(int a, int b, int c, const GeomColor &color){
    m_primitives.push_back(Primitive(a,b,c,color));
  }
  
  void SceneObject::addQuad(int a, int b, int c, int d, const GeomColor &color){
    m_primitives.push_back(Primitive(a,b,c,d,color));
  }

  void SceneObject::addTexture(int a, int b, int c, int d, const Img8u &texture,bool deepCopy){
    m_primitives.push_back(Primitive(a,b,c,d,texture,deepCopy));
  }

  SceneObject *SceneObject::copy() const{
    return new SceneObject(*this);
  }

  void SceneObject::updateZFromPrimitives(){
    m_z = 0;
    if(m_primitives.size()){
      for(unsigned int i=0;i<m_primitives.size();++i){
        m_z += m_primitives[i].z;
      }
      m_z /= m_primitives.size();
    }
  }

  void SceneObject::setColor(Primitive::Type t,const GeomColor &color){
    for(unsigned int i=0;i<m_primitives.size();++i){
      if(m_primitives[i].type == t){
        m_primitives[i].color = color;
      }
    }
  }

  SceneObject::SceneObject(const std::string &type,const float *params):
    m_lineColorsFromVertices(false),
    m_triangleColorsFromVertices(false),
    m_quadColorsFromVertices(false),
    m_pointSize(1),
    m_lineWidth(1)
  {
    std::fill(m_visible,m_visible+5,true);
    if(type == "cube"){
      float x = *params++;
      float y = *params++;
      float z = *params++;
      float k = *params/2.0;

      addVertex(Vec(x+k,y-k,z+k,1));
      addVertex(Vec(x+k,y+k,z+k,1));
      addVertex(Vec(x-k,y+k,z+k,1));
      addVertex(Vec(x-k,y-k,z+k,1));
      
      addVertex(Vec(x+k,y-k,z-k,1));
      addVertex(Vec(x+k,y+k,z-k,1));
      addVertex(Vec(x-k,y+k,z-k,1));
      addVertex(Vec(x-k,y-k,z-k,1));
    
      addLine(0,1);
      addLine(1,2);
      addLine(2,3);
      addLine(3,0);
      
      addLine(4,5);
      addLine(5,6);
      addLine(6,7);
      addLine(7,4);
      
      addLine(0,4);
      addLine(1,5);    
      addLine(2,6);
      addLine(3,7);
      
      addQuad(0,1,2,3,GeomColor(0,100,120,155));//
      addQuad(7,6,5,4,GeomColor(0,100,140,155)); // ?
      addQuad(0,3,7,4,GeomColor(0,100,160,155));//
      addQuad(5,6,2,1,GeomColor(0,100,180,155)); // ?
      addQuad(4,5,1,0,GeomColor(0,100,200,155));
      addQuad(3,2,6,7,GeomColor(0,100,220,155));

    }else if(type == "cuboid"){
      float x = *params++;
      float y = *params++;
      float z = *params++;
      float dx = *params++/2.0;
      float dy = *params++/2.0;
      float dz = *params++/2.0;

      addVertex(Vec(x+dx,y-dy,z+dz,1));
      addVertex(Vec(x+dx,y+dy,z+dz,1));
      addVertex(Vec(x-dx,y+dy,z+dz,1));
      addVertex(Vec(x-dx,y-dy,z+dz,1));
      
      addVertex(Vec(x+dx,y-dy,z-dz,1));
      addVertex(Vec(x+dx,y+dy,z-dz,1));
      addVertex(Vec(x-dx,y+dy,z-dz,1));
      addVertex(Vec(x-dx,y-dy,z-dz,1));
    
      addLine(0,1);
      addLine(1,2);
      addLine(2,3);
      addLine(3,0);
      
      addLine(4,5);
      addLine(5,6);
      addLine(6,7);
      addLine(7,4);
      
      addLine(0,4);
      addLine(1,5);    
      addLine(2,6);
      addLine(3,7);
      
      // Vertex order: alwas counter clock-wise
      addQuad(0,1,2,3,GeomColor(0,100,120,155));//
      addQuad(7,6,5,4,GeomColor(0,100,140,155)); // ?
      addQuad(0,3,7,4,GeomColor(0,100,160,155));//
      addQuad(5,6,2,1,GeomColor(0,100,180,155)); // ?
      addQuad(4,5,1,0,GeomColor(0,100,200,155));
      addQuad(3,2,6,7,GeomColor(0,100,220,155));
      
    }else if(type == "sphere"){
      float x = *params++;
      float y = *params++;
      float z = *params++;
      float r = *params++/2.0;
      int slices = (int)*params++;
      int steps = (int)*params;
      
      float dA = (2*M_PI)/slices;
      float dB = (2*M_PI)/steps;

      Vec v(r,0,0,1),offs(x,y,z),zAxis(0,0,1),yAxis(0,1,0);
      int idx = 0;
      
      for(int i=0;i<slices;++i){
        //float a = i*dA/2.0;//-M_PI/2.0;
        Vec v2 = rotate_vector(zAxis,i*dA,v);
        for(int j=0;j<steps;++j,++idx){
          addVertex(offs+rotate_vector(yAxis,j*dB,v2));
          if(i>0 && j>0){
            addLine(idx,idx-1);
            addLine(idx,idx-slices);
          }
        }
      }
    }else{
      ERROR_LOG("unknown type:" << type);
    }
  }

  void SceneObject::setColorsFromVertices(Primitive::Type t, bool on){
    switch(t){
      case Primitive::line:
        m_lineColorsFromVertices = on;
        break;
      case Primitive::triangle:
        m_triangleColorsFromVertices = on;
        break;
      case Primitive::quad:
        m_quadColorsFromVertices = on;
        break;
      default:
        ERROR_LOG("this operations is only supported for line, triangle and quad primitive types");
        break;
    }
  }
}
