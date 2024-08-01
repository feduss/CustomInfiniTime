
#include "displayapp/screens/DoubleTimer.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

const DoubleTimer::Time convertTicksToTimeSegments(const TickType_t timeElapsed) {
    const int timeElapsedCentis = timeElapsed * 100 / configTICK_RATE_HZ;

    const int ms = (timeElapsedCentis % 100);
    const int secs = (timeElapsedCentis / 100) % 60;
    const int mins = (timeElapsedCentis / 100) / 60;
    return DoubleTimer::Time {mins, secs, ms};
  }

constexpr TickType_t blinkInterval = pdMS_TO_TICKS(1000);

void onFirstBtnPressed(lv_obj_t* obj, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) { return; }
    auto* stopWatch = static_cast<DoubleTimer*>(obj->user_data);
    if (stopWatch->getFirstTimerState() == TimerStates::Running) {
        stopWatch->stopTimerEventHandler(TimerTypes::First);
    } else if (stopWatch->getSecondTimerState() != TimerStates::Running) {
        stopWatch->playTimerEventHandler(TimerTypes::First);
    }
    
}

void onSecondBtnPressed(lv_obj_t* obj, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) { return; }
    auto* stopWatch = static_cast<DoubleTimer*>(obj->user_data);
    if (stopWatch->getSecondTimerState() == TimerStates::Running) {
        stopWatch->stopTimerEventHandler(TimerTypes::Second);
    } else if (stopWatch->getFirstTimerState() != TimerStates::Running) {
        stopWatch->playTimerEventHandler(TimerTypes::Second);
    }
    
}

