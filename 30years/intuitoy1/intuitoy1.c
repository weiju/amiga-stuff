/* intuitoy1.c - Intuition experiments for Amiga OS 1.x

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

#include <exec/libraries.h>
#include <intuition/intuition.h>

#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

#ifdef __VBCC__
#include <clib/alib_stdio_protos.h>
#endif

#define WIN_LEFT       10
#define WIN_TOP        10
#define WIN_WIDTH      320
#define WIN_HEIGHT     200
#define WIN_TITLE      "IntuiToy 1"
#define WIN_MIN_WIDTH  10
#define WIN_MIN_HEIGHT 10
#define WIN_MAX_WIDTH  200
#define WIN_MAX_HEIGHT 200

#define BUTTON_HEIGHT 20
#define BUTTON_TEXT_XOFFSET 4
#define TOPAZ_BASELINE 6

#define FILE_MENU_NUM       0
#define NUM_FILE_MENU_ITEMS 2

#define OPEN_MENU_ITEM_NUM  0
#define QUIT_MENU_ITEM_NUM  1

struct NewWindow newwin = {
  WIN_LEFT, WIN_TOP, WIN_WIDTH, WIN_HEIGHT, 0, 1,
  IDCMP_CLOSEWINDOW | IDCMP_MENUPICK | IDCMP_GADGETUP,
  WINDOWCLOSE | SMART_REFRESH | ACTIVATE | WINDOWSIZING | WINDOWDRAG | WINDOWDEPTH | NOCAREREFRESH,
  NULL, NULL, WIN_TITLE,
  NULL, NULL,
  WIN_MIN_WIDTH, WIN_MIN_HEIGHT,
  WIN_MAX_WIDTH, WIN_MAX_HEIGHT,
  WBENCHSCREEN
};

struct IntuiText menutext[] = {
  {0, 1, JAM2, 0, 1, NULL, "Open...", NULL},
  {0, 1, JAM2, 0, 1, NULL, "Quit", NULL}
};

struct MenuItem fileMenuItems[] = {
  {&fileMenuItems[1], 0, 0, 0, 0, ITEMTEXT | ITEMENABLED | HIGHCOMP | COMMSEQ, 0,
   &menutext[0], NULL, 'O', NULL, 0},
  {NULL, 0, 0, 0, 0, ITEMTEXT | ITEMENABLED | HIGHCOMP | COMMSEQ, 0,
   &menutext[1], NULL, 'Q', NULL, 0}
};

struct Menu menus[] = {
  {NULL, 20, 0, 0, 0, MENUENABLED | MIDRAWN, "File", &fileMenuItems[0], 0, 0, 0, 0}
};

struct Window *window;

void cleanup()
{
  if (window) {
    ClearMenuStrip(window);
    CloseWindow(window);
  }
}

#define REQ_WIDTH 180
#define REQ_HEIGHT 70
#define REQ_TEXT_XOFFSET 10

#define OK_BUTTON_X 10
#define OK_BUTTON_Y 40
#define OK_BUTTON_WIDTH  24

#define CANCEL_BUTTON_X 120
#define CANCEL_BUTTON_Y 40
#define CANCEL_BUTTON_WIDTH  54

#define PATH_GADGET_WIDTH 160


/*
 * This opens a very rudimentary file dialog, that demonstrates a true requester, which
 * is dependent on the parent window.
 */
