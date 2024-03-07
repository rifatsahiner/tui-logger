
#include <cstdint>
#include <thread>

#include "final/final.h"
#include "fwDialog.h"

void runInThread(void) {  
  finalcut::FApplication::getApplicationObject()->exec();
}

void editInThread(FwDialog* ref) {
  using namespace std::chrono_literals;
  uint16_t i;

  for(i = 0; i < 500; i++)
  {
    std::wstring tempStr{L"this is log " + std::to_wstring(i)};

    switch (i % 4)
    {
    case 0:
      tempStr += L" trace";
      ref->addLog(std::move(tempStr), FwDialog::LogLevel::LOG_TRACE);
      break;
    
    case 1:
      tempStr += L" info";
      ref->addLog(std::move(tempStr), FwDialog::LogLevel::LOG_INFO);
      break;

    case 2:
      tempStr += L" warning";
      ref->addLog(std::move(tempStr), FwDialog::LogLevel::LOG_WARNING);
      break;

    case 3:
      tempStr += L" error";
      ref->addLog(std::move(tempStr), FwDialog::LogLevel::LOG_ERROR);
      break;
    }

    std::this_thread::sleep_for(30ms);
  }

  std::this_thread::sleep_for(20s);

}

int main (int argc, char* argv[])
{
  // uint8_t i;

  // Create the application object
  finalcut::FApplication app(argc, argv);

  // Force terminal initialization without calling show()
  app.initTerminal();

  // create fw-dialog
  FwDialog fwDialog(&app, 300);
  finalcut::FWidget::setMainWidget(&fwDialog);
  fwDialog.show();

  std::thread independentThread(runInThread);
  std::thread independentThread2(editInThread, &fwDialog);

  independentThread2.join();

  finalcut::FApplication::getApplicationObject()->quit();
  independentThread.join();
  
  return 0;
}

