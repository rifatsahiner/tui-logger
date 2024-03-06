
#include "fwDialog.h"

static inline uint_fast8_t getLogLevelFromChar(wchar_t);
static inline wchar_t getChFromLogLevel(FwDialog::LogLevel);

//////////////////////////////////////////////////////////////////////
//                                                                  //
//////////////////////////////////////////////////////////////////////

FwDialog::FwDialog(finalcut::FWidget* parent, uint_fast16_t logSize) : finalcut::FDialog{parent}, _logSize{logSize}
{
    finalcut::FDialog::setText("FW");
    unsetShadow();
    unsetBorder();

    _fwLogger.addCallback("mouse-wheel-up", this, &FwDialog::_loggerScrollUpCb);

    _toggleAutoScroll.setChecked(true);
    _toggleAutoScroll.addCallback("toggled", this, &FwDialog::_autoScrollToggleCb);

    _buttonPlay.addCallback("clicked", this, &FwDialog::_playButtonCb);

    _info.setChecked();
    _trace.addCallback("clicked", this, &FwDialog::_logLevelClickCallback, LogLevel::LOG_TRACE);
    _info.addCallback("clicked", this, &FwDialog::_logLevelClickCallback, LogLevel::LOG_INFO);
    _warning.addCallback("clicked", this, &FwDialog::_logLevelClickCallback, LogLevel::LOG_WARNING);
    _error.addCallback("clicked", this, &FwDialog::_logLevelClickCallback, LogLevel::LOG_ERROR);
}

