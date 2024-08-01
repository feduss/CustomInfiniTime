#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>
#include <FreeRTOS.h>
#include "portmacro_cmsis.h"
#include "systemtask/SystemTask.h"
#include "components/motor/MotorController.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      enum class TimerStates { Init, Running, Expired };
      enum class TimerTypes { First, Second };

      class DoubleTimer : public Screen {
      public:

        DoubleTimer(Controllers::MotorController& motorController, System::SystemTask& systemTask);
        ~DoubleTimer() override;
        void Refresh() override;

        struct Time {
          int mins;
          int secs;
          int ms;
        };

        //Events that handler the play and stop of the first timer
        void playTimerEventHandler(TimerTypes timerType);
        void stopTimerEventHandler(TimerTypes timerType);

        TimerStates getFirstTimerState();
        TimerStates getSecondTimerState();

        void updateTimer(TimerTypes timerType, lv_obj_t* label, TickType_t startTimer, TickType_t stopTimer, const int timeInSeconds);
        void resetTimer(TimerTypes timerType);

      private:
        Pinetime::Controllers::MotorController& motorController;
        Pinetime::System::SystemTask& systemTask;
        TimerStates firstTimerState = TimerStates::Init;
        TimerStates secondTimerState = TimerStates::Init;
        lv_obj_t *appTitleLabel;
        lv_obj_t *firstTimerPlayStopBtn, *firstTimerIcon, *firstTimerLabel;
        lv_obj_t *secondTimerPlayStopBtn, *secondTimerIcon, *secondTimerLabel;

        lv_task_t* taskRefresh;

        const int appVersionMajor = 1;
        const int appVersionMinor = 0;
        const int appVersionPatch = 5;

        const int firstTimerMinutes = 1;
        const int firstTimerSeconds = 30;
        const int firstTimerInSeconds = (firstTimerMinutes * 60) + firstTimerSeconds;
        TickType_t startFirstTimer;
        TickType_t stopFirstTimer;

        const int secondTimerMinutes = 3;
        const int secondTimerSeconds = 0;
        const int secondTimerInSeconds = (secondTimerMinutes * 60) + secondTimerSeconds;
        TickType_t startSecondTimer;
        TickType_t stopSecondTimer;
      };
    }	

    template <>
    struct AppTraits<Apps::DoubleTimer> {
      static constexpr Apps app = Apps::DoubleTimer;
      static constexpr const char* icon = Screens::Symbols::stopWatch;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::DoubleTimer(controllers.motorController, *controllers.systemTask);
      };
    };
  }
}

