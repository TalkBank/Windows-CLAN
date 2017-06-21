#include "ced.h"

#define	min(x,y)		((x) < (y) ? (x) : (y))
#define	max(x,y)		((x) < (y) ? (y) : (x))

static Point position_offset = { 0, 0 };
static BitMap			rdbmap;
static Rect				rdbounds;
static Ptr				rdbuffer = 0L, *rdbufferhand;
static long				rdbuffsize = 0L;
static PixMapHandle		rdpixmap;

void BeginWindowRedraw(Rect *area, short copy, char isErase);
void EndWindowRedraw(short validateRegion);

static short AllocateOffscreenBuffer(BitMap *bmap, GrafPtr port, short color)
/* Used to allocate a Bitmap or PixMap. Don't call it directly (you can't) */
{
	short buf_rowBytes, depth;
	unsigned long buf_size;
	Ptr				temp;
	Rect			box;
	PixMapHandle	pixmap;
	
	box=rdbounds;
	
	if (color) {
#if (TARGET_API_MAC_CARBON == 1)
		const BitMap *portBits;
		portBits = GetPortBitMapForCopyBits(port);
		depth=(**(PixMapHandle)portBits->baseAddr).pixelSize;
#else
		depth=(**(PixMapHandle)port->portBits.baseAddr).pixelSize;
#endif
	} else
		depth=1;
	
	buf_rowBytes = ((depth*(box.right-box.left))/8)+1;

	/* make sure bytes % 4 = 0; CopyBits likes this. */
	while (buf_rowBytes % 4)
		buf_rowBytes++;
		
	buf_size = (unsigned long) ((long)(box.bottom-box.top)*(long) buf_rowBytes);

	bmap->bounds = box;
	
	if (rdbuffer && !(*rdbufferhand)) { /* Handle has been purged: */
		rdbuffer = 0L;
		DisposeHandle((Handle)rdbufferhand);
		rdbuffsize = 0;
	}
	
	/* Allocate the memory to draw in */
	if (buf_size > rdbuffsize){
		if (rdbuffer)
			DisposeHandle(rdbufferhand);
		
		rdbufferhand = NewHandle(buf_size);
		if (!rdbufferhand) {
			rdbuffer = 0L;
			rdbuffsize = 0;
			return 0;
		}
		
		HPurge(rdbufferhand);
		HLock(rdbufferhand);
		temp = rdbuffer = *rdbufferhand;
		rdbuffsize = buf_size;
	} else {
		HLock(rdbufferhand);
		temp = rdbuffer = *rdbufferhand;
	}
		
	if (!color){
		bmap->baseAddr = temp;
		bmap->rowBytes = buf_rowBytes;
	} else {
		if (!rdpixmap) {
			rdpixmap = NewPixMap();
			DisposeHandle((Handle)(**rdpixmap).pmTable);
		}
		pixmap = rdpixmap;
		HLock((Handle)pixmap);
		(**pixmap).baseAddr = temp;
		(**pixmap).rowBytes = buf_rowBytes | 0x8000;
		(**pixmap).bounds = box;
#if (TARGET_API_MAC_CARBON == 1)
		const BitMap *portBits;
		portBits = GetPortBitMapForCopyBits(port);
		(**pixmap).cmpSize=(**(PixMapHandle)portBits->baseAddr).cmpSize;
		(**pixmap).pixelSize=(**(PixMapHandle)portBits->baseAddr).pixelSize;
		(**pixmap).pmTable=(**(PixMapHandle)portBits->baseAddr).pmTable;
#else
		(**pixmap).cmpSize=(**(PixMapHandle)port->portBits.baseAddr).cmpSize;
		(**pixmap).pixelSize=(**(PixMapHandle)port->portBits.baseAddr).pixelSize;
		(**pixmap).pmTable=(**(PixMapHandle)port->portBits.baseAddr).pmTable;
#endif
		bmap->rowBytes = buf_rowBytes | 0xC000;
		HUnlock((Handle)pixmap);
		bmap->baseAddr=(Ptr)pixmap;
	}
	
	return 1;
}

static void SetUpClipRgn(Rect *area, GrafPtr port, short copy) 
/* Set up the clip region but remember the old one */
{

	RgnHandle	saveRgn = 0L, tempRgn;
	
	if (!copy)
		if (area) {
			EraseRect(area);
			if (saveRgn = NewRgn()) {
#if (TARGET_API_MAC_CARBON == 1)
				RgnHandle portClipRgn = NewRgn();
				CopyRgn(GetPortClipRegion(port, portClipRgn), saveRgn);
				DisposeRgn(portClipRgn);
#else
				CopyRgn(port->clipRgn, saveRgn);
#endif
				if (tempRgn = NewRgn()) {
					RectRgn(tempRgn, area);
#if (TARGET_API_MAC_CARBON == 1)
					RgnHandle portClipRgn = NewRgn();
					portClipRgn = GetPortClipRegion(port, portClipRgn);
					SectRgn(tempRgn, portClipRgn, portClipRgn);
					DisposeRgn(portClipRgn);
#else
					SectRgn(tempRgn, port->clipRgn, port->clipRgn);
#endif
					DisposeRgn(tempRgn);
				}
			}
		} else {
#if (TARGET_API_MAC_CARBON == 1)
			Rect rect;
			EraseRect(GetPortBounds(port, &rect));
#else
			EraseRect(&port->portRect);
#endif
		}
}

