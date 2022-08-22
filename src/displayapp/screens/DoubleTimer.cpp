
#include "displayapp/screens/DoubleTimer.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

const int convertTicksToTimeSeconds(const TickType_t timeElapsed) {
    // Centiseconds
    const int timeElapsedCentis = timeElapsed * 100 / configTICK_RATE_HZ;

    const int secs = (timeElapsedCentis / 100) % 60;
    const int mins = (timeElapsedCentis / 100) / 60;
    return (mins * 60) + secs;
  }

void onFirstBtnPressed(lv_obj_t* obj, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) {
    return;
    }
    auto* stopWatch = static_cast<DoubleTimer*>(obj->user_data);
    if (stopWatch->getFirstTimerState() == TimerStates::Running) {
        stopWatch->stopTimerEventHandler(event, TimerTypes::First);
    } else {
        stopWatch->playTimerEventHandler(event, TimerTypes::First);
    }
    
}

void onSecondBtnPressed(lv_obj_t* obj, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) {
    return;
    }
    auto* stopWatch = static_cast<DoubleTimer*>(obj->user_data);
    if (stopWatch->getSecondTimerState() == TimerStates::Running) {
        stopWatch->stopTimerEventHandler(event, TimerTypes::Second);
    } else {
        stopWatch->playTimerEventHandler(event, TimerTypes::Second);
    }
    
}

//Constructor
DoubleTimer::DoubleTimer(DisplayApp* app, Controllers::MotorController& motorController, System::SystemTask& systemTask) 
: Screen(app), motorController {motorController}, systemTask {systemTask} {

  /*** UI FIRST SETUP***/
  firstTimerLabel = lv_label_create(lv_scr_act(), nullptr); //Label creation
  lv_obj_set_style_local_text_font(firstTimerLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20); // Label font init
  resetTimer(TimerTypes::First); // Label text init
  lv_obj_align(firstTimerLabel, lv_scr_act(), LV_ALIGN_CENTER, -48, -32); // Label position init (which object, align to, align type, x offset, y offset) --> 

  secondTimerLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(secondTimerLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  resetTimer(TimerTypes::Second); 
  lv_obj_align(secondTimerLabel, lv_scr_act(), LV_ALIGN_CENTER, 48, -32); 

  firstTimerPlayStopBtn = lv_btn_create(lv_scr_act(), nullptr);
  firstTimerPlayStopBtn->user_data = this;
  lv_obj_set_event_cb(firstTimerPlayStopBtn, onFirstBtnPressed);
  lv_obj_set_size(firstTimerPlayStopBtn, 50, 50);
  lv_obj_align(firstTimerPlayStopBtn, firstTimerLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);
  firstTimerIcon = lv_label_create(firstTimerPlayStopBtn, nullptr);
  lv_label_set_text_static(firstTimerIcon, Symbols::play); //Play icon is the first rendered

  secondTimerPlayStopBtn = lv_btn_create(lv_scr_act(), nullptr);
  secondTimerPlayStopBtn->user_data = this;
  lv_obj_set_event_cb(secondTimerPlayStopBtn, onSecondBtnPressed);
  lv_obj_set_size(secondTimerPlayStopBtn, 50, 50);
  lv_obj_align(secondTimerPlayStopBtn, secondTimerLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);
  secondTimerIcon = lv_label_create(secondTimerPlayStopBtn, nullptr);
  lv_label_set_text_static(secondTimerIcon, Symbols::play); //Play icon is the first rendered

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

DoubleTimer::~DoubleTimer() {
  lv_task_del(taskRefresh);
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
  lv_obj_clean(lv_scr_act());
}

void DoubleTimer::playTimerEventHandler(lv_event_t event, TimerTypes timerType) {
    if (timerType == TimerTypes::First) {
        firstTimerState = TimerStates::Running;
        lv_label_set_text_static(firstTimerIcon, Symbols::stop);
        startFirstTimer = xTaskGetTickCount();
    } else {
        secondTimerState = TimerStates::Running;
        lv_label_set_text_static(secondTimerIcon, Symbols::stop);
        startSecondTimer = xTaskGetTickCount();
    }

    if (firstTimerState == TimerStates::Running || secondTimerState == TimerStates::Running) {
        systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);
    }
} 

void DoubleTimer::stopTimerEventHandler(lv_event_t event, TimerTypes timerType) {
    if (timerType == TimerTypes::First) {
        firstTimerState = TimerStates::Halted;
        lv_label_set_text_static(firstTimerIcon, Symbols::play);
    } else {
        secondTimerState = TimerStates::Halted;
        lv_label_set_text_static(secondTimerIcon, Symbols::play);
    }

    if (firstTimerState == TimerStates::Halted && secondTimerState == TimerStates::Halted) {
        systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
    }
}

TimerStates DoubleTimer::getFirstTimerState() {
    return firstTimerState;
}

TimerStates DoubleTimer::getSecondTimerState() {
    return secondTimerState;
}

void DoubleTimer::Refresh() {
    if (firstTimerState == TimerStates::Running) {
        updateTimer(firstTimerLabel, startFirstTimer, stopFirstTimer, firstTimerInSeconds);
    } else if (firstTimerState == TimerStates::Halted) {
        resetTimer(TimerTypes::First);
    }
    
    if (secondTimerState == TimerStates::Running) {
        updateTimer(secondTimerLabel, startSecondTimer, stopSecondTimer, secondTimerInSeconds);
    } else if (secondTimerState == TimerStates::Halted) {
        resetTimer(TimerTypes::Second);
    }
}

void DoubleTimer::updateTimer(lv_obj_t* label, TickType_t startTimer, TickType_t stopTimer, const int timeInSeconds) {
    const int currentTime = xTaskGetTickCount();
    stopTimer = (currentTime - startTimer) & 0XFFFFFFFF;
    const int secondsElapsed = convertTicksToTimeSeconds(stopTimer);
    const int secondsDiff = timeInSeconds - secondsElapsed;
    if(secondsDiff >= 0) {
        const int minutes = (secondsDiff) / 60;
        const int seconds = minutes == 0 ? secondsDiff : (secondsDiff - (minutes * 60));
        lv_label_set_text_fmt(label, "%02d:%02d", minutes, seconds);
    } else {
        lv_label_set_text_fmt(label, "00:00");
        ///Red label for expired timer (not halted)
        lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);

        ///WIP, i can't test it in simulator
        if(firstTimerState != TimerStates::Running && secondTimerState != TimerStates::Running) {
            motorController.RunForDuration(60);
            motorController.RunForDuration(60);
            motorController.RunForDuration(60);
        }
    }
}

void DoubleTimer::resetTimer(TimerTypes timerType) {
    if (timerType == TimerTypes::First) {
        lv_label_set_text_fmt(firstTimerLabel, "%02d:%02d", firstTimerMinutes, firstTimerSeconds);
        lv_obj_set_style_local_text_color(firstTimerLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE); // Label color init
        firstTimerState = TimerStates::Init;
    } else {
        lv_label_set_text_fmt(secondTimerLabel, "%02d:%02d", secondTimerMinutes, secondTimerSeconds);
        lv_obj_set_style_local_text_color(secondTimerLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE); 
        secondTimerState = TimerStates::Init;
    }
}