void FwDialog::addLog(std::wstring&& logLine, LogLevel logLevel) {
    bool isOverflow {false};
    
    if(_isPlaying == false){
        return;
    }

    if(static_cast<uint_fast8_t>(logLevel) < static_cast<uint_fast8_t>(_currentLogLevel)){
        return;
    }

    std::lock_guard<std::mutex> lg(_loggerViewMtx);

    // rollover if log display is reached
    if(_currentLogSize < _logSize){
        _currentLogSize++;
    } else {
        isOverflow = true;
        _fwLogger.deleteLine(0);
    }
    
    switch (logLevel) {
    case LogLevel::LOG_TRACE:
        logLine.insert(0, L"[TRACE]   ");
        _fwLogger.append(finalcut::FString{std::move(logLine)});    
        _fwLogger.addHighlight(_fwLogger.getLines().size()-1, finalcut::FTextView::FTextHighlight{0, 7, finalcut::FColorPair{finalcut::FColor::White, finalcut::FColor::LightGray}});
        break;

    case LogLevel::LOG_INFO:
        logLine.insert(0, L"[INFO]    ");
        _fwLogger.append(finalcut::FString{std::move(logLine)});    
        _fwLogger.addHighlight(_fwLogger.getLines().size()-1, finalcut::FTextView::FTextHighlight{0, 6, finalcut::FColorPair{finalcut::FColor::White, finalcut::FColor::Blue}});
        break;

    case LogLevel::LOG_WARNING:
        logLine.insert(0, L"[WARNING] ");
        _fwLogger.append(finalcut::FString{std::move(logLine)});    
        _fwLogger.addHighlight(_fwLogger.getLines().size()-1, finalcut::FTextView::FTextHighlight{0, 9, finalcut::FColorPair{finalcut::FColor::White, finalcut::FColor::DarkOrange}});
        break;

    case LogLevel::LOG_ERROR:
        logLine.insert(0, L"[ERROR]   ");
        _fwLogger.append(finalcut::FString{std::move(logLine)});    
        _fwLogger.addHighlight(_fwLogger.getLines().size()-1, finalcut::FTextView::FTextHighlight{0, 7, finalcut::FColorPair{finalcut::FColor::White, finalcut::FColor::Red}});
        break;

    default:
        _fwLogger.append(L"[UNDEF]    PRINT LOG ATTEMPT WITH UNKNOWN LOG TYPE");    
        _fwLogger.addHighlight(_fwLogger.getLines().size()-1, finalcut::FTextView::FTextHighlight{0, 7, finalcut::FColorPair{finalcut::FColor::White, finalcut::FColor::Purple1}});
        break;
    }

    if(_autoScroll){
        _fwLogger.scrollToEnd();
    } else {
        // scroll up by 1 to keep the same view when a new log is added and oldest one removed
        if(isOverflow && (_fwLogger.getScrollPos().getY() > 0)) {
            _fwLogger.scrollBy(0, -1);
        }
    }

    //redraw();
    _fwLogger.redraw();
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
    _buttonPlay.setGeometry(finalcut::FPoint{3,2}, finalcut::FSize{4, 1});

    // play/pause indicator
    _labelPlay.setGeometry(finalcut::FPoint{9,2}, finalcut::FSize{3, 1});

    // button group
    _radiobutton_group.setGeometry(finalcut::FPoint{16,1}, finalcut::FSize{30, 3});
    _error.setGeometry(finalcut::FPoint{1,1}, finalcut::FSize{6, 1});
    _warning.setGeometry(finalcut::FPoint{8,1}, finalcut::FSize{6, 1});
    _info.setGeometry(finalcut::FPoint{15,1}, finalcut::FSize{6, 1});
    _trace.setGeometry(finalcut::FPoint{22,1}, finalcut::FSize{6, 1});

    // auto scroll toggle
    finalcut::FPoint togglePosition{getClientWidth() - 23, 2};
    finalcut::FSize toggleSize{22, 1};
    _toggleAutoScroll.setGeometry(togglePosition, toggleSize);

    // logger view
    finalcut::FPoint loggerPosition{1,4};
    finalcut::FSize loggerSize{getClientWidth(), getClientHeight() - 3};
    _fwLogger.setGeometry(loggerPosition, loggerSize);
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

void FwDialog::_logLevelClickCallback(LogLevel newLogLevel)
{
    if(newLogLevel == _currentLogLevel){
        return;
    } else {
        std::lock_guard<std::mutex> lg(_loggerViewMtx);

        if(static_cast<uint_fast8_t>(newLogLevel) > static_cast<uint_fast8_t>(_currentLogLevel)){
            // düşük seviye logları sil
            uint_fast16_t i;
            //std::lock_guard<std::mutex> lg(_loggerViewMtx);
            
            for(i = _fwLogger.getLines().size(); i > 0; i--){
                uint_fast8_t lineLevel = getLogLevelFromChar(_fwLogger.getLine(i-1).text[1]);
                if(lineLevel < static_cast<uint_fast8_t>(newLogLevel)){
                    _fwLogger.deleteLine(i-1);
                }
            }

            //adjustSize();
            //redraw();
            //finalcut::FDialog::adjustSize();
        }

        std::wstring indication {L"[LOGGER]  LOG LEVEL CHANGED TO "};
        indication.push_back(getChFromLogLevel(newLogLevel));
        _fwLogger.append(finalcut::FString{std::move(indication)});    
        _fwLogger.addHighlight(_fwLogger.getLines().size()-1, finalcut::FTextView::FTextHighlight{0, 8, finalcut::FColorPair{finalcut::FColor::White, finalcut::FColor::Green}});

        _currentLogLevel = newLogLevel;
        _fwLogger.redraw();
    }
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
//////////////////////////////////////////////////////////////////////

uint_fast8_t getLogLevelFromChar(wchar_t ch){
    switch (ch)
    {
    case L'E':
        return static_cast<uint_fast8_t>(FwDialog::LogLevel::LOG_ERROR);
    case L'W':
        return static_cast<uint_fast8_t>(FwDialog::LogLevel::LOG_WARNING);
    case L'I':
        return static_cast<uint_fast8_t>(FwDialog::LogLevel::LOG_INFO);
    case L'T':
        return static_cast<uint_fast8_t>(FwDialog::LogLevel::LOG_TRACE);
    case L'L':
    case L'U':
        return (static_cast<uint_fast8_t>(FwDialog::LogLevel::LOG_ERROR) + 1);
    default:
        assert(false);
    }
}

wchar_t getChFromLogLevel(FwDialog::LogLevel logLevel){
    switch (logLevel)
    {
    case FwDialog::LogLevel::LOG_ERROR:
        return L'E';
    case FwDialog::LogLevel::LOG_WARNING:
        return L'W';
    case FwDialog::LogLevel::LOG_INFO:
        return L'I';
    case FwDialog::LogLevel::LOG_TRACE:
        return L'T';
    default:
        assert(false);
    }
}
