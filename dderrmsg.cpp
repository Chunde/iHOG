#include "stdafx.h"
#include "dderrmsg.h"
#include "ddraw.h"
// Das ist von Microsoft geklaut

char* DDErrCodeToString(HRESULT hRErrno,char *szFile,int iLine)
{
	static char szRetHelp[200];
	char szHelp[200];

    switch(hRErrno) {
		case DD_OK:
            /* Also includes D3D_OK and D3DRM_OK */
            strcpy(szHelp, "No error.\0");
        case DDERR_ALREADYINITIALIZED:
            strcpy(szHelp, "This object is already initialized.\0");
        case DDERR_BLTFASTCANTCLIP:
            strcpy(szHelp, "strcpy(szHelp, if a clipper object is attached to the source surface passed into a BltFast call.\0");
        case DDERR_CANNOTATTACHSURFACE:
            strcpy(szHelp, "This surface can not be attached to the requested surface.\0");
        case DDERR_CANNOTDETACHSURFACE:
            strcpy(szHelp, "This surface can not be detached from the requested surface.\0");
        case DDERR_CANTCREATEDC:
            strcpy(szHelp, "Windows can not create any more DCs.\0");
        case DDERR_CANTDUPLICATE:
            strcpy(szHelp, "Can't duplicate primary & 3D surfaces, or surfaces that are implicitly created.\0");
        case DDERR_CLIPPERISUSINGHWND:
            strcpy(szHelp, "An attempt was made to set a cliplist for a clipper object that is already monitoring an hwnd.\0");
        case DDERR_COLORKEYNOTSET:
            strcpy(szHelp, "No src color key specified for this operation.\0");
        case DDERR_CURRENTLYNOTAVAIL:
            strcpy(szHelp, "Support is currently not available.\0");
        case DDERR_DIRECTDRAWALREADYCREATED:
            strcpy(szHelp, "A DirectDraw object representing this driver has already been created for this process.\0");
        case DDERR_EXCEPTION:
            strcpy(szHelp, "An exception was encountered while performing the requested operation.\0");
        case DDERR_EXCLUSIVEMODEALREADYSET:
            strcpy(szHelp, "An attempt was made to set the cooperative level when it was already set to exclusive.\0");
        case DDERR_GENERIC:
            strcpy(szHelp, "Generic failure.\0");
        case DDERR_HEIGHTALIGN:
            strcpy(szHelp, "Height of rectangle provided is not a multiple of reqd alignment.\0");
        case DDERR_HWNDALREADYSET:
            strcpy(szHelp, "The CooperativeLevel HWND has already been set. It can not be reset while the process has surfaces or palettes created.\0");
        case DDERR_HWNDSUBCLASSED:
            strcpy(szHelp, "HWND used by DirectDraw CooperativeLevel has been subclaSsed, this prevents DirectDraw from restoring state.\0");
        case DDERR_IMPLICITLYCREATED:
            strcpy(szHelp, "This surface can not be restored because it is an implicitly created surface.\0");
        case DDERR_INCOMPATIBLEPRIMARY:
            strcpy(szHelp, "Unable to match primary surface creation request with existing primary surface.\0");
        case DDERR_INVALIDCAPS:
            strcpy(szHelp, "One or more of the caps bits passed to the callback are incorrect.\0");
        case DDERR_INVALIDCLIPLIST:
            strcpy(szHelp, "DirectDraw does not support the provided cliplist.\0");
        case DDERR_INVALIDDIRECTDRAWGUID:
            strcpy(szHelp, "The GUID passed to DirectDrawCreate is not a valid DirectDraw driver identifier.\0");
        case DDERR_INVALIDMODE:
            strcpy(szHelp, "DirectDraw does not support the requested mode.\0");
        case DDERR_INVALIDOBJECT:
            strcpy(szHelp, "DirectDraw received a pointer that was an invalid DIRECTDRAW object.\0");
        case DDERR_INVALIDPARAMS:
            strcpy(szHelp, "One or more of the parameters passed to the function are incorrect.\0");
        case DDERR_INVALIDPIXELFORMAT:
            strcpy(szHelp, "The pixel format was invalid as specified.\0");
        case DDERR_INVALIDPOSITION:
            strcpy(szHelp, "strcpy(szHelp,ed when the position of the overlay on the destination is no longer legal for that destination.\0");
        case DDERR_INVALIDRECT:
            strcpy(szHelp, "Rectangle provided was invalid.\0");
        case DDERR_LOCKEDSURFACES:
            strcpy(szHelp, "Operation could not be carried out because one or more surfaces are locked.\0");
        case DDERR_NO3D:
            strcpy(szHelp, "There is no 3D present.\0");
        case DDERR_NOALPHAHW:
            strcpy(szHelp, "Operation could not be carried out because there is no alpha accleration hardware present or available.\0");
        case DDERR_NOBLTHW:
            strcpy(szHelp, "No blitter hardware present.\0");
        case DDERR_NOCLIPLIST:
            strcpy(szHelp, "No cliplist available.\0");
        case DDERR_NOCLIPPERATTACHED:
            strcpy(szHelp, "No clipper object attached to surface object.\0");
        case DDERR_NOCOLORCONVHW:
            strcpy(szHelp, "Operation could not be carried out because there is no color conversion hardware present or available.\0");
        case DDERR_NOCOLORKEY:
            strcpy(szHelp, "Surface doesn't currently have a color key\0");
        case DDERR_NOCOLORKEYHW:
            strcpy(szHelp, "Operation could not be carried out because there is no hardware support of the destination color key.\0");
        case DDERR_NOCOOPERATIVELEVELSET:
            strcpy(szHelp, "Create function called without DirectDraw object method SetCooperativeLevel being called.\0");
        case DDERR_NODC:
            strcpy(szHelp, "No DC was ever created for this surface.\0");
        case DDERR_NODDROPSHW:
            strcpy(szHelp, "No DirectDraw ROP hardware.\0");
        case DDERR_NODIRECTDRAWHW:
            strcpy(szHelp, "A hardware-only DirectDraw object creation was attempted but the driver did not support any hardware.\0");
        case DDERR_NOEMULATION:
            strcpy(szHelp, "Software emulation not available.\0");
        case DDERR_NOEXCLUSIVEMODE:
            strcpy(szHelp, "Operation requires the application to have exclusive mode but the application does not have exclusive mode.\0");
        case DDERR_NOFLIPHW:
            strcpy(szHelp, "Flipping visible surfaces is not supported.\0");
        case DDERR_NOGDI:
            strcpy(szHelp, "There is no GDI present.\0");
        case DDERR_NOHWND:
            strcpy(szHelp, "Clipper notification requires an HWND or no HWND has previously been set as the CooperativeLevel HWND.\0");
        case DDERR_NOMIRRORHW:
            strcpy(szHelp, "Operation could not be carried out because there is no hardware present or available.\0");
        case DDERR_NOOVERLAYDEST:
            strcpy(szHelp, "strcpy(szHelp,ed when GetOverlayPosition is called on an overlay that UpdateOverlay has never been called on to establish a destination.\0");
        case DDERR_NOOVERLAYHW:
            strcpy(szHelp, "Operation could not be carried out because there is no overlay hardware present or available.\0");
        case DDERR_NOPALETTEATTACHED:
            strcpy(szHelp, "No palette object attached to this surface.\0");
        case DDERR_NOPALETTEHW:
            strcpy(szHelp, "No hardware support for 16 or 256 color palettes.\0");
        case DDERR_NORASTEROPHW:
            strcpy(szHelp, "Operation could not be carried out because there is no appropriate raster op hardware present or available.\0");
        case DDERR_NOROTATIONHW:
            strcpy(szHelp, "Operation could not be carried out because there is no rotation hardware present or available.\0");
        case DDERR_NOSTRETCHHW:
            strcpy(szHelp, "Operation could not be carried out because there is no hardware support for stretching.\0");
        case DDERR_NOT4BITCOLOR:
            strcpy(szHelp, "DirectDrawSurface is not in 4 bit color palette and the requested operation requires 4 bit color palette.\0");
        case DDERR_NOT4BITCOLORINDEX:
            strcpy(szHelp, "DirectDrawSurface is not in 4 bit color index palette and the requested operation requires 4 bit color index palette.\0");
        case DDERR_NOT8BITCOLOR:
            strcpy(szHelp, "DirectDrawSurface is not in 8 bit color mode and the requested operation requires 8 bit color.\0");
        case DDERR_NOTAOVERLAYSURFACE:
            strcpy(szHelp, "strcpy(szHelp,ed when an overlay member is called for a non-overlay surface.\0");
        case DDERR_NOTEXTUREHW:
            strcpy(szHelp, "Operation could not be carried out because there is no texture mapping hardware present or available.\0");
        case DDERR_NOTFLIPPABLE:
            strcpy(szHelp, "An attempt has been made to flip a surface that is not flippable.\0");
        case DDERR_NOTFOUND:
            strcpy(szHelp, "Requested item was not found.\0");
        case DDERR_NOTLOCKED:
            strcpy(szHelp, "Surface was not locked.  An attempt to unlock a surface that was not locked at all, or by this process, has been attempted.\0");
        case DDERR_NOTPALETTIZED:
            strcpy(szHelp, "The surface being used is not a palette-based surface.\0");
        case DDERR_NOVSYNCHW:
            strcpy(szHelp, "Operation could not be carried out because there is no hardware support for vertical blank synchronized operations.\0");
        case DDERR_NOZBUFFERHW:
            strcpy(szHelp, "Operation could not be carried out because there is no hardware support for zbuffer blitting.\0");
        case DDERR_NOZOVERLAYHW:
            strcpy(szHelp, "Overlay surfaces could not be z layered based on their BltOrder because the hardware does not support z layering of overlays.\0");
        case DDERR_OUTOFCAPS:
            strcpy(szHelp, "The hardware needed for the requested operation has already been allocated.\0");
        case DDERR_OUTOFMEMORY:
            strcpy(szHelp, "DirectDraw does not have enough memory to perform the operation.\0");
        case DDERR_OUTOFVIDEOMEMORY:
            strcpy(szHelp, "DirectDraw does not have enough memory to perform the operation.\0");
        case DDERR_OVERLAYCANTCLIP:
            strcpy(szHelp, "The hardware does not support clipped overlays.\0");
        case DDERR_OVERLAYCOLORKEYONLYONEACTIVE:
            strcpy(szHelp, "Can only have ony color key active at one time for overlays.\0");
        case DDERR_OVERLAYNOTVISIBLE:
            strcpy(szHelp, "strcpy(szHelp,ed when GetOverlayPosition is called on a hidden overlay.\0");
        case DDERR_PALETTEBUSY:
            strcpy(szHelp, "Access to this palette is being refused because the palette is already locked by another thread.\0");
        case DDERR_PRIMARYSURFACEALREADYEXISTS:
            strcpy(szHelp, "This process already has created a primary surface.\0");
        case DDERR_REGIONTOOSMALL:
            strcpy(szHelp, "Region passed to Clipper::GetClipList is too small.\0");
        case DDERR_SURFACEALREADYATTACHED:
            strcpy(szHelp, "This surface is already attached to the surface it is being attached to.\0");
        case DDERR_SURFACEALREADYDEPENDENT:
            strcpy(szHelp, "This surface is already a dependency of the surface it is being made a dependency of.\0");
        case DDERR_SURFACEBUSY:
            strcpy(szHelp, "Access to this surface is being refused because the surface is already locked by another thread.\0");
        case DDERR_SURFACEISOBSCURED:
            strcpy(szHelp, "Access to surface refused because the surface is obscured.\0");
        case DDERR_SURFACELOST:
            strcpy(szHelp, "Access to this surface is being refused because the surface memory is gone. The DirectDrawSurface object representing this surface should have Restore called on it.\0");
        case DDERR_SURFACENOTATTACHED:
            strcpy(szHelp, "The requested surface is not attached.\0");
        case DDERR_TOOBIGHEIGHT:
            strcpy(szHelp, "Height requested by DirectDraw is Too large.\0");
        case DDERR_TOOBIGSIZE:
            strcpy(szHelp, "Size requested by DirectDraw is too large, but the individual height and width are OK.\0");
        case DDERR_TOOBIGWIDTH:
            strcpy(szHelp, "Width requested by DirectDraw is too large.\0");
        case DDERR_UNSUPPORTED:
            strcpy(szHelp, "Action not supported.\0");
        case DDERR_UNSUPPORTEDFORMAT:
            strcpy(szHelp, "FOURCC format requested is unsupported by DirectDraw.\0");
        case DDERR_UNSUPPORTEDMASK:
            strcpy(szHelp, "Bitmask in the pixel format requested is unsupported by DirectDraw.\0");
        case DDERR_VERTICALBLANKINPROGRESS:
            strcpy(szHelp, "Vertical blank is in progress.\0");
        case DDERR_WASSTILLDRAWING:
            strcpy(szHelp, "Informs DirectDraw that the previous Blt which is transfering information to or from this Surface is incomplete.\0");
        case DDERR_WRONGMODE:
            strcpy(szHelp, "This surface can not be restored because it was created in a different mode.\0");
        case DDERR_XALIGN:
            strcpy(szHelp, "Rectangle provided was not horizontally aligned on required boundary.\0");
/*        case D3DERR_BADMAJORVERSION:
            strcpy(szHelp, "D3DERR_BADMAJORVERSION\0");
        case D3DERR_BADMINORVERSION:
            strcpy(szHelp, "D3DERR_BADMINORVERSION\0");
        case D3DERR_EXECUTE_LOCKED:
            strcpy(szHelp, "D3DERR_EXECUTE_LOCKED\0");
        case D3DERR_EXECUTE_NOT_LOCKED:
            strcpy(szHelp, "D3DERR_EXECUTE_NOT_LOCKED\0");
        case D3DERR_EXECUTE_CREATE_FAILED:
            strcpy(szHelp, "D3DERR_EXECUTE_CREATE_FAILED\0");
        case D3DERR_EXECUTE_DESTROY_FAILED:
            strcpy(szHelp, "D3DERR_EXECUTE_DESTROY_FAILED\0");
        case D3DERR_EXECUTE_LOCK_FAILED:
            strcpy(szHelp, "D3DERR_EXECUTE_LOCK_FAILED\0");
        case D3DERR_EXECUTE_UNLOCK_FAILED:
            strcpy(szHelp, "D3DERR_EXECUTE_UNLOCK_FAILED\0");
        case D3DERR_EXECUTE_FAILED:
            strcpy(szHelp, "D3DERR_EXECUTE_FAILED\0");
        case D3DERR_EXECUTE_CLIPPED_FAILED:
            strcpy(szHelp, "D3DERR_EXECUTE_CLIPPED_FAILED\0");
        case D3DERR_TEXTURE_NO_SUPPORT:
            strcpy(szHelp, "D3DERR_TEXTURE_NO_SUPPORT\0");
        case D3DERR_TEXTURE_NOT_LOCKED:
            strcpy(szHelp, "D3DERR_TEXTURE_NOT_LOCKED\0");
        case D3DERR_TEXTURE_LOCKED:
            strcpy(szHelp, "D3DERR_TEXTURELOCKED\0");
        case D3DERR_TEXTURE_CREATE_FAILED:
            strcpy(szHelp, "D3DERR_TEXTURE_CREATE_FAILED\0");
        case D3DERR_TEXTURE_DESTROY_FAILED:
            strcpy(szHelp, "D3DERR_TEXTURE_DESTROY_FAILED\0");
        case D3DERR_TEXTURE_LOCK_FAILED:
            strcpy(szHelp, "D3DERR_TEXTURE_LOCK_FAILED\0");
        case D3DERR_TEXTURE_UNLOCK_FAILED:
            strcpy(szHelp, "D3DERR_TEXTURE_UNLOCK_FAILED\0");
        case D3DERR_TEXTURE_LOAD_FAILED:
            strcpy(szHelp, "D3DERR_TEXTURE_LOAD_FAILED\0");
        case D3DERR_MATRIX_CREATE_FAILED:
            strcpy(szHelp, "D3DERR_MATRIX_CREATE_FAILED\0");
        case D3DERR_MATRIX_DESTROY_FAILED:
            strcpy(szHelp, "D3DERR_MATRIX_DESTROY_FAILED\0");
        case D3DERR_MATRIX_SETDATA_FAILED:
            strcpy(szHelp, "D3DERR_MATRIX_SETDATA_FAILED\0");
        case D3DERR_SETVIEWPORTDATA_FAILED:
            strcpy(szHelp, "D3DERR_SETVIEWPORTDATA_FAILED\0");
        case D3DERR_MATERIAL_CREATE_FAILED:
            strcpy(szHelp, "D3DERR_MATERIAL_CREATE_FAILED\0");
        case D3DERR_MATERIAL_DESTROY_FAILED:
            strcpy(szHelp, "D3DERR_MATERIAL_DESTROY_FAILED\0");
        case D3DERR_MATERIAL_SETDATA_FAILED:
            strcpy(szHelp, "D3DERR_MATERIAL_SETDATA_FAILED\0");
        case D3DERR_LIGHT_SET_FAILED:
            strcpy(szHelp, "D.DERR_LIGHT_SET_FAILED\0");
        case D3DRMERR_BADOBJECT:
            strcpy(szHelp, "D3DRMERR_BADOBJECT\0");
        case D3DRMERR_BADTYPE:
            strcpy(szHelp, "D3DRMERR_BADTYPE\0");
        case D3DRMERR_BADALLOC:
            strcpy(szHelp, "D3DRMERR_BADALLOC\0");
        case D3DRMERR_FACEUSED:
            strcpy(szHelp, "D3DRMERR_FACEUSED\0");
        case D3DRMERR_NOTFOUND:
            strcpy(szHelp, "D3DRMERR_NOTFOUND\0");
        case D3DRMERR_NOTDONEYET:
            strcpy(szHelp, "D3DRMERR_NOTDONEYET\0");
        case D3DRMERR_FILENOTFOUND:
            strcpy(szHelp, "The file was not found.\0");
        case D3DRMERR_BADFILE:
            strcpy(szHelp, "D3DRMERR_BADFILE\0");
        case D3DRMERR_BADDEVICE:
            strcpy(szHelp, "D3DRMERR_BADDEVICE\0");
        case D3DRMERR_BADVALUE:
            strcpy(szHelp, "D3DRMERR_BADVALUE\0");
        case D3DRMERR_BADMAJORVERSION:
            strcpy(szHelp, "D3DRMERR_BADMAJORVERSION\0");
        case D3DRMERR_BADMINORVERSION:
            strcpy(szHelp, "D3DRMERR_BADMINORVERSION\0");
        case D3DRMERR_UNABLETOEXECUTE:
            strcpy(szHelp, "D3DRMERR_UNABLETOEXECUTE\0");
  */      default:
            strcpy(szHelp, "Unrecognized error value.\0");
    }
	sprintf(szRetHelp,"Datei: %s Zeile %i DDFehler: %s\n",szFile,iLine,szHelp);
	return szRetHelp;
}
