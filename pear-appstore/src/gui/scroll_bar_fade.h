#ifndef SCROLL_BAR_FADE_H
#define SCROLL_BAR_FADE_H

class QScrollArea;

/** Aplică fade in/out pe scrollbar: ascuns implicit, fade in la scroll, fade out la ~1s după ce scroll-ul se oprește. */
void applyScrollBarFade(QScrollArea* area);

#endif // SCROLL_BAR_FADE_H
