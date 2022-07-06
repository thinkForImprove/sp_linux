#pragma once

#include "QtTypeDef.h"
#include "XFSPTR.H"


class CWFSPTRSTATUS : public WFSPTRSTATUS
{
public:
    CWFSPTRSTATUS()
    {
        fwDevice    = WFS_PTR_DEVONLINE;
        fwMedia     = WFS_PTR_MEDIANOTPRESENT;
        fwPaper     = nullptr;
        lpszExtra   = nullptr;
        //fwToner     =
    }
};


class CWFSPTRCap : public WFSPTRCAPS
{
public:
    CWFSPTRCap()
    {
        BOOL bJournal = FALSE;//m_pPrinter->IsJournalPrinter();

        dev_class           = WFS_SERVICE_CLASS_PTR;
        dev_type            = bJournal ? WFS_PTR_TYPEJOURNAL : WFS_PTR_TYPERECEIPT;
        compound            = FALSE;
        read_form         = 0;
        write_form      = WFS_PTR_WRITETEXT | (bJournal ? 0 : WFS_PTR_WRITEBARCODE);
        extents         = 0;
        control         = (bJournal ? 0 : (WFS_PTR_CTRLCUT | WFS_PTR_CTRLEJECT)) | WFS_PTR_CTRLFLUSH;
        max_media_on_stacker = 0;
        accept_media        = FALSE;
        media_taken     = bJournal ? FALSE : TRUE;
        multi_page      = FALSE;
        paper_sources     = WFS_PTR_PAPERUPPER;
        retract_bins        = 0;
        max_retract       = nullptr;
        image_type      = 0;
        front_image_color_format = 0;
        back_image_color_format = 0;
        codeline_format       = 0;
        image_source              = 0;
        char_support              = WFS_PTR_UTF8;
        dispense_paper        = FALSE;
        resolution            = WFS_PTR_RESHIGH;
        extra                   = (LPSTR)("\0\0");
    }
};
