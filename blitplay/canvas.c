/* canvas.c - implementation of the Canvas MUI class */

#include <stdio.h>
#include <libraries/mui.h>

#include <clib/alib_protos.h>
#include <inline/muimaster_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>

#include "canvas.h"

/*
 * For now, we use the real dimensions and use those as a base for the
 * blit view area. This makes things easy, we also just use one bitplane
 * at the moment
 */
#define CANVAS_WIDTH 320
#define CANVAS_HEIGHT 32
#define CANVAS_DEPTH 1
#define CANVAS_FLAGS BMF_DISPLAYABLE | BMF_CLEAR

extern struct Library *MUIMasterBase;

struct CanvasData {
  /* dimensions of the canvas */
  LONG left, top, width, height;

  /* bitmap pointer */
  struct BitMap *bitmap;

  /* pointers to the x display and y display gadgets */
  Object *xdisplay, *ydisplay;

  /* drawing state */
  int mousex, mousey, is_drawing;
};

/* This is the new method dispatcher for our custom bitmap class */
ULONG mSetup(struct IClass *cl, Object *obj, struct MUIP_Setup *msg)
{
  if (!DoSuperMethodA(cl, obj, (APTR) msg)) return FALSE;
  MUI_RequestIDCMP(obj, IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE);
  return TRUE;
}

ULONG mCleanup(struct IClass *cl, Object *obj, struct MUIP_Cleanup *msg)
{
  MUI_RejectIDCMP(obj, IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE);
  return DoSuperMethodA(cl, obj, (APTR) msg);
}
ULONG mAskMinMax(struct IClass *cl, Object *obj, struct MUIP_AskMinMax *msg)
{
  DoSuperMethodA(cl, obj, (APTR) msg);
  msg->MinMaxInfo->MinWidth  += CANVAS_WIDTH;
  msg->MinMaxInfo->DefWidth  += CANVAS_WIDTH;
  msg->MinMaxInfo->MaxWidth  += CANVAS_WIDTH;
  msg->MinMaxInfo->MinHeight += CANVAS_HEIGHT;
  msg->MinMaxInfo->DefHeight += CANVAS_HEIGHT;
  msg->MinMaxInfo->MaxHeight += CANVAS_HEIGHT;
  return 0;
}
ULONG mDraw(struct IClass *cl, Object *obj, struct MUIP_Draw *msg)
{
  struct CanvasData *data = INST_DATA(cl, obj);
  ULONG result = 0;
  int pixelnum = (data->mousey - data->top) * CANVAS_WIDTH + (data->mousex - data->left),
    byteOff = 0, bitnum = 0;
  DoSuperMethodA(cl, obj, (APTR) msg);

  if (msg->flags & MADF_DRAWUPDATE) {
    /*
      we draw directly into the window's RastPort, note that there is no
      clipping !!!
    */
    byteOff = pixelnum / 8;
    bitnum = pixelnum & 0x07;
    data->bitmap->Planes[0][byteOff] |= 1 << ((7 - bitnum));

    BltBitMapRastPort(data->bitmap, 0, 0, _rp(obj),
                      data->left, data->top,
                      CANVAS_WIDTH, CANVAS_HEIGHT,
                      0xc0);

  } else if (msg->flags & MADF_DRAWOBJECT) {
    LONG leftEdge, topEdge, innerLeft, innerTop;

    GetAttr(MUIA_LeftEdge, obj, &leftEdge);
    GetAttr(MUIA_TopEdge, obj, &topEdge);
    GetAttr(MUIA_InnerLeft, obj, &innerLeft);
    GetAttr(MUIA_InnerTop, obj, &innerTop);

    data->left = leftEdge + innerLeft + 2; /* I think the "off" sizes */
    data->top = topEdge + innerTop + 1; /* are actually the borders TODO */
    data->width = CANVAS_WIDTH;
    data->height = CANVAS_HEIGHT;
    /*
    printf("updating component l = %d, t = %d, il = %d it = %d\n",
           (int) leftEdge, (int) topEdge, (int) innerLeft, (int) innerTop);
    */
    BltBitMapRastPort(data->bitmap, 0, 0, _rp(obj),
                      data->left, data->top,
                      CANVAS_WIDTH, CANVAS_HEIGHT,
                      0xc0);
  }
  return 0;
}

