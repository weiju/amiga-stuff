#include <string.h>
#include <intuition/intuition.h>
#include <dos/dosextens.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_stdio_protos.h>

#include "filereq.h"
#include "dos13.h"

#define LABEL_DRAWER 0
#define LABEL_FILE   1
#define LABEL_PARENT 2
#define LABEL_DRIVES 3
#define LABEL_OPEN   4
#define LABEL_CANCEL 5

#define REQWIN_WIDTH 260
#define REQWIN_HEIGHT 180
#define REQ_X 10
#define REQ_Y 16
#define REQ_WIDTH 240
#define REQ_HEIGHT 175
#define TOPAZ_BASELINE 8

#define BUTTON_Y      135
#define BUTTON_HEIGHT 18

#define BUTTON_TEXT_XOFFSET  5
#define BUTTON_TEXT_YOFFSET  (TOPAZ_BASELINE - 3)

#define OK_BUTTON_X          0
#define OK_BUTTON_WIDTH      40
#define DRIVES_BUTTON_X      46
#define DRIVES_BUTTON_WIDTH  58
#define PARENT_BUTTON_X      110
#define PARENT_BUTTON_WIDTH  58
#define CANCEL_BUTTON_X      174
#define CANCEL_BUTTON_WIDTH  58

#define DRAWER_GADGET_X  60
#define DRAWER_GADGET_Y  98
#define FILE_GADGET_X  DRAWER_GADGET_X
#define FILE_GADGET_Y  115
#define PATH_GADGET_WIDTH    160
#define PATH_GADGET_HEIGHT   10
#define STR_LABEL_XOFFSET -60
#define STR_LABEL_YOFFSET 2

#define FILE_LIST_X     0
#define FILE_LIST_Y     0
#define FILE_LIST_WIDTH 229
#define FILE_LIST_HEIGHT 86

#define FILE_LIST_BM_X     1
#define FILE_LIST_BM_Y     1
#define FILE_LIST_BM_WIDTH (FILE_LIST_WIDTH - 2)
#define FILE_LIST_BM_HEIGHT (FILE_LIST_HEIGHT - 2)

#define LIST_VSLIDER_X FILE_LIST_WIDTH
#define LIST_VSLIDER_Y 0
#define LIST_VSLIDER_WIDTH 11
#define LIST_VSLIDER_HEIGHT 65

#define LIST_UP_X FILE_LIST_WIDTH
#define LIST_UP_Y 65
#define LIST_DOWN_X LIST_UP_X
#define LIST_DOWN_Y (LIST_UP_Y + LIST_BUTTON_HEIGHT)
#define LIST_BUTTON_WIDTH 11
#define LIST_BUTTON_HEIGHT 11

#define REQ_OK_BUTTON_ID     101
#define REQ_DRIVES_BUTTON_ID 102
#define REQ_PARENT_BUTTON_ID 103
#define REQ_CANCEL_BUTTON_ID 104

#define REQ_DIR_TEXT_ID 110
#define REQ_FILE_TEXT_ID 111
#define REQ_VSLIDER_ID 112
#define REQ_UP_ID 113
#define REQ_DOWN_ID 114

static struct Requester requester;
static BOOL req_opened = FALSE;
static struct Window *req_window;

// if image data is in fast RAM, we see nothing !!!
static UWORD __chip up_imdata[] = { 65504, 32800, 32800, 32800, 33824, 36384, 39712, 32800,
                                    32800, 32800, 65504 };
static struct Image up_image = { 0, 0, 11, 11, 1, up_imdata, 1, 0, NULL };

static UWORD __chip down_imdata[] = { 65504, 32800, 32800, 32800, 39712, 36384, 33824, 32800,
                                      32800, 32800, 65504 };
static struct Image down_image = { 0, 0, 11, 11, 1, down_imdata, 1, 0, NULL };

static struct IntuiText labels[] = {
    {1, 0, JAM2, STR_LABEL_XOFFSET, STR_LABEL_YOFFSET, NULL, "Drawer", NULL},
    {1, 0, JAM2, STR_LABEL_XOFFSET, STR_LABEL_YOFFSET, NULL, "File", NULL},
    {1, 0, JAM2, BUTTON_TEXT_XOFFSET, BUTTON_TEXT_YOFFSET, NULL, "Parent", NULL},
    {1, 0, JAM2, BUTTON_TEXT_XOFFSET, BUTTON_TEXT_YOFFSET, NULL, "Drives", NULL},
    {1, 0, JAM2, BUTTON_TEXT_XOFFSET, BUTTON_TEXT_YOFFSET, NULL, "Open", NULL},
    {1, 0, JAM2, BUTTON_TEXT_XOFFSET, BUTTON_TEXT_YOFFSET, NULL, "Cancel", NULL}
};


