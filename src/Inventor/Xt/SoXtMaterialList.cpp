/**************************************************************************
 *
 *  This file is part of the Coin SoXt GUI binding library.
 *  Copyright (C) 2000 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License version
 *  2.1 as published by the Free Software Foundation.  See the file
 *  LICENSE.LGPL at the root directory of the distribution for all the
 *  details.
 *
 *  If you want to use Coin SoXt for applications not compatible with the
 *  LGPL, please contact SIM to acquire a Professional Edition License.
 *
 *  Systems in Motion, Prof Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ support@sim.no Voice: +47 22114160 Fax: +47 22207097
 *
 **************************************************************************/

#if SOXT_DEBUG
static const char rcsid[] =
  "$Id$";
#endif // SOXT_DEBUG

#include <assert.h>
#include <stdlib.h>

#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeBG.h>
#include <Xm/PushBG.h>
#include <Xm/List.h>

#include <Inventor/SbPList.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/SoDB.h>
#include <Inventor/SoInput.h>
#include <Inventor/nodes/SoMaterial.h>

#include <soxtdefs.h>
#include <Inventor/Xt/SoXt.h>
#include <Inventor/Xt/SoAnyMaterialList.h>

#include <Inventor/Xt/SoXtMaterialList.h>

// *************************************************************************

/*!
  \class SoXtMaterialList Inventor/Xt/SoXtMaterialList.h
  \brief The SoXtMaterialList class is a component for adding material
  selection to scene graph editors.
*/

// *************************************************************************

SOXT_TYPED_OBJECT_SOURCE(SoXtMaterialList, SoXtComponent);

// *************************************************************************

/*!
  Public constructor.
*/

SoXtMaterialList::SoXtMaterialList(
  Widget parent,
  const char * const name,
  SbBool embed,
  const char * const dir )
: inherited( parent, name, embed )
{
  this->constructor( dir, TRUE );
} // SoXtMaterialList()

/*!
  Protected constructor.
*/

SoXtMaterialList::SoXtMaterialList( // protected
  Widget parent,
  const char * const name,
  SbBool embed,
  const char * const dir,
  SbBool build )
: inherited( parent, name, embed )
{
  this->constructor( dir, build );
} // SoXtMaterialList()

/*!
  Common constructor code.
*/

void
SoXtMaterialList::constructor( // private
  const char * const dir,
  const SbBool build )
{
  this->common = new SoAnyMaterialList( dir );
  this->setSize( SbVec2s( 200, 300 ) );

  if ( build ) {
    this->setClassName( "SoXtMaterialList" );
    Widget materials = this->buildWidget( this->getParentWidget() );
    this->setBaseWidget( materials );
  }
} // constructor()

/*!
  Destructor.
*/

SoXtMaterialList::~SoXtMaterialList(
  void )
{
  delete this->common;
} // ~SoXtMaterialList()

// *************************************************************************

/*!
  FIXME: write doc
*/

void
SoXtMaterialList::addCallback(
  SoXtMaterialListCB * const callback,
  void * const closure )
{
  common->addCallback( callback, closure );
} // addCallback()

/*!
  FIXME: write doc
*/

void
SoXtMaterialList::removeCallback(
  SoXtMaterialListCB * const callback,
  void * const closure )
{
  this->common->removeCallback( callback, closure );
} // removeCallback()

// *************************************************************************

/*!
  FIXME: write doc
*/

const char *
SoXtMaterialList::getDefaultWidgetName( // virtual, protected
  void ) const
{
  static const char defaultWidgetName[] = "SoXtMaterialList";
  return defaultWidgetName;
} // getDefaultWidgetName()

/*!
  FIXME: write doc
*/

const char *
SoXtMaterialList::getDefaultTitle( // virtual, protected
  void ) const
{
  static const char defaultTitle[] = "Material List";
  return defaultTitle;
} // getDefaultTitle()

/*!
  FIXME: write doc
*/

