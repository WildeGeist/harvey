/*
 * This file is part of the UCB release of Plan 9. It is subject to the license
 * terms in the LICENSE file found in the top-level directory of this
 * distribution and at http://akaros.cs.berkeley.edu/files/Plan9License. No
 * part of the UCB release of Plan 9, including this file, may be copied,
 * modified, propagated, or distributed except according to the terms contained
 * in the LICENSE file.
 */

#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>
#include <memlayer.h>

int
memload(Memimage *dst, Rectangle r, uint8_t *data, int n, int iscompressed)
{
	int (*loadfn)(Memimage*, Rectangle, uint8_t*, int);
	Memimage *tmp;
	Memlayer *dl;
	Rectangle lr;
	int dx;

	loadfn = loadmemimage;
	if(iscompressed)
		loadfn = cloadmemimage;

    Top:
	dl = dst->layer;
	if(dl == nil)
		return loadfn(dst, r, data, n);

	/*
 	 * Convert to screen coordinates.
	 */
	lr = r;
	r.min.x += dl->delta.x;
	r.min.y += dl->delta.y;
	r.max.x += dl->delta.x;
	r.max.y += dl->delta.y;
	dx = dl->delta.x&(7/dst->depth);
	if(dl->clear && dx==0){
		dst = dl->screen->image;
		goto Top;
	}

	/*
	 * dst is an obscured layer or data is unaligned
	 */
	if(dl->save && dx==0){
		n = loadfn(dl->save, lr, data, n);
		if(n > 0)
			memlexpose(dst, r);
		return n;
	}
	tmp = allocmemimage(lr, dst->chan);
	if(tmp == nil)
		return -1;
	n = loadfn(tmp, lr, data, n);
	memdraw(dst, lr, tmp, lr.min, nil, lr.min, S);
	freememimage(tmp);
	return n;
}