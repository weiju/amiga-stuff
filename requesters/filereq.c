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
#define LIST_BUTTON_WIDTH  11
#define LIST_BUTTON_HEIGHT 11
#define FILE_LIST_BM_MARGIN 1

#define NUM_FILE_ENTRIES 10

/*
  this is the wait time between handling mouse drag
  events of the vslider, 50ms will roughly result in
  20 updates/redraws per second which is fairly smooth
*/
#define VSLIDER_UPDATE_MS 50

// *********************
//  This is essentially the requester's state, should actually be a struct

static int filelist_width;
static int filelist_height;
static int filelist_bm_width;
static int filelist_bm_height;
static struct FileListEntry *current_files;
static int num_current_files;
static int slider_increment;

/*
 * The first visible entry in the file list. Since the
 * number of visible entries is fixed, we only need this one.
 */
static struct FileListEntry *first_visible_entry;

// *********************

enum { LABEL_DRAWER = 0, LABEL_FILE, LABEL_PARENT, LABEL_DRIVES, LABEL_OPEN, LABEL_CANCEL } Labels;

#define REQ_HEIGHT 175
#define REQWIN_HEIGHT (REQ_HEIGHT + 5)
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
    REQ_CANCEL_BUTTON_ID, REQ_DIR_TEXT_ID, REQ_FILE_TEXT_ID, VSLIDER_ID,
    LIST_UP_ID, LIST_DOWN_ID
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

#define DIRSTRING_SIZE 80
#define DIRBUFFER_SIZE (DIRSTRING_SIZE + 2)
static UBYTE buffer1[DIRBUFFER_SIZE], undobuffer1[DIRBUFFER_SIZE];

static struct StringInfo strinfo1 = {buffer1, undobuffer1, 0, DIRSTRING_SIZE, 0, 0, 0, 0, 0, 0,
                                     NULL, 0, NULL};
static UBYTE buffer2[82], undobuffer2[82];
static struct StringInfo strinfo2 = {buffer2, undobuffer2, 0, 80, 0, 0, 0, 0, 0, 0, NULL, 0, NULL};

static struct Image slider_image;
static struct PropInfo propinfo = {AUTOKNOB | FREEVERT, 0, 0, MAXBODY, MAXBODY,
                                   0, 0, 0, 0, 0, 0};

#define WIN_TITLE "Open File..."

static struct NewWindow newwin = {
    0, 0, 0, REQWIN_HEIGHT, 0, 1,
    IDCMP_GADGETUP | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE,
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
                                  LIST_DOWN_ID, NULL};

static struct Gadget list_up = {&list_down, 0, 0,
                                LIST_BUTTON_WIDTH, LIST_BUTTON_HEIGHT,
                                GFLG_GADGHCOMP | GFLG_GADGIMAGE, GACT_RELVERIFY,
                                GTYP_BOOLGADGET | GTYP_REQGADGET,
                                &up_image, NULL,
                                NULL, 0, NULL,
                                LIST_UP_ID, NULL};

static struct Gadget list_vslider = {&list_up, 0, 0, LIST_VSLIDER_WIDTH, 0,
                                     GFLG_GADGHCOMP, GACT_RELVERIFY|GACT_FOLLOWMOUSE, GTYP_PROPGADGET,
                                     &slider_image, NULL, NULL, 0, &propinfo,
                                     VSLIDER_ID, NULL};

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

static UWORD font_baseline, font_height;

static void close_requester()
{
    if (req_opened) {
        EndRequest(&requester, req_window);
        req_opened = FALSE;
    }
}

// for a window coordinate, return the visual index in the file list
int file_list_index(int mx, int my) {
    int x1 = REQ_HMARGIN;
    int y1 = REQ_VMARGIN + FILE_LIST_VMARGIN + FILE_LIST_BM_MARGIN;
    int x2 = x1 + filelist_width;
    int y2 = y1 + filelist_height;
    if (mx >= x1 && mx <= x2 && my >= y1 && my <= y2) {
        int rely = my - y1;
        int index = rely / (font_height + FILE_LIST_LINE_DIST);
        // printf("rely: %d, new sel index: %d\n", rely, index);
        return index < NUM_FILE_ENTRIES ? index : -1;
    }
    return -1;
}

// map file list index to index in the file list
struct FileListEntry *entry_at_list_index(int index)
{
    if (index < 0) return NULL;
    struct FileListEntry *cur = first_visible_entry;
    int count = 0;
    while (cur && count < index) {
        cur = cur->next;
        count++;
    }
    return cur;
}

int vertpot2entry(int vertpot)
{
    int index = vertpot / slider_increment, final_index = index;
    if (index < 0 || num_current_files < NUM_FILE_ENTRIES) final_index = 0;
    else if ((num_current_files - index) < NUM_FILE_ENTRIES) {
        final_index = num_current_files - NUM_FILE_ENTRIES;
    }
    //printf("computed index: %d, final index: %d\n", index, final_index);
    return final_index;
}