const char *
SoXtMaterialList::getDefaultIconTitle( // virtual, protected
  void ) const
{
  static const char defaultIconTitle[] = "Material List";
  return defaultIconTitle;
} // getDefaultIconTitle()

// *************************************************************************

/*!
  FIXME: write doc
*/

Widget
SoXtMaterialList::buildWidget( // protected
  Widget parent )
{
  // we're building pulldown menu automatically...
  Widget root = XtVaCreateManagedWidget( this->getClassName(),
    xmFormWidgetClass, parent,
    NULL );

  Widget menubar = XtVaCreateManagedWidget( "menubar",
    xmRowColumnWidgetClass, root,
    XmNleftAttachment, XmATTACH_FORM,
    XmNtopAttachment, XmATTACH_FORM,
    XmNrightAttachment, XmATTACH_FORM,
    XmNrowColumnType, XmMENU_BAR,
    NULL );

  Widget palettemenu = this->buildPulldownMenu( menubar );

  SoXtMaterialDirectory * dir = common->getMaterialDirectory();

  int group, materials;
  XmStringTable list;
  if ( dir->numGroups > 0 ) {
    group = dir->current;
    materials = dir->groups[group]->numMaterials;
    list = (XmStringTable) XtMalloc( sizeof(XmString) * materials );
    for ( int i = 0; i < materials; i++ )
      list[i] =
        SoXt::encodeString( dir->groups[group]->materials[i]->name );
  } else {
    materials = 0;
    list = NULL;
  }

  Arg argv[10];
  int argc = 0;
  XtSetArg( argv[argc], XmNleftAttachment, XmATTACH_FORM ); argc++;
  XtSetArg( argv[argc], XmNtopAttachment, XmATTACH_WIDGET ); argc++;
  XtSetArg( argv[argc], XmNtopWidget, menubar ); argc++;
  XtSetArg( argv[argc], XmNrightAttachment, XmATTACH_FORM ); argc++;
  XtSetArg( argv[argc], XmNbottomAttachment, XmATTACH_FORM ); argc++;
  XtSetArg( argv[argc], XmNselectionPolicy, XmSINGLE_SELECT ); argc++;
  XtSetArg( argv[argc], XmNscrollBarDisplayPolicy, XmSTATIC ); argc++;
  XtSetArg( argv[argc], XmNitems, list ); argc++;
  XtSetArg( argv[argc], XmNitemCount, materials ); argc++;

  this->listwidget = XmCreateScrolledList( root, "materials", argv, argc );
  XtManageChild( this->listwidget );
  XtAddCallback( this->listwidget, XmNdefaultActionCallback,
    SoXtMaterialList::selection_cb, (XtPointer) this );
  for ( int i = 0; i < materials; i++ )
    XmStringFree( list[i] );
  XtFree( (char *) list );
  return root;
} // buildWidget()

// *************************************************************************

/*!
  \internal
  FIXME: write doc
*/

void
SoXtMaterialList::selectionCallback( // private
  int materialid )
{
  materialid--; // get index
  SoXtMaterialDirectory * data = common->getMaterialDirectory();

  assert( materialid >= 0 &&
          materialid < data->groups[data->current]->numMaterials );
  const char * materialdata =
    data->groups[data->current]->materials[materialid]->data;

  SoInput reader;
  if ( data->flags & SOXT_BUILTIN_MATERIALS ) {
    reader.setBuffer( (void *) materialdata, strlen(materialdata) );
  } else {
    if ( ! reader.openFile( materialdata, FALSE ) ) {
      SoDebugError::postWarning( "SoXtMaterialList::selectionCallback",
        "could not open file: \"%s\"", materialdata );
      return;
    }
  }

  SoNode * material = NULL;
  if ( ! SoDB::read( &reader, material ) ) {
    SoDebugError::postWarning( "SoXtMaterialList::selectionCallback",
      "failed to read material" );
    return;
  }
  if ( ! material ) {
    SoDebugError::postWarning( "SoXtMaterialList::selectionCallback",
      "read returned no data" );
    return;
  }
  material->ref();
  if ( ! material->isOfType( SoMaterial::getClassTypeId() ) ) {
    SoDebugError::postWarning( "SoXtMaterialList::selectionCallback",
      "not a material node!" );
    material->unref();
    return;
  }
  common->invokeCallbacks( (SoMaterial *) material );
  material->unref();
} // selectionCallback()

