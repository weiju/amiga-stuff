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

#define BUTTON_HMARGIN 5
#define REQ_HMARGIN 10
#define REQ_VMARGIN 14

#define FILE_LIST_HMARGIN   5
#define FILE_LIST_VMARGIN   3
#define FILE_LIST_LINE_DIST 1
#define LIST_VSLIDER_WIDTH 11
#define LIST_BUTTON_WIDTH 11
#define LIST_BUTTON_HEIGHT 11
#define FILE_LIST_BM_X     1
#define FILE_LIST_BM_Y     1

#define NUM_FILE_ENTRIES 10

// some of these variables don't need to be static
static int filelist_width;
static int filelist_height;
static int filelist_bm_width;
static int filelist_bm_height;

enum { LABEL_DRAWER = 0, LABEL_FILE, LABEL_PARENT, LABEL_DRIVES, LABEL_OPEN, LABEL_CANCEL } Labels;

#define REQWIN_HEIGHT 180
#define REQ_HEIGHT 175
#define TOPAZ_BASELINE 8

#define DRAWER_GADGET_X  60
#define DRAWER_GADGET_Y  106
#define FILE_GADGET_X  DRAWER_GADGET_X
#define FILE_GADGET_Y  (DRAWER_GADGET_Y + 17)
#define PATH_GADGET_WIDTH    160
#define PATH_GADGET_HEIGHT   10
#define STR_LABEL_XOFFSET -60
#define STR_LABEL_YOFFSET 2

#define BUTTON_Y      (FILE_GADGET_Y + 18)
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

enum {
    REQ_OK_BUTTON_ID = 101, REQ_DRIVES_BUTTON_ID, REQ_PARENT_BUTTON_ID,
    REQ_CANCEL_BUTTON_ID, REQ_DIR_TEXT_ID, REQ_FILE_TEXT_ID, REQ_VSLIDER_ID,
    REQ_UP_ID, REQ_DOWN_ID
} GadgetIDs;

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
static WORD list_border_points[10];

/* the -2 is the margin to set to avoid that the string gadget will overdraw the
   borders */
static WORD string_border_points[] =  {-2, -2, PATH_GADGET_WIDTH, -2, PATH_GADGET_WIDTH,
                                       10, -2, 10, -2, -2};

static struct Border ok_button_border = {0, 0, 1, 0, JAM1, 5, ok_border_points, NULL};
static struct Border drives_button_border = {0, 0, 1, 0, JAM1, 5, cancel_border_points, NULL};
static struct Border parent_button_border = {0, 0, 1, 0, JAM1, 5, cancel_border_points, NULL};
static struct Border cancel_button_border = {0, 0, 1, 0, JAM1, 5, cancel_border_points, NULL};
static struct Border str_gadget_border = {0, 0, 1, 0, JAM1, 5, string_border_points, NULL};
static struct Border file_list_border = {0, 0, 1, 0, JAM1, 5, list_border_points, NULL};

static UBYTE buffer1[82], undobuffer1[82];
static struct StringInfo strinfo1 = {buffer1, undobuffer1, 0, 80, 0, 0, 0, 0, 0, 0, NULL, 0, NULL};
static UBYTE buffer2[82], undobuffer2[82];
static struct StringInfo strinfo2 = {buffer2, undobuffer2, 0, 80, 0, 0, 0, 0, 0, 0, NULL, 0, NULL};

static struct Image slider_image;
static struct PropInfo propinfo = {AUTOKNOB | FREEVERT, 0, 0, MAXBODY, MAXBODY,
                                   0, 0, 0, 0, 0, 0};

#define WIN_TITLE "Open File..."

static struct NewWindow newwin = {
  0, 0, 0, REQWIN_HEIGHT, 0, 1,
  IDCMP_GADGETUP | IDCMP_MOUSEBUTTONS,
  WFLG_CLOSEGADGET | WFLG_SMART_REFRESH | WFLG_ACTIVATE | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_NOCAREREFRESH,
  NULL, NULL, WIN_TITLE,
  NULL, NULL,
  0, REQWIN_HEIGHT,
  0, REQWIN_HEIGHT,
  WBENCHSCREEN
};

static struct Gadget list_down = {NULL, 0, 0,
                                  LIST_BUTTON_WIDTH, LIST_BUTTON_HEIGHT,
                                  GFLG_GADGHCOMP | GFLG_GADGIMAGE, GACT_RELVERIFY,
                                  GTYP_BOOLGADGET | GTYP_REQGADGET,
                                  &down_image, NULL,
                                  NULL, 0, NULL,
                                  REQ_DOWN_ID, NULL};

static struct Gadget list_up = {&list_down, 0, 0,
                                LIST_BUTTON_WIDTH, LIST_BUTTON_HEIGHT,
                                GFLG_GADGHCOMP | GFLG_GADGIMAGE, GACT_RELVERIFY,
                                GTYP_BOOLGADGET | GTYP_REQGADGET,
                                &up_image, NULL,
                                NULL, 0, NULL,
                                REQ_UP_ID, NULL};

