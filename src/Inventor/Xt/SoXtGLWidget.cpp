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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <assert.h>

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/DrawingA.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xmu/StdCmap.h>

#include <Inventor/errors/SoDebugError.h>
#include <Inventor/misc/SoBasic.h>
#include <Inventor/SbViewportRegion.h>

#include <soxtdefs.h>
#include <Inventor/Xt/SoXt.h>
#include <Inventor/Xt/SoXtResource.h>
#include <Inventor/Xt/widgets/SoXtGLArea.h>

#include <Inventor/Xt/SoXtGLWidget.h>

// *************************************************************************

/*!
*/

SoXtGLWidget::SoXtGLWidget( // protected
  Widget parent,
  const char * name,
  SbBool embed,
  int glModes,
  SbBool build )
: inherited( parent, name, embed )
, waitForExpose( TRUE )
, drawToFrontBuffer( FALSE )
, glxWidget( NULL )
{
  this->glLockLevel = 0;
  this->doubleBuffer = TRUE;
  this->normalContext = NULL;
  this->overlayContext = NULL;
  this->normalVisual = NULL;
  this->overlayVisual = NULL;

  this->border = FALSE;
  this->borderwidth = 2;
  this->glSize = SbVec2s( -1, -1 );

  if ( build ) {
    Widget glarea = this->buildWidget( this->getParentWidget() );
    this->setBaseWidget( glarea );
    XtVaSetValues( glarea,
      XmNleftAttachment, XmATTACH_FORM,
      XmNtopAttachment, XmATTACH_FORM,
      XmNrightAttachment, XmATTACH_FORM,
      XmNbottomAttachment, XmATTACH_FORM,
      NULL );
  }
} // SoXtGLWidget()

/*!
*/

SoXtGLWidget::~SoXtGLWidget( // virtual, protected
  void )
{
} // ~SoXtGLWidget()

// *************************************************************************

/*!
*/

unsigned long
SoXtGLWidget::getOverlayTransparentPixel(
  void )
{
  return this->transparentPixel;
} // getOverlayTransparentPixel()

/*!
*/

int
SoXtGLWidget::getOverlayColorMapSize(
  void )
{
  return this->overlayColorMapSize;
} // getOverlayColorMapSize()

/*!
*/

int
SoXtGLWidget::getColorMapSize(
  void )
{
  return this->normalColorMapSize;
} // getColorMapSize()

/*!
*/

Window
SoXtGLWidget::getNormalWindow(
  void )
{
  assert( this->glxWidget != (Widget) NULL );
  return XtWindow( this->glxWidget );
} // getNormalWindow()

/*!
*/

Window
SoXtGLWidget::getOverlayWindow(
  void )
{
  SOXT_STUB();
  return (Window) NULL;
} // getOverlayWindow()

/*!
*/

GLXContext
SoXtGLWidget::getNormalContext(
  void )
{
  return this->normalContext;
} // getNormalContext()

/*!
*/

GLXContext
SoXtGLWidget::getOverlayContext(
  void )
{
  return this->overlayContext;
} // getOverlayContext()

/*!
*/

Widget
SoXtGLWidget::getNormalWidget(
  void )
{
  return this->glxWidget;
} // getNormalWidget()

/*!
*/

Widget
SoXtGLWidget::getOverlayWidget(
  void )
{
  SOXT_STUB();
  return (Widget) NULL;
} // getOverlayWidget()

/*!
*/

void
SoXtGLWidget::setNormalVisual( // virtual
  XVisualInfo * visual )
{
  this->normalVisual = visual;
} // setNormalVisual()

/*!
*/

XVisualInfo *
SoXtGLWidget::getNormalVisual(
  void )
{
  return this->normalVisual;
} // setNormalVisual()

/*!
*/

void
SoXtGLWidget::setOverlayVisual( // virtual
  XVisualInfo * visual )
{
  this->overlayVisual = visual;
} // setOverlayVisual()

/*!
*/

XVisualInfo *
SoXtGLWidget::getOverlayVisual(
  void )
{
  return this->overlayVisual;
} // getOverlayVisual()

/*!
*/

void
SoXtGLWidget::setDoubleBuffer( // virtual
  SbBool enable )
{
  this->doubleBuffer = enable;
} // setDoubleBuffer()

/*!
*/

SbBool
SoXtGLWidget::isDoubleBuffer(
  void )
{
  return this->doubleBuffer;
} // isDoubleBuffer()

/*!
*/

