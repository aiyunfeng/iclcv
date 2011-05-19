/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMarkers/FiducialDetectorPluginForQuads.h    **
** Module : ICLBlob                                                **
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

#ifndef ICL_FUDUCIAL_DETECTOR_PLUGIN_FOR_QUADS_H
#define ICL_FUDUCIAL_DETECTOR_PLUGIN_FOR_QUADS_H

#include <ICLMarkers/FiducialDetectorPlugin.h>

namespace icl{
  
  /** \cond */
  class FiducialDetector;
  /** \endcond */
  
  
  /// FiducialDetectorPlugin for quad-markers like ARToolkit and BCH-Code markers \ingroup PLUGINS
  class FiducialDetectorPluginForQuads : public FiducialDetectorPlugin{
    /// Internal Data
    struct Data;
    
    /// Internal data pointer
    Data *data;

    protected:

    /// only the FiducialDetector can instantiate this class
    FiducialDetectorPluginForQuads();
    public:

    /// This class cannot be used 
    friend class icl::FiducialDetector;

    /// Destructor
    ~FiducialDetectorPluginForQuads();
    
    // use the edges
    virtual void getKeyPoints2D(std::vector<Fiducial::KeyPoint> &dst, FiducialImpl &impl);
    virtual void getFeatures(Fiducial::FeatureSet &dst);
    virtual void detect(std::vector<FiducialImpl*> &dst, const Img8u &image);

    /// this plugin uses the binarisation from the internally used quad-detector
    virtual SourceImageType getPreProcessing() const {  return Gray;  }
    
    /// loads markers ID's (also implemented in the subclasses)
    /** @param def this any instance can be ...
        * of type int (then, only the corresponding marker ID is loaded) 
        * of type Range32s "[a,b]", (then all markers within the range are loaded)
        * of something like {a,b,c,d,...} then all marker IDs in the list are loaded

        Please note that other types might be interpreted in the wrong way.
        Mandatory parameter is "size". Please refer to the 
        documentation of icl::FiducialDetector::loadMarkers for more details
    */
    virtual void addOrRemoveMarkers(bool add, const Any &which, const ParamList &params) = 0;

    /// this plugin provides some extra intermediate images
    std::string getIntermediateImageNames() const;
    
    /// returns the intermediate image, that is associated with the given name
    /** @see getIntermediateImageNames for more details */
    const ImgBase *getIntermediateImage(const std::string &name) const throw (ICLException);

    /// this method must be called in the subclasses
    virtual FiducialImpl *classifyPatch(const Img8u &image, int *rot) = 0;
    
    /// this method is also implemented in the subclasses
    /** The method describes the parameters for the marker rectification */
    virtual void getQuadRectificationParameters(Size &markerSizeWithBorder,
                                                Size &markerSizeWithoutBorder) = 0;
        
  };
}

#endif