/*!
  \internal
  FIXME: write doc
*/

void
SoXtMaterialList::selection_cb( // static, private
  Widget,
  XtPointer closure,
  XtPointer call_data )
{
  SoXtMaterialList * component = (SoXtMaterialList *) closure;
  XmListCallbackStruct * data = (XmListCallbackStruct *) call_data;
  component->selectionCallback( data->item_position );
} // selection_cb()

// *************************************************************************

/*!
  \internal
  FIXME: write doc
*/

void
SoXtMaterialList::paletteMenuCallback( // private
  Widget menuitem )
{
  SoXtMaterialDirectory * data = common->getMaterialDirectory();
  int i, j;
  for ( i = 0; i < data->numGroups; i++ ) {
    if ( data->groups[i]->menuitem == menuitem ) {
      XmStringTable list = (XmStringTable) XtMalloc( sizeof(XmString) *
        data->groups[i]->numMaterials );
      for ( j = 0; j < data->groups[i]->numMaterials; j++ )
        list[j] = SoXt::encodeString( data->groups[i]->materials[j]->name );
      XtVaSetValues( this->listwidget,
        XmNitemCount, data->groups[i]->numMaterials,
        XmNitems, list,
        NULL );
      for ( j = 0; j < data->groups[i]->numMaterials; j++ )
        XmStringFree( list[j] );
      XtFree( (char *) list );
      data->current = i;
      return;
    }
  }
} // paletteMenuCallback()

/*!
  \internal
  FIXME: write doc
*/

void
SoXtMaterialList::palette_menu_cb( // static, private
  Widget widget,
  XtPointer closure,
  XtPointer )
{
  assert( closure != NULL );
  SoXtMaterialList * component = (SoXtMaterialList *) closure;
  component->paletteMenuCallback( widget );
} /* palette_menu_cb */

// *************************************************************************

/*!
  This method builds the pulldown menu.
  The current implementation demands that the \a parent argument should be
  the menubar RowColumn widget - only the "Palettes" menubar button and the
  pulldown menu is built by this method.
*/

Widget
SoXtMaterialList::buildPulldownMenu( // protected
  Widget parent )
{
  Widget palettes = XtVaCreateManagedWidget( "palettes",
    xmCascadeButtonGadgetClass, parent,
    XtVaTypedArg,
      XmNlabelString, XmRString,
      "Palettes", sizeof("Palettes") + 1,
    NULL );

  Widget shell = SoXt::getShellWidget( parent );
  assert( shell != (Widget) NULL );

  Visual * visual;
  Colormap colormap;
  int depth;

  XtVaGetValues( shell,
    XmNvisual, &visual,
    XmNcolormap, &colormap,
    XmNdepth, &depth,
    NULL );

  Widget menu =
    XmVaCreateSimplePulldownMenu( parent, "materialsmenu", 0, NULL, NULL );

  SoXtMaterialDirectory * data = common->getMaterialDirectory();
  for ( int i = 0; i < data->numGroups; i++ ) {
    Widget item;
    item = XtVaCreateManagedWidget( data->groups[i]->name,
             xmPushButtonGadgetClass, menu,
             XtVaTypedArg,
               XmNlabelString, XmRString,
               data->groups[i]->name, strlen( data->groups[i]->name ) + 1,
             NULL );
    data->groups[i]->menuitem = item;
    XtAddCallback( item, XmNactivateCallback,
      SoXtMaterialList::palette_menu_cb, (XtPointer) this );
  }

  return palettes;
} // buildPulldownMenu()

// *************************************************************************

#if SOXT_DEBUG
static const char * getSoXtMaterialListRCSId(void) { return rcsid; }
#endif // SOXT_DEBUG

