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

// *************************************************************************


// Class documentation in common/SoGuiCommon.cpp.in.

/*!
  \mainpage

  %SoXt is an Xt/Motif glue library for Coin.  It can also be used on
  top of the SGI or TGS implementation of Open Inventor, and is
  designed to be source code compatible with SGI's InventorXt library.

  You will need either Motif (<a
  href="http://www.opengroup.org/openmotif/">Open Motif</a>) or <a
  href="http://www.lesstif.org/">Lesstif</a> installed on your system
  to use %SoXt.

  %SoXt is still in alpha mode, meaning that it is buggy and lacks
  functionality.  In other words, hope for the best but expect the
  worst.

  The corresponding documentation for Coin is located 
  <a href="http://doc.coin3d.org/Coin/">here</a>.
*/

// *************************************************************************

#if SOXT_DEBUG
// Flip to '1' for getting debugging information from the process
// which sets up the X11 visual and colormap we're using.
#define SOXT_SELECTBESTVISUAL_DEBUG 0
#else
#define SOXT_SELECTBESTVISUAL_DEBUG 0
#endif

// *************************************************************************

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <Inventor/errors/SoDebugError.h>
#include <Inventor/SoDB.h>
#include <Inventor/SoInteraction.h>
#include <Inventor/misc/SoBasic.h>
#include <Inventor/SbPList.h>
#include <Inventor/nodekits/SoNodeKit.h>
#include <Inventor/sensors/SoSensorManager.h>
#include <Inventor/SbTime.h>

#include <soxtdefs.h>

#include <Inventor/Xt/SoXt.h>
#include <Inventor/Xt/SoXtObject.h>
#include <Inventor/Xt/devices/SoXtDevice.h>
#include <Inventor/Xt/SoXtComponent.h>
#include <Inventor/Xt/SoAny.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#ifdef HAVE_LIBXMU
#include <X11/Xmu/Editres.h>
#include <X11/Xmu/StdCmap.h>
#include <X11/Xmu/Atoms.h>
#endif // HAVE_LIBXMU
#include <Xm/Xm.h>
#include <Xm/Protocols.h>
#include <Xm/MessageB.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>

#include <assert.h>
#include <stdlib.h>
#include <limits.h>

// *************************************************************************

// The private data for the SoXt class.

class SoXtP {
public:
  static Display * display;
  static XtAppContext xtappcontext;
  static Widget mainwidget;

  static XtIntervalId timersensorid;
  static SbBool timersensoractive;

  static XtIntervalId delaysensorid;
  static SbBool delaysensoractive;

  static XtIntervalId idlesensorid;
  static SbBool idlesensoractive;

  static char * appname;
  static char * appclass;

  static SbPList * eventhandlers;
  static String fallbackresources[];

  static int SOXT_XSYNC;
  static XErrorHandler previous_handler;
  static int X11Errorhandler(Display * d, XErrorEvent * ee);

  static void timerSensorCB(XtPointer, XtIntervalId *);
  static void delaySensorCB(XtPointer, XtIntervalId *);
  static Boolean idleSensorCB(XtPointer);
};

Display * SoXtP::display = NULL;
XtAppContext SoXtP::xtappcontext;
Widget SoXtP::mainwidget = (Widget) NULL;
XtIntervalId SoXtP::timersensorid = 0;
SbBool SoXtP::timersensoractive = FALSE;
XtIntervalId SoXtP::delaysensorid = 0;
SbBool SoXtP::delaysensoractive = FALSE;
XtIntervalId SoXtP::idlesensorid = 0;
SbBool SoXtP::idlesensoractive = FALSE;
SbPList * SoXtP::eventhandlers = NULL;
char * SoXtP::appname = NULL;
char * SoXtP::appclass = NULL;

#define ENVVAR_NOT_INITED INT_MAX
int SoXtP::SOXT_XSYNC = ENVVAR_NOT_INITED;
XErrorHandler SoXtP::previous_handler = NULL;

static Atom WM_PROTOCOLS = 0;
static Atom WM_DELETE_WINDOW = 0;

// *************************************************************************

int
SoXtP::X11Errorhandler(Display * d, XErrorEvent * ee)
{
  // Include misc information on the Display to aid further debugging
  // on our end upon bugreports.

  SbString depthsstr = "";
  int num;
  int * depths = XListDepths(d, DefaultScreen(d), &num);
  if ((depths != NULL) && (num > 0)) {
    depthsstr = "(Available Display depths are:";
    for (int i=0; i < num; i++) {
      depthsstr += ' ';
      depthsstr += SbString(depths[i]);
    }
    depthsstr += ". Default depth is ";
    depthsstr += SbString(DefaultDepth(d, DefaultScreen(d)));
    depthsstr += ".)";
    XFree(depths);
  }

  // Then the instructions:

  SoDebugError::post("SoXtP::X11Errorhandler",
                     "Detected possibly internal SoXt bug. %s %s",

                     SoXtP::SOXT_XSYNC == 1 ? "" :
                     "Set environment variable SOXT_XSYNC to \"1\" and "
                     "re-run the application in a debugger with a breakpoint "
                     "set on SoXtP::X11Errorhandler() to get a valid "
                     "backtrace. "

                     "Then please forward the following information in an "
                     "e-mail to <coin-bugs@coin3d.org> along with the "
                     "backtrace. ",

                     depthsstr.getString());

  SoXtP::previous_handler(d, ee);
  return -1; // shouldn't get here, the system handler will normally exit
}

// *************************************************************************

