/**************************************************************************
 *
 *  Copyright (C) 2000 by Systems in Motion.  All rights reserved.
 *
 *  This file is part of the Coin library.
 *
 *  This file may be distributed under the terms of the Q Public License
 *  as defined by Troll Tech AS of Norway and appearing in the file
 *  LICENSE.QPL included in the packaging of this file.
 *
 *  If you want to use Coin in applications not covered by licenses
 *  compatible with the QPL, you can contact SIM to aquire a
 *  Professional Edition license for Coin.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
 **************************************************************************/

#if SOXT_DEBUG
static const char rcsid[] =
  "$Id$";
#endif // SOXT_DEBUG

#include <Inventor/errors/SoDebugError.h>

#include <soxtdefs.h>
#include <Inventor/Xt/SoXtBasic.h>
#include <Inventor/Xt/viewers/SoAnyConstrainedViewer.h>
#include <Inventor/Xt/viewers/SoXtConstrainedViewer.h>

// *************************************************************************

/*!
  Protected constructor, used by derived components.
*/

SoXtConstrainedViewer::SoXtConstrainedViewer( // protected
  Widget parent,
  const char * const name,
  SbBool embed,
  SoXtFullViewer::BuildFlag flag,
  SoXtViewer::Type type,
  SbBool build )
: inherited( parent, name, embed, flag, type, FALSE )
, common( new SoAnyConstrainedViewer( this ) )
{
  this->upDirection.setValue( 0, 1, 0 );
  this->prevUpDirection.setValue( 0, 1, 0 );
//  this->sceneHeight = 1.0f;
  this->setLeftWheelString( "Tilt" );
  this->setBottomWheelString( "Rotate" );
  this->setRightWheelString( "Dolly" );
} // SoXtConstrainedViewer()

/*!
  Destructor.
*/

SoXtConstrainedViewer::~SoXtConstrainedViewer(
  void )
{
  delete this->common;
} // ~SoXtConstrainedViewer()

// *************************************************************************

/*!
*/

void
SoXtConstrainedViewer::setUpDirection(
  const SbVec3f & vector )
{
  // adjust camera
  this->upDirection = vector;
} // setUpDirection()

/*!
  This method returns the up direction of the viewer.
*/

const SbVec3f &
SoXtConstrainedViewer::getUpDirection(
  void ) const
{
  return this->upDirection;
} // getUpDirection()

// *************************************************************************

/*!
*/

void
SoXtConstrainedViewer::setCamera( // virtual
  SoCamera * camera )
{
  // enforce camera up direction? or get camera up direction?
  inherited::setCamera( camera );
} // setCamera()

/*!
*/

void
SoXtConstrainedViewer::saveHomePosition( // virtual
  void )
{
  this->prevUpDirection = this->upDirection;
  inherited::saveHomePosition();
} // saveHomePosition()

/*!
*/

void
SoXtConstrainedViewer::resetToHomePosition( // virtual
  void )
{
  // restore upDirection in camera
  this->upDirection = this->prevUpDirection;
  inherited::resetToHomePosition();
} // resetToHomePosition()

/*!
*/

void
SoXtConstrainedViewer::recomputeSceneSize( // virtual
  void )
{
  // do anything?
  SOXT_STUB();
  inherited::recomputeSceneSize();
} // recomputeSceneSize()

// *************************************************************************

/*!
*/

void
SoXtConstrainedViewer::tiltCamera( // virtual, protected
  float delta )
{
  common->tiltCamera( delta );
} // tiltCamera()

/*!
*/

void
SoXtConstrainedViewer::bottomWheelMotion( // virtual, protected
  float value )
{
  common->rotateCamera( value - this->getBottomWheelValue() );
  inherited::bottomWheelMotion( value );
} // bottomWheelMotion()

/*!
*/

void
SoXtConstrainedViewer::leftWheelMotion( // virtual, protected
  float value )
{
  this->tiltCamera( value - this->getLeftWheelValue() );
  inherited::leftWheelMotion( value );
} // leftWheelMotion()

/*!
*/

void
SoXtConstrainedViewer::changeCameraValues( // virtual, protected
  SoCamera * camera )
{
  SOXT_STUB();
} // changeCameraValues()

/*!
*/

void
SoXtConstrainedViewer::findUpDirection( // protected
  SbVec2s mousepos )
{
  SOXT_STUB();
} // findUpDirection()

/*!
*/

void
SoXtConstrainedViewer::checkForCameraUpConstrain( // protected
  void )
{
  SOXT_STUB();
} // checkForCamerUpConstrain()

/*!
*/

void
SoXtConstrainedViewer::computeSeekFinalOrientation( // virtual, protected
  void )
{
  SOXT_STUB();
} // computeSeekFinalOrientation()

// *************************************************************************

#if SOXT_DEBUG
static const char * getSoXtConstrainedViewerRCSId(void) { return rcsid; }
#endif // SOXT_DEBUG