static void OffscreenWindowUtilsStart(Rect *area, short copy, RgnHandle rgn, short offDraw)
/* Set up for offscreen drawing. area is the bounding box for drawing; NULL =>
   entire port. Drawing is cliped to area. If copy is true, the current screen will
   be copied to the offscreen, otherwise the offscreen area will be erased. rgn can
   be used to further mask the drawing area. offDraw => switch drawing to the
   offscreen. OffscreenEffectProc is called after the offscreen is set up and drawing is
   switched. Usually called by BeginWindowRedraw or SaveWindowArea. */
{

	GrafPtr		port;
	Rect		screen_box, clip_box;
	BitMap		bmap;
	
	GetPort(&port);
	
#if (TARGET_API_MAC_CARBON == 1)
	rdbmap = *(GetPortBitMapForCopyBits(port));
#else
	rdbmap=port->portBits;
#endif
	
	if (area)
		rdbounds=*area;
	else {
#if (TARGET_API_MAC_CARBON == 1)
		Rect rect;
		GetPortBounds(port, &rect);
		rdbounds=rect;
#else
		rdbounds=port->portRect;
#endif
	}

	if (!AllocateOffscreenBuffer(&bmap, port, rdbmap.rowBytes & 0xC000)){
		if (offDraw)
			SetUpClipRgn(area, port, copy);
		return;
	}
	
	if (copy) {
		
		// Clip to grafport bounding box:
	
#if (TARGET_API_MAC_CARBON == 1)
		RgnHandle visRgn = NewRgn();
		GetRegionBounds(GetPortVisibleRegion(port, visRgn), &screen_box);
		DisposeRgn(visRgn);
#else
		screen_box = (*port->visRgn)->rgnBBox;
#endif
		clip_box.right = min(screen_box.right, rdbounds.right);
		clip_box.bottom = min(screen_box.bottom, rdbounds.bottom);
		clip_box.top = max(screen_box.top, rdbounds.top);
		clip_box.left = max(screen_box.left, rdbounds.left);
	
		CopyBits(&rdbmap, &bmap, &clip_box, &clip_box, srcCopy, rgn);
	}
	
	if (offDraw){	
		position_offset.v = rdbounds.top;
		position_offset.h = rdbounds.left;
		LocalToGlobal(&position_offset);
		if (!(rdbmap.rowBytes & 0xC000))
			SetPortBits(&bmap);
		else
			SetPortPix((PixMapHandle)bmap.baseAddr);
		if (!copy)
			EraseRect(&rdbounds);
	} else
		rdbmap = bmap;
}

static void FreeOffscreenBuffer(BitMap *bmap)
/* Free the allocated PixMap? Not really. */
{  
	if (rdbuffer)
		HUnlock(rdbufferhand);
}

static void OffscreenWindowUtilsEnd(short validateRegion, RgnHandle rgn, short endDraw)
/* End offscreen drawing. validateRegion => set the window region to be valid (i.e.
   update to region not needed). rgn is masking region. endDraw => was drawing 
   offscreen, need to switch back to normal. For endDraw, Effect is called to
   move the offscreen image back to the screen; NULL => CopyBits.
   Usually called by EndWindowRdraw or RestoreWindowArea. */
{

	GrafPtr		port;
	BitMap		bmap;

	GetPort(&port);

	if (endDraw){	
		if (!rdbuffsize) {
			if (validateRegion) {
#if (TARGET_API_MAC_CARBON == 1)
				RgnHandle portClipRgn = NewRgn();
				portClipRgn = GetPortClipRegion(port, portClipRgn);
				ValidWindowRgn(GetWindowFromPort(port), portClipRgn);
				DisposeRgn(portClipRgn);
#else
				ValidRgn(port->clipRgn);
#endif
			}
			return;
		}
	}
	
#if (TARGET_API_MAC_CARBON == 1)
	const BitMap *portBits;
	portBits = GetPortBitMapForCopyBits(port);
	bmap = *portBits;
#else
	bmap=port->portBits;
#endif

	if (endDraw){
		if (!(rdbmap.rowBytes & 0xC000))
			SetPortBits(&rdbmap);
		else
			SetPortPix((PixMapHandle)rdbmap.baseAddr);
			
		position_offset.v = position_offset.h = 0;
			
		CopyBits(&bmap, &rdbmap, &rdbounds, &rdbounds, srcCopy, rgn);
	} else
		CopyBits(&rdbmap, &bmap, &rdbounds, &rdbounds, srcCopy, rgn);

	FreeOffscreenBuffer(&bmap);
	
	if (validateRegion) {
#if (TARGET_API_MAC_CARBON == 1)
		ValidWindowRect(GetWindowFromPort(port), &rdbounds);
#else
		ValidRect(&rdbounds);
#endif
	}
}

void BeginWindowRedraw(Rect *area, short copy, char isErase)
/* Set up for offscreen drawing. area is the bounding box for drawing; NULL =>
   entire port. Drawing is cliped to area. If copy is true, the current screen will
   be copied to the offscreen, otherwise the offscreen area will be erased. 
   Balance with EndWindowRedraw. Can be nested: higher-level calls will constrain
   the drawing area of nested calls. */
{
	if (isErase)
		EraseRect(area);
}

void EndWindowRedraw(short validateRegion)
/* Done with offscreen drawing; balances BeginWindowRedraw. */
{
}