/*!
  \fn Widget SoXt::init(int & argc, char ** argv, const char * appname, const char * appclass)

  This function initializes the SoXt library.

  The returned Widget is a toplevel shell widget for the application, which
  can be used as a shell for the main component.
*/
Widget
SoXt::internal_init(int & argc, char ** argv,
                    const char * appname, const char * appclass)
{
  assert(SoXtP::previous_handler == NULL && "call SoXt::init() only once!");
  // Intervene upon X11 errors.
  SoXtP::previous_handler = XSetErrorHandler((XErrorHandler)SoXtP::X11Errorhandler);

  // FIXME: as far as I can see, no SoXt::init() method in InventorXt
  // matches the signature of this constructor. So we should probably
  // mark this as SoXt/Coin-only (but look over InventorXt again).
  //
  // Also investigate our other So*-libraries to see if they contain
  // this Coin-specific init() method. 20010919 mortene.

  if (appname)
    SoXtP::appname = strcpy(new char [strlen(appname) + 1], appname);
  if (appclass)
    SoXtP::appclass = strcpy(new char [strlen(appclass) + 1], appclass);

  XtAppContext tempcontext; // SoXtP::xtappcontext is set later

  Display * display = XOpenDisplay(NULL);
  if (display == NULL) {
    SoDebugError::postInfo("SoXt::internal_init", "Failed to open display.");
    // FIXME: invoke the fatal error handler. 20011220 mortene.
    exit(-1);
  }

  // This is _extremely_ useful for debugging X errors: activate this
  // code (set the SOXT_XSYNC environment variable on your system to
  // "1"), then rerun the application code in a debugger with a
  // breakpoint set at SoXtP::X11Errorhandler(). Now you can backtrace
  // to the exact source location of the failing X request.
  if (SoXtP::SOXT_XSYNC == ENVVAR_NOT_INITED) {
    const char * env = SoAny::si()->getenv("SOXT_XSYNC");
    SoXtP::SOXT_XSYNC = env ? atoi(env) : 0;
    if (SoXtP::SOXT_XSYNC) {
      SoDebugError::postInfo("SoXt::internal_init", "Turning on X synchronization.");
      XSynchronize(display, True);
    }
  }

  int depth = 0;
  Visual * visual = NULL;
  Colormap colormap = 0;

  SoXt::selectBestVisual(display, visual, colormap, depth);

  Widget toplevel = (Widget) NULL;
  if (visual) {
    toplevel = XtVaOpenApplication(&tempcontext, SoXtP::appclass, NULL, 0,
                                   &argc, argv,
                                   SoXtP::fallbackresources, topLevelShellWidgetClass,
                                   XmNvisual, visual,
                                   XmNdepth, depth,
                                   XmNcolormap, colormap,
                                   NULL);
  }
  else {
    SoDebugError::postInfo("SoXt::internal_init", "default toplevel! (error)");
    // FIXME: if we get here, a segfault comes up later for me on
    // embla.trh.sim.no, at least. 20020117 mortene.
    toplevel = XtVaOpenApplication(&tempcontext, SoXtP::appclass, NULL, 0,
                                   &argc, argv,
                                   SoXtP::fallbackresources, topLevelShellWidgetClass,
                                   NULL);
  }
  if (appname) { XtVaSetValues(toplevel, XmNtitle, SoXtP::appname, NULL); }
  SoXt::internal_init(toplevel);
  return toplevel;
}

// documented in common/SoGuiObject.cpp.in
void
SoXtObject::init(void)
{
  SoXtObject::initClass();
  SoXtDevice::initClasses();
  SoXtComponent::initClasses();
}

// internal
static void
wm_close_handler(Widget widget, XtPointer user, XEvent * e, Boolean * dispatch)
{
  if (e->type == ClientMessage) {
    XClientMessageEvent * event = (XClientMessageEvent *) e;
    if (WM_PROTOCOLS == None) {
      WM_PROTOCOLS = XInternAtom(SoXt::getDisplay(), "WM_PROTOCOLS", True);
      if (WM_PROTOCOLS == None) WM_PROTOCOLS = (Atom) -1;
    }
    if (WM_DELETE_WINDOW == None) {
      WM_DELETE_WINDOW = XInternAtom(SoXt::getDisplay(),
                                     "WM_DELETE_WINDOW", True);
      if (WM_DELETE_WINDOW == None) { WM_DELETE_WINDOW = (Atom) -1; }
    }
    if (event->message_type == WM_PROTOCOLS &&
        (unsigned) event->data.l[0] == WM_DELETE_WINDOW) {
      XtAppSetExitFlag(SoXt::getAppContext());
      *dispatch = False;
    }
  }
}

/*!
  \fn void SoXt::init(Widget toplevel)

  This function initializes the SoXt library.

  It should be called for applications that want to create the toplevel
  shell widget themselves.
*/
void
SoXt::internal_init(Widget toplevel)
{
  // Intervene upon X11 errors.
  if (SoXtP::previous_handler == NULL) {
    SoXtP::previous_handler = XSetErrorHandler((XErrorHandler)SoXtP::X11Errorhandler);
  }

#if SOXT_DEBUG
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);
#endif // SOXT_DEBUG

  SoXtP::mainwidget = toplevel;
  SoXtP::display = XtDisplay(toplevel);
  SoXtP::xtappcontext = XtWidgetToApplicationContext(toplevel);

  SoDB::init();
  SoNodeKit::init();
  SoInteraction::init();
  SoXtObject::init();

  SoDB::getSensorManager()->setChangedCallback(SoXt::sensorQueueChanged, NULL);

  XtAppSetFallbackResources(SoXt::getAppContext(), SoXtP::fallbackresources);

  XtAddEventHandler(toplevel, (EventMask) 0, True, wm_close_handler, NULL);

  // FIXME: why the HAVE_LIBXMU wrapper? 20020117 mortene.
#ifdef HAVE_LIBXMU
#if SOXT_DEBUG
  XtEventHandler editres_hook = (XtEventHandler) _XEditResCheckMessages;
  XtAddEventHandler(toplevel, (EventMask) 0, True, editres_hook, NULL);
#endif // SOXT_DEBUG
#endif // HAVE_LIBXMU
}

// *************************************************************************

