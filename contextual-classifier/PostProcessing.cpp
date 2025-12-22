// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

struct WLPostprocessCBData {
    std::string mCmdline;
    uint32_t mSigId;
    uint32_t mSigSubtype
};

enum USECASE {
    UNDETERMINED,
    DECODE,
    ENCODE_720,
    ENCODE_1080,
    ENCODE_2160,
    ENCODE_MANY,
    ENCODE_DECODE,
};

enum USECASE FetchUsecaseDetails(char *buf, size_t sz) {
    /* What are possibilities for cmdline
     * For encoder, width of encoding, v4l2h264enc in line
     * For decoder, v4l2h264dec, or may be 265 as well, decoder bit
     * width determination is difficult as that can change runtime as
     * the format may have variable width per frame.
     * For snapshot and preview, need to check usecases, what should be
     * added, gst-pipeline-app or something
     */
    printf("find_usecase\n");
    int encode = 0, decode = 0, height = 0;
    char *e = buf, *h = buf;
    const char *e_str = "v4l2h264enc";
    size_t e_str_sz = strlen(e_str);
    const char *h_str = "height=";
    size_t h_str_sz = strlen(h_str);

    while ((e = strstr(e, e_str)) != NULL) {
        e += e_str_sz;
        encode += 1;
        h = strstr(h, h_str);
        height = atoi(h + h_str_sz);
        h += h_str_sz;
        printf("encode = %d, height = %d\n", encode, height);
    }

    const char *d_str = "v4l2h264dec";
    char *d = buf;
    size_t d_str_sz = strlen(d_str);
    while ((d = strstr(d, d_str)) != NULL) {
        d += d_str_sz;
        decode += 1;
        printf("decode = %d\n", decode);
    }
    /*Preview case*/
    if (encode == 0 && decode == 0) {
        const char *d_str = "qtiqmmfsrc";
        char *d = buf;
        size_t d_str_sz = strlen(d_str);
        while ((d = strstr(d, d_str)) != NULL) {
            d += d_str_sz;
            encode += 1;
            printf("Preview: encode = %d\n", encode);
        }
    }
    enum USECASE u = UNDETERMINED;
    if (decode > 0)
        u = DECODE;
    // CC_VIDEO_DECODE
    if (encode > 1)
        u = ENCODE_MANY;
    // CC_CAMERA_ENCODE_MULTI_STREAMS
    else if (encode == 1) {
        if (height <= 720)
            u = ENCODE_720;
        ////CC_CAMERA_ENCODE
        else if (height <= 1080)
            u = ENCODE_1080;
        else
            u = ENCODE_2160;
    }

    if (encode > 0 && decode > 0)
        u = ENCODE_DECODE;
    return u;
}

int WorkloadPostprocessCallback(void *cbData) {
    WLPostprocessCBData *cbdata = static_cast<WLPostprocessCBData *>(cbData);
    if (cbdata == NULL) {
        return;
    }
}

URM_REGISTER_WORKLOAD_POSTPROCESS_CB("gst-launch-", WorkloadPostprocessCallback)