ULONG mHandleInput(struct IClass *cl, Object *obj, struct MUIP_HandleInput *msg)
{
  struct CanvasData *data = INST_DATA(cl, obj);
  LONG mx, my;

  if (msg->imsg) {
    switch (msg->imsg->Class) {
    case IDCMP_MOUSEBUTTONS:
      mx = msg->imsg->MouseX;
      my = msg->imsg->MouseY;
      if (mx >= data->left && mx < data->left + data->width
          && my >= data->top && my < data->top + data->height) {
        data->is_drawing = msg->imsg->Code == SELECTDOWN;
        if (data->is_drawing) {
          data->mousex = msg->imsg->MouseX;
          data->mousey = msg->imsg->MouseY;
          MUI_Redraw(obj, MADF_DRAWUPDATE);
        }
      }
      break;
    case IDCMP_MOUSEMOVE:
      /* only handle movements inside the boundaries */
      mx = msg->imsg->MouseX;
      my = msg->imsg->MouseY;
      
      if (mx >= data->left && mx < data->left + data->width
          && my >= data->top && my < data->top + data->height) {
        int index = (my - data->top) * CANVAS_WIDTH + (mx - data->left); 
        if (data->xdisplay)
          set(data->xdisplay, MUIA_String_Integer, index / 16);
        if (data->ydisplay)
          set(data->ydisplay, MUIA_String_Integer, index % 16);

        if (data->is_drawing) {
          data->mousex = msg->imsg->MouseX;
          data->mousey = msg->imsg->MouseY;

          MUI_Redraw(obj, MADF_DRAWUPDATE);
        }
      }
      break;
    default:
      break;
    }
  }
  return DoSuperMethodA(cl, obj, (APTR) msg);
}

ULONG mNewInstance(struct IClass *cl, Object *obj, struct opSet *msg)
{
  struct TagItem *tagListState = msg->ops_AttrList, *tag;
  Object *instance = (Object *) DoSuperMethodA(cl, obj, (APTR) msg);
  struct CanvasData *data = INST_DATA(cl, instance);

  /* This is the way to allocate the bitmap in an application for > V39 */
  data->bitmap = AllocBitMap(CANVAS_WIDTH, CANVAS_HEIGHT, CANVAS_DEPTH,
                             CANVAS_FLAGS, NULL);

  while (tag = NextTagItem(&tagListState)) {
    switch (tag->ti_Tag) {
    case CANVASA_XDISPLAY:
      data->xdisplay = (Object *) tag->ti_Data;
      break;
    case CANVASA_YDISPLAY:
      data->ydisplay = (Object *) tag->ti_Data;
      break;
    }
  }
  return (ULONG) instance;
}

ULONG mDisposeInstance(struct IClass *cl, Object *obj, struct opSet *msg)
{
  struct CanvasData *data = INST_DATA(cl, obj);
  /* Wait for blitter always, when freeing bitmap */
  WaitBlit();
  FreeBitMap(data->bitmap);
  return DoSuperMethodA(cl, obj, (APTR) msg);
}

ULONG mGet(struct IClass *cl, Object *obj, struct opGet *msg)
{
  struct CanvasData *data = INST_DATA(cl, obj);
  switch (msg->opg_AttrID) {
  case CANVASA_BITMAP:
    *(msg->opg_Storage) = (ULONG) data->bitmap;
    return TRUE;
  default:
    break;
  }
  return DoSuperMethodA(cl, obj, (APTR) msg);
}

ULONG CanvasDispatcher(__reg("a0") struct IClass *cl,
                       __reg("a2") Object *obj,
                       __reg("a1") Msg msg)
{
  switch (msg->MethodID) {
  case OM_NEW:           return mNewInstance(cl, obj, (APTR) msg);
  case OM_DISPOSE:       return mDisposeInstance(cl, obj, (APTR) msg);
  case OM_GET:           return mGet(cl, obj, (APTR) msg);

  case MUIM_AskMinMax:   return mAskMinMax(cl, obj, (APTR) msg);
  case MUIM_Draw:        return mDraw(cl, obj, (APTR) msg);
  case MUIM_HandleInput: return mHandleInput(cl, obj, (APTR) msg);
  case MUIM_Setup:       return mSetup(cl, obj, (APTR) msg);
  case MUIM_Cleanup:     return mCleanup(cl, obj, (APTR) msg);
  default:
    break;
  }
  return DoSuperMethodA(cl, obj, msg);
}

struct MUI_CustomClass *CreateCanvasClass()
{
  return MUI_CreateCustomClass(NULL, MUIC_Area, NULL,
                                      sizeof(struct CanvasData),
                                      (APTR) CanvasDispatcher);
}