// A debug aid function for translating X11 event type id to event name.
static const char *
debug_eventname(const int type)
{
  static const char * names[LASTEvent];
  static int first = 1;
  if (first) {
    for (int i = 0; i < LASTEvent; i++) { names[i] = "<not set>"; }
    names[KeyPress]          = "KeyPress";
    names[KeyRelease]        = "KeyRelease";
    names[ButtonPress]       = "ButtonPress";
    names[ButtonRelease]     = "ButtonRelease";
    names[MotionNotify]      = "MotionNotify";
    names[EnterNotify]       = "EnterNotify";
    names[LeaveNotify]       = "LeaveNotify";
    names[FocusIn]           = "FocusIn";
    names[FocusOut]          = "FocusOut";
    names[KeymapNotify]      = "KeymapNotify";
    names[Expose]            = "Expose";
    names[GraphicsExpose]    = "GraphicsExpose";
    names[NoExpose]          = "NoExpose";
    names[VisibilityNotify]  = "VisibilityNotify";
    names[DestroyNotify]     = "DestroyNotify";
    names[CreateNotify]      = "CreateNotify";
    first = 0;
  }
  if (type >= 0 && type < LASTEvent) { return names[type]; }
  return "<unknown>";
}

/*!
  This function should be used instead of XtAppMainLoop().  The
  current implementation is no different than the standard event loop,
  but in the future it will add support for input device extensions.

  If you need your own event dispatching loop, base your code on this
  one, and use SoXt::dispatchEvent() when appropriate.
*/
void
SoXt::mainLoop(void)
{
#if SOXT_DEBUG && 0
  SoDebugError::postInfo("SoXt::mainLoop", "[enter]");
#endif // SOXT_DEBUG
  XEvent event;
  XtAppContext context = SoXt::getAppContext();
  Boolean exit = XtAppGetExitFlag(context);
  while (! exit) {
    SoXt::nextEvent(context, &event);
    Boolean dispatched = SoXt::dispatchEvent(&event);
#if SOXT_DEBUG && 0
    if (! dispatched) {
      SoDebugError::postInfo("SoXt::mainLoop", "no event handler (%2d : %s)",
                             event.type, debug_eventname(event.type));
    }
#endif // SOXT_DEBUG
    exit = XtAppGetExitFlag(context);
  }
}

/*!
  This function calls XtAppNextEvent() to fill in the event structure
  with the next event for the given context.
*/
void
SoXt::nextEvent(XtAppContext context, XEvent * event)
{
  XtAppNextEvent(context, event);
}

/*!
  This method dispatches the event by calling XtDispatchEvent().
  Special handling of extension input devices will be handled in this
  method in the future.
*/
Boolean
SoXt::dispatchEvent(XEvent * event)
{
  Boolean dispatched = XtDispatchEvent(event);
  if (! dispatched) {
    XtEventHandler handler;
    XtPointer data;
    Widget widget;
    SoXt::getExtensionEventHandler(event, widget, handler, data);
    if (handler) {
      Boolean dispatch = False;
      (*handler)(widget, data, event, &dispatch);
      if (dispatch == False) { dispatched = True; }
    }
  }
  return dispatched;
}

/*!
  This method makes the event loop stop processing events and exit so
  the program can finish.

  This is an SoXt extension not found in InventorXt.

  \sa SoXt::mainLoop
*/
void
SoXt::exitMainLoop(void)
{
  XtAppSetExitFlag(SoXt::getAppContext());
}

// *************************************************************************

/*!
  This function returns the Xt Application Context for the application.
*/
XtAppContext
SoXt::getAppContext(void)
{
  return SoXtP::xtappcontext;
}

/*!
  This function returns the X Display of the application.
*/
Display *
SoXt::getDisplay(void)
{
  return SoXtP::display;
}

/*!
  This function returns the toplevel shell for the application.
*/
Widget
SoXt::getTopLevelWidget(void)
{
  return SoXtP::mainwidget;
}

// *************************************************************************

/*!
  This function returns the application name, given to SoXt::init().
*/
const char *
SoXt::getAppName(void)
{
  return SoXtP::appname;
}

/*!
  This function returns the application class, given to SoXt::init().
*/
const char *
SoXt::getAppClass(void)
{
  return SoXtP::appclass;
}

// *************************************************************************

/*!
  This function realizes the given \a widget.
*/
void
SoXt::show(Widget const widget)
{
  if (XtIsTopLevelShell(widget)) {
    XtRealizeWidget(widget);
  }
  else {
    XtManageChild(widget);
  }
}

/*!
  This function hides the given \a widget.
*/
void
SoXt::hide(Widget const widget)
{
  if (XtIsTopLevelShell(widget)) {
    XtUnrealizeWidget(widget);
  }
  else {
    XtUnmanageChild(widget);
  }
}

// *************************************************************************

/*!
  Create and return a localized string from \a str.

  The caller is responsible for freeing the returned XmString by using
  XmStringFree() or just plain XtFree((char *) str).
*/
XmString
SoXt::encodeString(const char * const str)
{
  return XmStringCreateLocalized((char *)str);
}

/*!
  Decode a left-to-right localized string into a simple character
  array, and return its memory pointer.

  The caller is responsible for freeing the returned character string
  with XtFree() to avoid memory leaks.
*/
char *
SoXt::decodeString(XmString xstring)
{
  char * str;
  /* set str to point to the text */
  XmStringGetLtoR(xstring, XmSTRING_DEFAULT_CHARSET, &str);
  return str;
}

// *************************************************************************

/*!
  This function resizes the widget to the given \a size.
*/
void
SoXt::setWidgetSize(Widget const widget, const SbVec2s size)
{
  if (! widget) {
#if SOXT_DEBUG
    SoDebugError::postInfo("SoXt::setWidgetSize", "called with no widget");
#endif // SOXT_DEBUG
    return;
  }
  XtVaSetValues(widget,
                XmNwidth, size[0],
                XmNheight, size[1],
                NULL);
}

/*!
  This function returns the size of the given widget.
*/
SbVec2s
SoXt::getWidgetSize(Widget const widget)
{
  if (! widget) {
#if SOXT_DEBUG
    SoDebugError::postInfo("SoXt::getWidgetSize", "called with no widget");
#endif // SOXT_DEBUG
    return SbVec2s(0, 0);
  }
  Dimension width, height;
  XtVaGetValues(widget,
                XtNwidth, &width,
                XtNheight, &height,
                NULL);
  return SbVec2s(width, height);
}

// *************************************************************************

