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

#include <assert.h>

#include <X11/X.h>

#include <Inventor/SoSceneManager.h>

#include <soxtdefs.h>
#include <Inventor/Xt/devices/SoXtMouse.h>
#include <Inventor/Xt/devices/SoXtKeyboard.h>

#include <Inventor/Xt/SoXtRenderArea.h>

// *************************************************************************

/*!
*/

void
SoXtRenderArea::processEvent( // virtual, protected
  XAnyEvent * event )
{
  if ( this->invokeAppCB( event ) )
    return;

  inherited::processEvent( event );

  const SoEvent * sceneEvent = NULL;

  if ( this->devices.extra )
    for ( int i = 0; i < this->devices.extra->getLength() &&
                     sceneEvent == NULL; i++ )
      ((SoXtDevice *)(*this->devices.extra)[i])->translateEvent( event );

  if ( ! sceneEvent && this->devices.mouse )
    sceneEvent = this->devices.mouse->translateEvent( event );

  if ( ! sceneEvent && this->devices.keyboard )
    sceneEvent = this->devices.keyboard->translateEvent( event );

  if ( sceneEvent != NULL ) {
    assert( this->normalManager != NULL );
    assert( this->overlayManager != NULL );
    if ( ! this->overlayManager->processEvent( sceneEvent ) )
      this->normalManager->processEvent( sceneEvent );
  }
} // processEvent()

// *************************************************************************

#if SOXT_DEBUG
static const char * getSoXtRenderAreaExtraRCSId(void) { return rcsid; }
#endif // SOXT_DEBUG

