pragma Singleton
import QtQuick

QtObject {
    property int currentIdx: 4
    property var _history: [4]
    property int _historyIdx: 0
    property bool _fromHistory: false

    property bool canGoBack:    _historyIdx > 0
    property bool canGoForward: _historyIdx < _history.length - 1

    function navigateTo(idx) {
        if (idx === currentIdx) return
        var h = _history.slice(0, _historyIdx + 1)
        h.push(idx)
        _history = h
        _historyIdx = h.length - 1
        _fromHistory = true
        currentIdx = idx
        _fromHistory = false
    }

    function goBack() {
        if (!canGoBack) return
        _historyIdx--
        _fromHistory = true
        currentIdx = _history[_historyIdx]
        _fromHistory = false
    }

    function goForward() {
        if (!canGoForward) return
        _historyIdx++
        _fromHistory = true
        currentIdx = _history[_historyIdx]
        _fromHistory = false
    }
}