static struct Gadget list_vslider = {&list_up, 0, 0, LIST_VSLIDER_WIDTH, 0,
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
                                      GFLG_GADGHCOMP | GFLG_DISABLED, GACT_RELVERIFY,
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

int file_index(int mx, int my) {
    int x1 = REQ_HMARGIN;
    int y1 = REQ_VMARGIN;
    int x2 = x1 + filelist_width;
    int y2 = y1 + filelist_height;
    if (mx >= x1 && mx <= x2 && my >= y1 && my <= y2) {
        int rely = my - y1;
        printf("rely: %d\n", rely);
        return 0;
    }
    return -1;
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
            case IDCMP_MOUSEBUTTONS:
                if (msg->Code == SELECTUP) {
                    WORD mx = msg->MouseX, my = msg->MouseY, file_i = file_index(mx, my);
                    if (file_i >= 0) {
                        // TODO
                    }
                }
                ReplyMsg((struct Message *) msg);
                break;
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
                                                  filelist_width,
                                                  filelist_height);
    }
}


void draw_list()
{
    struct FileListEntry *files = scan_dir(NULL), *cur;

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
        if (ypos > filelist_bm_height) break;
    }
    SetDrMd(src_rp, COMPLEMENT);
    // Draw selection rectangle
    RectFill(src_rp, 8, 12, filelist_bm_width - 8, 18);
    free_file_list(files);

    // Done drawing, offscreen bitmap is rendered, copy to the requester's layer
    ClipBlit(src_rp, 0, 0, dst_rp,
            FILE_LIST_BM_X, FILE_LIST_BM_Y, filelist_bm_width, filelist_bm_height,
            0xc0);
}

/* Initialize the requester window and gadget sizes according to the current font and
   screen resolutions. The window parameter is used to determine the font and the parent
   window position.
*/
void init_sizes(struct Window *window, struct Requester *requester)
{
    int scrw = window->WScreen->Width;
    int scrh = window->WScreen->Height;

    // Determine the requester's dimensions
    filelist_bm_width = TextLength(window->RPort, "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWW", 30) + 10;
    filelist_width = filelist_bm_width + 2;

    struct TextFont *font = window->IFont;
    int font_height = font->tf_YSize;
    int font_baseline = font->tf_Baseline;
    filelist_height = font_height * NUM_FILE_ENTRIES + 2 * FILE_LIST_VMARGIN +
        (NUM_FILE_ENTRIES - 1) * FILE_LIST_LINE_DIST;
    filelist_bm_height = filelist_height + 2;

    // controls at the right of the list
    int filelist_vslider_height = filelist_height - 2 * LIST_BUTTON_HEIGHT + 1;
    int list_up_y = filelist_vslider_height;
    int list_down_y = list_up_y + LIST_BUTTON_HEIGHT;

    list_vslider.Height = filelist_vslider_height;
    list_up.TopEdge = list_up_y;
    list_down.TopEdge = list_down_y;
    printf("Font ysize: %d, list height: %d\n", (int) font_height, filelist_height);

    int buttonbar_width = TextLength(window->RPort, "OpenDrivesParentCancel", 22)
        + (BUTTON_HMARGIN * 2 * 4);
    int req_width = filelist_width > buttonbar_width ? filelist_width : buttonbar_width;
    printf("filelist width: %d, button bar width: %d, req_width: %d\n", filelist_width,
           buttonbar_width, req_width);
    req_width += LIST_VSLIDER_WIDTH;

    newwin.MinWidth = newwin.MaxWidth = newwin.Width = req_width + 2 * REQ_HMARGIN;

    // position the requester relative to the parent window
    // TODO: respect screen size
    newwin.LeftEdge = window->LeftEdge + 10;
    newwin.TopEdge = window->TopEdge + 10;

    // adjust border
    list_border_points[2] = filelist_width;
    list_border_points[4] = filelist_width;
    list_border_points[5] = filelist_height;
    list_border_points[7] = filelist_height;

    // adjust gadgets
    list_down.LeftEdge = filelist_width;
    list_up.LeftEdge = filelist_width;
    list_vslider.LeftEdge = filelist_width;

    requester->Width = req_width;
    requester->LeftEdge = REQ_HMARGIN;
    requester->TopEdge = REQ_VMARGIN;
    requester->Height = REQ_HEIGHT;
    requester->Flags = SIMPLEREQ | NOREQBACKFILL | NOISYREQ;
    requester->BackFill = 0;
    requester->ReqGadget = &ok_button;
    requester->ReqBorder = &file_list_border;
    requester->ReqText = NULL;
}

void open_file(struct Window *window)
{
    BOOL result;
    InitRequester(&requester);
    init_sizes(window, &requester);
    if (req_window = OpenWindow(&newwin)) {
        reqwin_font = req_window->IFont;
        font_height = reqwin_font->tf_YSize;
        font_baseline = reqwin_font->tf_Baseline;

        printf("Font ysize: %d, baseline: %d\n", (int) font_height, (int) font_baseline);

        // TODO: we are scrolling by whole lines, this way we can avoid
        // corrupting memory by drawing over the offline bitmap memory
        // we copy the previous content a line up/down and insert the new line
        InitBitMap(&filelist_bitmap, filelist_bm_depth, filelist_bm_width, filelist_bm_height);
        for (int i = 0; i < filelist_bm_depth; i++) filelist_bitmap.Planes[i] = NULL;
        for (int i = 0; i < filelist_bm_depth; i++) {
            if (!(filelist_bitmap.Planes[i] = AllocRaster(filelist_width, filelist_height))) {
                cleanup();
                return;
            } else {
                BltClear(filelist_bitmap.Planes[i],
                         RASSIZE(filelist_width, filelist_height), 1);
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
