#include <intuition/intuition.h>
#include <clib/intuition_protos.h>

#ifdef __VBCC__
#include <clib/alib_stdio_protos.h>
#endif

#include "filereq.h"

#define REQ_TEXT_XOFFSET 10
#define BUTTON_TEXT_XOFFSET 14
#define OK_BUTTON_WIDTH  40
#define OK_BUTTON_HEIGHT 24
#define REQ_WIDTH 200
#define REQ_HEIGHT 100
#define TOPAZ_BASELINE 8


struct Requester requester;

struct Requester *open_file(struct Window *window)
{
    /*
     * note that these are setup statically, otherwise they are lost as soon as
     * the functions exits
     */
    static struct IntuiText labels[] = {
        {1, 0, JAM2, REQ_TEXT_XOFFSET, TOPAZ_BASELINE, NULL, "Enter file path", NULL},
        {1, 0, JAM2, 10, TOPAZ_BASELINE, NULL, "Ok", NULL},
        {1, 0, JAM2, 50, TOPAZ_BASELINE, NULL, "Cancel", NULL} /* TOPAZ_BASELINE is 8 */
    };

    static WORD button_border_points[] = {
        0, 0, OK_BUTTON_WIDTH, 0, OK_BUTTON_WIDTH, OK_BUTTON_HEIGHT, 0, OK_BUTTON_HEIGHT, 0, 0
    };
    static struct Border button_border = {0, 0, 1, 0, JAM1, 5, button_border_points, NULL};

    static WORD req_border_points[] = {
        0, 0, REQ_WIDTH - 1, 0, REQ_WIDTH - 1, REQ_HEIGHT - 1, 0, REQ_HEIGHT - 1, 0, 0
    };
    static struct Border req_border = {0, 0, 1, 0, JAM1, 5, req_border_points, NULL};
    BOOL result;

    static struct Gadget button1 = {
        NULL, 20, 50, OK_BUTTON_WIDTH, OK_BUTTON_HEIGHT, GFLG_GADGHCOMP,
        GACT_RELVERIFY,
        /* GACT_RELVERIFY | GACT_ENDGADGET, */
        GTYP_BOOLGADGET | GTYP_REQGADGET,
        &button_border, NULL, &labels[1],
        0, NULL, REQ_OK_BUTTON_ID, NULL
    };

    InitRequester(&requester);
    requester.LeftEdge = 20;
    requester.TopEdge = 20;
    requester.Width = REQ_WIDTH;
    requester.Height = REQ_HEIGHT;
    requester.Flags = 0;
    requester.BackFill = 0;
    requester.ReqGadget = &button1;
    requester.ReqBorder = &req_border;
    requester.ReqText = &labels[0];

    result = Request(&requester, window);
    if (result) {
        puts("Requester could be opened");
        return &requester;
    }
    else {
        puts("Requester could not be opened");
        return NULL;
    }
}
