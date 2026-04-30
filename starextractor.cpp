#include "starextractor.h"

StarExtractor::StarExtractor(QObject *parent)
{
    setBackground_box(64);
    setBackground_filter(3);
    setBackgroup_filter_threshold(0.0);
    setMin_area_pixels(5);
    setDetect_threshold(0.0);
    setDeblend_threshold(32);
    setDeblend_cont(0.005);
    setClean_flag(1);
    setClean_param(1.0);
}

int StarExtractor::loadFits(QString filePath)
{
    fitsfile *fptr;
    int status = 0;

    if (fits_open_file(&fptr, filePath.toUtf8().constData(), READONLY, &status)) {
        fits_report_error(stderr, status);
        return 1;
    }

    int bitpix, naxis;
    long naxes[2] = {0, 0};
    fits_get_img_param(fptr, 2, &bitpix, &naxis, naxes, &status);
    setNx(naxes[1]);
    setNy(naxes[0]);

    qInfo() << "File has a width of " << getNx() << " and height of " << getNy();
    long npixels = getNx() * getNy();


    int anynul;
    QVector<float> tmp_img_data(npixels);
    fits_read_img(fptr, TFLOAT, 1, npixels, nullptr, tmp_img_data.data(), &anynul,
                  &status);
    setImageData(tmp_img_data);
    fits_close_file(fptr, &status);

    return 0;   // All ok
}

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
    setNx(naxes[1]);
    setNy(naxes[0]);

    qInfo() << "File has a width of " << getNx() << " and height of " << getNy();
    long npixels = getNx() * getNy();

    QVector<float> imageData(npixels);
    int anynul;

    fits_read_img(fptr, TFLOAT, 1, npixels, nullptr, imageData.data(), &anynul,
                  &status);
    fits_close_file(fptr, &status);

    if (status)
        return;

    // --- NEW: Construct the sep_image struct ---
    int dims[2] = {static_cast<int>(getNx()), static_cast<int>(getNy())};

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
                                    getBackground_box(), getBackground_box(), // Background box width and height
                                    getBackground_filter(), getBackground_filter(),   // Filter width and height
                                    getBackgroup_filter_threshold(),    // Filter threshold
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

    double detect_threshold = 3.0;
    int filter_type=SEP_FILTER_MATCHED;   // Matched as we are not using CONV



    double clean_param=1.0;

    sep_status = sep_extract(
        &im,               // 1
        getDetect_threshold(),
        SEP_THRESH_REL,   // Check Units of Standard Deviation
        getMin_area_pixels(),                 // 3
        nullptr,           // 4 conv_array
        0,0,        //Conv height and width
        filter_type,
        getDeblend_threshold(),
        getDeblend_cont(),
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

long StarExtractor::getNx() const
{
    return nx;
}

void StarExtractor::setNx(long newNx)
{
    nx = newNx;
}

long StarExtractor::getNy() const
{
    return ny;
}

void StarExtractor::setNy(long newNy)
{
    ny = newNy;
}

int StarExtractor::getBackground_box() const
{
    return background_box;
}

void StarExtractor::setBackground_box(int newBackground_box)
{
    background_box = newBackground_box;
}

int StarExtractor::getBackground_filter() const
{
    return background_filter;
}

void StarExtractor::setBackground_filter(int newBackground_filter)
{
    background_filter = newBackground_filter;
}

double StarExtractor::getBackgroup_filter_threshold() const
{
    return backgroup_filter_threshold;
}

void StarExtractor::setBackgroup_filter_threshold(double newBackgroup_filter_threshold)
{
    backgroup_filter_threshold = newBackgroup_filter_threshold;
}

int StarExtractor::getMin_area_pixels() const
{
    return min_area_pixels;
}

void StarExtractor::setMin_area_pixels(int newMin_area_pixels)
{
    min_area_pixels = newMin_area_pixels;
}

double StarExtractor::getDetect_threshold() const
{
    return detect_threshold;
}

void StarExtractor::setDetect_threshold(double newDetect_threshold)
{
    detect_threshold = newDetect_threshold;
}

int StarExtractor::getDeblend_threshold() const
{
    return deblend_threshold;
}

void StarExtractor::setDeblend_threshold(int newDeblend_threshold)
{
    deblend_threshold = newDeblend_threshold;
}

double StarExtractor::getDeblend_cont() const
{
    return deblend_cont;
}

void StarExtractor::setDeblend_cont(double newDeblend_cont)
{
    deblend_cont = newDeblend_cont;
}

int StarExtractor::getClean_flag() const
{
    return clean_flag;
}

void StarExtractor::setClean_flag(int newClean_flag)
{
    clean_flag = newClean_flag;
}

double StarExtractor::getClean_param() const
{
    return clean_param;
}

void StarExtractor::setClean_param(double newClean_param)
{
    clean_param = newClean_param;
}

QVector<float> StarExtractor::getImageData() const
{
    return imageData;
}

void StarExtractor::setImageData(const QVector<float> &newImageData)
{
    qInfo() << "Storing ImageData size " << newImageData.size();
    imageData = newImageData;
}
