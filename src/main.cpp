
#include <cstdint>
#include <thread>

#include "flogview.h"

volatile bool g_quit{false};

void quitCb(void) {
  g_quit = true;
  finalcut::FApplication::getApplicationObject()->quit();
}

void runInThread(void) {  
  finalcut::FApplication::getApplicationObject()->exec();
}

void editInThread(FLogViewMulti* ref) {
  using namespace std::chrono_literals;
  uint16_t i;

  for(i = 0; i < 400; i++)
  {
    std::wstring tempStr{L"this is log " + std::to_wstring(i)};

    switch (i % 4)
    {
    case 0:
      tempStr += L" trace";
      ref->log(std::move(tempStr), FLogView::LogLevel::LOG_TRACE, 1);
      break;
    
    case 1:
      tempStr += L" info";
      ref->log(std::move(tempStr), FLogView::LogLevel::LOG_INFO, 3);
      break;

    case 2:
      tempStr += L" warning";
      ref->log(std::move(tempStr), FLogView::LogLevel::LOG_WARNING, 2);
      break;

    case 3:
      tempStr += L" error";
      ref->log(std::move(tempStr), FLogView::LogLevel::LOG_ERROR, 1);
      break;
    }

    if(g_quit)
      break;

    std::this_thread::sleep_for(40ms);
  }

  if(g_quit == false){
    std::this_thread::sleep_for(5s);
    ref->removeView(3);
    std::this_thread::sleep_for(3s);
    ref->removeView(2);
    std::this_thread::sleep_for(3s);
    ref->removeView(1);
    std::this_thread::sleep_for(10s);
  }
  g_quit = true;
}

int main (int argc, char* argv[])
{
  // Create the application object
  finalcut::FApplication app(argc, argv);

  // Force terminal initialization without calling show()
  app.initTerminal();

  // create fw-dialog
  FLogViewMulti logger(&app, 300);
  finalcut::FWidget::setMainWidget(&logger);
  logger.setText(L"Logger");
  logger.unsetShadow();
  //logger.unsetBorder();

  logger.setResizeable(true);
  logger.setMinimizable(true);
  logger.registerOnQuit(&quitCb);

  logger.setViewSelectText("Task list");
  logger.createView(1, "deneme-task");
  logger.createView(2, "try-task");
  logger.createView(3, "test-task");

  // modal denenecek
  // hide denenecek

  finalcut::FPoint fwPosition{1,1};
  finalcut::FSize fwSize{app.getDesktopWidth(), app.getDesktopHeight()};
  logger.setGeometry(fwPosition, fwSize);
  logger.show();

  std::thread independentThread(runInThread);
  std::thread independentThread2(editInThread, &logger);

  independentThread2.join();

  quitCb();
  independentThread.join();
  
  return 0;
}