static WORD ok_border_points[] = {0, 0, OK_BUTTON_WIDTH, 0, OK_BUTTON_WIDTH, BUTTON_HEIGHT, 0,
                                  BUTTON_HEIGHT, 0, 0};
static WORD drives_border_points[] = {0, 0, DRIVES_BUTTON_WIDTH, 0, DRIVES_BUTTON_WIDTH,
                                      BUTTON_HEIGHT, 0, BUTTON_HEIGHT, 0, 0};

static WORD cancel_border_points[] = {0, 0, CANCEL_BUTTON_WIDTH, 0, CANCEL_BUTTON_WIDTH,
                                      BUTTON_HEIGHT, 0, BUTTON_HEIGHT, 0, 0};
static WORD list_border_points[] = {0, 0, FILE_LIST_WIDTH, 0, FILE_LIST_WIDTH,
                                    FILE_LIST_HEIGHT, 0, FILE_LIST_HEIGHT, 0, 0};

/* the -2 is the margin to set to avoid that the string gadget will overdraw the
   borders */
static WORD string_border_points[] =  {-2, -2, PATH_GADGET_WIDTH, -2, PATH_GADGET_WIDTH,
                                       10, -2, 10, -2, -2};

static struct Border ok_button_border = {0, 0, 1, 0, JAM1, 5, ok_border_points, NULL};
static struct Border drives_button_border = {0, 0, 1, 0, JAM1, 5, cancel_border_points, NULL};
static struct Border parent_button_border = {0, 0, 1, 0, JAM1, 5, cancel_border_points, NULL};
static struct Border cancel_button_border = {0, 0, 1, 0, JAM1, 5, cancel_border_points, NULL};
static struct Border str_gadget_border = {0, 0, 1, 0, JAM1, 5, string_border_points, NULL};
static struct Border file_list_border = {FILE_LIST_X, FILE_LIST_Y, 1, 0, JAM1, 5,
                                         list_border_points, NULL};

static UBYTE buffer1[82], undobuffer1[82];
static struct StringInfo strinfo1 = {buffer1, undobuffer1, 0, 80, 0, 0, 0, 0, 0, 0, NULL, 0, NULL};
static UBYTE buffer2[82], undobuffer2[82];
static struct StringInfo strinfo2 = {buffer2, undobuffer2, 0, 80, 0, 0, 0, 0, 0, 0, NULL, 0, NULL};

static struct Image slider_image;
static struct PropInfo propinfo = {AUTOKNOB | FREEVERT, 0, 0, MAXBODY, MAXBODY,
                                   0, 0, 0, 0, 0, 0};

#define WIN_TITLE "Open File..."

static struct NewWindow newwin = {
  0, 0, REQWIN_WIDTH, REQWIN_HEIGHT, 0, 1,
  IDCMP_GADGETUP,
  WFLG_CLOSEGADGET | WFLG_SMART_REFRESH | WFLG_ACTIVATE | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_NOCAREREFRESH,
  NULL, NULL, WIN_TITLE,
  NULL, NULL,
  REQWIN_WIDTH, REQWIN_HEIGHT,
  REQWIN_WIDTH, REQWIN_HEIGHT,
  WBENCHSCREEN
};

static struct Gadget list_down = {NULL, LIST_DOWN_X, LIST_DOWN_Y,
                                  LIST_BUTTON_WIDTH, LIST_BUTTON_HEIGHT,
                                  GFLG_GADGHCOMP | GFLG_GADGIMAGE, GACT_RELVERIFY,
                                  GTYP_BOOLGADGET | GTYP_REQGADGET,
                                  &down_image, NULL,
                                  NULL, 0, NULL,
                                  REQ_DOWN_ID, NULL};

static struct Gadget list_up = {&list_down, LIST_UP_X, LIST_UP_Y,
                                LIST_BUTTON_WIDTH, LIST_BUTTON_HEIGHT,
                                GFLG_GADGHCOMP | GFLG_GADGIMAGE, GACT_RELVERIFY,
                                GTYP_BOOLGADGET | GTYP_REQGADGET,
                                &up_image, NULL,
                                NULL, 0, NULL,
                                REQ_UP_ID, NULL};

