/* iffview.c - IFF ILBM viewer application for Amiga OS >= 1.3

   This file is part of amiga30yrs.

   amiga30yrs is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   amiga30yrs is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with amiga30yrs.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <string.h>
#include <stdlib.h>

#include <exec/libraries.h>
#include <intuition/intuition.h>

#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

#ifdef __VBCC__
#include <clib/alib_stdio_protos.h>
#endif
#include "ilbm.h"

#define WIN_TITLE      "IFF Viewer"
#define FILE_MENU_NUM       0
#define NUM_FILE_MENU_ITEMS 2

#define QUIT_MENU_ITEM_NUM  0

struct NewWindow newwin = {
  0, 0, 0, 0, 0, 1,
  IDCMP_CLOSEWINDOW | IDCMP_MENUPICK,
  WFLG_CLOSEGADGET | WFLG_ACTIVATE | WFLG_NOCAREREFRESH | WFLG_BACKDROP,
  NULL, NULL, WIN_TITLE,
  NULL, NULL,
  0, 0,
  0, 0,
  CUSTOMSCREEN
};

struct IntuiText menutext[] = {
  {0, 1, JAM2, 0, 1, NULL, "Open...", NULL},
  {0, 1, JAM2, 0, 1, NULL, "Quit", NULL}
};

struct MenuItem fileMenuItems[] = {
  {NULL, 0, 0, 0, 0, ITEMTEXT | ITEMENABLED | HIGHCOMP | COMMSEQ, 0,
   &menutext[1], NULL, 'Q', NULL, 0}
};

struct Menu menus[] = {
  {NULL, 20, 0, 0, 0, MENUENABLED | MIDRAWN, "File", &fileMenuItems[0], 0, 0, 0, 0}
};

struct Window *window;
struct Screen *screen;

void cleanup()
{
    if (window) {
        ClearMenuStrip(window);
        CloseWindow(window);
    }
    if (screen) {
        CloseScreen(screen);
    }
}

BOOL handle_menu(UWORD menuNum, UWORD itemNum, UWORD subItemNum)
{
    printf("menu, menu num: %d, item num: %d, sub item num: %d\n",
           (int) menuNum, (int) itemNum, (int) subItemNum);
    if (menuNum == FILE_MENU_NUM && itemNum == QUIT_MENU_ITEM_NUM) {
        /* quit */
        return TRUE;
    }
    return FALSE;
}

void handle_events()
{
    BOOL done = FALSE;
    struct IntuiMessage *msg;
    ULONG msgClass;
    UWORD menuCode;
    int buttonId;

    while (!done) {
        Wait(1 << window->UserPort->mp_SigBit);
        if (msg = (struct IntuiMessage *) GetMsg(window->UserPort)) {
            msgClass = msg->Class;
            switch (msgClass) {
            case IDCMP_CLOSEWINDOW:
                done = TRUE;
                break;
            case IDCMP_MENUPICK:
                menuCode = msg->Code;
                done = handle_menu(MENUNUM(menuCode), ITEMNUM(menuCode), SUBNUM(menuCode));
                break;
            case IDCMP_REQCLEAR:
                puts("requester closed");
                break;
            default:
                break;
            }
            ReplyMsg((struct Message *) msg);
        }
    }
}

void setup_menu()
{
    UWORD txWidth, txHeight, txBaseline, txSpacing, itemWidth, itemHeight, numItems;
    struct RastPort *rp = &window->WScreen->RastPort;
    int i;

    txWidth = rp->TxWidth;
    txHeight = rp->TxHeight;
    txBaseline = rp->TxBaseline;
    txSpacing = rp->TxSpacing;
    printf("TxWidth: %d, TxHeight: %d, TxBaseline: %d, TxSpacing: %d\n",
           (int) txWidth, (int) txHeight, (int) txBaseline, (int) txSpacing);

    /* Set file menu bounds */
    menus[0].Width = TextLength(rp, "File", strlen("File")) + txWidth;
    menus[0].Height = txHeight;

    /* Set file menu items bounds */
    /* We actually need to know what the command uses up */
    itemWidth = txWidth * strlen("Open...") + 50;
    itemHeight = txHeight + 2;  /* 2 pixels adjustment */

    numItems = sizeof(fileMenuItems) / sizeof(struct MenuItem);
    printf("# file items: %d\n", (int) numItems);
    for (i = 0; i < numItems; i++) {
        fileMenuItems[i].TopEdge = i * itemHeight;
        fileMenuItems[i].Height = itemHeight;
        fileMenuItems[i].Width = itemWidth;
    }

    SetMenuStrip(window, &menus[0]);
}

struct Image image = { 0, 0, 0, 0, 0, NULL, 0, 0, NULL};
struct NewScreen newscreen = {
    0, 0, 320, 200, 5,
    0, 1, // pens
    0, // view modes
    CUSTOMSCREEN, // type
    NULL, // font
    "My Screen", // title
    NULL, // unused (gadgets)
    NULL // custom bitmap
};

/* Defined automatically in VBCC */
extern struct Library *DOSBase;

int main(int argc, char **argv)
{
    if (argc <= 1) {
        puts("Usage: iffview <iff-file>");
        return 0;
    }
    ILBMData *ilbm_data = NULL;
    ilbm_data = parse_file(argv[1]);
    if (!ilbm_data) exit(0);
    image.Width = ilbm_data->bmheader->w;
    image.Height = ilbm_data->bmheader->h;
    image.Depth = ilbm_data->bmheader->nPlanes;
    // if image.Width must be a multiple of 16
    int mod16 = image.Width % 16;
    image.Width = mod16 == 0 ? image.Width : image.Width + (16 - mod16);
    int wordwidth = image.Width / 16;
    int finalsize = wordwidth * image.Height * image.Depth * sizeof(UWORD);
    image.ImageData = AllocMem(finalsize, MEMF_CHIP|MEMF_CLEAR);
    ilbm_to_image_data((char *) image.ImageData, ilbm_data, image.Width, image.Height);
    image.PlanePick = (1 << image.Depth) - 1;

    /* Adjust the new screen according to the IFF image */
    newscreen.ViewModes = ilbm_data->bmheader->camgFlags;
    newscreen.Depth = image.Depth;
    screen = OpenScreen(&newscreen);
    if (screen) {
        ShowTitle(screen, FALSE);
        for (int i = 0; i < ilbm_data->num_colors; i++) {
            ColorRegister *color = &ilbm_data->colors[i];
            SetRGB4(&screen->ViewPort, i, color->red >> 4,
                    color->green >> 4, color->blue >> 4);
        }
        newwin.Width = newwin.MinWidth = newwin.MaxWidth = newscreen.Width;
        newwin.Height = newwin.MinHeight = newwin.MaxWidth = newscreen.Height;
        newwin.Screen = screen;
        if (window = OpenWindow(&newwin)) {
            setup_menu();
            // When drawing to a screen's rastport directly, the image must be contained
            // within its boundaries otherwise it crashes, draw to a Window and it will be clipped
            // Note: rastport is the entire window, including title bars
            if (screen) DrawImage(window->RPort, &image, 0, 0);
            handle_events();
        }
    }
    if (ilbm_data) free_ilbm_data(ilbm_data);
    cleanup();
    return 1;
}
