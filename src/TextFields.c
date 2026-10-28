#include <SDL.h>
#include "DoomRPG.h"
#include "DoomCanvas.h"
#include "Hud.h"
#include "SDL_Video.h"
#include "TextFields.h"

float TextFields_scale(void)
{
    static const float sizes[] = {1.0f, 0.75f, 0.60f, 0.50f};
    int size = SDL_clamp(sdlVideo.textSize, 0, 3);
    if (sdlVideo.rendererH <= 0) return 1.0f;
    /* Keep at least one framebuffer pixel per source-font pixel. */
    return SDL_min(1.0f, SDL_max(sizes[size], sdlVideo.uiH / (float)sdlVideo.rendererH));
}

const char* TextFields_sizeLabel(void)
{
    static const char* labels[] = {"100%", "75%", "60%", "50%"};
    return labels[SDL_clamp(sdlVideo.textSize, 0, 3)];
}

int TextFields_dialogWidth(DoomCanvas_t* canvas)
{
    static const int percentages[] = {0, 50, 60, 70, 80, 90, 100};
    int index = SDL_clamp(sdlVideo.textFieldWidth, 0, (int)SDL_arraysize(percentages) - 1);
    if (index == 0) return SDL_min(128, canvas->displayRect.w);
    return SDL_max(1, canvas->displayRect.w * percentages[index] / 100);
}

const char* TextFields_widthLabel(void)
{
    static const char* labels[] = {"Original", "50%", "60%", "70%", "80%", "90%", "Full"};
    return labels[SDL_clamp(sdlVideo.textFieldWidth, 0, (int)SDL_arraysize(labels) - 1)];
}

int TextFields_dialogLeft(DoomCanvas_t* canvas)
{
    return (canvas->displayRect.w - TextFields_dialogWidth(canvas)) / 2;
}

int TextFields_dialogRows(void)
{
    return (int)(48.0f / (12.0f * TextFields_scale()) + 0.01f);
}

void TextFields_draw(DoomCanvas_t* canvas, const char* text, float x, float y, int flags, int start, int count)
{
    int length = (int)SDL_strlen(text);
    if (start < 0 || start >= length || count == 0) return;
    length -= start;
    if (count >= 0) length = SDL_min(length, count);
    float scale = TextFields_scale();
    float advance = 7 * scale, height = 12 * scale;
    if (flags & 8) x -= length * advance;
    else if (flags & 16) x -= length * advance / 2;
    if (flags & 2) y -= height;
    else if (flags & 32) y -= height / 2;
    float origin = x;
    SDL_Texture* texture = canvas->imgFont.imgBitmap;
    SDL_SetTextureColorMod(texture, (canvas->fontColor >> 16) & 255, (canvas->fontColor >> 8) & 255, canvas->fontColor & 255);
    for (int i = start; i < start + length; ++i) {
        unsigned char c = (unsigned char)text[i];
        if (c == '\n') { y += height; x = origin; continue; }
        if (c >= 33) {
            SDL_Rect source = {9 * ((c - 33) & 15), 12 * ((c - 33) >> 4), 9, 12};
            SDL_FRect target = {
                (canvas->displayRect.x + x) * sdlVideo.rendererW / sdlVideo.uiW,
                (canvas->displayRect.y + y) * sdlVideo.rendererH / sdlVideo.uiH,
                9 * scale * sdlVideo.rendererW / sdlVideo.uiW,
                height * sdlVideo.rendererH / sdlVideo.uiH
            };
            SDL_RenderCopyF(sdlVideo.renderer, texture, &source, &target);
        }
        x += advance;
    }
}

void TextFields_prepareDialog(DoomCanvas_t* canvas, const char* text)
{
    char source[sizeof(canvas->dialogSource)];
    SDL_strlcpy(source, text, sizeof(source));
    SDL_strlcpy(canvas->dialogSource, source, sizeof(canvas->dialogSource));
    boolean compact = (TextFields_scale() < 1 || sdlVideo.textFieldWidth > 0) && canvas->state != ST_DIALOGPASSWORD;
    int used = 0;
    for (int i = 0; source[i] && used < (int)sizeof(canvas->dialogBuffer) - 1; ++i) {
        char c = source[i];
        if (compact && c == '|') {
            if (source[i + 1] == '|') { c = '\n'; ++i; }
            else c = ' ';
        }
        canvas->dialogBuffer[used++] = c;
    }
    canvas->dialogBuffer[used] = '\0';
    /* Leave seven pixels for the scrollbar and account for glyph overhang. */
    int columns = SDL_max(1, (int)((TextFields_dialogWidth(canvas) - 7 - 2 * TextFields_scale()) / (7 * TextFields_scale())));
    int start = 0;
    canvas->numDialogLines = 0;
    do {
        int end = start, space = -1;
        while (end < used && canvas->dialogBuffer[end] != '|' && canvas->dialogBuffer[end] != '\n' && end - start < columns) {
            if (canvas->dialogBuffer[end] == ' ') space = end;
            ++end;
        }
        if (end < used && canvas->dialogBuffer[end] != '|' && canvas->dialogBuffer[end] != '\n' && space > start)
            end = space;
        int row = canvas->numDialogLines++;
        canvas->dialogIndexes[2 * row] = (short)start;
        canvas->dialogIndexes[2 * row + 1] = (short)(end - start);
        start = end;
        if (start < used && (canvas->dialogBuffer[start] == '|' || canvas->dialogBuffer[start] == '\n')) ++start;
        else while (start < used && canvas->dialogBuffer[start] == ' ') ++start;
    } while (start < used && canvas->numDialogLines < (int)SDL_arraysize(canvas->dialogIndexes) / 2);
    if (used > 0 && (canvas->dialogBuffer[used - 1] == '|' || canvas->dialogBuffer[used - 1] == '\n') &&
        canvas->numDialogLines < (int)SDL_arraysize(canvas->dialogIndexes) / 2) {
        canvas->dialogIndexes[2 * canvas->numDialogLines] = (short)used;
        canvas->dialogIndexes[2 * canvas->numDialogLines + 1] = 0;
        ++canvas->numDialogLines;
    }
}

void TextFields_refresh(DoomCanvas_t* canvas)
{
    canvas->hud->msgMaxChars = SDL_max(1, (int)((canvas->displayRect.w - 4) / (7 * TextFields_scale())));
    if (canvas->dialogSource[0]) {
        int offset = canvas->dialogIndexes[canvas->currentDialogLine * 2];
        TextFields_prepareDialog(canvas, canvas->dialogSource);
        canvas->currentDialogLine = 0;
        while (canvas->currentDialogLine + 1 < canvas->numDialogLines &&
               canvas->dialogIndexes[(canvas->currentDialogLine + 1) * 2] <= offset)
            ++canvas->currentDialogLine;
        canvas->currentDialogLine = SDL_min(canvas->currentDialogLine, SDL_max(0, canvas->numDialogLines - TextFields_dialogRows()));
        canvas->dialogTypeLineIdx = TextFields_dialogRows();
    }
}
