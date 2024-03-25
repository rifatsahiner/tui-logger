
#ifndef F_LOG_VIEW_H
#define F_LOG_VIEW_H

#include "final/final.h"

class FLogViewBase : public finalcut::FDialog
{
  public:
    explicit FLogViewBase(finalcut::FWidget*, uint_fast16_t);
    virtual ~FLogViewBase() {};

    void registerOnQuit(std::function<void(void)>);

    enum class LogLevel : uint_fast8_t {
      LOG_TRACE = 0,
      LOG_INFO,
      LOG_WARNING,
      LOG_ERROR
    };

  private:
    void _printLog(const std::wstring&, LogLevel, std::string::size_type = std::string::npos);
    void _adjustView(bool isPrinted, bool isShifted);
    void _adjust(void);

    void _autoScrollToggleCb(void);
    void _loggerScrollUpCb(void);
    void _playButtonCb(void);

    // overriden FDialog functions
    void onClose (finalcut::FCloseEvent*) override;
    void onKeyPress (finalcut::FKeyEvent*) override;

    // widgets
    finalcut::FTextView _textView{this};
    finalcut::FButton _buttonPlay{L"▶/▮▮", this};
    finalcut::FLabel _labelPlay{L"🟢", this};
    finalcut::FButtonGroup _radiobutton_group {L"Log Level", this};
    finalcut::FButtonGroup _toggleGroup {L"Auto Scroll", this};
    finalcut::FSwitch _toggleAutoScroll{&_toggleGroup};

    // data members
    bool _autoScroll{true};
    std::wstring _searchString;
    std::function<void(void)> _quitCb;

  protected:
    struct LogItem {
      LogLevel logLevel;
      std::wstring logString;
    };
    using LogList = std::list<LogItem>;

    // overriden FDialog functions
    void initLayout(void) override;
    void adjustSize(void) override;

    // functions should be overriden by single and multi views (for single LogList or multiple)
    virtual void clear(void);

    virtual void _logLevelClickCb(LogLevel);
    virtual void _filterChangedCb(void) = 0;

    // protected functions for single and multi
    void _filter(const LogList&);
    void _log(LogList&, std::wstring&&, LogLevel, bool);
    void _printWithSearch(const LogItem&);

    std::mutex _loggerViewMtx;
    LogLevel _currentLogLevel{LogLevel::LOG_INFO};
    bool _isPlaying{true};
    const uint_fast16_t _logSize;

    finalcut::FLineEdit _lineEditFilter {this};
    finalcut::FRadioButton _error {"E", &_radiobutton_group};
    finalcut::FRadioButton _warning {"W", &_radiobutton_group};
    finalcut::FRadioButton _info {"I", &_radiobutton_group};
    finalcut::FRadioButton _trace {"T", &_radiobutton_group};
    finalcut::FButton _buttonClear{L"Clear", this};

    static constexpr uint_fast16_t DEFAULT_LOG_BUFFER_SIZE = 2000;
};

class FLogView : public FLogViewBase {
  public:
    explicit FLogView(finalcut::FWidget* parent = nullptr, uint_fast16_t scrollBackLimit = DEFAULT_LOG_BUFFER_SIZE);

    void log(std::wstring&& logLine, LogLevel logLevel);
    void clear(void) override;

  private:
    void _logLevelClickCb(LogLevel) override;
    void _filterChangedCb(void) override;

    LogList _mainLogList;
};

class FLogViewMulti : public FLogViewBase {
  public:
    explicit FLogViewMulti(finalcut::FWidget* parent = nullptr, uint_fast16_t scrollBackLimit = DEFAULT_LOG_BUFFER_SIZE, uint_fast16_t viewLimit = DEFAULT_MAX_VIEW_COUNT);

    void log(std::wstring&& logLine, LogLevel logLevel, uint_fast16_t viewId);
    void clear(void) override;
    bool createView(uint_fast16_t viewId, const std::string& viewName);
    void removeView(uint_fast16_t viewId);
    void setViewSelectText(const std::string&);

  private:
    void initLayout(void) override; 
    void adjustSize(void) override; 

    void _adjust(void);

    void _logLevelClickCb(LogLevel) override;
    void _filterChangedCb(void) override;

    finalcut::FComboBox _dropdownViewSelect{this};
    
    std::unordered_map<uint_fast16_t, LogList> _viewMap;
    uint_fast16_t _activeViewId;
    const uint_fast16_t _viewSize;

    static constexpr uint_fast16_t DEFAULT_MAX_VIEW_COUNT = 200;
};

// append with FString r-value referance
// moving a whole FTextViewList - this will call clear and than use new text list
// FSwitch text orientation olmalı
// Dialog menüsünden close veya diğerleri kaldırılabilmeli, custom olarak hazırlanabilmeli

#endif  // F_LOG_VIEW_H
