###############################################################################
#####                 This makefile generates ELWPIG.DLL                  #####
###############################################################################
!include $(WGDEV)\src\gcrules.mak

!if $(OS) == DOS
!    if $(MODEL) == LARGE
!         error ELWPIG not supported in Large Model under DOS!
!    endif
     DLLLIB = $(DLLLIB) $(COMPILER)\lib\mathh.lib
     PRODLL = $(PRODLL) $(DOSEXT)\bc5\lib\fpdmy
!else
     PARSEOPTS = $(PARSEOPTS) -I. -DIS30
!endif

VIRDIR = .\GCVIRDAT

DEFILE =

VIRFILES = GCVIRDAT\elwpig.vir

DLLLIB =            \
     $(DLLLIB)      \
     wgserver.lib   \
     galgsbl.lib

OBJFILES =          \
     elwpig.obj     

start:              \
     $(GCBUILD)     \
     elwpig.dll

elwpig.dll:         \
     $(OBJFILES)    \
     $(DLLLIB)      \
     $(DEFILE)      \
     $(VIRFILES)
     $(LINK) $(LIBCMD) @&&|
$(PRODLL) $(OBJFILES)
$(RUNDIR)\$&
$(RUNDIR)\$&
$(DLLLIB) $(LINKDLL)
$(DEFILE)
|