//Constructor
DoubleTimer::DoubleTimer(Controllers::MotorController& motorController, System::SystemTask& systemTask) 
: motorController {motorController}, systemTask {systemTask} {

  /*** UI FIRST SETUP***/
  appTitleLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(appTitleLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_label_set_text_fmt(appTitleLabel, "DoubleTimer v%d.%d.%d", appVersionMajor, appVersionMinor, appVersionPatch);
  lv_obj_align(appTitleLabel, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);

  firstTimerLabel = lv_label_create(lv_scr_act(), nullptr); //Label creation
  lv_obj_set_style_local_text_font(firstTimerLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20); // Label font init

  // Label position init (object to align, align to object, align type, x offset (if increased, it goes to the right), y offset (if increased, it goes to the bottom))
  // https://docs.lvgl.io/master/widgets/obj.html#alignment
  lv_obj_align(firstTimerLabel, lv_scr_act(), LV_ALIGN_CENTER, -48, -32); 

  secondTimerLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(secondTimerLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_obj_align(secondTimerLabel, lv_scr_act(), LV_ALIGN_CENTER, 48, -32); 

  firstTimerPlayStopBtn = lv_btn_create(lv_scr_act(), nullptr);
  firstTimerPlayStopBtn->user_data = this;
  lv_obj_set_event_cb(firstTimerPlayStopBtn, onFirstBtnPressed);
  lv_obj_set_size(firstTimerPlayStopBtn, 50, 50);

  //TODO: LV_ALIGN_OUT_BOTTOM_MID should center align to the bottom of the anchor item...but it doesn't actually...so i added an x-axis padding of 6
  lv_obj_align(firstTimerPlayStopBtn, firstTimerLabel, LV_ALIGN_OUT_BOTTOM_MID, 6, 8);

  firstTimerIcon = lv_label_create(firstTimerPlayStopBtn, nullptr);
  lv_label_set_text_static(firstTimerIcon, Symbols::play); //Play icon is the first rendered

  secondTimerPlayStopBtn = lv_btn_create(lv_scr_act(), nullptr);
  secondTimerPlayStopBtn->user_data = this;
  lv_obj_set_event_cb(secondTimerPlayStopBtn, onSecondBtnPressed);
  lv_obj_set_size(secondTimerPlayStopBtn, 50, 50);

  //TODO: LV_ALIGN_OUT_BOTTOM_MID should center align to the bottom of the anchor item...but it doesn't actually...so i added an x-axis padding of 6
  lv_obj_align(secondTimerPlayStopBtn, secondTimerLabel, LV_ALIGN_OUT_BOTTOM_MID, 6, 8);

  secondTimerIcon = lv_label_create(secondTimerPlayStopBtn, nullptr);
  lv_label_set_text_static(secondTimerIcon, Symbols::play); //Play icon is the first rendered

  resetTimer(TimerTypes::First); // Label text/icon init
  resetTimer(TimerTypes::Second); 

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

DoubleTimer::~DoubleTimer() {
  lv_task_del(taskRefresh);
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
  lv_obj_clean(lv_scr_act());
}

void DoubleTimer::playTimerEventHandler(TimerTypes timerType) {
    blinkTime = xTaskGetTickCount() + blinkInterval;
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

void DoubleTimer::stopTimerEventHandler(TimerTypes timerType) {
    resetTimer(timerType);
    if (firstTimerState == TimerStates::Init && secondTimerState == TimerStates::Init) {
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
        updateTimer(TimerTypes::First, firstTimerLabel, startFirstTimer, stopFirstTimer, firstTimerInSeconds);
    }
    
    if (secondTimerState == TimerStates::Running) {
        updateTimer(TimerTypes::Second, secondTimerLabel, startSecondTimer, stopSecondTimer, secondTimerInSeconds);
    }
}

void DoubleTimer::updateTimer(TimerTypes timerType, lv_obj_t* label, TickType_t startTimer, TickType_t stopTimer, const int timeInSeconds) {
    const TickType_t currentTime = xTaskGetTickCount();
    stopTimer = (currentTime - startTimer) & 0XFFFFFFFF;
    DoubleTimer::Time times = convertTicksToTimeSegments(stopTimer);
    if (currentTime > blinkTime) {
        //printf("\nupdate label --- min: %d, sec: %d, ms: %d, currentTime: %d, blinkTime: %d", times.mins, times.secs, times.ms, currentTime, blinkTime);
        blinkTime = xTaskGetTickCount() + blinkInterval;
        const double secondsElapsed = (times.mins * 60.0) + times.secs + times.ms / 60.0;
        const double secondsDiff = std::ceil(double(timeInSeconds) - secondsElapsed);
        const int secondsDiffInt = int(secondsDiff);
        if(secondsDiffInt > 0) {
            const int minutes = (secondsDiff) / 60;
            const int seconds = minutes == 0 ? secondsDiff : (secondsDiff - (minutes * 60));
            //const int milliseconds = (secondsDiff - int(secondsDiff)) * 100;
            lv_label_set_text_fmt(label, "%02d:%02d", minutes, seconds);
            
            ///Orange label for expiring timer
            if (secondsDiffInt <= 5) {
                lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
                //The timer is about to expire, activate a short vibration
                motorController.RunForDuration(90);
            } 
            
        } else if (secondsDiffInt == 0){
            lv_label_set_text_fmt(label, "00:00");
            ///Red label for expired timer
            lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
            //The timer is expired, activate a long vibration
            motorController.RunForDuration(180);
            if (timerType == TimerTypes::First) {
                firstTimerState = TimerStates::Expired;
            } else {
                secondTimerState = TimerStates::Expired;
            }
        } else {
            resetTimer(timerType);
        }
    }
}

void DoubleTimer::resetTimer(TimerTypes timerType) {
    if (timerType == TimerTypes::First) {
        lv_label_set_text_static(firstTimerIcon, Symbols::play);
        lv_label_set_text_fmt(firstTimerLabel, "%02d:%02d", firstTimerMinutes, firstTimerSeconds);
        lv_obj_set_style_local_text_color(firstTimerLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE); // Label color init
        firstTimerState = TimerStates::Init;
    } else {
        lv_label_set_text_static(secondTimerIcon, Symbols::play);
        lv_label_set_text_fmt(secondTimerLabel, "%02d:%02d", secondTimerMinutes, secondTimerSeconds);
        lv_obj_set_style_local_text_color(secondTimerLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE); 
        secondTimerState = TimerStates::Init;
    }
}