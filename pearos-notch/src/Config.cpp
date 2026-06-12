#include "Config.h"

Config& Config::instance() {
    static Config inst;
    return inst;
}

Config::Config()
    : QObject(nullptr)
    , m_settings("pearos-notch", "config")
{}

// --- mode ---
QString Config::mode() const {
    return m_settings.value("mode", "island").toString();
}
void Config::setMode(const QString& v) {
    if (mode() == v) return;
    m_settings.setValue("mode", v);
    emit modeChanged(v);
}

// --- smallW ---
int Config::smallW() const {
    return m_settings.value("smallW", 170).toInt();
}
void Config::setSmallW(int v) {
    if (smallW() == v) return;
    m_settings.setValue("smallW", v);
    emit smallWChanged(v);
}

// --- smallH ---
int Config::smallH() const {
    return m_settings.value("smallH", 30).toInt();
}
void Config::setSmallH(int v) {
    if (smallH() == v) return;
    m_settings.setValue("smallH", v);
    emit smallHChanged(v);
}

// --- bigW ---
int Config::bigW() const {
    return m_settings.value("bigW", 510).toInt();
}
void Config::setBigW(int v) {
    if (bigW() == v) return;
    m_settings.setValue("bigW", v);
    emit bigWChanged(v);
}

// --- bigH ---
int Config::bigH() const {
    return m_settings.value("bigH", 110).toInt();
}
void Config::setBigH(int v) {
    if (bigH() == v) return;
    m_settings.setValue("bigH", v);
    emit bigHChanged(v);
}

// --- screenGap ---
int Config::screenGap() const {
    return m_settings.value("screenGap", 6).toInt();
}
void Config::setScreenGap(int v) {
    if (screenGap() == v) return;
    m_settings.setValue("screenGap", v);
    emit screenGapChanged(v);
}

// --- hoverDelayMs ---
int Config::hoverDelayMs() const {
    return m_settings.value("hoverDelayMs", 300).toInt();
}
void Config::setHoverDelayMs(int v) {
    if (hoverDelayMs() == v) return;
    m_settings.setValue("hoverDelayMs", v);
    emit hoverDelayMsChanged(v);
}

// --- animDurationMs ---
int Config::animDurationMs() const {
    return m_settings.value("animDurationMs", 280).toInt();
}
void Config::setAnimDurationMs(int v) {
    if (animDurationMs() == v) return;
    m_settings.setValue("animDurationMs", v);
    emit animDurationMsChanged(v);
}
