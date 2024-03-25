
#include "flogview.h"

FLogViewBase::FLogViewBase(finalcut::FWidget* parent, uint_fast16_t logSize) : finalcut::FDialog{parent}, _logSize{logSize} {
    //
    // -widget config-
    //
    _textView.addCallback("mouse-wheel-up", this, &FLogViewBase::_loggerScrollUpCb);
    //_textView.ignorePadding();

    _toggleAutoScroll.setChecked(true);
    _toggleAutoScroll.addCallback("toggled", this, &FLogViewBase::_autoScrollToggleCb);

    _buttonPlay.addCallback("clicked", this, &FLogViewBase::_playButtonCb);
    _buttonPlay.setForegroundColor(finalcut::FColor::White);
    _buttonPlay.setFocusForegroundColor(finalcut::FColor::White);
    _buttonPlay.setFocus();

    _buttonClear.setForegroundColor(finalcut::FColor::White);
    _buttonClear.setFocusForegroundColor(finalcut::FColor::White);

    _info.setChecked();

    _lineEditFilter.setLabelText(L"Filter");
    _lineEditFilter.setLabelOrientation(finalcut::FLineEdit::LabelOrientation::Above);
    _lineEditFilter.unsetShadow();

    //
    // -static layout-
    //
    // play/pause button
    _buttonPlay.setGeometry(finalcut::FPoint{1,2}, finalcut::FSize{8, 1});

    // play/pause indicator
    _labelPlay.setGeometry(finalcut::FPoint{11,2}, finalcut::FSize{3, 1});

    // clear button
    _buttonClear.setGeometry(finalcut::FPoint{14,2}, finalcut::FSize{7, 1});

    // button group
    _radiobutton_group.setGeometry(finalcut::FPoint{23,1}, finalcut::FSize{25, 3});
    _error.setGeometry(finalcut::FPoint{1,1}, finalcut::FSize{5, 1});
    _warning.setGeometry(finalcut::FPoint{7,1}, finalcut::FSize{5, 1});
    _info.setGeometry(finalcut::FPoint{13,1}, finalcut::FSize{5, 1});
    _trace.setGeometry(finalcut::FPoint{19,1}, finalcut::FSize{5, 1});

    // filter box
    _lineEditFilter.setGeometry(finalcut::FPoint{49,2}, finalcut::FSize{12, 3});
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//////////////////////////////////////////////////////////////////////

void FLogViewBase::clear(void) {
    _textView.clear();
}

void FLogViewBase::registerOnQuit(std::function<void(void)> cb) {
    _quitCb = cb;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//////////////////////////////////////////////////////////////////////

void FLogViewBase::initLayout(void) {
    _adjust();
    finalcut::FDialog::initLayout();
}

void FLogViewBase::adjustSize(void) {
    _adjust();
    finalcut::FDialog::adjustSize();
}

void FLogViewBase::onClose(finalcut::FCloseEvent* event) {
    _quitCb();
    event->accept();
}

void FLogViewBase::onKeyPress (finalcut::FKeyEvent* event) {
    if(event->key() == finalcut::FKey::Ctrl_c) {
        _quitCb();
        event->accept();
    } else {
        finalcut::FDialog::onKeyPress(event);
    }
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//////////////////////////////////////////////////////////////////////

void FLogViewBase::_adjust(void) {
    // auto scroll toggle
    finalcut::FPoint togglePosition{static_cast<int>(getClientWidth()) - 14, 1};
    finalcut::FSize toggleSize{15, 3};
    _toggleGroup.setGeometry(togglePosition, toggleSize);
    _toggleAutoScroll.setGeometry(finalcut::FPoint{2,1}, finalcut::FSize{10, 1});

    // logger view
    finalcut::FPoint loggerPosition{1,4};
    finalcut::FSize loggerSize{getWidth(), getClientHeight() - 3};
    _textView.setGeometry(loggerPosition, loggerSize);
}

void FLogViewBase::_printLog(const std::wstring& logLine, LogLevel logLevel, std::string::size_type hglPos){
    finalcut::FString logLineFstring{logLine};

    switch (logLevel) {
    case LogLevel::LOG_TRACE:
        logLineFstring.insert(L"[TRACE]   ", 0);
        _textView.append(logLineFstring);    
        _textView.addHighlight(_textView.getLines().size()-1, finalcut::FTextView::FTextHighlight{0, 7, finalcut::FColorPair{finalcut::FColor::White, finalcut::FColor::LightGray}});
        break;

    case LogLevel::LOG_INFO:
        logLineFstring.insert(L"[INFO]    ", 0);
        _textView.append(logLineFstring);    
        _textView.addHighlight(_textView.getLines().size()-1, finalcut::FTextView::FTextHighlight{0, 6, finalcut::FColorPair{finalcut::FColor::White, finalcut::FColor::Blue}});
        break;

    case LogLevel::LOG_WARNING:
        logLineFstring.insert(L"[WARNING] ", 0);
        _textView.append(logLineFstring);    
        _textView.addHighlight(_textView.getLines().size()-1, finalcut::FTextView::FTextHighlight{0, 9, finalcut::FColorPair{finalcut::FColor::White, finalcut::FColor::DarkOrange}});
        break;

    case LogLevel::LOG_ERROR:
        logLineFstring.insert(L"[ERROR]   ", 0);
        _textView.append(logLineFstring);    
        _textView.addHighlight(_textView.getLines().size()-1, finalcut::FTextView::FTextHighlight{0, 7, finalcut::FColorPair{finalcut::FColor::White, finalcut::FColor::Red}});
        break;

    default:
        _textView.append(L"[UNDEF]    PRINT LOG ATTEMPT WITH UNKNOWN LOG TYPE");    
        _textView.addHighlight(_textView.getLines().size()-1, finalcut::FTextView::FTextHighlight{0, 7, finalcut::FColorPair{finalcut::FColor::White, finalcut::FColor::Purple}});
        return;
    }

    if(hglPos != std::string::npos){
        _textView.addHighlight(_textView.getLines().size()-1, finalcut::FTextView::FTextHighlight{hglPos + 10, _searchString.length(), finalcut::FColorPair{finalcut::FColor::Black, finalcut::FColor::Yellow}});
    }
}

void FLogViewBase::_adjustView(bool isPrinted, bool isShifted) {
    if(isPrinted){
        if(_autoScroll){
            _textView.scrollToEnd();
        } else {
            // scroll up by 1 to keep the same view when a new log is added and oldest one removed
            if(isShifted && (_textView.getScrollPos().getY() > 0)) {
                _textView.scrollBy(0, -1);
            }
        }
        _textView.redraw();
    } else {
        if(isShifted){
            _textView.redraw();
        }
    }
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//////////////////////////////////////////////////////////////////////

void FLogViewBase::_autoScrollToggleCb(void) {
    _autoScroll = _toggleAutoScroll.isChecked();
}

void FLogViewBase::_loggerScrollUpCb(void) {
    if(_autoScroll){
        _autoScroll = false;
        _toggleAutoScroll.setChecked(false);
        _toggleAutoScroll.redraw();
    }
}

void FLogViewBase::_playButtonCb(void){
    if(_isPlaying){
        _isPlaying = false;
        _labelPlay.setText(finalcut::FString{std::wstring{L"\U0001F534"}});
    } else {
        _isPlaying = true;
        _labelPlay.setText(finalcut::FString{std::wstring{L"\U0001F7E2"}});
    }
    _labelPlay.redraw();
}

void FLogViewBase::_logLevelClickCb(LogLevel newLogLevel) {
    if(newLogLevel != _currentLogLevel) {
        if(static_cast<uint_fast8_t>(newLogLevel) > static_cast<uint_fast8_t>(_currentLogLevel)) {
            // scroll if needed and redraw logger view
            _adjustView(true, false);
        }

        _currentLogLevel = newLogLevel;
    }
}

void FLogViewBase::_filter(const LogList& logList) {
    bool clearFlag{false};
    
    _searchString = _lineEditFilter.getText().toWString();
    if(_searchString == L" "){
        _searchString.clear();
        clearFlag = true;
    }
    
    // clear log view
    _textView.clear();

    // search main list and print matching logs
    for(auto it = logList.cbegin(); it != logList.cend(); it++){
        if(clearFlag){
            _printLog(it->logString, it->logLevel);
        } else {
            std::string::size_type pos = it->logString.find(_searchString);
            if(pos != std::string::npos){
                _printLog(it->logString, it->logLevel, pos);
            }
        }
    }

    // scroll if needed and redraw logger view
    _adjustView(true, false);
}

void FLogViewBase::_log(LogList& logList, std::wstring&& logLine, LogLevel logLevel, bool isActive) {
    bool isShifted {false};
    bool isPrinted {false};

    // rollover if log display size is reached
    if(logList.size() >= _logSize) {
        if(isActive){
            //remove oldest log from display
            if(_searchString.empty()){
                // if search is not active than remove oldest log
                _textView.deleteLine(0);
                isShifted = true;
            } else {
                // if search is active than remove oldest log only if it matches the search string
                if(logList.front().logString.find(_searchString) != std::string::npos){
                    _textView.deleteLine(0);
                    isShifted = true;
                }
            }
        }

        logList.pop_front();
    }

    // add log to display if necessary
    if(isActive) {
        if(_searchString.empty()) {
            // search is not active, print log
            _printLog(logLine, logLevel);
            isPrinted = true;
        } else {
            // search is active, print if log matches the search
            std::string::size_type pos = logLine.find(_searchString);
            if(pos != std::string::npos){
                _printLog(logLine, logLevel, pos);
                isPrinted = true;
            } 
        }

        // adjust text view component
        _adjustView(isPrinted, isShifted);
    }

    // add log to main list
    logList.emplace_back(LogItem{logLevel, std::move(logLine)});
}

void FLogViewBase::_printWithSearch(const LogItem& logItem) {
    bool emptyFlag = _searchString.empty();
    
    if(emptyFlag) {
        _printLog(logItem.logString, logItem.logLevel);
    } else {
        std::string::size_type pos = logItem.logString.find(_searchString);
        if(pos != std::string::npos){
            _printLog(logItem.logString, logItem.logLevel, pos);
        }
    }
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//     ***FLogView (single view)**                                  //
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

FLogView::FLogView(finalcut::FWidget* parent, uint_fast16_t scrollBackLimit) : FLogViewBase{parent, scrollBackLimit}
{
    _trace.addCallback("clicked", this, &FLogView::_logLevelClickCb, LogLevel::LOG_TRACE);
    _info.addCallback("clicked", this, &FLogView::_logLevelClickCb, LogLevel::LOG_INFO);
    _warning.addCallback("clicked", this, &FLogView::_logLevelClickCb, LogLevel::LOG_WARNING);
    _error.addCallback("clicked", this, &FLogView::_logLevelClickCb, LogLevel::LOG_ERROR);

    _lineEditFilter.addCallback("changed", this, &FLogView::_filterChangedCb);

    _buttonClear.addCallback("clicked", this, &FLogView::clear);
}

void FLogView::clear(void) {
    // lock logger view access
    std::lock_guard<std::mutex> lg(_loggerViewMtx);
    _mainLogList.clear();
    //_currentLogSize = 0;

    // call base class clear
    FLogViewBase::clear();
}

void FLogView::log(std::wstring&& logLine, LogLevel logLevel) {
    // return on low level
    if(static_cast<uint_fast8_t>(logLevel) < static_cast<uint_fast8_t>(_currentLogLevel)){
        return;
    }
    
    if(_isPlaying == false){
        return;
    }

    // lock logger view access
    std::lock_guard<std::mutex> lg(_loggerViewMtx);

    _log(_mainLogList, std::move(logLine), logLevel, true);
}

void FLogView::_filterChangedCb(void) {
    // lock logger view access
    std::lock_guard<std::mutex> lg(_loggerViewMtx);
    
    // pass logList for filter
    _filter(_mainLogList);    
}

void FLogView::_logLevelClickCb(LogLevel newLogLevel) {
    if(static_cast<uint_fast8_t>(newLogLevel) > static_cast<uint_fast8_t>(_currentLogLevel)) {        
        // lock logger view access
        std::lock_guard<std::mutex> lg(_loggerViewMtx);
        
        // clear logger view
        FLogViewBase::clear();

        // remove lower level logs and print others
        for(auto it = _mainLogList.cbegin(); it != _mainLogList.cend();) {
            if(static_cast<uint_fast8_t>(it->logLevel) < static_cast<uint_fast8_t>(newLogLevel)) {
                it = _mainLogList.erase(it);
            } else {
                _printWithSearch(*it);
                it++;
            }
        }
    }
    // call base class cb
    FLogViewBase::_logLevelClickCb(newLogLevel);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//     ***FLogViewMulti (multi view)**                                  //
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

FLogViewMulti::FLogViewMulti(finalcut::FWidget* parent, uint_fast16_t scrollBackLimit, uint_fast16_t viewLimit) : FLogViewBase{parent, scrollBackLimit}, _viewSize{viewLimit}
{
    // configure dd-list
    _dropdownViewSelect.setLabelOrientation (finalcut::FLineEdit::LabelOrientation::Above);
    _dropdownViewSelect.unsetShadow();

    _trace.addCallback("clicked", this, &FLogViewMulti::_logLevelClickCb, LogLevel::LOG_TRACE);
    _info.addCallback("clicked", this, &FLogViewMulti::_logLevelClickCb, LogLevel::LOG_INFO);
    _warning.addCallback("clicked", this, &FLogViewMulti::_logLevelClickCb, LogLevel::LOG_WARNING);
    _error.addCallback("clicked", this, &FLogViewMulti::_logLevelClickCb, LogLevel::LOG_ERROR);

    _lineEditFilter.addCallback("changed", this, &FLogViewMulti::_filterChangedCb);

    _buttonClear.addCallback("clicked", this, &FLogViewMulti::clear);
    
    // finalcut::FListBoxItem item ("deneme-1", 5);
    // finalcut::FListBoxItem item2 ("deneme-2", 10);
    // _dropdownViewSelect.insert(item);
    // _dropdownViewSelect.insert(item2);
}

bool FLogViewMulti::createView(uint_fast16_t viewId, const std::string& viewName) {
    if(_viewMap.size() >= _viewSize) {
        return false;
    }
    
    // create a new log-list with id
    auto [_, result] = _viewMap.try_emplace(viewId, LogList{});
    
    // return false if emplace is not succesful
    if(result) {
        _dropdownViewSelect.insert(viewName, viewId);
        if(_viewMap.size() == 1){
            _activeViewId = viewId;
            _dropdownViewSelect.setCurrentItem(0);
        }
        _dropdownViewSelect.redraw();
    } 
    return result;
}

void FLogViewMulti::setViewSelectText(const std::string& textStr) {
    _dropdownViewSelect.setLabelText(textStr);
}

void FLogViewMulti::log(std::wstring&& logLine, LogLevel logLevel, uint_fast16_t viewId) {
    if(_isPlaying == false){
        return;
    }

    if(static_cast<uint_fast8_t>(logLevel) < static_cast<uint_fast8_t>(_currentLogLevel)){
        return;
    }

    // lock logger view access
    std::lock_guard<std::mutex> lg(_loggerViewMtx);

    // get desired view log-list
    auto it = _viewMap.find(viewId);
    if(it == _viewMap.cend()) {
        return;
    }
    LogList& logList = it->second;

    // get isActive info
    bool isActive = (viewId == _activeViewId) ? true : false;

    _log(logList, std::move(logLine), logLevel, isActive);
}

void FLogViewMulti::clear(void){
    if(_viewMap.size() != 0) {
        // lock logger view access
        std::lock_guard<std::mutex> lg(_loggerViewMtx);
        _viewMap.find(_activeViewId)->second.clear();

        // call base class clear
        FLogViewBase::clear();
    }
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//////////////////////////////////////////////////////////////////////

void FLogViewMulti::initLayout(void) {
    _adjust();
    FLogViewBase::initLayout();
}

void FLogViewMulti::adjustSize(void) {
    _adjust();
    FLogViewBase::adjustSize();
}

void FLogViewMulti::_adjust(void) {
    _dropdownViewSelect.setGeometry({static_cast<int>(getClientWidth()) - 30, 2}, {15, 2});
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//////////////////////////////////////////////////////////////////////

void FLogViewMulti::_logLevelClickCb(LogLevel newLogLevel) {
    if(static_cast<uint_fast8_t>(newLogLevel) > static_cast<uint_fast8_t>(_currentLogLevel)) {
        // lock logger view access
        std::lock_guard<std::mutex> lg(_loggerViewMtx);
        
        // clear logger view
        FLogViewBase::clear();

        // iterate over all loglists
        for(auto viewIter = _viewMap.begin(); viewIter != _viewMap.cend(); viewIter++){
            bool isActive =  (viewIter->first == _activeViewId);
            LogList& logList = viewIter->second;
            for(auto logIter = logList.cbegin(); logIter != logList.cend();) {
                if(static_cast<uint_fast8_t>(logIter->logLevel) < static_cast<uint_fast8_t>(newLogLevel)) {
                    logIter = logList.erase(logIter);
                } else {
                    if(isActive){
                        _printWithSearch(*logIter);
                    }
                    logIter++;
                }
            }
        }
    }
    // call base class cb
    FLogViewBase::_logLevelClickCb(newLogLevel);
}

void FLogViewMulti::_filterChangedCb(void) {    
    // pass active logList for filter
    if(_viewMap.size() != 0){
        // lock logger view access
        std::lock_guard<std::mutex> lg(_loggerViewMtx); 

        _filter(_viewMap.find(_activeViewId)->second);
    }
}