void
SoXtGLWidget::setBorder(
  SbBool enable )
{
  this->border = enable;
  if ( this->glxWidget != (Widget) NULL ) {
    if ( this->border == FALSE )
      XtVaSetValues( this->glxWidget,
        XmNleftOffset, 0,
        XmNtopOffset, 0,
        XmNrightOffset, 0,
        XmNbottomOffset, 0,
        NULL );
    else
      XtVaSetValues( this->glxWidget,
        XmNleftOffset, this->borderwidth,
        XmNtopOffset, this->borderwidth,
        XmNrightOffset, this->borderwidth,
        XmNbottomOffset, this->borderwidth,
        NULL );
  }
} // setBorder()

/*!
*/

SbBool
SoXtGLWidget::isBorder(
  void ) const
{
  return this->border;
} // isBorder()

/*!
*/

void
SoXtGLWidget::setDrawToFrontBufferEnable(
  SbBool enable )
{
  this->drawToFrontBuffer = enable;
} // setDrawToFrontBufferEnable()

/*!
*/

SbBool
SoXtGLWidget::isDrawToFrontBufferEnable(
  void ) const
{
  return this->drawToFrontBuffer;
} // isDrawToFrontBufferEnable()

/*!
*/

void
SoXtGLWidget::redrawOverlay( // virtual, protected
  void )
{
  SOXT_STUB();
} // redrawOverlay()

/*!
*/

void
SoXtGLWidget::processEvent( // virtual, protected
  XAnyEvent * event )
{
  switch ( event->type ) {
  case MapNotify:
    this->glInit();
    break;

  case Expose:
    this->glRender();
    this->waitForExpose = FALSE; // Gets flipped from TRUE on first expose.
    break;

  case ConfigureNotify:
    if ( this->glxWidget != (Widget) NULL ) {
      Dimension width, height;
      XtVaGetValues( this->glxWidget,
          XmNwidth, &width, XmNheight, &height, NULL );
      this->glReshape( width, height );
    }
    break;

  } // switch ( event->type )

} // processEvent()

/*!
*/

void
SoXtGLWidget::initGraphic( // virtual, protected
  void )
{
  SOXT_STUB();
} // initGraphic()

/*!
*/

void
SoXtGLWidget::initOverlayGraphic( // virtual, protected
  void )
{
  SOXT_STUB();
} // initOverlayGraphic()

/*!
*/

void
SoXtGLWidget::sizeChanged( // virtual, protected
  const SbVec2s size )
{
  // nothing to do
} // sizeChanged()

/*!
*/

void
SoXtGLWidget::widgetChanged( // virtual, protected
  Widget widget )
{
  SOXT_STUB();
} // widgetChanged()

// *************************************************************************

/*!
*/

void
SoXtGLWidget::setGlxSize( // protected
  const SbVec2s size )
{
  this->setGLSize( size );
} // setGlxSize()

/*!
*/

void
SoXtGLWidget::setGLSize( // protected
  const SbVec2s size )
{
  this->glSize = size;
  if ( this->glxWidget != (Widget) NULL ) {
    Dimension width = size[0];
    Dimension height = size[1];
    XtVaSetValues( this->glxWidget,
      XmNwidth, width,
      XmNheight, height,
      NULL );
  }
} // setGLSize()

/*!
*/

const SbVec2s
SoXtGLWidget::getGlxSize( // protected
  void ) const
{
  return this->getGLSize();
} // getGlxSize()

/*!
*/

const SbVec2s
SoXtGLWidget::getGLSize( // protected
  void ) const
{
  return this->glSize;
} // getGLSize()

/*!
*/

float
SoXtGLWidget::getGLAspectRatio(
  void ) const
{
  assert( this->glxWidget != (Widget) NULL );
  Dimension width, height;
  XtVaGetValues( this->glxWidget, XmNwidth, &width, XmNheight, &height, NULL );
  return float(width) / float(height);
} // getGLAspectRatio()

// *************************************************************************

/*!
*/

void
SoXtGLWidget::setStereoBuffer( // protected
  SbBool enable )
{
  SOXT_STUB();
} // setStereoBuffer()

/*!
*/

SbBool
SoXtGLWidget::isStereoBuffer( // protected
  void )
{
  SOXT_STUB();
  return FALSE;
} // isStereoBuffer()

/*!
*/

SbBool
SoXtGLWidget::isRGBMode( // protected
  void )
{
  SOXT_STUB();
  return TRUE;
} // isRGBMode()

/*!
*/

int
SoXtGLWidget::getDisplayListShareGroup( // protected
  GLXContext context )
{
  SOXT_STUB();
  return 0;
} // getDisplayListShareGroup()

// *************************************************************************

/*!
*/

void
SoXtGLWidget::eventHandler( // static, protected
  Widget widget,
  SoXtGLWidget * user,
  XEvent * event,
  Boolean * dispatch )
{
  user->processEvent( (XAnyEvent *) event );
  *dispatch = False;
} // eventHandler()

/*!
*/

