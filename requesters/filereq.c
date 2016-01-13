#include <intuition/intuition.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_stdio_protos.h>

#include "filereq.h"

#define REQ_TEXT_XOFFSET 10
#define REQ_WIDTH 240
#define REQ_HEIGHT 175
#define TOPAZ_BASELINE 8

#define BUTTON_Y      135
#define BUTTON_HEIGHT 18

#define BUTTON_TEXT_XOFFSET 14
#define OK_BUTTON_X          20
#define CANCEL_BUTTON_X      160
#define OK_BUTTON_WIDTH      40
#define CANCEL_BUTTON_WIDTH  60

#define STR_GADGET_X  90
#define STR_GADGET_Y  120
#define PATH_GADGET_WIDTH    100

static struct Requester requester;
static BOOL req_opened = FALSE;
static struct Window *req_window;

static struct IntuiText labels[] = {
    {1, 0, JAM2, REQ_TEXT_XOFFSET, STR_GADGET_Y, NULL, "Enter file path", NULL},
    {1, 0, JAM2, 10, TOPAZ_BASELINE - 4, NULL, "Ok", NULL},
    {1, 0, JAM2, 10, TOPAZ_BASELINE - 4, NULL, "Cancel", NULL} /* TOPAZ_BASELINE is 8 */
};

static WORD gadget_border_points[3][10] = {
    {0, 0, OK_BUTTON_WIDTH, 0, OK_BUTTON_WIDTH, BUTTON_HEIGHT, 0, BUTTON_HEIGHT, 0, 0},
    {0, 0, CANCEL_BUTTON_WIDTH, 0, CANCEL_BUTTON_WIDTH, BUTTON_HEIGHT, 0, BUTTON_HEIGHT, 0, 0},
    /* the -2 is the margin to set to avoid that the string gadget will overdraw the
     borders */
    {-2, -2, PATH_GADGET_WIDTH, -2, PATH_GADGET_WIDTH, 10, -2, 10, -2, -2}
};

static struct Border gadget_borders[] = {
    {0, 0, 1, 0, JAM1, 5, gadget_border_points[0], NULL},
    {0, 0, 1, 0, JAM1, 5, gadget_border_points[1], NULL},
    {0, 0, 1, 0, JAM1, 5, gadget_border_points[2], NULL}
};

static UBYTE buffer[81], undobuffer[81];
static struct StringInfo strinfo = {buffer, undobuffer, 0, 80, 0, 0, 0, 0, 0, 0, NULL, 0, NULL};

#define REQWIN_WIDTH 260
#define REQWIN_HEIGHT 180
#define WIN_TITLE "Open File..."

static struct NewWindow newwin = {
  0, 0, REQWIN_WIDTH, REQWIN_HEIGHT, 0, 1,
  IDCMP_CLOSEWINDOW | IDCMP_GADGETUP,
  WFLG_CLOSEGADGET | WFLG_SMART_REFRESH | WFLG_ACTIVATE | WFLG_DRAGBAR | WFLG_DEPTHGADGET,
  NULL, NULL, WIN_TITLE,
  NULL, NULL,
  REQWIN_WIDTH, REQWIN_HEIGHT,
  REQWIN_WIDTH, REQWIN_HEIGHT,
  WBENCHSCREEN
};

/*
  Note: Cancel does not specify the GACT_ENDGADGET flag, it seems that
  IDCMP_REQCLEAR is not fired when Intuition closes the requester automatically
*/
static struct Gadget gadgets[] = {
    {&gadgets[1], OK_BUTTON_X, BUTTON_Y, OK_BUTTON_WIDTH, BUTTON_HEIGHT, GFLG_GADGHCOMP,
     GACT_RELVERIFY, GTYP_BOOLGADGET | GTYP_REQGADGET, &gadget_borders[0], NULL,
     &labels[1], 0, NULL, REQ_OK_BUTTON_ID, NULL},
    {&gadgets[2], CANCEL_BUTTON_X, BUTTON_Y, CANCEL_BUTTON_WIDTH, BUTTON_HEIGHT, GFLG_GADGHCOMP,
     GACT_RELVERIFY, GTYP_BOOLGADGET | GTYP_REQGADGET, &gadget_borders[1], NULL,
     &labels[2], 0, NULL, REQ_CANCEL_BUTTON_ID, NULL},
    {NULL, STR_GADGET_X, STR_GADGET_Y, PATH_GADGET_WIDTH, 10,
     GFLG_GADGHCOMP, GACT_RELVERIFY, GTYP_STRGADGET, &gadget_borders[2], NULL,
     &labels[3], 0, &strinfo, 103, NULL},
};

#define PATHBUFFER_SIZE 200
static char dirname[PATHBUFFER_SIZE + 1];
static BPTR flock;
static LONG error;
static struct FileInfoBlock fileinfo;

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
    puts("close_requester()");
    if (req_opened) {
        puts("EndRequest()");
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
            case IDCMP_CLOSEWINDOW:
                close_requester();
                done = TRUE;
                break;
            case IDCMP_GADGETUP:
                buttonId = (int) ((struct Gadget *) (msg->IAddress))->GadgetID;
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
            ReplyMsg((struct Message *) msg);
        }
    }
}

void open_file(struct Window *window)
{
    BOOL result;
    if (req_window = OpenWindow(&newwin)) {
        InitRequester(&requester);
        requester.LeftEdge = 20;
        requester.TopEdge = 20;
        requester.Width = REQ_WIDTH;
        requester.Height = REQ_HEIGHT;
        requester.Flags = 0;
        requester.BackFill = 0;
        requester.ReqGadget = &gadgets[0];
        requester.ReqBorder = NULL; // &req_border;
        requester.ReqText = &labels[0];

        /* scan current directory */
        /*
          on AmigaOS versions before 36 (essentially all 1.x versions), the
          function GetCurrentDirName() does not exist, but it's obtainable
          by calling Cli() and querying the returned CommandLineInterface
          structure
        */
        //puts("scanning directory...");
        /*
        // on AmigaOS 1.x, this function does not exist !!!
        GetCurrentDirName(dirname, PATHBUFFER_SIZE);
        printf("current dir: '%s'\n", dirname);
        flock = Lock(dirname, SHARED_LOCK);
        if (Examine(flock, &fileinfo)) {
        while (ExNext(flock, &fileinfo)) {
        print_fileinfo(&fileinfo);
        }
        error = IoErr();
        if (error != ERROR_NO_MORE_ENTRIES) {
        puts("unknown I/O error, TODO handle");
        }
        }
        UnLock(flock);
        */
        if (req_opened = Request(&requester, req_window)) {
            puts("handle requester events...");
            handle_events();
            puts("quit handling requester events");
            CloseWindow(req_window);
            req_window = NULL;
        } else {
            puts("Request() failed !!!");
        }
    } else {
        puts("OpenWindow() failed !!!");
    }
}
