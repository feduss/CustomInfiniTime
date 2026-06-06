#pragma once

#include "displayapp/screens/Screen.h"
#include "Symbols.h"
#include <lvgl/lvgl.h>
#include <FreeRTOS.h>
#include "systemtask/SystemTask.h"
#include "components/motor/MotorController.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      enum class TimerStates { Init, Running };
      enum class TimerTypes { First, Second };

      class DoubleTimer : public Screen {
      public:

        DoubleTimer(
          Controllers::MotorController& motorController, 
          System::SystemTask& systemTask
        );
        ~DoubleTimer() override;
        void Refresh() override;

        struct Time {
          int mins;
          int secs;
          int ms;
        };

        void setupViews();
        void setupBindings();
        void firstReactionShock();
        //Events that handler the play and stop of the first timer
        void playTimerEventHandler(TimerTypes timerType);
        void stopTimerEventHandler(TimerTypes timerType);

        TimerStates getFirstTimerState();
        TimerStates getSecondTimerState();

        static void onFirstBtnPressed(lv_obj_t* obj, lv_event_t event);
        static void onSecondBtnPressed(lv_obj_t* obj, lv_event_t event);

        void updateTimer(TimerTypes timerType);
        void resetTimer(TimerTypes timerType);
        bool isTimerActive();

        static Time convertTicksToTimeSegments(const TickType_t timeElapsed);

        bool OnButtonPushed() override;
        bool OnTouchEvent(Pinetime::Applications::TouchEvents event) override;

        void DeleteRefreshTask();
        void enableScreenSleeping();
        void disableScreenSleeping();
        void prepareAppToExit();

      private:
        Pinetime::Controllers::MotorController& motorController;
        Pinetime::System::SystemTask& systemTask;
        TimerStates firstTimerState = TimerStates::Init;
        TimerStates secondTimerState = TimerStates::Init;
        lv_obj_t *appTitleLabel;
        lv_obj_t *firstTimerPlayStopBtn, *firstTimerIcon, *firstTimerLabel;
        lv_obj_t *secondTimerPlayStopBtn, *secondTimerIcon, *secondTimerLabel;

        lv_task_t* taskRefresh = nullptr;

        const int appVersionMajor = 1;
        const int appVersionMinor = 0;
        const int appVersionPatch = 7;

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

        TickType_t blinkTime = 0;

        bool isExiting = false;
      };
    }	

    template <>
    struct AppTraits<Apps::DoubleTimer> {
      static constexpr Apps app = Apps::DoubleTimer;
      static constexpr const char* icon = Screens::Symbols::stopWatch;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::DoubleTimer(
          controllers.motorController, 
          *controllers.systemTask
        );
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}