static void render_list_backbuffer()
{
    ClipBlit(&filelist_rastport, 0, 0, requester.ReqLayer->rp,
             FILE_LIST_BM_MARGIN, FILE_LIST_BM_MARGIN, filelist_bm_width, filelist_bm_height,
             0xc0);
}

static void draw_selection(struct RastPort *src_rp, int view_index)
{
    // Draw selection rectangle
    SetDrMd(src_rp, COMPLEMENT);
    int y1 = FILE_LIST_VMARGIN + view_index * (font_height + FILE_LIST_LINE_DIST) - FILE_LIST_BM_MARGIN;
    int y2 = y1 + font_height;
    RectFill(src_rp, 0, y1, filelist_bm_width, y2);
}

static void clear_list()
{
    for (int i = 0; i < filelist_bm_depth; i++) {
        BltClear(filelist_bitmap.Planes[i],
                 RASSIZE(filelist_width, filelist_height), 1);
    }
}

static void clear_selections()
{
    struct FileListEntry *cur = current_files;
    while (cur) {
        cur->selected = 0;
        cur = cur->next;
    }
}

/*
 * Render the currently visible portion of the current file list.
 * We can simply clear the whole thing and render the visible
 * entries.
 */
static void draw_list()
{
    // Note that we need to render into the requester
    // layer's rastport, because it is rendered on top of the
    // parent window and obscures the content
    struct RastPort *src_rp = &filelist_rastport;
    struct RastPort *dst_rp = requester.ReqLayer->rp;

    // make sure drawing is clipped, otherwise it will
    // draw somewhere else into memory
    SetAPen(src_rp, 1);
    int ypos = FILE_LIST_VMARGIN + font_baseline;

    struct FileListEntry *cur = first_visible_entry;
    int count = 0;
    while (cur && count < NUM_FILE_ENTRIES) {
        Move(src_rp, 8, ypos);
        Text(src_rp, cur->name, strlen(cur->name));
        ypos += font_height + FILE_LIST_LINE_DIST;
        if (cur->selected) draw_selection(src_rp, count);
        cur = cur->next;
        count++;
    }
    // Done drawing, offscreen bitmap is rendered, copy to the requester's layer
    render_list_backbuffer();
}

static void update_list(int new_first_index)
{
    struct FileListEntry *cur = first_visible_entry;
    int current_index = cur->index;
    if (new_first_index < current_index) {
        while (cur->index > new_first_index) cur = cur->prev;
    } else if (new_first_index > current_index) {
        while (cur->index < new_first_index) cur = cur->next;
    }
    first_visible_entry = cur;
    clear_list();
    draw_list();
}

static void update_string_gadgets(struct FileListEntry *entry)
{
    // set the name to the gadget and refresh
    RemoveGList(req_window, &dir_text, 1);
    strncpy(buffer1, entry->name, DIRBUFFER_SIZE);
    AddGList(req_window, &dir_text, 0, 1, &requester);
    RefreshGList(&dir_text, req_window, &requester, 1);
}

static void reload_file_list(const char *dirpath)
{
    int num_files;
    struct FileListEntry *dir_entries = scan_dir(dirpath, &num_files);
    if (current_files) free_file_list(current_files);
    current_files = dir_entries;
    num_current_files = num_files;
    first_visible_entry = current_files;
    clear_list();
    draw_list();

    // set the length of the slider thumb according to the current file list
    int vertbody = MAXBODY;
    int vertpot = MAXBODY;
    if (num_current_files > NUM_FILE_ENTRIES) {
        // because we can only use integer division, so we have to determine
        // the body size by solving the equation
        // MAXBODY / vertbody = num_current_files / NUM_FILE_ENTRIES
        vertbody = (MAXBODY * NUM_FILE_ENTRIES) / num_current_files;

        // this is surprisingly different that I thought:
        // we need to compute the increment over the drag space not over
        // the vslider space, so we can ignore the VertBody setting (in short
        // VertPot and VertBody are independent)
        slider_increment = MAXBODY / (num_current_files - NUM_FILE_ENTRIES);
        vertpot = first_visible_entry->index * slider_increment;
        printf("vpot: %d, vbody: %d inc: %d\n", vertpot, vertbody, slider_increment);
    }
    NewModifyProp(&list_vslider, req_window, &requester, AUTOKNOB | FREEVERT,
                  0, vertpot, MAXBODY, vertbody, 1);
}

