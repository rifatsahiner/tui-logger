
#include <cstdint>
#include <thread>

#include "flogview.h"

void runInThread(void) {  
  finalcut::FApplication::getApplicationObject()->exec();
}

void editInThread(FLogView* ref) {
  using namespace std::chrono_literals;
  uint16_t i;

  for(i = 0; i < 500; i++)
  {
    std::wstring tempStr{L"this is log " + std::to_wstring(i)};

    switch (i % 4)
    {
    case 0:
      tempStr += L" trace";
      ref->log(std::move(tempStr), FLogView::LogLevel::LOG_TRACE);
      break;
    
    case 1:
      tempStr += L" info";
      ref->log(std::move(tempStr), FLogView::LogLevel::LOG_INFO);
      break;

    case 2:
      tempStr += L" warning";
      ref->log(std::move(tempStr), FLogView::LogLevel::LOG_WARNING);
      break;

    case 3:
      tempStr += L" error";
      ref->log(std::move(tempStr), FLogView::LogLevel::LOG_ERROR);
      break;
    }

    std::this_thread::sleep_for(40ms);
  }

  std::this_thread::sleep_for(10s);

}

int main (int argc, char* argv[])
{
  // uint8_t i;

  // Create the application object
  finalcut::FApplication app(argc, argv);

  // Force terminal initialization without calling show()
  app.initTerminal();

  // create fw-dialog
  FLogView logger(&app, 300);
  finalcut::FWidget::setMainWidget(&logger);
  logger.setText(L"FW");
  logger.unsetShadow();
  //FLogView.unsetBorder();
  logger.setResizeable(true);
  finalcut::FPoint fwPosition{1,1};
  finalcut::FSize fwSize{app.getDesktopWidth(), app.getDesktopHeight()};
  logger.setGeometry(fwPosition, fwSize);
  logger.show();

  std::thread independentThread(runInThread);
  std::thread independentThread2(editInThread, &logger);

  independentThread2.join();

  finalcut::FApplication::getApplicationObject()->quit();
  independentThread.join();
  
  return 0;
}

