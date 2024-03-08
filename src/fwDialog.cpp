
#include "fwDialog.h"

//////////////////////////////////////////////////////////////////////
//                                                                  //
//////////////////////////////////////////////////////////////////////

FwDialog::FwDialog(finalcut::FWidget* parent, uint_fast16_t logSize) : finalcut::FDialog{parent}, _logSize{logSize}
{
    finalcut::FDialog::setText("FW");
    unsetShadow();
    unsetBorder();
    setResizeable(false);

    _fwLogger.addCallback("mouse-wheel-up", this, &FwDialog::_loggerScrollUpCb);

    _toggleAutoScroll.setChecked(true);
    _toggleAutoScroll.addCallback("toggled", this, &FwDialog::_autoScrollToggleCb);

    _buttonPlay.addCallback("clicked", this, &FwDialog::_playButtonCb);
    _buttonPlay.setForegroundColor(finalcut::FColor::White);
    _buttonPlay.setFocusForegroundColor(finalcut::FColor::White);
    _buttonPlay.setFocus();

    _buttonClear.addCallback("clicked", this, &FwDialog::_clearButtonCb);
    _buttonClear.setForegroundColor(finalcut::FColor::White);
    _buttonClear.setFocusForegroundColor(finalcut::FColor::White);

    _info.setChecked();
    _trace.addCallback("clicked", this, &FwDialog::_logLevelClickCb, LogLevel::LOG_TRACE);
    _info.addCallback("clicked", this, &FwDialog::_logLevelClickCb, LogLevel::LOG_INFO);
    _warning.addCallback("clicked", this, &FwDialog::_logLevelClickCb, LogLevel::LOG_WARNING);
    _error.addCallback("clicked", this, &FwDialog::_logLevelClickCb, LogLevel::LOG_ERROR);

    _lineEditFilter.setLabelText(L"Filter");
    _lineEditFilter.setLabelOrientation(finalcut::FLineEdit::LabelOrientation::Above);
    _lineEditFilter.unsetShadow();
    _lineEditFilter.addCallback("changed", this, &FwDialog::_filterChangedCb);
}