Widget
SoXtGLWidget::buildWidget( // protected
  Widget parent )
{
#if SOXT_DEBUG && 0
  SoDebugError::postInfo( "SoXtGLWidget::buildWidget", "[enter]" );
#endif // SOXT_DEBUG

  this->glxManager = XtVaCreateManagedWidget( "SoXtRenderArea",
    xmFormWidgetClass, parent,
    XmNleftAttachment, XmATTACH_FORM,
    XmNtopAttachment, XmATTACH_FORM,
    XmNrightAttachment, XmATTACH_FORM,
    XmNbottomAttachment, XmATTACH_FORM,
    NULL );
  this->registerWidget( this->glxManager );

  SoXtResource rsc( this->glxManager );

  short width = 0;
  if ( rsc.getResource( "borderThickness", XmRShort, width ) )
    this->borderwidth = width;

  SbBool haveborder = FALSE;
  if ( rsc.getResource( "border", XmRBoolean, haveborder ) )
    this->border = haveborder;

  static int double_good_attrs[] = {
    GLX_RGBA,
    GLX_RED_SIZE, 4, GLX_GREEN_SIZE, 4, GLX_BLUE_SIZE, 4,
    GLX_DOUBLEBUFFER,
    GLX_DEPTH_SIZE, 1,
    GLX_STENCIL_SIZE, 1,
    None
  };

  static int double_poor_attrs[] = {
    GLX_RGBA,
    GLX_DOUBLEBUFFER,
    GLX_DEPTH_SIZE, 1,
    GLX_STENCIL_SIZE, 1,
    None
  };

  static int single_good_attrs[] = {
    GLX_RGBA,
    GLX_RED_SIZE, 4, GLX_GREEN_SIZE, 4, GLX_BLUE_SIZE, 4,
    GLX_DEPTH_SIZE, 1,
    GLX_STENCIL_SIZE, 1,
    None
  };

  static int single_poor_attrs[] = {
    GLX_RGBA,
    GLX_DEPTH_SIZE, 1,
    GLX_STENCIL_SIZE, 1,
    None
  };

  Display * const display = SoXt::getDisplay();

  if ( this->normalVisual == NULL )
    this->normalVisual =
      glXChooseVisual( display, DefaultScreen(display), double_good_attrs );

  if ( this->normalVisual == NULL )
    this->normalVisual =
      glXChooseVisual( display, DefaultScreen(display), double_poor_attrs );

  if ( this->normalVisual == NULL ) {
#if SOXT_DEBUG
    SoDebugError::postInfo( "SoXtGLWidget::buildWidget",
      "could not double-buffer - trying single-buffering" );
#endif // SOXT_DEBUG
    this->doubleBuffer = FALSE;
    this->normalVisual =
      glXChooseVisual( display, DefaultScreen( display ), single_good_attrs );

    if ( this->normalVisual == NULL )
      this->normalVisual =
        glXChooseVisual( display, DefaultScreen( display ), single_poor_attrs );

    if ( this->normalVisual == NULL ) {
      SoDebugError::post( "SoXtGLWidget::buildWidget",
        "could not get visual with RGB and ZBUFFER capabilities" );
      XtAppError( SoXt::getAppContext(), "SoXtGLWidget::buildWidget()" );
    }
  }

  if ( (this->normalVisual->c_class != TrueColor) &&
       (this->normalVisual->c_class != PseudoColor) ) {
    SoDebugError::post( "SoXtGLWidget::buildWidget",
        "Visual hasn't the necessary color capabilities" );
    XtAppError( SoXt::getAppContext(), "SoXtGLWidget::buildWidget()" );
  }

  Colormap colors = NULL;
  XStandardColormap * cmaps = NULL;
  int nmaps = 0;

  if ( XmuLookupStandardColormap(
         display, this->normalVisual->screen, this->normalVisual->visualid,
         this->normalVisual->depth, XA_RGB_DEFAULT_MAP, False, True ) &&
       XGetRGBColormaps( display,
         RootWindow( display, this->normalVisual->screen ), &cmaps, &nmaps,
         XA_RGB_DEFAULT_MAP ) )
  {
    SbBool found = FALSE;
    for ( int i = 0; i < nmaps && ! found; i++ ) {
      if ( cmaps[i].visualid == this->normalVisual->visualid ) {
#if SOXT_DEBUG && 0
        SoDebugError::postInfo( "SoXtGLWidget::buildWidget",
          "got shared color map" );
#endif // SOXT_DEBUG
        colors = cmaps[i].colormap;
        XFree( cmaps );
        found = TRUE;
      }
    }
    if ( ! found )
      colors = XCreateColormap( display,
                   RootWindow( display, this->normalVisual->screen ),
                   this->normalVisual->visual, AllocNone );
  } else {
    colors = XCreateColormap( display,
                 RootWindow( display, this->normalVisual->screen ),
                 this->normalVisual->visual, AllocNone );
  }

  this->glxWidget = XtVaCreateManagedWidget( "SoXtGLWidget",
    soxtGLAreaWidgetClass, this->glxManager,
    SoXtNvisualInfo, this->normalVisual,
    XmNcolormap, colors,
    SoXtNstencilSize, 1,
    XmNleftAttachment, XmATTACH_FORM,
    XmNtopAttachment, XmATTACH_FORM,
    XmNrightAttachment, XmATTACH_FORM,
    XmNbottomAttachment, XmATTACH_FORM,
    NULL );
  this->registerWidget( this->glxWidget );

  this->setBorder( this->isBorder() ); // "refresh" the widget offsets

  // Our callback has this signature:
  // (void (*)(_WidgetRec *, SoXtGLWidget *, XAnyEvent *, char *))
  // ..so we need to cast to avoid a compiler warning or error.
  XtAddEventHandler( this->glxWidget,
      ExposureMask | StructureNotifyMask | ButtonPressMask |
      ButtonReleaseMask | PointerMotionMask | KeyPressMask | KeyReleaseMask,
      False,
      (void (*)(_WidgetRec *, void *, _XEvent *, char *))
        SoXtGLWidget::eventHandler,
      this );

#if SOXT_DEBUG && 0
  SoDebugError::postInfo( "SoXtGLWidget::buildWidget", "[exit]" );
#endif // SOXT_DEBUG
  return this->glxManager;
} // buildWidget()