/*!
  This function returns the shell of the given \a widget.
*/
Widget
SoXt::getShellWidget(const Widget widget)
{
  Widget p = widget;
  while (p != (Widget) NULL) {
    if (XtIsShell(p)) { return p; }
    p = XtParent(p);
  }
#if SOXT_DEBUG
  SoDebugError::postInfo("SoXt::getShellWidget",
                         "couldn't find shell for widget at %p", widget);
#endif // SOXT_DEBUG
  return (Widget) NULL;
}

// *************************************************************************

// Internal callback.
static void
close_dialog_cb(Widget, XtPointer closure, XtPointer)
{
  Widget dialogshell = (Widget)closure;
  XtPopdown(dialogshell);
  XtDestroyWidget(dialogshell);
}

// Documented in common/SoGuiCommon.cpp.in.
void
SoXt::createSimpleErrorDialog(Widget parent, const char * title,
                              const char * string1, const char * string2)
{
  // FIXME: placement is not implemented yet. 200XXXXX larsa.

  Arg args[10];
  int argc = 0;

  if (! title) title = "";
  if (! string1) string1 = "";

  Visual * vis;
  Colormap cmap;
  int depth;

  SoXt::selectBestVisual(SoXt::getDisplay(), vis, cmap, depth);

  // The XtVaCreatePopupShell() call will exit the application on a
  // NULL parent pointer.
  if (parent == NULL) { parent = SoXt::getTopLevelWidget(); }

  Widget errdialog = XtVaCreatePopupShell("errordialog",
                                          topLevelShellWidgetClass, parent,
                                          XmNvisual, vis,
                                          XmNcolormap, cmap,
                                          XmNdepth, depth,
                                          XtVaTypedArg,
                                          XmNtitle, XmRString,
                                          title, strlen(title)+1,
                                          XmNresizable, False,
                                          NULL);

  Widget root = XtVaCreateManagedWidget("root",
                                        xmFormWidgetClass, errdialog,
                                        NULL);

  Widget label1 = XtVaCreateManagedWidget("label1",
                                          xmLabelWidgetClass, root,
                                          XmNleftAttachment, XmATTACH_FORM,
                                          XmNleftOffset, 20,
                                          XmNtopAttachment, XmATTACH_FORM,
                                          XmNtopOffset, 10,
                                          XmNrightAttachment, XmATTACH_FORM,
                                          XmNrightOffset, 20,
                                          XtVaTypedArg,
                                          XmNlabelString, XmRString,
                                          string1, strlen(string1),
                                          NULL);

  Widget label2 = (Widget) NULL;
  if (string2 != NULL) {
    label2 = XtVaCreateManagedWidget("label2",
                                     xmLabelWidgetClass, root,
                                     XmNleftAttachment, XmATTACH_FORM,
                                     XmNleftOffset, 20,
                                     XmNtopAttachment, XmATTACH_WIDGET,
                                     XmNtopWidget, label1,
                                     XmNtopOffset, 5,
                                     XmNrightAttachment, XmATTACH_FORM,
                                     XmNrightOffset, 20,
                                     XtVaTypedArg,
                                     XmNlabelString, XmRString,
                                     string2, strlen(string2),
                                     NULL);
  }

  Widget close = XtVaCreateManagedWidget("close",
                                         xmPushButtonWidgetClass, root,
                                         XmNtopAttachment, XmATTACH_WIDGET,
                                         XmNtopWidget, string2 ? label2 : label1,
                                         XmNtopOffset, 5,
                                         XmNrightAttachment, XmATTACH_FORM,
                                         XmNrightOffset, 10,
                                         XmNbottomAttachment, XmATTACH_FORM,
                                         XmNbottomOffset, 10,
                                         XtVaTypedArg,
                                         XmNlabelString, XmRString,
                                         " Close ", 5,
                                         NULL);

  XtAddCallback(close, XmNactivateCallback, close_dialog_cb, errdialog);

  XtPopup(errdialog, XtGrabNone);

  Dimension width = 0;
  Dimension height = 0;
  XtVaGetValues(root,
                XmNwidth, &width,
                XmNheight, &height,
                NULL);
  XtVaSetValues(errdialog,
                XmNheight, height,
                XmNwidth, width,
                XmNminHeight, height,
                XmNmaxHeight, height,
                XmNminWidth, width,
                NULL);
}

// *************************************************************************

/*!
  This function should add the necessary arguments to the \a args
  argument list so the popup shell gets the same visual and colormap
  as the rest of the application.

  This function is not implemented.
*/
void
SoXt::getPopupArgs(Display * display, int screen, ArgList args, int * n)
{
  SOXT_STUB();
}

/*!
  This function is not implemented.
*/
void
SoXt::registerColormapLoad(Widget widget, Widget shell)
{
  SOXT_STUB();
}

/*!
  This function is not implemented.
*/
void
SoXt::addColormapToShell(Widget widget, Widget shell)
{
  SOXT_STUB();
}

/*!
  This function is not implemented.
*/
void
SoXt::removeColormapFromShell(Widget widget, Widget shell)
{
  SOXT_STUB();
}

// *************************************************************************

typedef struct SoXt_EventHandlerInfo {
  int type;
  Widget widget;
  XtEventHandler handler;
  XtPointer data;
} SoXtEventHandlerInfo;

/*!
  This function adds an extension event handler to the application,
  which will be considered in the SoXt::mainLoop event dispatching
  loop.

  \sa SoXt::removeExtensionEventHandler(), SoXt::getExtensionEventHandler()
*/

void
SoXt::addExtensionEventHandler(Widget widget, int type,
                               XtEventHandler proc, XtPointer data)
{
  SoXtEventHandlerInfo * info = new SoXtEventHandlerInfo;
  info->type = type;
  info->widget = widget;
  info->handler = proc;
  info->data = data;

  if (SoXtP::eventhandlers == NULL) { SoXtP::eventhandlers = new SbPList; }

#if SOXT_DEBUG
  const int handlers = SoXtP::eventhandlers->getLength();
  for (int i = 0; i < handlers; i++) {
    SoXtEventHandlerInfo * query = (SoXtEventHandlerInfo *) (*SoXtP::eventhandlers)[i];
    if (query->type == type) {
      SoDebugError::postWarning("SoXt::addExtensionEventHandler",
                                "handler of type %d already exists, shadowing the new handler");
    }
  }
#endif // SOXT_DEBUG

  SoXtP::eventhandlers->append((void *) info);
}