void open_file()
{
  /* Note that all data here is static.
   The reason is that Intuition expects the data to still exist for the lifetime of the
   requester. If it's gone, the Guru will greet you.
  */
  static struct Requester requester;
  static WORD req_border_points[] = {
    0, 0, REQ_WIDTH - 1, 0, REQ_WIDTH - 1, REQ_HEIGHT - 1, 0, REQ_HEIGHT - 1, 0, 0
  };
  static struct Border req_border = {0, 0, 1, 0, JAM1, 5, req_border_points, NULL};

  static struct IntuiText labels[] = {
    {1, 0, JAM2, REQ_TEXT_XOFFSET, TOPAZ_BASELINE, NULL, "Enter file path", NULL},
    {1, 0, JAM2, BUTTON_TEXT_XOFFSET, TOPAZ_BASELINE, NULL, "Ok", NULL},
    {1, 0, JAM2, BUTTON_TEXT_XOFFSET, TOPAZ_BASELINE, NULL, "Cancel", NULL}
  };
  static WORD gadget_border_points[3][10] = {
    {0, 0, OK_BUTTON_WIDTH, 0, OK_BUTTON_WIDTH, BUTTON_HEIGHT, 0, BUTTON_HEIGHT, 0, 0},
    {0, 0, CANCEL_BUTTON_WIDTH, 0, CANCEL_BUTTON_WIDTH, BUTTON_HEIGHT, 0, BUTTON_HEIGHT, 0, 0},
    {-2, -2, PATH_GADGET_WIDTH, -2, PATH_GADGET_WIDTH, 10, -2, 10, -2, -2}
  };
  static struct Border gadget_borders[] = {
    {0, 0, 1, 0, JAM1, 5, gadget_border_points[0], NULL},
    {0, 0, 1, 0, JAM1, 5, gadget_border_points[1], NULL},
    {0, 0, 1, 0, JAM1, 5, gadget_border_points[2], NULL}
  };
  static UBYTE buffer[81], undobuffer[81];
  static struct StringInfo strinfo = {buffer, undobuffer, 0, 80, 0, 0, 0, 0, 0, 0, NULL, 0, NULL};

  static struct Gadget gadgets[] = {
    {&gadgets[1], OK_BUTTON_X, OK_BUTTON_Y, OK_BUTTON_WIDTH, BUTTON_HEIGHT, GFLG_GADGHCOMP,
     GACT_RELVERIFY | GACT_ENDGADGET, GTYP_BOOLGADGET | GTYP_REQGADGET,
     &gadget_borders[0], NULL, &labels[1], 0, NULL, 101, NULL},
    {&gadgets[2], CANCEL_BUTTON_X, CANCEL_BUTTON_Y, CANCEL_BUTTON_WIDTH, BUTTON_HEIGHT, GFLG_GADGHCOMP,
     GACT_RELVERIFY | GACT_ENDGADGET, GTYP_BOOLGADGET | GTYP_REQGADGET,
     &gadget_borders[1], NULL, &labels[2], 0, NULL, 102, NULL},
    {NULL, OK_BUTTON_X, 20, PATH_GADGET_WIDTH, 10,
     GFLG_GADGHCOMP, GACT_RELVERIFY, GTYP_STRGADGET, &gadget_borders[2], NULL, &labels[3],
     0, &strinfo, 103, NULL},
  };

  BOOL result;
  InitRequester(&requester);
  requester.LeftEdge = 50;
  requester.TopEdge = 50;
  requester.Width = REQ_WIDTH;
  requester.Height = REQ_HEIGHT;
  requester.Flags = 0;
  requester.BackFill = 0;
  requester.ReqGadget = &gadgets[0];
  requester.ReqBorder = &req_border;
  requester.ReqText = &labels[0];
  result = Request(&requester, window);
  if (result) puts("Requester could be opened");
  else puts("Requester could not be opened");
}

BOOL handle_menu(UWORD menuNum, UWORD itemNum, UWORD subItemNum)
{
  printf("menu, menu num: %d, item num: %d, sub item num: %d\n",
         (int) menuNum, (int) itemNum, (int) subItemNum);
  if (menuNum == FILE_MENU_NUM && itemNum == QUIT_MENU_ITEM_NUM) {
    /* quit */
    return TRUE;
  }
  if (menuNum == FILE_MENU_NUM && itemNum == OPEN_MENU_ITEM_NUM) {
    open_file();
  }
  return FALSE;
}

/*
 * The main event loop.
 */
void handle_events() {
  BOOL done = FALSE;
  struct IntuiMessage *msg;
  ULONG msgClass;
  UWORD menuCode;

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
      case IDCMP_GADGETUP:
        printf("Click me clicked, id: %d\n", (int) ((struct Gadget *) (msg->IAddress))->GadgetID);
        break;
      default:
        break;
      }
      ReplyMsg((struct Message *) msg);
    }
  }
}

/*
 * Laying out the pulldown menu strip.
 * The menu is also the invocation point to test out requester types.
 */
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

/*
 * Create some application gadgets inside the window here. Playing around with
 * settings, events and layout.
 */
#define CLICKME_BUTTON_WIDTH 90
#define CLICKME_BUTTON_X 10
#define CLICKME_BUTTON_Y 20

#define CLICKME2_BUTTON_WIDTH 120
#define CLICKME2_BUTTON_X 110
#define CLICKME2_BUTTON_Y 20

#define SLIDER_WIDTH 200
#define SLIDER_HEIGHT 10
#define SLIDER_LABEL_YOFFSET -10
#define SLIDER_X 10
#define SLIDER_Y 65

#define STRING_X 12
#define STRING_Y 94
#define STRING_WIDTH 80
#define STRING_HEIGHT 10
#define STRING_LABEL_YOFFSET -14
#define STRING_BORDER_X -2
#define STRING_BORDER_Y -2

#define INTEGER_X 12
#define INTEGER_Y 124
#define INTEGER_WIDTH 80
#define INTEGER_HEIGHT 10

