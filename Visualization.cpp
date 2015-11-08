//
// Created by André Schnabel on 25.10.15.
//

#include "Visualization.h"

#define USE_PODOFO

#ifdef USE_PODOFO
#include <podofo/podofo.h>

using namespace PoDoFo;

void initAttributes(const PdfStreamedDocument &document) {
    document.GetInfo()->SetCreator(PdfString("CPP-RCPSP-OC"));
    document.GetInfo()->SetAuthor(PdfString("Andre Schnabel"));
    document.GetInfo()->SetTitle(PdfString("Schedule"));
    document.GetInfo()->SetSubject(PdfString("Visualized schedule"));
    document.GetInfo()->SetKeywords(PdfString("Schedule;RCPSP;Ablaufplan;"));
}

void drawJob(PdfPainter &painter, int j, int stj, int dj, int demand) {
}

void Visualization::drawScheduleToPDF(ProjectWithOvertime &p, vector<int> sts, string filename) {
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

    for(int i=0; i<p.numJobs; i++) {
        drawJob(painter, (i+1), sts[i], p.durations[i], p.demands(i,0));
    }

    painter.FinishPage();

    initAttributes(document);

    document.Close();
}
#endif

string Visualization::activityOnNodeGraphDOT(Project &p) {
    string dotCode = "digraph precedence {\n";
    P_EACH_JOBi(P_EACH_JOB(if(p.adjMx(i,j)) dotCode += to_string(i+1) + "->" + to_string(j+1) + "\n"))
    P_EACH_JOB(
        string demandsStr = "";
        auto jobDemands = p.demands.row(j);
        P_EACH_RES(demandsStr += to_string(jobDemands[r]) + (r == p.numRes-1 ? "" : " "))
        dotCode += to_string(j+1) + "[label=\"(" + to_string(p.durations[j]) + ") " + to_string(j+1) + " (" + demandsStr + ")\"]\n")
    dotCode += "\n}";
    return dotCode;
}

void Visualization::drawActivityOnNodeGraphToPDF(Project &p, string filename) {
    const string tmpfilename = "ActivityOnNodeGraphTEMP.dot";
    Utils::spit(activityOnNodeGraphDOT(p), tmpfilename);
    std::system(("dot -Tpdf " + tmpfilename + " -o" + filename).c_str());
}