static struct Gadget list_vslider = {&list_up, LIST_VSLIDER_X, LIST_VSLIDER_Y,
                                     LIST_VSLIDER_WIDTH, LIST_VSLIDER_HEIGHT,
                                     GFLG_GADGHCOMP, GACT_RELVERIFY, GTYP_PROPGADGET,
                                     &slider_image, NULL, NULL, 0, &propinfo,
                                     REQ_VSLIDER_ID, NULL};

static struct Gadget file_text = {&list_vslider, FILE_GADGET_X, FILE_GADGET_Y,
                                  PATH_GADGET_WIDTH, PATH_GADGET_HEIGHT,
                                  GFLG_GADGHCOMP, GACT_RELVERIFY, GTYP_STRGADGET,
                                  &str_gadget_border, NULL, &labels[LABEL_FILE], 0, &strinfo2,
                                  REQ_FILE_TEXT_ID, NULL};

static struct Gadget dir_text = {&file_text, DRAWER_GADGET_X, DRAWER_GADGET_Y,
                                 PATH_GADGET_WIDTH, PATH_GADGET_HEIGHT,
                                 GFLG_GADGHCOMP, GACT_RELVERIFY, GTYP_STRGADGET,
                                 &str_gadget_border,
                                 NULL, &labels[LABEL_DRAWER], 0, &strinfo1,REQ_DIR_TEXT_ID, NULL};

//  Note: Cancel does not specify the GACT_ENDGADGET flag, it seems that
//  IDCMP_REQCLEAR is not fired when Intuition closes the requester automatically
static struct Gadget cancel_button = {&dir_text, CANCEL_BUTTON_X, BUTTON_Y, CANCEL_BUTTON_WIDTH,
                                      BUTTON_HEIGHT, GFLG_GADGHCOMP, GACT_RELVERIFY,
                                      GTYP_BOOLGADGET | GTYP_REQGADGET, &cancel_button_border, NULL,
                                      &labels[LABEL_CANCEL], 0, NULL, REQ_CANCEL_BUTTON_ID, NULL};

static struct Gadget parent_button = {&cancel_button, PARENT_BUTTON_X, BUTTON_Y,
                                      PARENT_BUTTON_WIDTH, BUTTON_HEIGHT,
                                      GFLG_GADGHCOMP, GACT_RELVERIFY,
                                      GTYP_BOOLGADGET | GTYP_REQGADGET,
                                      &parent_button_border, NULL,
                                      &labels[LABEL_PARENT], 0, NULL,
                                      REQ_PARENT_BUTTON_ID, NULL};

static struct Gadget drives_button = {&parent_button, DRIVES_BUTTON_X, BUTTON_Y,
                                      DRIVES_BUTTON_WIDTH, BUTTON_HEIGHT,
                                      GFLG_GADGHCOMP, GACT_RELVERIFY,
                                      GTYP_BOOLGADGET | GTYP_REQGADGET,
                                      &drives_button_border, NULL,
                                      &labels[LABEL_DRIVES], 0, NULL,
                                      REQ_DRIVES_BUTTON_ID, NULL};
static struct Gadget ok_button = {&drives_button, OK_BUTTON_X, BUTTON_Y,
                                  OK_BUTTON_WIDTH, BUTTON_HEIGHT,
                                  GFLG_GADGHCOMP, GACT_RELVERIFY,
                                  GTYP_BOOLGADGET | GTYP_REQGADGET,
                                  &ok_button_border, NULL,
                                  &labels[LABEL_OPEN], 0, NULL,
                                  REQ_OK_BUTTON_ID, NULL};

static int filelist_bm_depth = 1;
static struct BitMap filelist_bitmap;
static struct RastPort filelist_rastport;

#define PATHBUFFER_SIZE 200
static char dirname[PATHBUFFER_SIZE + 1];
static BPTR flock;
static LONG error;
static struct FileInfoBlock fileinfo;

static struct TextFont *reqwin_font;
static UWORD font_baseline, font_height;

static void print_fileinfo(struct FileInfoBlock *fileinfo)
{
    if (fileinfo->fib_DirEntryType > 0) {
        printf("dir: '%s'\n", fileinfo->fib_FileName);
    } else {
        printf("file: '%s'\n", fileinfo->fib_FileName);
    }
}