/*!
  This method removes an extension event handler.

  \sa SoXt::addExtensionEventHandler(), SoXt::getExtensionEventHandler()
*/
void
SoXt::removeExtensionEventHandler(Widget widget, int type,
                                  XtEventHandler proc, XtPointer data)
{
  if (SoXtP::eventhandlers == NULL) {
#if SOXT_DEBUG
    SoDebugError::postInfo("SoXt::removeExtensionEventHandler",
                           "no extension event handlers registered.");
#endif // SOXT_DEBUG
    return;
  }
  int handlers = SoXtP::eventhandlers->getLength();
  for (int i = 0; i < handlers; i++) {
    SoXtEventHandlerInfo * info = (SoXtEventHandlerInfo *) (*SoXtP::eventhandlers)[i];
    if (info->widget == widget && info->type == type &&
        info->handler == proc && info->data == data) {
      SoXtP::eventhandlers->remove(i);
      delete info;
      return;
    }
  }
#if SOXT_DEBUG
  SoDebugError::postInfo("SoXt::removeExtensionEventHandler",
                         "no such extension event handler registered.");
#endif // SOXT_DEBUG
}

/*!
  This method returns the extension event handler for the given \a event.

  \sa SoXt::addExtensionEventHandler
  \sa SoXt::removeExtensionEventHandler
*/
void
SoXt::getExtensionEventHandler(XEvent * event, Widget & widget,
                               XtEventHandler & proc, XtPointer & data)
{
  proc = (XtEventHandler) NULL;
  data = (XtPointer) NULL;
  widget = (Widget) NULL;

  if (SoXtP::eventhandlers == NULL) { return; }

  const int handlers = SoXtP::eventhandlers->getLength();
  for (int i = 0; i < handlers; i++) {
    SoXtEventHandlerInfo * info = (SoXtEventHandlerInfo *) (*SoXtP::eventhandlers)[i];
    if (info->type == event->type) {
      widget = info->widget;
      proc = info->handler;
      data = info->data;
    }
  }
}

// *************************************************************************

// private
void
SoXtP::timerSensorCB(XtPointer closure, XtIntervalId * id)
{
#if SOXT_DEBUG && 0
  SoDebugError::postInfo("SoXtP::timerSensorCB", "called");
#endif // SOXT_DEBUG
  SoXtP::timersensorid = 0;
  SoXtP::timersensoractive = FALSE;
  SoDB::getSensorManager()->processTimerQueue();
  SoXt::sensorQueueChanged(NULL);
}

// private
void
SoXtP::delaySensorCB(XtPointer closure, XtIntervalId * id)
{
#if SOXT_DEBUG && 0
  SoDebugError::postInfo("SoXtP::delaySensorCB", "called");
#endif // SOXT_DEBUG
  SoXtP::delaysensorid = 0;
  SoXtP::delaysensoractive = FALSE;
  SoDB::getSensorManager()->processDelayQueue(FALSE);
  SoXt::sensorQueueChanged(NULL);
}

// private
Boolean
SoXtP::idleSensorCB(XtPointer closure)
{
#if SOXT_DEBUG && 0
  SoDebugError::postInfo("SoXtP::idleSensorCB", "called");
#endif // SOXT_DEBUG
  SoXtP::idlesensorid = 0;
  SoXtP::idlesensoractive = FALSE;
  SoDB::getSensorManager()->processDelayQueue(TRUE);
  SoXt::sensorQueueChanged(NULL);
  return True;
}

// *************************************************************************

/*!
  This callback handles events from sensors in the scene graph, needed
  to deal with scene interaction.
*/
void
SoXt::sensorQueueChanged(void *)
{
#if SOXT_DEBUG && 0
  SoDebugError::postInfo("SoXt::sensorQueueChanged", "start");
#endif // SOXT_DEBUG
  SoSensorManager * sensormanager = SoDB::getSensorManager();

  SbTime timevalue;
  if (sensormanager->isTimerSensorPending(timevalue)) {
    SbTime interval = timevalue - SbTime::getTimeOfDay();

#if SOXT_DEBUG && 0
    SoDebugError::postInfo("SoXt::sensorQueueChanged",
                           "interval: %f (msec: %d)", interval.getValue(),
                           interval.getMsecValue());
#endif // debug

    // On a system with some load, the interval value can easily get
    // negative. For Xt, this means it will never trigger -- which
    // causes all kinds of problems. Setting it to 0 will make it
    // trigger more-or-less immediately.
    if (interval.getValue() < 0.0) { interval.setValue(0.0); }

    if (SoXtP::timersensoractive) { XtRemoveTimeOut(SoXtP::timersensorid); }

    SoXtP::timersensorid = XtAppAddTimeOut(SoXt::getAppContext(),
                                           interval.getMsecValue(),
                                           SoXtP::timerSensorCB, NULL);
    SoXtP::timersensoractive = TRUE;

  }
  else if (SoXtP::timersensoractive) {
    XtRemoveTimeOut(SoXtP::timersensorid);
    SoXtP::timersensorid = 0;
    SoXtP::timersensoractive = FALSE;
  }

  if (sensormanager->isDelaySensorPending()) {
    if (! SoXtP::idlesensoractive) {
      SoXtP::idlesensorid =
        XtAppAddWorkProc(SoXt::getAppContext(), SoXtP::idleSensorCB, NULL);
      SoXtP::idlesensoractive = TRUE;
    }

    if (! SoXtP::delaysensoractive) {
      unsigned long timeout = SoDB::getDelaySensorTimeout().getMsecValue();
      SoXtP::delaysensorid =
        XtAppAddTimeOut(SoXt::getAppContext(), timeout, SoXtP::delaySensorCB,
                        NULL);
      SoXtP::delaysensoractive = TRUE;
    }
  }
  else {
    if (SoXtP::idlesensoractive) {
      XtRemoveWorkProc(SoXtP::idlesensorid);
      SoXtP::idlesensorid = 0;
      SoXtP::idlesensoractive = FALSE;
    }
    if (SoXtP::delaysensoractive) {
      XtRemoveTimeOut(SoXtP::delaysensorid);
      SoXtP::delaysensorid = 0;
      SoXtP::delaysensoractive = FALSE;
    }
  }
}

