#include<stdbool.h>
#include<stdio.h>
#include<math.h>

#define  NUMBER_IS_2_POW_K(x)   ((!((x)&((x)-1)))&&((x)>1))  // x is pow(2, k), k=1,2, ...
#define  FT_DIRECT        -1    // Direct transform.
#define  FT_INVERSE        1    // Inverse transform.

bool FFT(float *Rdat, float *Idat, int N, int LogN, int Ft_Flag) {
    // parameters error check:
    if ((Rdat == NULL) || (Idat == NULL)) return false;
    if ((N > 16384) || (N < 1)) return false;
    if (!NUMBER_IS_2_POW_K(N)) return false;
    if ((LogN < 2) || (LogN > 14)) return false;
    if ((Ft_Flag != FT_DIRECT) && (Ft_Flag != FT_INVERSE)) return false;

    register int i, j, n, k, io, ie, in, nn;
    float ru, iu, rtp, itp, rtq, itq, rw, iw, sr;

    static const float Rcoef[14] =
            {-1.0000000000000000F, 0.0000000000000000F, 0.7071067811865475F,
             0.9238795325112867F, 0.9807852804032304F, 0.9951847266721969F,
             0.9987954562051724F, 0.9996988186962042F, 0.9999247018391445F,
             0.9999811752826011F, 0.9999952938095761F, 0.9999988234517018F,
             0.9999997058628822F, 0.9999999264657178F
            };
    static const float Icoef[14] =
            {0.0000000000000000F, -1.0000000000000000F, -0.7071067811865474F,
             -0.3826834323650897F, -0.1950903220161282F, -0.0980171403295606F,
             -0.0490676743274180F, -0.0245412285229122F, -0.0122715382857199F,
             -0.0061358846491544F, -0.0030679567629659F, -0.0015339801862847F,
             -0.0007669903187427F, -0.0003834951875714F
            };

    nn = N >> 1;
    ie = N;
    for (n = 1; n <= LogN; n++) {
        rw = Rcoef[LogN - n];
        iw = Icoef[LogN - n];
        if (Ft_Flag == FT_INVERSE) iw = -iw;
        in = ie >> 1;
        ru = 1.0F;
        iu = 0.0F;
        for (j = 0; j < in; j++) {
            for (i = j; i < N; i += ie) {
                io = i + in;
                rtp = Rdat[i] + Rdat[io];
                itp = Idat[i] + Idat[io];
                rtq = Rdat[i] - Rdat[io];
                itq = Idat[i] - Idat[io];
                Rdat[io] = rtq * ru - itq * iu;
                Idat[io] = itq * ru + rtq * iu;
                Rdat[i] = rtp;
                Idat[i] = itp;
            }

            sr = ru;
            ru = ru * rw - iu * iw;
            iu = iu * rw + sr * iw;
        }

        ie >>= 1;
    }

    for (j = i = 1; i < N; i++) {
        if (i < j) {
            io = i - 1;
            in = j - 1;
            rtp = Rdat[in];
            itp = Idat[in];
            Rdat[in] = Rdat[io];
            Idat[in] = Idat[io];
            Rdat[io] = rtp;
            Idat[io] = itp;
        }

        k = nn;

        while (k < j) {
            j = j - k;
            k >>= 1;
        }

        j = j + k;
    }

    if (Ft_Flag == FT_DIRECT) return true;

    rw = 1.0F / N;

    for (i = 0; i < N; i++) {
        Rdat[i] *= rw;
        Idat[i] *= rw;
    }

    return true;
}