static void handle_events()
{
    BOOL done = FALSE;
    struct IntuiMessage *msg;
    ULONG msgClass;
    UWORD menuCode;
    int buttonId;
    ULONG last_scroll_seconds = 0, last_scroll_micros = 0, scroll_seconds, scroll_micros;
    ULONG start_click_secs = 0, start_click_micros = 0, click_secs, click_micros;
    int idx;

    while (!done) {
        Wait(1 << req_window->UserPort->mp_SigBit);
        // since we expect mouse move operations, we need to process all events until
        // the message queue is empty, otherwise we'll get funny effects by processing
        // the queued up mouse move events when we actually were notified about a different
        // event
        while (msg = (struct IntuiMessage *) GetMsg(req_window->UserPort)) {
            msgClass = msg->Class;
            switch (msgClass) {
            case IDCMP_MOUSEMOVE:
                if (last_scroll_seconds == 0) {
                    CurrentTime(&last_scroll_seconds, &last_scroll_micros);
                } else {
                    CurrentTime(&scroll_seconds, &scroll_micros);
                    ULONG diff = (scroll_seconds - last_scroll_seconds) * 1000 +
                        (scroll_micros - last_scroll_micros) / 1000;
                    if (diff > VSLIDER_UPDATE_MS) {
                        // update the list, but ignore most of the move events,
                        // otherwise the we need to  process too many events and
                        // refresh too often
                        idx = vertpot2entry(propinfo.VertPot);
                        last_scroll_seconds = scroll_seconds;
                        last_scroll_micros = scroll_micros;
                        update_list(idx);
                    }
                }
                ReplyMsg((struct Message *) msg);
                break;
            case IDCMP_MOUSEBUTTONS:
                if (msg->Code == SELECTUP) {
                    BOOL is_doubleclick = FALSE;
                    // map to virtual file list indexes, only select if not already selected
                    CurrentTime(&click_secs, &click_micros);
                    if (start_click_secs == 0) {
                        start_click_secs = click_secs;
                        start_click_micros = click_micros;
                    } else {
                        is_doubleclick = DoubleClick(start_click_secs, start_click_micros,
                                                     click_secs, click_micros);
                        start_click_secs = start_click_micros = 0;
                    }
                    struct FileListEntry *entry = entry_at_list_index(file_list_index(msg->MouseX,
                                                                                      msg->MouseY));
                    if (entry != NULL && !entry->selected) {
                        clear_selections();
                        //printf("select '%s', index: %d\n", entry->name, (int) entry->index);
                        entry->selected = 1;
                        clear_list();
                        draw_list();
                        update_string_gadgets(entry);
                    } else if (entry != NULL && is_doubleclick) {
                        printf("double click on: %s\n", entry->name);
                        if (entry->file_type == FILETYPE_VOLUME || entry->file_type == FILETYPE_DIR) {
                            reload_file_list(entry->name);
                        }
                    }
                }
                ReplyMsg((struct Message *) msg);
                break;
            case IDCMP_GADGETUP:
                buttonId = (int) ((struct Gadget *) (msg->IAddress))->GadgetID;
                ReplyMsg((struct Message *) msg);
                switch (buttonId) {
                case REQ_OK_BUTTON_ID:
                    close_requester();
                    done = TRUE;
                    break;
                case REQ_CANCEL_BUTTON_ID:
                    close_requester();
                    done = TRUE;
                    break;
                case LIST_UP_ID:
                    {
                        int newpot = propinfo.VertPot - slider_increment;
                        if (newpot < 0) newpot = 0;
                        NewModifyProp(&list_vslider, req_window, &requester, AUTOKNOB | FREEVERT,
                                      0, newpot, MAXBODY, propinfo.VertBody, 1);
                        int idx = vertpot2entry(newpot);
                        update_list(idx);
                    }
                    break;
                case LIST_DOWN_ID:
                    {
                        int newpot = propinfo.VertPot + slider_increment;
                        if (newpot > MAXBODY) newpot = MAXBODY;
                        NewModifyProp(&list_vslider, req_window, &requester, AUTOKNOB | FREEVERT,
                                      0, newpot, MAXBODY, propinfo.VertBody, 1);
                        int idx = vertpot2entry(newpot);
                        update_list(idx);
                    }
                    break;
                case VSLIDER_ID:
                    // determine the portion to be displayed
                    idx = vertpot2entry(propinfo.VertPot);
                    //printf("gadget up, vslider, vertpot: %d, incr: %d, idx: %d\n",
                    //       (int) propinfo.VertPot, slider_increment, idx);
                    last_scroll_seconds = last_scroll_micros = 0;
                    update_list(idx);
                    break;
                default:
                    break;
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
    font_height = font->tf_YSize;
    font_baseline = font->tf_Baseline;
    filelist_bm_height = font_height * NUM_FILE_ENTRIES + 2 * FILE_LIST_VMARGIN +
        (NUM_FILE_ENTRIES - 1) * FILE_LIST_LINE_DIST;
    filelist_height = filelist_bm_height + (FILE_LIST_BM_MARGIN * 2);

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
    InitRequester(&requester);
    init_sizes(window, &requester);
    if (req_window = OpenWindow(&newwin)) {
        InitBitMap(&filelist_bitmap, filelist_bm_depth, filelist_bm_width, filelist_bm_height);
        for (int i = 0; i < filelist_bm_depth; i++) filelist_bitmap.Planes[i] = NULL;
        for (int i = 0; i < filelist_bm_depth; i++) {
            if (!(filelist_bitmap.Planes[i] = AllocRaster(filelist_width, filelist_height))) {
                cleanup();
                return;
            } else {
                clear_list();
            }
        }
        InitRastPort(&filelist_rastport);
        filelist_rastport.BitMap = &filelist_bitmap;

        if (req_opened = Request(&requester, req_window)) {
            reload_file_list(NULL);
            handle_events();
            free_file_list(current_files);
            current_files = first_visible_entry = NULL;
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