// *************************************************************************

static const char *
debug_visualclassname(const int vclass)
{
  switch (vclass) {
  case StaticGray:   return "StaticGray";
  case GrayScale:    return "GrayScale";
  case StaticColor:  return "StaticColor";
  case PseudoColor:  return "PseudoColor";
  case TrueColor:    return "TrueColor";
  case DirectColor:  return "DirectColor";
  default:           return "<unknown>";
  }
}

static void
debug_dumpvisualinfo(const XVisualInfo * v)
{
  SoDebugError::postInfo("debug_dumpvisualinfo",
                         "Visual==%p, VisualID==%d, screen==%d, "
                         "depth==%d, class==%s, "
                         "rgb masks==[0x%04x, 0x%04x, 0x%04x], "
                         "colormap_size==%d, bits_per_rgb==%d",
                         v->visual,
                         v->visualid,
                         v->screen,
                         v->depth,
                         debug_visualclassname(v->c_class),
                         v->red_mask, v->green_mask, v->blue_mask,
                         v->colormap_size,
                         v->bits_per_rgb);
}

static void
debug_dumpvisualinfo(Display * d, Visual * v)
{
  XVisualInfo xvitemplate;
  xvitemplate.visualid = XVisualIDFromVisual(v);
  int nrret;
  XVisualInfo * xvi = XGetVisualInfo(d, VisualIDMask, &xvitemplate, &nrret);
  assert(nrret == 1);
  debug_dumpvisualinfo(xvi);
}

/*!
  This function tries to find the best visual type, depth, and
  colormap combination for the display.  The display argument may be
  given, or set to \c NULL - then the default display will be used and
  returned.

  When SoXt doesn't get it's graphics displayed correctly, this
  routine may be the root of the problem.  Let's hope it solves more
  problems than it introduces...

  Misc Links:

  "Beyond the Default Visual", by John Cwikla
    http://www.motifzone.com/tmd/articles/DefaultVisual/DefaultVisual.html

  This function is not part of the original SGI InventorXt API.
*/
void
SoXt::selectBestVisual(Display * dpy, Visual * & visual,
                       Colormap & colormap, int & depth)
{
  assert(dpy != NULL);

#if SOXT_SELECTBESTVISUAL_DEBUG // debug
  { // Dump all visuals available on the X server.
    static SbBool first = TRUE;
    if (first) {
      first = FALSE;
      int num;
      XVisualInfo templ;
      XVisualInfo * all = XGetVisualInfo(dpy, VisualNoMask, &templ, &num);
      SoDebugError::postInfo("SoXt::selectBestVisual",
                             "%d available visual%s, dumping:",
                             num, num == 1 ? "" : "s");
      for (int i=0; i < num; i++) { debug_dumpvisualinfo(&all[i]); }
    }
  }
#endif // debug

  int snum = XDefaultScreen(dpy);

  if (XDefaultDepth(dpy, snum) >= 16) { // me like...
    visual = XDefaultVisual(dpy, snum);
#if SOXT_SELECTBESTVISUAL_DEBUG // debug
    SoDebugError::postInfo("SoXt::selectBestVisual", "using default visual:");
    debug_dumpvisualinfo(dpy, visual);
#endif // debug
    depth = XDefaultDepth(dpy, snum);
    colormap = XDefaultColormap(dpy, snum);
    return;
  }

  static struct {
    int depth;
    int vclass;
  } pri[] = { // how things are prioritized
    { 24, DirectColor },
    { 24, TrueColor },
    { 24, PseudoColor },
    { 16, DirectColor },
    { 16, TrueColor },
    { 16, PseudoColor },
    { 12, DirectColor },
    { 12, TrueColor },
    { 12, PseudoColor },
    { 8, PseudoColor },
    { 0, 0 }
  };

  XVisualInfo vinfo;
  for (int i = 0; pri[i].depth != 0; i++) {
    if (XMatchVisualInfo(dpy, snum, pri[i].depth, pri[i].vclass, &vinfo)) {
      visual = vinfo.visual;
#if SOXT_SELECTBESTVISUAL_DEBUG // debug
      SoDebugError::postInfo("SoXt::selectBestVisual", "found visual to use:");
      debug_dumpvisualinfo(&vinfo);
#endif // debug
      depth = vinfo.depth;

      int numcmaps;
      XStandardColormap * stdcolormaps = NULL;

#ifdef HAVE_LIBXMU
      if (XmuLookupStandardColormap(dpy, vinfo.screen, vinfo.visualid,
                                    vinfo.depth, XA_RGB_DEFAULT_MAP,
                                    False, True)
          && XGetRGBColormaps(dpy, RootWindow(dpy, vinfo.screen),
                              &stdcolormaps, &numcmaps, XA_RGB_DEFAULT_MAP)) {
        SbBool found = FALSE;
        for (i = 0; i < numcmaps && ! found; i++) {
          if (stdcolormaps[i].visualid == vinfo.visualid) {
            colormap = stdcolormaps[i].colormap;
            found = TRUE;
          }
        }
        if (! found) {
          SoDebugError::postInfo("SoXt::selectBestVisual",
                                 "standard RGB colormaps did not work with visual - creating own colormap");
          colormap = XCreateColormap(dpy, RootWindow(dpy, vinfo.screen),
                                     vinfo.visual, AllocNone);
        }
      }
      else {
        SoDebugError::postInfo("SoXt::selectBestVisual",
                               "no standard RGB colormaps - creating own colormap");
        colormap = XCreateColormap(dpy, RootWindow(dpy, vinfo.screen),
                                   vinfo.visual, AllocNone);
      }
      XtFree((char *) stdcolormaps);
#else
      SoDebugError::postInfo("SoXt::selectBestVisual",
                             "SoXt does not support detecting best visual/colormap without the Xmu library (yet)");
      // FIXME: couldn't we do better than this? Exiting here doesn't
      // seem particularly robust.. 20020117 mortene.
      exit(1);
#endif /* ! HAVE_LIBXMU */
      return;
    }
  }
}

