
#ifndef FW_DIALOG_H
#define FW_DIALOG_H

#include "final/final.h"
#include <mutex>

class FwDialog : public finalcut::FDialog
{
  public:
    explicit FwDialog(finalcut::FWidget* = nullptr, uint_fast16_t = DEFAULT_LOG_WINDOW_SIZE);

    enum class LogLevel : uint_fast8_t {
      LOG_TRACE = 0,
      LOG_INFO,
      LOG_WARNING,
      LOG_ERROR
    };

    void addLog(std::wstring&& logLine, LogLevel logLevel);
    void clearLogs(void);
    void setAutoScroll(bool);

  private:
    void initLayout(void) override;
    void adjustSize(void) override;

    void _formLayout(void);

    void _autoScrollToggleCb(void);
    void _loggerScrollUpCb(void);
    void _playButtonCb(void);
    void _logLevelClickCallback(LogLevel);

    // widgets
    finalcut::FTextView _fwLogger{this};
    finalcut::FSwitch _toggleAutoScroll{L"Auto Scroll", this};
    finalcut::FButton _buttonPlay{"\u23EF ", this};
    finalcut::FLabel _labelPlay{"\U0001F7E2", this};
    finalcut::FButtonGroup _radiobutton_group {"Log Level", this};
    finalcut::FRadioButton _error {"E", &_radiobutton_group};
    finalcut::FRadioButton _warning {"W", &_radiobutton_group};
    finalcut::FRadioButton _info {"I", &_radiobutton_group};
    finalcut::FRadioButton _trace {"T", &_radiobutton_group};

    // data members
    const uint_fast16_t _logSize;
    uint_fast16_t _currentLogSize{0};
    bool _autoScroll{true};
    bool _isPlaying{true};
    LogLevel _currentLogLevel{LogLevel::LOG_INFO};
    std::mutex _loggerViewMtx;


    static constexpr uint_fast16_t DEFAULT_LOG_WINDOW_SIZE = 1000;
};

#endif  // FW_DIALOG_H