void FwDialog::addLog(std::wstring&& logLine, LogLevel logLevel) {
    bool isShifted {false};
    bool isPrinted {false};
    
    if(_isPlaying == false){
        return;
    }

    if(static_cast<uint_fast8_t>(logLevel) < static_cast<uint_fast8_t>(_currentLogLevel)){
        return;
    }

    // lock logger view access
    std::lock_guard<std::mutex> lg(_loggerViewMtx);

    // rollover if log display is reached
    if(_currentLogSize < _logSize){
        _currentLogSize++;
    } else {
        //remove oldest log from display
        if(_searchString.empty()){
            // if search is not active than remove oldest log
            _fwLogger.deleteLine(0);
            isShifted = true;
        } else {
            // if search is active than remove oldest log only if it matches the search string
            if(_mainLogList.front().logString.find(_searchString) != std::string::npos){
                _fwLogger.deleteLine(0);
                isShifted = true;
            }
        }

        //remove oldest log from main log list
        _mainLogList.pop_front();
    }

    // add log to display if necessary
    if(_searchString.empty()){
        _printLog(logLine, logLevel);
        isPrinted = true;
    } else {
        std::string::size_type pos = logLine.find(_searchString);
        if(pos != std::string::npos){
            _printLog(logLine, logLevel, pos);
            isPrinted = true;
        } 
    }

    if(isPrinted){
        if(_autoScroll){
            _fwLogger.scrollToEnd();
        } else {
            // scroll up by 1 to keep the same view when a new log is added and oldest one removed
            if(isShifted && (_fwLogger.getScrollPos().getY() > 0)) {
                _fwLogger.scrollBy(0, -1);
            }
        }
        _fwLogger.redraw();
    } else {
        if(isShifted){
            _fwLogger.redraw();
        }
    }

    // add log to main list
    _mainLogList.emplace_back(LogItem{logLevel, std::move(logLine)});
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//////////////////////////////////////////////////////////////////////

void FwDialog::initLayout(void) {
    _formLayout();
    finalcut::FDialog::initLayout();
}

void FwDialog::adjustSize(void) {
    _formLayout();
    finalcut::FDialog::adjustSize();
}

void FwDialog::_formLayout(void)
{
    // dialog
    finalcut::FPoint fwPosition{1,1};
    finalcut::FSize fwSize{finalcut::FApplication::getApplicationObject()->getDesktopWidth(), finalcut::FApplication::getApplicationObject()->getDesktopHeight()};
    setGeometry(fwPosition, fwSize);

    // play/pause button
    _buttonPlay.setGeometry(finalcut::FPoint{3,2}, finalcut::FSize{8, 1});

    // play/pause indicator
    _labelPlay.setGeometry(finalcut::FPoint{13,2}, finalcut::FSize{3, 1});

    // clear button
    _buttonClear.setGeometry(finalcut::FPoint{17,2}, finalcut::FSize{7, 1});

    // button group
    _radiobutton_group.setGeometry(finalcut::FPoint{27,1}, finalcut::FSize{29, 3});
    _error.setGeometry(finalcut::FPoint{1,1}, finalcut::FSize{6, 1});
    _warning.setGeometry(finalcut::FPoint{8,1}, finalcut::FSize{6, 1});
    _info.setGeometry(finalcut::FPoint{15,1}, finalcut::FSize{6, 1});
    _trace.setGeometry(finalcut::FPoint{22,1}, finalcut::FSize{6, 1});

    // filter box
    _lineEditFilter.setGeometry(finalcut::FPoint{58,2}, finalcut::FSize{16, 3});

    // auto scroll toggle
    finalcut::FPoint togglePosition{static_cast<int>(getClientWidth()) - 14, 1};
    finalcut::FSize toggleSize{15, 3};
    _toggleGroup.setGeometry(togglePosition, toggleSize);
    _toggleAutoScroll.setGeometry(finalcut::FPoint{2,1}, finalcut::FSize{10, 1});

    // logger view
    finalcut::FPoint loggerPosition{1,4};
    finalcut::FSize loggerSize{getClientWidth(), getClientHeight() - 3};
    _fwLogger.setGeometry(loggerPosition, loggerSize);
}

void FwDialog::_printLog(const std::wstring& logLine, LogLevel logLevel, std::string::size_type hglPos){
    finalcut::FString logLineFstring{logLine};

    switch (logLevel) {
    case LogLevel::LOG_TRACE:
        logLineFstring.insert(L"[TRACE]   ", 0);
        _fwLogger.append(logLineFstring);    
        _fwLogger.addHighlight(_fwLogger.getLines().size()-1, finalcut::FTextView::FTextHighlight{0, 7, finalcut::FColorPair{finalcut::FColor::White, finalcut::FColor::LightGray}});
        break;

    case LogLevel::LOG_INFO:
        logLineFstring.insert(L"[INFO]    ", 0);
        _fwLogger.append(logLineFstring);    
        _fwLogger.addHighlight(_fwLogger.getLines().size()-1, finalcut::FTextView::FTextHighlight{0, 6, finalcut::FColorPair{finalcut::FColor::White, finalcut::FColor::Blue}});
        break;

    case LogLevel::LOG_WARNING:
        logLineFstring.insert(L"[WARNING] ", 0);
        _fwLogger.append(logLineFstring);    
        _fwLogger.addHighlight(_fwLogger.getLines().size()-1, finalcut::FTextView::FTextHighlight{0, 9, finalcut::FColorPair{finalcut::FColor::White, finalcut::FColor::DarkOrange}});
        break;

    case LogLevel::LOG_ERROR:
        logLineFstring.insert(L"[ERROR]   ", 0);
        _fwLogger.append(logLineFstring);    
        _fwLogger.addHighlight(_fwLogger.getLines().size()-1, finalcut::FTextView::FTextHighlight{0, 7, finalcut::FColorPair{finalcut::FColor::White, finalcut::FColor::Red}});
        break;

    default:
        _fwLogger.append(L"[UNDEF]    PRINT LOG ATTEMPT WITH UNKNOWN LOG TYPE");    
        _fwLogger.addHighlight(_fwLogger.getLines().size()-1, finalcut::FTextView::FTextHighlight{0, 7, finalcut::FColorPair{finalcut::FColor::White, finalcut::FColor::Purple}});
        return;
    }

    if(hglPos != std::string::npos){
        _fwLogger.addHighlight(_fwLogger.getLines().size()-1, finalcut::FTextView::FTextHighlight{hglPos + 10, _searchString.length(), finalcut::FColorPair{finalcut::FColor::Black, finalcut::FColor::Yellow}});
    }
}

void FwDialog::_filter(void){

}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//////////////////////////////////////////////////////////////////////

void FwDialog::_autoScrollToggleCb(void) {
    _autoScroll = _toggleAutoScroll.isChecked();
}

void FwDialog::_loggerScrollUpCb(void) {
    if(_autoScroll){
        _autoScroll = false;
        _toggleAutoScroll.setChecked(false);
        _toggleAutoScroll.redraw();
    }
}

void FwDialog::_playButtonCb(void){
    if(_isPlaying){
        _isPlaying = false;
        _labelPlay.setText(finalcut::FString{std::wstring{L"\U0001F534"}});
    } else {
        _isPlaying = true;
        _labelPlay.setText(finalcut::FString{std::wstring{L"\U0001F7E2"}});
    }
    _labelPlay.redraw();
}

void FwDialog::_clearButtonCb(void){
    // lock logger view access
    std::lock_guard<std::mutex> lg(_loggerViewMtx);
    _fwLogger.clear();
    _mainLogList.clear();
    _currentLogSize = 0;
}

void FwDialog::_logLevelClickCb(LogLevel newLogLevel) {
    if(newLogLevel != _currentLogLevel) {
        if(static_cast<uint_fast8_t>(newLogLevel) > static_cast<uint_fast8_t>(_currentLogLevel)) {
            bool emptyFlag = _searchString.empty();
            
            // lock logger view access
            std::lock_guard<std::mutex> lg(_loggerViewMtx);
            
            // clear logger view
            _fwLogger.clear();

            // remove lower level logs and print others
            for(auto it = _mainLogList.cbegin(); it != _mainLogList.cend();) {
                if(static_cast<uint_fast8_t>(it->logLevel) < static_cast<uint_fast8_t>(newLogLevel)) {
                    it = _mainLogList.erase(it);
                } else {
                    if(emptyFlag) {
                        _printLog(it->logString, it->logLevel);
                    } else {
                        std::string::size_type pos = it->logString.find(_searchString);
                        if(pos != std::string::npos){
                            _printLog(it->logString, it->logLevel, pos);
                        }
                    }
                    it++;
                }
            }

            // scroll if needed and redraw logger view
            if(_autoScroll){
                _fwLogger.scrollToEnd();
            }
            _fwLogger.redraw();
        }

        _currentLogLevel = newLogLevel;
    }
}

void FwDialog::_filterChangedCb(void){
    bool clearFlag{false};

    // lock logger view access
    std::lock_guard<std::mutex> lg(_loggerViewMtx);
    
    _searchString = _lineEditFilter.getText().toWString();
    if(_searchString == L" "){
        _searchString.clear();
        clearFlag = true;
    }
    
    // clear logger
    _fwLogger.clear();

    // search main list and print matching logs
    for(auto it = _mainLogList.cbegin(); it != _mainLogList.cend(); it++){
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
    if(_autoScroll){
        _fwLogger.scrollToEnd();
    }
    _fwLogger.redraw();
}