// *************************************************************************

/*!
  \var SoXtP::fallbackresources

  This is an array of X resources.

  They are not really in use yet - the implementation is mostly
  experimental, and resource names will change in the future.
*/

String
SoXtP::fallbackresources[] =
{
#define SOXT_COMPONENT "*SoXtGLWidget"
  SOXT_COMPONENT ".background:" "white", // not used
#undef SOXT_COMPONENT

#define SOXT_COMPONENT "*SoXtRenderArea"
  SOXT_COMPONENT ".border:" "false",
  SOXT_COMPONENT ".borderThickness:" "2",
  SOXT_COMPONENT ".background:" "black",
#undef SOXT_COMPONENT

#define SOXT_COMPONENT "*SoXtExaminerViewer"
  SOXT_COMPONENT ".title:" "Examiner Viewer",
  SOXT_COMPONENT "*LeftWheelLabel.labelString:" "Rot Y",
  SOXT_COMPONENT "*BottomWheelLabel.labelString:" "Rot X",
  SOXT_COMPONENT "*RightWheelLabel.dollyString:" "Dolly",
  SOXT_COMPONENT "*RightWheelLabel.zoomString:" "Zoom",
#undef SOXT_COMPONENT

#define SOXT_COMPONENT "*SoXtPlaneViewer"
  SOXT_COMPONENT ".title:" "Plane Viewer",
  SOXT_COMPONENT "*LeftWheelLabel.labelString:" "Trans Y",
  SOXT_COMPONENT "*BottomWheelLabel.labelString:" "Trans X",
  SOXT_COMPONENT "*RightWheelLabel.dollyString:" "Dolly",
  SOXT_COMPONENT "*RightWheelLabel.zoomString:" "Zoom",
#undef SOXT_COMPONENT

#define SOXT_COMPONENT "*SoXtWalkViewer"
  SOXT_COMPONENT ".title:" "Walk Viewer",
#undef SOXT_COMPONENT

#define SOXT_COMPONENT "*SoXtFlyViewer"
  SOXT_COMPONENT ".title:" "Fly Viewer",
  SOXT_COMPONENT ".LeftWheelLabel:" "Tilt",
  SOXT_COMPONENT ".BottomWheelLabel:" "Rotate",
  SOXT_COMPONENT ".RotateWheelLabel:" "Dolly",
#undef SOXT_COMPONENT

#define SOXT_COMPONENT "*SoXtDirectionalLightEditor"
#undef SOXT_COMPONENT

#define SOXT_COMPONENT "*SoXtMaterialEditor"
#undef SOXT_COMPONENT

#define SOXT_COMPONENT "*SoXtLightSliderSet"
  SOXT_COMPONENT ".title:" "Light Sliders",
  SOXT_COMPONENT ".sliderset1:" "LightIntensity",
  SOXT_COMPONENT ".sliderset2:" "LightColor",
#undef SOXT_COMPONENT

#define SOXT_COMPONENT "*SoXtMaterialSliderSet"
  SOXT_COMPONENT ".title:" "Material Sliders",
  SOXT_COMPONENT ".sliderset1:" "AmbientColor",
  SOXT_COMPONENT ".sliderset2:" "DiffuseColor",
  SOXT_COMPONENT ".sliderset3:" "SpecularColor",
  SOXT_COMPONENT ".sliderset4:" "EmissiveColor",
  SOXT_COMPONENT ".sliderset5:" "Shininess",
  SOXT_COMPONENT ".sliderset6:" "Transparency",
#undef SOXT_COMPONENT

#define SOXT_COMPONENT "*SoXtTransformSliderSet"
  SOXT_COMPONENT ".title:" "Transform Sliders",
  SOXT_COMPONENT ".sliderset1:" "Translations",
  SOXT_COMPONENT ".sliderset2:" "Scales",
  SOXT_COMPONENT ".sliderset3:" "Rotations",
  SOXT_COMPONENT ".sliderset4:" "ScaleOrientation",
  SOXT_COMPONENT ".sliderset5:" "Center",
#undef SOXT_COMPONENT

#define SOXT_COMPONENT "*SoXtMaterialList"
#undef SOXT_COMPONENT

#define SOXT_COMPONENT "*SoXtPrintDialog"
  SOXT_COMPONENT "*title:" "Print",
#undef SOXT_COMPONENT

  // *************************************************************************

#define SOXT_COMPONENT "*SoXtAmbientColorSliderModule"
  SOXT_COMPONENT ".title:" "AMBIENT COLOR",
  SOXT_COMPONENT ".slider1Title:" "Ambient Red",
  SOXT_COMPONENT ".slider1Field:" "ambientColor[0]",
  SOXT_COMPONENT ".slider2Title:" "Ambient Green",
  SOXT_COMPONENT ".slider2Field:" "ambientColor[1]",
  SOXT_COMPONENT ".slider3Title:" "Ambient Blue",
  SOXT_COMPONENT ".slider3Field:" "ambientColor[2]",
#undef SOXT_COMPONENT

#define SOXT_COMPONENT "*SoXtCenterSliderModule"
  SOXT_COMPONENT ".title:" "CENTER",
  SOXT_COMPONENT ".slider1Title:" "X Center",
  SOXT_COMPONENT ".slider1Field:" "center[0]",
  SOXT_COMPONENT ".slider2Title:" "Y Center",
  SOXT_COMPONENT ".slider2Field:" "center[1]",
  SOXT_COMPONENT ".slider3Title:" "Z Center",
  SOXT_COMPONENT ".slider3Field:" "center[2]",
#undef SOXT_COMPONENT

#define SOXT_COMPONENT "*SoXtDiffuseColorSliderModule"
  SOXT_COMPONENT ".title:" "DIFFUSE COLOR",
  SOXT_COMPONENT ".slider1Title:" "Diffuse Red",
  SOXT_COMPONENT ".slider1Field:" "diffuseColor[0]",
  SOXT_COMPONENT ".slider2Title:" "Diffuse Green",
  SOXT_COMPONENT ".slider2Field:" "diffuseColor[1]",
  SOXT_COMPONENT ".slider3Title:" "Diffuse Blue",
  SOXT_COMPONENT ".slider3Field:" "diffuseColor[2]",
#undef SOXT_COMPONENT

#define SOXT_COMPONENT "*SoXtEmissiveColorSliderModule"
  SOXT_COMPONENT ".title:" "EMISSIVE COLOR",
  SOXT_COMPONENT ".slider1Title:" "Emissive Red",
  SOXT_COMPONENT ".slider1Field:" "emissiveColor[0]",
  SOXT_COMPONENT ".slider2Title:" "Emissive Green",
  SOXT_COMPONENT ".slider2Field:" "emissiveColor[1]",
  SOXT_COMPONENT ".slider3Title:" "Emissive Blue",
  SOXT_COMPONENT ".slider3Field:" "emissiveColor[2]",
#undef SOXT_COMPONENT

#define SOXT_COMPONENT "*SoXtLightColorSliderModule"
  SOXT_COMPONENT ".title:" "COLOR",
  SOXT_COMPONENT ".slider1Title:" "Red",
  SOXT_COMPONENT ".slider1Field:" "color[0]",
  SOXT_COMPONENT ".slider2Title:" "Green",
  SOXT_COMPONENT ".slider2Field:" "color[1]",
  SOXT_COMPONENT ".slider3Title:" "Blue",
  SOXT_COMPONENT ".slider3Field:" "color[2]",
#undef SOXT_COMPONENT

#define SOXT_COMPONENT "*SoXtLightIntensitySliderModule"
  SOXT_COMPONENT ".title:" "INTENSITY",
  SOXT_COMPONENT ".slider1Title:" "Intensity",
  SOXT_COMPONENT ".slider1Field:" "intensity",
#undef SOXT_COMPONENT

#define SOXT_COMPONENT "*SoXtRotationsSliderModule"
  SOXT_COMPONENT ".title:" "ROTATIONS",
  SOXT_COMPONENT ".slider1Title:" "X Rotate",
  SOXT_COMPONENT ".slider1Field:" "rotation[0]",
  SOXT_COMPONENT ".slider2Title:" "Y Rotate",
  SOXT_COMPONENT ".slider2Field:" "rotation[1]",
  SOXT_COMPONENT ".slider3Title:" "Z Rotate",
  SOXT_COMPONENT ".slider3Field:" "rotation[2]",
#undef SOXT_COMPONENT

#define SOXT_COMPONENT "*SoXtScalesSliderModule"
  SOXT_COMPONENT ".title:" "SCALES",
  SOXT_COMPONENT ".slider1Title:" "X Scale",
  SOXT_COMPONENT ".slider1Field:" "scaleFactor[0]",
  SOXT_COMPONENT ".slider2Title:" "Y Scale",
  SOXT_COMPONENT ".slider2Field:" "scaleFactor[1]",
  SOXT_COMPONENT ".slider3Title:" "Z Scale",
  SOXT_COMPONENT ".slider3Field:" "scaleFactor[2]",
#undef SOXT_COMPONENT

#define SOXT_COMPONENT "*SoXtScaleOrientationSliderModule"
  SOXT_COMPONENT ".title:" "SCALE ORIENTATION",
  SOXT_COMPONENT ".slider1Title:" "X Rotate",
  SOXT_COMPONENT ".slider1Field:" "scaleOrientation[0]",
  SOXT_COMPONENT ".slider2Title:" "Y Rotate",
  SOXT_COMPONENT ".slider2Field:" "scaleOrientation[1]",
  SOXT_COMPONENT ".slider3Title:" "Z Rotate",
  SOXT_COMPONENT ".slider3Field:" "scaleOrientation[2]",
#undef SOXT_COMPONENT

#define SOXT_COMPONENT "*SoXtShininessSliderModule"
  SOXT_COMPONENT ".title:" "SHININESS",
  SOXT_COMPONENT ".slider1Title:" "Shininess",
  SOXT_COMPONENT ".slider1Field:" "shininess",
#undef SOXT_COMPONENT

#define SOXT_COMPONENT "*SoXtSpecularColorSliderModule"
  SOXT_COMPONENT ".title:" "SPECULAR COLOR",
  SOXT_COMPONENT ".slider1Title:" "Specular Red",
  SOXT_COMPONENT ".slider1Field:" "specularColor[0]",
  SOXT_COMPONENT ".slider2Title:" "Specular Green",
  SOXT_COMPONENT ".slider2Field:" "specularColor[1]",
  SOXT_COMPONENT ".slider3Title:" "Specular Blue",
  SOXT_COMPONENT ".slider3Field:" "specularColor[2]",
#undef SOXT_COMPONENT

#define SOXT_COMPONENT "*SoXtTranslationsSliderModule"
  SOXT_COMPONENT ".title:" "TRANSLATIONS",
  SOXT_COMPONENT ".slider1Title:" "X Translation",
  SOXT_COMPONENT ".slider1Field:" "translation[0]",
  SOXT_COMPONENT ".slider2Title:" "Y Translation",
  SOXT_COMPONENT ".slider2Field:" "translation[1]",
  SOXT_COMPONENT ".slider3Title:" "Z Translation",
  SOXT_COMPONENT ".slider3Field:" "translation[2]",
#undef SOXT_COMPONENT

#define SOXT_COMPONENT "*SoXtTransparencySliderModule"
  SOXT_COMPONENT ".title:" "TRANSPARENCY",
  SOXT_COMPONENT ".slider1Title:" "Transparency",
  SOXT_COMPONENT ".slider1Field:" "transparency",
#undef SOXT_COMPONENT

  NULL
};

// *************************************************************************
