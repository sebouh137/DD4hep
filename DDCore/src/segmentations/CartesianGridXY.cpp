/*
 * CartesianGridXY.cpp
 *
 *  Created on: Jun 28, 2013
 *      Author: Christian Grefe, CERN
 */

#include "DDSegmentation/CartesianGridXY.h"

namespace dd4hep {
namespace DDSegmentation {

/// default constructor using an encoding string
CartesianGridXY::CartesianGridXY(const std::string& cellEncoding) :
		CartesianGrid(cellEncoding) {
	// define type and description
	_type = "CartesianGridXY";
	_description = "Cartesian segmentation in the local XY-plane";

	// register all necessary parameters
	registerParameter("grid_size_x", "Cell size in X", _gridSizeX, 1., SegmentationParameter::LengthUnit);
	registerParameter("grid_size_y", "Cell size in Y", _gridSizeY, 1., SegmentationParameter::LengthUnit);
	registerParameter("offset_x", "Cell offset in X", _offsetX, 0., SegmentationParameter::LengthUnit, true);
	registerParameter("offset_y", "Cell offset in Y", _offsetY, 0., SegmentationParameter::LengthUnit, true);
	registerParameter("stagger_x", "Option to stagger the layers in x (ie, add grid_size_x/2 to offset_x for odd layers)", _staggerX, 0);
	registerParameter("stagger_y", "Option to stagger the layers in y (ie, add grid_size_y/2 to offset_y for odd layers)", _staggerY, 0);
	registerIdentifier("identifier_x", "Cell ID identifier for X", _xId, "x");
	registerIdentifier("identifier_y", "Cell ID identifier for Y", _yId, "y");
}

/// Default constructor used by derived classes passing an existing decoder
CartesianGridXY::CartesianGridXY(const BitFieldCoder* decode) :
		CartesianGrid(decode)
{
	// define type and description
	_type = "CartesianGridXY";
	_description = "Cartesian segmentation in the local XY-plane";

	// register all necessary parameters
	registerParameter("grid_size_x", "Cell size in X", _gridSizeX, 1., SegmentationParameter::LengthUnit);
	registerParameter("grid_size_y", "Cell size in Y", _gridSizeY, 1., SegmentationParameter::LengthUnit);
	registerParameter("offset_x", "Cell offset in X", _offsetX, 0., SegmentationParameter::LengthUnit, true);
	registerParameter("offset_y", "Cell offset in Y", _offsetY, 0., SegmentationParameter::LengthUnit, true);
	registerIdentifier("identifier_x", "Cell ID identifier for X", _xId, "x");
	registerIdentifier("identifier_y", "Cell ID identifier for Y", _yId, "y");
}

/// destructor
CartesianGridXY::~CartesianGridXY() {

}

/// determine the position based on the cell ID
Vector3D CartesianGridXY::position(const CellID& cID) const {
	Vector3D cellPosition;
	int layer= _decoder->get(cID,"layer");
	cellPosition.X = binToPosition( _decoder->get(cID,_xId ), _gridSizeX, _offsetX+_staggerX*_gridSizeX*(layer%2)/2.);
	cellPosition.Y = binToPosition( _decoder->get(cID,_yId ), _gridSizeY, _offsetY+_staggerY*_gridSizeY*(layer%2)/2.);
	return cellPosition;
}

/// determine the cell ID based on the position
  CellID CartesianGridXY::cellID(const Vector3D& localPosition, const Vector3D& /* globalPosition */, const VolumeID& vID) const {
        CellID cID = vID ;
	_decoder->set( cID,_xId, positionToBin(localPosition.X, _gridSizeX, _offsetX+_staggerX*_gridSizeX*(layer%2)/2) );
	_decoder->set( cID,_yId, positionToBin(localPosition.Y, _gridSizeY, _offsetY+_staggerY*_gridSizeY*(layer%2)/2) );
	return cID ;
}

std::vector<double> CartesianGridXY::cellDimensions(const CellID&) const {
#if __cplusplus >= 201103L
  return {_gridSizeX, _gridSizeY};
#else
  std::vector<double> cellDims(2,0.0);
  cellDims[0] = _gridSizeX;
  cellDims[1] = _gridSizeY;
  return cellDims;
#endif
}


} /* namespace DDSegmentation */
} /* namespace dd4hep */

// This is done DDCore/src/plugins/ReadoutSegmentations.cpp so the plugin is not part of libDDCore
// needs also #include "DD4hep/Factories.h"
// DECLARE_SEGMENTATION(CartesianGridXY,dd4hep::create_segmentation<dd4hep::DDSegmentation::CartesianGridXY>)
