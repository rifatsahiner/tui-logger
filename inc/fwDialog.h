
#ifndef FW_DIALOG_H
#define FW_DIALOG_H

#include "final/final.h"
#include <mutex>

class FwDialog : public finalcut::FDialog
{
  public:
    explicit FwDialog(finalcut::FWidget* = nullptr, uint_fast16_t scrollBackLimit = DEFAULT_LOG_BUFFER_SIZE);

    enum class LogLevel : uint_fast8_t {
      LOG_TRACE = 0,
      LOG_INFO,
      LOG_WARNING,
      LOG_ERROR
    };

    void addLog(std::wstring&& logLine, LogLevel logLevel);

  private:
    void initLayout(void) override;
    void adjustSize(void) override;

    void _formLayout(void);
    void _printLog(const std::wstring&, LogLevel, std::string::size_type = std::string::npos);
    void _filter(void);

    void _autoScrollToggleCb(void);
    void _loggerScrollUpCb(void);
    void _playButtonCb(void);
    void _clearButtonCb(void);
    void _logLevelClickCb(LogLevel);
    void _filterChangedCb(void);

    // widgets
    finalcut::FTextView _fwLogger{this};
    finalcut::FButton _buttonPlay{L"▶/▮▮", this};
    finalcut::FLabel _labelPlay{L"🟢", this};
    finalcut::FButton _buttonClear{L"Clear", this};
    finalcut::FButtonGroup _radiobutton_group {L"Log Level", this};
    finalcut::FRadioButton _error {"E", &_radiobutton_group};
    finalcut::FRadioButton _warning {"W", &_radiobutton_group};
    finalcut::FRadioButton _info {"I", &_radiobutton_group};
    finalcut::FRadioButton _trace {"T", &_radiobutton_group};
    finalcut::FLineEdit _lineEditFilter {this};
    finalcut::FButtonGroup _toggleGroup {L"Auto Scroll", this};
    finalcut::FSwitch _toggleAutoScroll{&_toggleGroup};

    // data members
    const uint_fast16_t _logSize;
    uint_fast16_t _currentLogSize{0};
    bool _autoScroll{true};
    bool _isPlaying{true};
    std::wstring _searchString;
    LogLevel _currentLogLevel{LogLevel::LOG_INFO};
    std::mutex _loggerViewMtx;

    struct LogItem {
      LogLevel logLevel;
      std::wstring logString;
    };
    std::list<LogItem> _mainLogList;

    static constexpr uint_fast16_t DEFAULT_LOG_BUFFER_SIZE = 1000;
};

// append with FString r-value referance
// moving a whole FTextViewList - this will call clear and than use new text list
// FSwitch text orientation olmalı

#endif  // FW_DIALOG_H