static void close_requester()
{
    if (req_opened) {
        EndRequest(&requester, req_window);
        req_opened = FALSE;
    }
}

static void handle_events()
{
    BOOL done = FALSE;
    struct IntuiMessage *msg;
    ULONG msgClass;
    UWORD menuCode;
    int buttonId;

    while (!done) {
        Wait(1 << req_window->UserPort->mp_SigBit);
        if (msg = (struct IntuiMessage *) GetMsg(req_window->UserPort)) {
            msgClass = msg->Class;
            switch (msgClass) {
            case IDCMP_GADGETUP:
                buttonId = (int) ((struct Gadget *) (msg->IAddress))->GadgetID;
                ReplyMsg((struct Message *) msg);
                if (buttonId == REQ_OK_BUTTON_ID) {
                    close_requester();
                    done = TRUE;
                }
                else if (buttonId == REQ_CANCEL_BUTTON_ID) {
                    close_requester();
                    done = TRUE;
                }
                break;
            default:
                break;
            }
        }
    }
}

static void cleanup()
{
    for (int i = 0; i < filelist_bm_depth; i++) {
        if (filelist_bitmap.Planes[i]) FreeRaster(filelist_bitmap.Planes[i],
                                                  FILE_LIST_WIDTH,
                                                  FILE_LIST_HEIGHT);
    }
}


void draw_list()
{
    struct FileListEntry *files = scan_current_dir(), *cur;

    // Note that we need to render into the requester
    // layer's rastport, because it is rendered on top of the
    // parent window and obscures the content
    struct RastPort *src_rp = &filelist_rastport;
    struct RastPort *dst_rp = requester.ReqLayer->rp;

    // make sure drawing is clipped, otherwise it will
    // draw somewhere else into memory
    SetAPen(src_rp, 1);
    int ypos = 10;

    cur = files;
    while (cur) {
        Move(src_rp, 8, ypos);
        Text(src_rp, cur->name, strlen(cur->name));
        cur = cur->next;
        ypos += font_height;
        if (ypos > FILE_LIST_BM_HEIGHT) break;
    }
    BltBitMapRastPort(&filelist_bitmap, 0, 0, dst_rp,
                      FILE_LIST_BM_X, FILE_LIST_BM_Y, FILE_LIST_BM_WIDTH, FILE_LIST_BM_HEIGHT,
                      0xc0);
    free_file_list(files);
}

void open_file(struct Window *window)
{
    BOOL result;
    if (req_window = OpenWindow(&newwin)) {
        InitRequester(&requester);
        requester.LeftEdge = REQ_X;
        requester.TopEdge = REQ_Y;
        requester.Width = REQ_WIDTH;
        requester.Height = REQ_HEIGHT;
        requester.Flags = SIMPLEREQ | NOREQBACKFILL;
        requester.BackFill = 0;
        requester.ReqGadget = &ok_button;
        requester.ReqBorder = &file_list_border;
        requester.ReqText = NULL;

        reqwin_font = req_window->IFont;
        font_height = reqwin_font->tf_YSize;
        font_baseline = reqwin_font->tf_Baseline;

        printf("Font ysize: %d, baseline: %d\n", (int) font_height, (int) font_baseline);

        InitBitMap(&filelist_bitmap, filelist_bm_depth, FILE_LIST_BM_WIDTH, FILE_LIST_BM_HEIGHT);
        for (int i = 0; i < filelist_bm_depth; i++) filelist_bitmap.Planes[i] = NULL;
        for (int i = 0; i < filelist_bm_depth; i++) {
            if (!(filelist_bitmap.Planes[i] = AllocRaster(FILE_LIST_WIDTH, FILE_LIST_HEIGHT))) {
                cleanup();
                return;
            } else {
                BltClear(filelist_bitmap.Planes[i],
                         RASSIZE(FILE_LIST_WIDTH, FILE_LIST_HEIGHT), 1);
            }
        }
        InitRastPort(&filelist_rastport);
        filelist_rastport.BitMap = &filelist_bitmap;

        if (req_opened = Request(&requester, req_window)) {
            draw_list();
            handle_events();
            CloseWindow(req_window);
            req_window = NULL;
        } else {
            puts("Request() failed !!!");
        }
        cleanup();
    } else {
        puts("OpenWindow() failed !!!");
    }
}
