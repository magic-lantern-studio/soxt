#! /bin/sh
# no to be executed - this script must be sourced

GUI="Xt"
lGUI="xt"
uGUI="XT"
WIDGET="Widget"
EVENT="XAnyEvent *"
COMPONENTHEADER=""

HAVE_RENDERAREA=true
HAVE_EXAMINERVIEWER=true
HAVE_PLANEVIEWER=false
HAVE_WALKVIEWER=false
HAVE_FLYVIEWER=false

export GUI lGUI uGUI
export WIDGET EVENT COMPONENTHEADER
export HAVE_RENDERAREA HAVE_EXAMINERVIEWER
export HAVE_PLANEVIEWER HAVE_WALKVIEWER HAVE_FLYVIEWER

