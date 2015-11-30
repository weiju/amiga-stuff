#ifndef __CANVAS_H__
#define __CANVAS_H__

/*
 * A custom MUI class for a canvas where the user can freely draw
 * with the mouse.
 * The CANVASA_XDISPLAY/CANVASA_YDISPLAY attributes are pointers
 * to MUI string gadgets and will be used to display the relative
 * mouse x and y coordinates if they are defined.
 */
#define CANVASA_XDISPLAY (TAG_USER + 1)
#define CANVASA_YDISPLAY (TAG_USER + 2)
#define CANVASA_BITMAP   (TAG_USER + 3)

extern struct MUI_CustomClass *CreateCanvasClass();

#endif /* __CANVAS_H__ */
