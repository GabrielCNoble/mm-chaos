#include "libu64/gfxprint.h"
#include "attributes.h"
#include "stdbool.h"
#include "fault.h"

// #define GFXP_FLAG_HIRAGANA (1 << 0)
// #define GFXP_FLAG_RAINBOW (1 << 1)
// #define GFXP_FLAG_SHADOW (1 << 2)
// #define GFXP_FLAG_UPDATE (1 << 3)
// #define GFXP_FLAG_ENLARGE (1 << 6)
// #define GFXP_FLAG_OPEN (1 << 7)

u64 sGfxPrintFontTLUT[] = {
#include "assets/boot/gfxprint/gfx_print_font_tlut.rgba16.inc.c"
};

u64 sGfxPrintRainbowTLUT[] = {
#include "assets/boot/gfxprint/gfx_print_rainbow_tlut.rgba16.inc.c"
};

u8 sGfxPrintRainbowFont[] = {
#include "assets/boot/gfxprint/sGfxPrintRainbowFont.bin.inc.c"
};

u8 sGfxPrintFont[] = {
#include "assets/boot/gfxprint/sGfxPrintFont.bin.inc.c"
};

void GfxPrint_Setup(GfxPrint* this) {
    s32 width = 16;
    s32 height = 256;
    s32 i;

    gDPPipeSync(this->dList++);
    gDPSetOtherMode(this->dList++,
                    G_AD_DISABLE | G_CD_DISABLE | G_CK_NONE | G_TC_FILT | G_TF_BILERP | G_TT_IA16 | G_TL_TILE |
                        G_TD_CLAMP | G_TP_NONE | G_CYC_1CYCLE | G_PM_NPRIMITIVE,
                    G_AC_NONE | G_ZS_PRIM | G_RM_XLU_SURF | G_RM_XLU_SURF2);
    gDPSetCombineMode(this->dList++, G_CC_DECALRGBA, G_CC_DECALRGBA);
    gDPLoadTextureBlock_4b(this->dList++, sGfxPrintFont, G_IM_FMT_CI, width, height, 0, G_TX_NOMIRROR | G_TX_WRAP,
                           G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    gDPLoadTLUT(this->dList++, 64, 256, sGfxPrintFontTLUT);

    for (i = 1; i < 4; i++) {
        gDPSetTile(this->dList++, G_IM_FMT_CI, G_IM_SIZ_4b, 1, 0, i * 2, i, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK,
                   G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
        gDPSetTileSize(this->dList++, i * 2, 0, 0, 60, 1020);
    }

    gDPSetColor(this->dList++, G_SETPRIMCOLOR, this->color.rgba);

    gDPLoadMultiTile_4b(this->dList++, sGfxPrintRainbowFont, 0, 1, G_IM_FMT_CI, 2, 8, 0, 0, 1, 7, 4,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 1, 3, G_TX_NOLOD, G_TX_NOLOD);

    gDPLoadTLUT(this->dList++, 16, 320, sGfxPrintRainbowTLUT);

    for (i = 1; i < 4; i++) {
        gDPSetTile(this->dList++, G_IM_FMT_CI, G_IM_SIZ_4b, 1, 0, i * 2 + 1, 4, G_TX_NOMIRROR | G_TX_WRAP, 3,
                   G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, 1, G_TX_NOLOD);
        gDPSetTileSize(this->dList++, i * 2 + 1, 0, 0, 4, 28);
    }
}

void GfxPrint_SetColor(GfxPrint* this, u32 r, u32 g, u32 b, u32 a) {
    this->color.r = r;
    this->color.g = g;
    this->color.b = b;
    this->color.a = a;
    gDPPipeSync(this->dList++);
    gDPSetColor(this->dList++, G_SETPRIMCOLOR, this->color.rgba);
}

void GfxPrint_SetPosPx(GfxPrint* this, s32 x, s32 y) {
    this->posX = this->baseX + (x << 2);
    this->posY = this->baseY + (y << 2);
}

void GfxPrint_SetPos(GfxPrint* this, s32 x, s32 y) {
    GfxPrint_SetPosPx(this, x << 3, y << 3);
}

void GfxPrint_SetBasePosPx(GfxPrint* this, s32 x, s32 y) {
    this->baseX = x << 2;
    this->baseY = y << 2;
}

void GfxPrint_PrintCharImpl(GfxPrint* this, u8 c) {
    u32 tile = (c & 0xFF) * 2;
    u16 s = c & 4;
    u16 t = c >> 3;

    if (this->flags & GFXP_FLAG_UPDATE) {
        this->flags &= ~GFXP_FLAG_UPDATE;

        gDPPipeSync(this->dList++);
        if (this->flags & GFXP_FLAG_RAINBOW) {
            gDPSetTextureLUT(this->dList++, G_TT_RGBA16);
            gDPSetCycleType(this->dList++, G_CYC_2CYCLE);
            gDPSetRenderMode(this->dList++, G_RM_PASS, G_RM_XLU_SURF2);
            gDPSetCombineMode(this->dList++, G_CC_INTERFERENCE, G_CC_PASS2);
        }
        if(this->flags & GFXP_FLAG_BLEND)
        {
            gDPSetTextureLUT(this->dList++, G_TT_IA16);
            gDPSetCycleType(this->dList++, G_CYC_1CYCLE);
            gDPSetRenderMode(this->dList++, G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2);
            // gDPSetCombineMode(this->dList++, G_CC_MODULATEIDECALA_PRIM, G_CC_MODULATEIDECALA_PRIM);
            gDPSetCombineMode(this->dList++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        } 
        else 
        {
            gDPSetTextureLUT(this->dList++, G_TT_IA16);
            gDPSetCycleType(this->dList++, G_CYC_1CYCLE);
            gDPSetRenderMode(this->dList++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
            gDPSetCombineMode(this->dList++, G_CC_MODULATEIDECALA_PRIM, G_CC_MODULATEIDECALA_PRIM);
        }
    }

    if (this->flags & GFXP_FLAG_SHADOW) {
        gDPSetColor(this->dList++, G_SETPRIMCOLOR, 0);

        gSPTextureRectangle(this->dList++, this->posX + 4, this->posY + 4, this->posX + 4 + 32, this->posY + 4 + 32,
                            tile, s << 6, t << 8, 1 << 10, 1 << 10);

        gDPSetColor(this->dList++, G_SETPRIMCOLOR, this->color.rgba);
    }

    gSPTextureRectangle(this->dList++, this->posX, this->posY, this->posX + 32, this->posY + 32, tile, s << 6, t << 8,
                        1 << 10, 1 << 10);

    this->posX += 32;
}

void GfxPrint_PrintChar(GfxPrint* this, u8 c) {
    if (c == ' ') {
        this->posX += 32;
    } else if (c > ' ' && c < 0x7F) {
        GfxPrint_PrintCharImpl(this, c);
    } else if (c >= 0xA0 && c < 0xE0) {
        if (this->flags & GFXP_FLAG_HIRAGANA) {
            if (c < 0xC0) {
                c -= 0x20;
            } else {
                c += 0x20;
            }
        }
        GfxPrint_PrintCharImpl(this, c);
    } else {
        switch (c) {
            case '\0':
                break;

            case '\n':
                this->posY += (this->y_increment != 0) ? this->y_increment : 32;
                FALLTHROUGH;
            case '\r':
                this->posX = this->baseX;
                break;

            case '\t':
                do {
                    GfxPrint_PrintCharImpl(this, ' ');
                } while ((this->posX - this->baseX) % 256);
                break;

            case GFXP_HIRAGANA_CHAR:
                this->flags |= GFXP_FLAG_HIRAGANA;
                break;

            case GFXP_KATAKANA_CHAR:
                this->flags &= ~GFXP_FLAG_HIRAGANA;
                break;

            case GFXP_RAINBOW_ON_CHAR:
                this->flags |= GFXP_FLAG_RAINBOW;
                this->flags |= GFXP_FLAG_UPDATE;
                break;

            case GFXP_RAINBOW_OFF_CHAR:
                this->flags &= ~GFXP_FLAG_RAINBOW;
                this->flags |= GFXP_FLAG_UPDATE;
                break;

            case GFXP_UNUSED_CHAR:
            default:
                break;
        }
    }
}

void GfxPrint_PrintStringWithSize(GfxPrint* this, const void* buffer, size_t charSize, size_t charCount) {
    const char* str = (const char*)buffer;
    size_t count = charSize * charCount;

    while (count != 0) {
        GfxPrint_PrintChar(this, *str++);
        count--;
    }
}

void GfxPrint_PrintString(GfxPrint* this, const char* str) {
    while (*str != '\0') {
        GfxPrint_PrintChar(this, *str++);
    }
}

void* GfxPrint_Callback(void* arg, const char* str, size_t size) {
    GfxPrint* this = arg;

    GfxPrint_PrintStringWithSize(this, str, sizeof(char), size);

    return this;
}

void GfxPrint_Init(GfxPrint* this) {
    this->flags &= ~GFXP_FLAG_OPEN;

    this->callback = GfxPrint_Callback;
    this->dList = NULL;
    this->posX = 0;
    this->posY = 0;
    this->baseX = 0;
    this->baseY = 0;
    this->color.rgba = 0;

    this->flags &= ~GFXP_FLAG_HIRAGANA;
    this->flags &= ~GFXP_FLAG_RAINBOW;
    this->flags |= GFXP_FLAG_SHADOW;
    this->flags |= GFXP_FLAG_UPDATE;
}

void GfxPrint_Destroy(GfxPrint* this) {
}

void GfxPrint_Open(GfxPrint* this, Gfx* dList) {
    if (!(this->flags & GFXP_FLAG_OPEN)) {
        this->flags |= GFXP_FLAG_OPEN;
        this->dList = dList;
        GfxPrint_Setup(this);
    }
}

Gfx* GfxPrint_Close(GfxPrint* this) {
    Gfx* ret;

    this->flags &= ~GFXP_FLAG_OPEN;
    ret = this->dList;
    this->dList = NULL;

    return ret;
}

s32 GfxPrint_VPrintf(GfxPrint* this, const char* fmt, va_list args) {
    return vaprintf(&this->callback, fmt, args);
}

u32 GfxPrint_CalcWrappedStringDimensions(GfxPrint *this, GfxTextSize *text_size, const char *str, u32 return_on_percent)
{
    u32 range_end = 0;
    u32 range_start = 0;
    u32 pos_x = 0;
    u32 used_space;
    u32 text_length;

    while(str[range_end] != '\0')
    {
        used_space = pos_x - this->wrap_start;
        
        if(str[range_end] == ' ')
        {
            while(str[range_end] == ' ')
            {
                range_end++;
            }

            text_length = range_end - range_start;
            pos_x += text_length * 32;
        }
        else
        {
            while(str[range_end] != ' ' && str[range_end] != '\0')
            {
                range_end++;
            }

            text_length = range_end - range_start;
        
            if(used_space + text_length * 32 > this->wrap_size)
            {
                pos_x = this->wrap_start;
                text_size->lines++;
            }
        }

        range_start = range_end;

        if(str[range_end] == '%' && return_on_percent)
        {
            break;
        }
    }

    return range_end;
}

void GfxPrint_CalcWrappedTextDimensions(GfxPrint *this, GfxTextSize *text_size, const char *fmt, ...)
{
    u32 range_start = 0;
    u32 range_end = 0;
    va_list args;
    va_start(args, fmt);

    text_size->lines = 1;
    text_size->length = 0;

    while(fmt[range_end] != '\0')
    {
        if(fmt[range_end] == '%')
        {
            range_end++;

            if(fmt[range_end] == 's')
            {
                char *str = va_arg(args, char *);
                range_end++;
                GfxPrint_CalcWrappedStringDimensions(this, text_size, str, false); 
            }
            else
            {
                Fault_AddHungupAndCrashImpl("Unimplemented!", NULL);
            }
        }
        else
        {
            range_end += GfxPrint_CalcWrappedStringDimensions(this, text_size, fmt + range_start, true);
        }

        range_start = range_end;
    }

    va_end(args);
}

s32 GfxPrint_Printf(GfxPrint* this, const char* fmt, ...) {
    s32 ret;
    va_list args;
    va_start(args, fmt);

    ret = GfxPrint_VPrintf(this, fmt, args);

    va_end(args);

    return ret;
}

u32 GfxPrint_PrintStringWrap(GfxPrint *this, const char *str, u32 return_on_percent)
{
    u32 range_start = 0;
    u32 range_end = 0;
    u32 used_space;
    u32 text_length;

    while(str[range_end] != '\0')
    {
        used_space = this->posX - this->wrap_start;
        
        if(str[range_end] == ' ')
        {
            while(str[range_end] == ' ')
            {
                range_end++;
            }

            text_length = range_end - range_start;
            this->callback(this, str + range_start, text_length);
        }
        else
        {
            while(str[range_end] != ' ' && str[range_end] != '\0')
            {
                range_end++;
            }

            text_length = range_end - range_start;
        
            if(used_space + text_length * 32 > this->wrap_size)
            {
                this->posX = this->wrap_start;
                this->posY += (this->y_increment != 0) ? this->y_increment : 32;
            }

            this->callback(this, str + range_start, text_length);
        }

        range_start = range_end;

        if(str[range_end] == '%' && return_on_percent)
        {
            break;
        }
    }

    return range_end;
}

s32 GfxPrint_PrintfWrap(GfxPrint *this, const char *fmt, ...)
{
    u32 range_start = 0;
    u32 range_end = 0;
    va_list args;
    va_list args_copy;
    va_start(args, fmt);

    this->posX = this->wrap_start;

    while(fmt[range_end] != '\0')
    {
        if(fmt[range_end] == '%')
        {
            range_end++;

            if(fmt[range_end] == 's')
            {
                char *str = va_arg(args, char *);
                range_end++;
                GfxPrint_PrintStringWrap(this, str, false); 
            }
            else
            {
                Fault_AddHungupAndCrashImpl("Unimplemented!", NULL);
            }
        }
        else
        {
            range_end += GfxPrint_PrintStringWrap(this, fmt + range_start, true);
        }

        range_start = range_end;
    }

    va_end(args);
}
