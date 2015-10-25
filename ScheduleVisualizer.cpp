//
// Created by André Schnabel on 25.10.15.
//

#include "ScheduleVisualizer.h"
#include <podofo/podofo.h>

using namespace PoDoFo;

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
    painter.FinishPage();

    document.GetInfo()->SetCreator(PdfString("CPP-RCPSP-OC"));
    document.GetInfo()->SetAuthor(PdfString("Andre Schnabel"));
    document.GetInfo()->SetTitle(PdfString("Schedule"));
    document.GetInfo()->SetSubject(PdfString("Visualized schedule"));
    document.GetInfo()->SetKeywords(PdfString("Schedule;RCPSP;Ablaufplan;"));

    document.Close();
}