struct Gadget *setup_gadgets()
{
  static struct IntuiText gadget_labels[] = {
    {1, 0, JAM2, BUTTON_TEXT_XOFFSET, TOPAZ_BASELINE, NULL, "Click me !", NULL},
    {1, 0, JAM2, BUTTON_TEXT_XOFFSET, TOPAZ_BASELINE, NULL, "Click me too !", NULL},
    {1, 0, JAM2, BUTTON_TEXT_XOFFSET, SLIDER_LABEL_YOFFSET, NULL, "A slider", NULL},
    {1, 0, JAM2, BUTTON_TEXT_XOFFSET, STRING_LABEL_YOFFSET, NULL, "A string gadget", NULL},
    {1, 0, JAM2, BUTTON_TEXT_XOFFSET, STRING_LABEL_YOFFSET, NULL, "An integer gadget", NULL}
  };
  static WORD gadget_border_points[3][10] = {
    {0, 0, CLICKME_BUTTON_WIDTH, 0, CLICKME_BUTTON_WIDTH, BUTTON_HEIGHT,
     0, BUTTON_HEIGHT, 0, 0},
    {0, 0, CLICKME2_BUTTON_WIDTH, 0, CLICKME2_BUTTON_WIDTH, BUTTON_HEIGHT,
     0, BUTTON_HEIGHT, 0, 0},
    {STRING_BORDER_X, STRING_BORDER_Y, STRING_WIDTH, STRING_BORDER_Y,
     STRING_WIDTH, STRING_HEIGHT, STRING_BORDER_X, STRING_HEIGHT,
     STRING_BORDER_X, STRING_BORDER_Y}
  };
  static struct Border gadget_border[] = {
    {0, 0, 1, 0, JAM1, 5, gadget_border_points[0], NULL},
    {0, 0, 1, 0, JAM1, 5, gadget_border_points[1], NULL},
    {0, 0, 1, 0, JAM1, 5, gadget_border_points[2], NULL}
  };
  static struct PropInfo propinfos[] = {
    {AUTOKNOB | FREEHORIZ, 0, 0, 10, MAXBODY, 0, 0, 0, 0, 0, 0}
  };
  static struct Image slider_image;
  static UBYTE buffer[11], undobuffer[11], buffer2[11], undobuffer2[11];
  static struct StringInfo strinfos[] = {
    {buffer, undobuffer, 0, 10, 0, 0, 0, 0, 0, 0, NULL, 0, NULL},
    {buffer2, undobuffer2, 0, 10, 0, 0, 0, 0, 0, 0, NULL, 0, NULL}
  };
  static struct Gadget gadgets[] = {
    {&gadgets[1], CLICKME_BUTTON_X, CLICKME_BUTTON_Y, CLICKME_BUTTON_WIDTH, BUTTON_HEIGHT,
     GFLG_GADGHBOX, GACT_RELVERIFY, GTYP_BOOLGADGET, &gadget_border[0], NULL, &gadget_labels[0],
     0, NULL, 1, NULL},
    {&gadgets[2], CLICKME2_BUTTON_X, CLICKME2_BUTTON_Y, CLICKME2_BUTTON_WIDTH, BUTTON_HEIGHT,
     GFLG_GADGHCOMP, GACT_RELVERIFY, GTYP_BOOLGADGET, &gadget_border[1], NULL, &gadget_labels[1],
     0, NULL, 2, NULL},
    {&gadgets[3], SLIDER_X, SLIDER_Y, SLIDER_WIDTH, SLIDER_HEIGHT,
     GFLG_GADGHCOMP, GACT_RELVERIFY, GTYP_PROPGADGET, &slider_image, NULL, &gadget_labels[2],
     0, &propinfos[0], 3, NULL},
    {&gadgets[4], STRING_X, STRING_Y, STRING_WIDTH, STRING_HEIGHT,
     GFLG_GADGHCOMP, GACT_RELVERIFY, GTYP_STRGADGET, &gadget_border[2], NULL, &gadget_labels[3],
     0, &strinfos[0], 3, NULL},
    {NULL, INTEGER_X, INTEGER_Y, INTEGER_WIDTH, INTEGER_HEIGHT,
     GFLG_GADGHCOMP, GACT_LONGINT, GTYP_STRGADGET, &gadget_border[2], NULL, &gadget_labels[4],
     0, &strinfos[1], 3, NULL},
  };
  buffer[0] = 0;
  undobuffer[0] = 0;
  buffer2[0] = 0;
  undobuffer2[0] = 0;
  strcpy(buffer2, "0");
  return &gadgets[0];
}

int main(int argc, char **argv)
{
  newwin.FirstGadget = setup_gadgets();
  if (window = OpenWindow(&newwin)) {
    setup_menu();
    handle_events();
  }
  cleanup();
  return 1;
}