/*!
*/

Widget
SoXtGLWidget::getGlxMgrWidget( // protected
  void )
{
  return this->glxManager;
} // getGlxMgrWidget()

/*!
*/

Widget
SoXtGLWidget::getGLWidget( // protected
  void )
{
  return this->glxWidget;
} // getGLWidget()

// *************************************************************************

/*!
*/

void
SoXtGLWidget::glLock(
  void )
{
  assert( this->glxWidget != (Widget) NULL );
  this->glLockLevel++;
  assert( this->glLockLevel < 10 && "must be programming error" );
  glXMakeCurrent( SoXt::getDisplay(), XtWindow( this->glxWidget ),
                  this->normalContext );
} // glLock()

/*!
*/

void
SoXtGLWidget::glUnlock(
  void )
{
  assert( this->glxWidget != (Widget) NULL );
  this->glLockLevel--;
  assert( this->glLockLevel >= 0 && "programming error" );
} // glUnlock()

/*!
*/

void
SoXtGLWidget::glSwapBuffers(
  void )
{
  assert( this->doubleBuffer != FALSE );
  assert( this->glLockLevel > 0 );
#if SOXT_DEBUG && 0
  SoDebugError::postInfo( "SoXtGLWidget::glSwapBuffers", "called" );
#endif // SOXT_DEBUG
  glXSwapBuffers( SoXt::getDisplay(), XtWindow( this->glxWidget ) );
} // glSwapBuffers()

/*!
*/

void
SoXtGLWidget::glFlushBuffer(
  void )
{
  assert( this->glxWidget != (Widget) NULL );
  assert( this->glLockLevel > 0 );
  // nothing to do...
  glFlush();
} // glFlushBuffer()

// *************************************************************************

/*!
*/

void
SoXtGLWidget::glInit( // virtual
  void )
{
  assert( this->glxWidget != (Widget) NULL );
  XVisualInfo * visual;
  Display * display = SoXt::getDisplay();
  XtVaGetValues( this->glxWidget, SoXtNvisualInfo, &visual, NULL );

  this->normalContext = glXCreateContext( display, visual, None, GL_TRUE );
  if ( ! this->normalContext ) {
    SoDebugError::postInfo( "SoXtGLWidget::glInit",
      "glXCreateContext() returned NULL" );
    XtAppError( SoXt::getAppContext(), "no context" );
  }
  glXMakeCurrent( display, XtWindow( this->glxWidget ), this->normalContext );

  Dimension width, height;
  XtVaGetValues( this->glxWidget, XmNwidth, &width, XmNheight, &height, NULL );
  this->glReshape( width, height );

  glLock();
  glEnable( GL_DEPTH_TEST );
  glUnlock();
} // glInit()

/*!
*/

void
SoXtGLWidget::glReshape( // virtual
  int width,
  int height )
{
  this->glSize = SbVec2s( width, height );
  this->sizeChanged( this->glSize );
} // glReshape()

/*!
*/

void
SoXtGLWidget::glRender( // virtual
  void )
{
  // do nothing - SoXtRenderArea overloads
} // glRender()

// *************************************************************************

#if SOXT_DEBUG
static const char * getSoXtGLWidgetRCSId(void) { return rcsid; }
#endif // SOXT_DEBUG

