#include "stdlib.h"
#include "string.h"
#include "PR/xstdio.h"

#define BUFF_LEN 0x20

short _Ldunscale(short*, _Pft*);
void _Genld(_Pft*, char, char*, short, short);

static const double pows[] = { 10e0L, 10e1L, 10e3L, 10e7L, 10e15L, 10e31L, 10e63L, 10e127L, 10e255L };

/* float properties */
#define _D0 0
#define _DBIAS 0x3FF
#define _DLONG 1
#define _DOFF 4
#define _FBIAS 0x7E
#define _FOFF 7
#define _FRND 1
#define _LBIAS 0x3FFE
#define _LOFF 15
/* integer properties */
#define _C2 1
#define _CSIGN 1
#define _ILONG 0
#define _MBMAX 8
#define NAN 2
#define INF 1
#define FINITE -1
#define _DFRAC ((1 << _DOFF) - 1)
#define _DMASK (0x7FFF & ~_DFRAC)
#define _DMAX ((1 << (15 - _DOFF)) - 1)
#define _DNAN (0x8000 | _DMAX << _DOFF | 1 << (_DOFF - 1))
#define _DSIGN 0x8000
#if _D0 == 3
#define _D1 2 /* little-endian order */
#define _D2 1
#define _D3 0
#else
#define _D1 1 /* big-endian order */
#define _D2 2
#define _D3 3
#endif

void _Ldtob(_Pft* args, char code) {
    char buff[BUFF_LEN];
    char* p = buff;
    LONG_DOUBLE_TYPE ldval = args->v.ld;
    short err;
    short nsig;
    short xexp;

    if (args->prec < 0) {
        args->prec = 6;
    } else if ((args->prec == 0) && ((code == 'g') || (code == 'G'))) {
        args->prec = 1;
    }

    err = _Ldunscale(&xexp, (_Pft*)args);
    if (err > 0) {
        memcpy(args->s, err == 2 ? "NaN" : "Inf", args->n1 = 3);
        return;
    } else if (err == 0) {
        nsig = 0;
        xexp = 0;
    } else {
        {
            int i;
            int n;

            if (ldval < 0) {
                ldval = -ldval;
            }
            xexp = xexp * 30103 / 0x000186A0 - 4;
            if (xexp < 0) {
                n = (3 - xexp) & ~3;
                xexp = -n;
                for (i = 0; n > 0; n >>= 1, i++) {
                    if ((n & 1) != 0) {
                        ldval *= pows[i];
                    }
                }
            } else if (xexp > 0) {
                LONG_DOUBLE_TYPE factor = 1;
                xexp &= ~3;

                for (n = xexp, i = 0; n > 0; n >>= 1, i++) {
                    if ((n & 1) != 0) {
                        factor *= pows[i];
                    }
                }
                ldval /= factor;
            }
        }
        {
            int gen = ((code == 'f') ? xexp + 10 : 6) + args->prec;
            if (gen > 0x13) {
                gen = 0x13;
            }

            for (*p++ = '0'; (gen > 0) && (0 < ldval); p += 8) {
                int j;
                long lo = ldval;

                if ((gen -= 8) > 0) {
                    ldval = (ldval - lo) * 1.0e8;
                }

                p += 8;
                for (j = 8; (lo > 0) && (--j >= 0);) {
                    ldiv_t qr;
                    qr = ldiv(lo, 10);
                    *--p = qr.rem + '0';
                    lo = qr.quot;
                }

                while (--j >= 0) {
                    *--p = '0';
                }
            }

            gen = p - &buff[1];
            for (p = &buff[1], xexp += 7; *p == '0'; p++) {
                --gen, --xexp;
            }

            nsig = args->prec + ((code == 'f') ? (xexp + 1) : (((code == 'e') || (code == 'E')) ? 1 : 0));
            if (gen < nsig) {
                nsig = gen;
            }

            if (nsig > 0) {
                const char drop = ((nsig < gen) && (p[nsig] > '4')) ? '9' : '0';
                int n;

                for (n = nsig; p[--n] == drop;) {
                    nsig--;
                }
                if (drop == '9') {
                    p[n]++;
                }
                if (n < 0) {
                    --p, ++nsig, ++xexp;
                }
            }
        }
    }
    _Genld((_Pft*)args, code, p, nsig, xexp);
}

