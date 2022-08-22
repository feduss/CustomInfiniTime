#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>
#include <FreeRTOS.h>
#include "portmacro_cmsis.h"
#include "systemtask/SystemTask.h"

namespace Pinetime::Applications::Screens {

  enum class TimerStates { Init, Running, Halted };
  enum class TimerTypes { First, Second };

  class DoubleStopWatch : public Screen {
  public:

    DoubleStopWatch(DisplayApp* app, System::SystemTask& systemTask);
    ~DoubleStopWatch() override;
    void Refresh() override;
    bool OnButtonPushed() override;

    //Events that handler the play and stop of the first timer
    void playTimerEventHandler(lv_event_t event, TimerTypes timerType);
    void stopTimerEventHandler(lv_event_t event, TimerTypes timerType);

    TimerStates getFirstTimerState();
    TimerStates getSecondTimerState();

    void updateTimer(lv_obj_t* label, TickType_t startTimer, TickType_t stopTimer, const int timeInSeconds);
    void resetTimer(TimerTypes timerType);

  private:
    Pinetime::System::SystemTask& systemTask;
    TimerStates firstTimerState = TimerStates::Init;
    TimerStates secondTimerState = TimerStates::Init;
    lv_obj_t *firstTimerPlayStopBtn, *firstTimerIcon, *firstTimerLabel;
    lv_obj_t *secondTimerPlayStopBtn, *secondTimerIcon, *secondTimerLabel;

    lv_task_t* taskRefresh;

    const int firstTimerMinutes = 0;
    const int firstTimerSeconds = 5;
    const int firstTimerInSeconds = (firstTimerMinutes * 60) + firstTimerSeconds;
    TickType_t startFirstTimer;
    TickType_t stopFirstTimer;

    const int secondTimerMinutes = 0;
    const int secondTimerSeconds = 7;
    const int secondTimerInSeconds = (secondTimerMinutes * 60) + secondTimerSeconds;
    TickType_t startSecondTimer;
    TickType_t stopSecondTimer;
  };
}
