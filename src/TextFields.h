#ifndef DOOMRPG_TEXT_FIELDS_H
#define DOOMRPG_TEXT_FIELDS_H
struct DoomCanvas_s;
float TextFields_scale(void);
const char* TextFields_sizeLabel(void);
int TextFields_dialogWidth(struct DoomCanvas_s* canvas);
int TextFields_dialogLeft(struct DoomCanvas_s* canvas);
int TextFields_dialogRows(void);
const char* TextFields_widthLabel(void);
void TextFields_draw(struct DoomCanvas_s* canvas, const char* text, float x, float y, int flags, int start, int count);
void TextFields_prepareDialog(struct DoomCanvas_s* canvas, const char* text);
void TextFields_refresh(struct DoomCanvas_s* canvas);
#endif
