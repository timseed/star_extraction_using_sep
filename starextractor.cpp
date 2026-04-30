#include "starextractor.h"

void StarExtractor::processFits(QString filePath) {
    fitsfile *fptr;
    int status = 0;

    if (fits_open_file(&fptr, filePath.toUtf8().constData(), READONLY, &status)) {
        fits_report_error(stderr, status);
        return;
    }

    int bitpix, naxis;
    long naxes[2] = {0, 0};
    fits_get_img_param(fptr, 2, &bitpix, &naxis, naxes, &status);

    long nx = naxes[0];
    long ny = naxes[1];
    qInfo() << "File has a width of " << nx << " and height of " << ny;
    long npixels = nx * ny;

    QVector<float> imageData(npixels);
    int anynul;

    fits_read_img(fptr, TFLOAT, 1, npixels, nullptr, imageData.data(), &anynul,
                  &status);
    fits_close_file(fptr, &status);

    if (status)
        return;

    // --- NEW: Construct the sep_image struct ---
    int dims[2] = {static_cast<int>(nx), static_cast<int>(ny)};

    sep_image im;
    // Zero out the struct so optional pointers (noise, mask, etc.) default to
    // null
    std::memset(&im, 0, sizeof(sep_image));

    im.data = imageData.data();
    im.dtype = SEP_TFLOAT;

    // im.ndim = 2;
    // im.dims = dims;

    // -------------------------------------------

    // a. Background estimation using sep_image
    sep_bkg *bkg = nullptr;
    int sep_status =
        sep_background(&im,    // Pass the address of our configured sep_image
                                    64, 64, // Background box width and height
                                    3, 3,   // Filter width and height
                                    0.0,    // Filter threshold
                                    &bkg);

    if (sep_status != 0) {
        qDebug() << "SEP background failed:" << sep_status;
        return;
    }

    float bkgrms = bkg->globalrms;

    // Subtract background globally (modifies the data inside our QVector)
    sep_bkg_subarray(bkg, imageData.data(), SEP_TFLOAT);

    // b. Extract sources using sep_image
    sep_catalog *catalog = nullptr;


    // Parameters:
    // 1.  im: the sep_image struct
    // 2.  thresh: detection threshold
    // 3.  minarea: min pixels for a source
    // 4.  filter: convolution filter (nullptr for none)
    // 5.  filter_type: 0 for flat, 1 for matched
    // 6.  deblend_nthresh: deblending thresholds
    // 7.  deblend_cont: deblending contrast
    // 8.  clean_flag: perform cleaning (1 = yes)
    // 9.  clean_param: cleaning parameter
    // 10. noise_type: SEP_NOISE_STDDEV or SEP_NOISE_VAR (0 if using internal RMS)
    // 11. noise: pointer to noise/variance array (nullptr to use globalrms from
    // bkg)
    // 12. gain: for Poisson noise (0.0 if not used)
    // 13. segmap: pointer to int array for segmentation map (nullptr if not
    // needed) Note: sep_extract also updated its signature in newer versions
    int min_area_pixels=5;
    double detect_threshold = 3.0;
    int filter_type=SEP_FILTER_MATCHED;   // Matched as we are not using CONV
    int deblend_threshold=32;
    double deblend_cont=0.005;
    int clean_flag=1;
    double clean_param=1.0;

    sep_status = sep_extract(
        &im,               // 1
        detect_threshold,
        SEP_THRESH_REL,   // Check Units of Standard Deviation
        min_area_pixels,                 // 3
        nullptr,           // 4 conv_array
        0,0,        //Conv height and width
        filter_type,
        deblend_threshold,
        deblend_cont,
        clean_flag,
        clean_param,
        &catalog
        );

    if (sep_status == 0 && catalog != nullptr) {
        int limit = qMin(catalog->nobj, 10);
        for (int i = 0; i < limit; ++i) {
            qDebug().nospace() << "Star " << i + 1 << ": "
                               << "X=" << catalog->x[i] << ", "
                               << "Y=" << catalog->y[i] << ", "
                               << "Flux=" << catalog->flux[i];
        }
    }

    if (catalog)
        sep_catalog_free(catalog);
    if (bkg)
        sep_bkg_free(bkg);
}
