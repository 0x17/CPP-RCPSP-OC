//
// Created by André Schnabel on 25.10.15.
//

#include "ScheduleVisualizer.h"
#include <podofo/podofo.h>

using namespace PoDoFo;

void initAttributes(const PdfStreamedDocument &document) {
    document.GetInfo()->SetCreator(PdfString("CPP-RCPSP-OC"));
    document.GetInfo()->SetAuthor(PdfString("Andre Schnabel"));
    document.GetInfo()->SetTitle(PdfString("Schedule"));
    document.GetInfo()->SetSubject(PdfString("Visualized schedule"));
    document.GetInfo()->SetKeywords(PdfString("Schedule;RCPSP;Ablaufplan;"));
}

void ScheduleVisualizer::drawScheduleToPDF(ProjectWithOvertime &p, vector<int> sts, string filename) {
    PdfStreamedDocument document(filename.c_str());
    PdfPage *page;
    PdfPainter painter;
    PdfFont *font;

    page = document.CreatePage(PdfPage::CreateStandardPageSize(ePdfPageSize_A4));
    if (!page) PODOFO_RAISE_ERROR(ePdfError_InvalidHandle);

    painter.SetPage(page);

    font = document.CreateFont("Arial");
    if (!font) PODOFO_RAISE_ERROR(ePdfError_InvalidHandle);

    font->SetFontSize(18.0);
    painter.SetFont(font);
    painter.DrawText(56.69, page->GetPageSize().GetHeight() - 56.69, "Hello World!");

    int origin[] = {50, 50};

    // Draw time axis (x)
    painter.DrawLine(origin[0], origin[1], origin[0]+100, origin[1]);

    // Draw res axis (y)
    painter.DrawLine(origin[0], origin[1], origin[0], origin[1]+100);

    painter.FinishPage();

    initAttributes(document);

    document.Close();
}