short _Ldunscale(short* pex, _Pft* px) {
    unsigned short* ps = (unsigned short*)px;
    short xchar = (ps[_D0] & _DMASK) >> _DOFF;

    if (xchar == _DMAX) { /* NaN or INF */
        *pex = 0;
        return ps[_D0] & _DFRAC || ps[_D1] || ps[_D2] || ps[_D3] ? NAN : INF;
    } else if (0 < xchar) {
        ps[_D0] = (ps[_D0] & ~_DMASK) | (_DBIAS << _DOFF);
        *pex = xchar - (_DBIAS - 1);
        return FINITE;
    }
    if (xchar < 0) {
        return NAN;
    } else {
        *pex = 0;
        return 0;
    }
}

void _Genld(_Pft* px, char code, char* p, short nsig, short xexp) {
    const char point = '.';

    if (nsig <= 0) {
        nsig = 1, p = "0";
    }

    if ((code == 'f') || (((code == 'g') || (code == 'G')) && (-4 <= xexp) && (xexp < px->prec))) { /* 'f' format */
        xexp++; /* change to leading digit count */

        if (code != 'f') { /* fixup for 'g' */
            if (!(px->flags & FLAGS_HASH) && nsig < px->prec) {
                px->prec = nsig;
            }
            if ((px->prec -= xexp) < 0) {
                px->prec = 0;
            }
        }

        if (xexp <= 0) { /* digits only to right of point */
            px->s[px->n1++] = '0';
            if ((px->prec > 0) || (px->flags & FLAGS_HASH)) {
                px->s[px->n1++] = point;
            }
            if (px->prec < -xexp) {
                xexp = -px->prec;
            }
            px->nz1 = -xexp;
            px->prec += xexp;
            if (px->prec < nsig) {
                nsig = px->prec;
            }
            memcpy(&px->s[px->n1], p, px->n2 = nsig);
            px->nz2 = px->prec - nsig;
        } else if (nsig < xexp) { /* zeros before point */
            memcpy(&px->s[px->n1], p, nsig);
            px->n1 += nsig;
            px->nz1 = xexp - nsig;
            if ((px->prec > 0) || (px->flags & FLAGS_HASH)) {
                px->s[px->n1] = point, ++px->n2;
            }
            px->nz2 = px->prec;
        } else { /* enough digits before point */
            memcpy(&px->s[px->n1], p, xexp);
            px->n1 += xexp;
            nsig -= xexp;
            if ((px->prec > 0) || (px->flags & FLAGS_HASH)) {
                px->s[px->n1++] = point;
            }
            if (px->prec < nsig) {
                nsig = px->prec;
            }
            memcpy(&px->s[px->n1], p + xexp, nsig);
            px->n1 += nsig;
            px->nz1 = px->prec - nsig;
        }
    } else {                                  /* 'e' format */
        if ((code == 'g') || (code == 'G')) { /* fixup for 'g' */
            if (nsig < px->prec) {
                px->prec = nsig;
            }
            if (--px->prec < 0) {
                px->prec = 0;
            }
            code = (code == 'g') ? 'e' : 'E';
        }
        px->s[px->n1++] = *p++;
        if ((px->prec > 0) || (px->flags & FLAGS_HASH)) {
            px->s[px->n1++] = point;
        }
        if (px->prec > 0) { /* put fraction digits */
            if (px->prec < --nsig) {
                nsig = px->prec;
            }
            memcpy(&px->s[px->n1], p, nsig);
            px->n1 += nsig;
            px->nz1 = px->prec - nsig;
        }
        p = &px->s[px->n1]; /* put exponent */
        *p++ = code;
        if (xexp >= 0) {
            *p++ = '+';
        } else { /* negative exponent */
            *p++ = '-';
            xexp = -xexp;
        }
        if (xexp >= 100) { /* put oversize exponent */
            if (xexp >= 1000) {
                *p++ = xexp / 1000 + '0', xexp %= 1000;
            }
            *p++ = xexp / 100 + '0', xexp %= 100;
        }
        *p++ = xexp / 10 + '0', xexp %= 10;

        *p++ = xexp + '0';
        px->n2 = p - &px->s[px->n1];
    }
    if ((px->flags & (FLAGS_ZERO | FLAGS_MINUS)) == FLAGS_ZERO) { /* pad with leading zeros */
        int n = px->n0 + px->n1 + px->nz1 + px->n2 + px->nz2;

        if (n < px->width) {
            px->nz0 = px->width - n;
        }
    }
}
