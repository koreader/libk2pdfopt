int tess_capi_get_word_boxes(void *vapi, PIX *pix, BOXA **out_boxa, int is_cjk, FILE *out)
{
    if (vapi==NULL)
        return -1;

    tesseract::TessBaseAPI *api;
    api=(tesseract::TessBaseAPI *)vapi;
    try {
        api->InitForAnalysePage();
        api->SetPageSegMode(tesseract::PSM_AUTO);
        api->SetImage(pix);
        if (is_cjk) {
            api->SetVariable("textord_use_cjk_fp_model","1");
            *out_boxa = api->GetConnectedComponents(NULL);
        } else {
            api->SetVariable("textord_use_cjk_fp_model","0");
            *out_boxa = api->GetWords(NULL);
        }
    } catch (const std::exception &e) {
       if (out!=NULL)
           fprintf(out,"tesscapi:  Error during page segmentation. %s\n", e.what());
       api->Clear();
       return -1;
    }
    api->ClearAdaptiveClassifier();
    api->Clear();
    return(0);
}
