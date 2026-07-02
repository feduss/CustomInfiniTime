
#include "displayapp/screens/DoubleTimer.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

constexpr TickType_t blinkInterval = pdMS_TO_TICKS(1000);

//Constructor
DoubleTimer::DoubleTimer(
    Controllers::MotorController& motorController, 
    System::SystemTask& systemTask
) 
: motorController {motorController}, systemTask {systemTask}, wakeLock(systemTask) {

  setupViews();
  setupBindings();

  resetTimer(TimerTypes::First);
  resetTimer(TimerTypes::Second); 
}

DoubleTimer::~DoubleTimer() {
    prepareAppToExit();
}

void DoubleTimer::setupViews() {

    appTitleLabel = lv_label_create(
        lv_scr_act(), 
        nullptr
    );
    lv_obj_set_style_local_text_font(
        appTitleLabel, 
        LV_LABEL_PART_MAIN, 
        LV_STATE_DEFAULT, 
        &jetbrains_mono_bold_20
    );
    lv_label_set_text_fmt(
        appTitleLabel, 
        "DoubleTimer v%d.%d.%d", 
        appVersionMajor, 
        appVersionMinor, 
        appVersionPatch
    );
    lv_obj_align(
        appTitleLabel, 
        lv_scr_act(), 
        LV_ALIGN_IN_TOP_MID, 
        0, 
        0
    );

    //Label creation
    firstTimerLabel = lv_label_create(
        lv_scr_act(), 
        nullptr
    ); 
    // Label font init
    lv_obj_set_style_local_text_font(
        firstTimerLabel, 
        LV_LABEL_PART_MAIN, 
        LV_STATE_DEFAULT, 
        &jetbrains_mono_bold_20
    );
    // Label position init (object to align, align to object, align type, x offset (if increased, it goes to the right), y offset (if increased, it goes to the bottom))
    // https://docs.lvgl.io/master/widgets/obj.html#alignment
    lv_obj_align(
        firstTimerLabel, 
        lv_scr_act(), 
        LV_ALIGN_CENTER, 
        -48, 
        -32
    ); 

    secondTimerLabel = lv_label_create(
        lv_scr_act(), 
        nullptr
    );
    lv_obj_set_style_local_text_font(
        secondTimerLabel, 
        LV_LABEL_PART_MAIN, 
        LV_STATE_DEFAULT, 
        &jetbrains_mono_bold_20
    );
    lv_obj_align(
        secondTimerLabel, 
        lv_scr_act(), 
        LV_ALIGN_CENTER, 
        48, 
        -32
    ); 

    firstTimerPlayStopBtn = lv_btn_create(
        lv_scr_act(), 
        nullptr
    );
    secondTimerPlayStopBtn = lv_btn_create(
        lv_scr_act(), 
        nullptr
    );

    lv_obj_set_size(
        firstTimerPlayStopBtn, 
        50, 
        50
    );

    //TODO: LV_ALIGN_OUT_BOTTOM_MID should center align to the bottom of the anchor item...but it doesn't actually...so i added an x-axis padding of 6
    lv_obj_align(
        firstTimerPlayStopBtn, 
        firstTimerLabel, 
        LV_ALIGN_OUT_BOTTOM_MID, 
        6, 
        8
    );

    firstTimerIcon = lv_label_create(
        firstTimerPlayStopBtn, 
        nullptr
    );
    //Play icon is the first rendered
    lv_label_set_text_static(
        firstTimerIcon, 
        Symbols::play
    ); 

    lv_obj_set_size(secondTimerPlayStopBtn, 50, 50);

    //TODO: LV_ALIGN_OUT_BOTTOM_MID should center align to the bottom of the anchor item...but it doesn't actually...so i added an x-axis padding of 6
    lv_obj_align(
        secondTimerPlayStopBtn, 
        secondTimerLabel, 
        LV_ALIGN_OUT_BOTTOM_MID, 
        6, 
        8
    );

    secondTimerIcon = lv_label_create(
        secondTimerPlayStopBtn, 
        nullptr
    );
    lv_label_set_text_static(
        secondTimerIcon, 
        Symbols::play
    );
}

void DoubleTimer::setupBindings() {
    firstTimerPlayStopBtn->user_data = this;
    lv_obj_set_event_cb(
        firstTimerPlayStopBtn, 
        onFirstBtnPressed
    );

    secondTimerPlayStopBtn->user_data = this;
    lv_obj_set_event_cb(
        secondTimerPlayStopBtn, 
        onSecondBtnPressed
    );
}

// MARK: - UI Interaction

void DoubleTimer::onFirstBtnPressed(lv_obj_t* obj, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) { 
        return; 
    }

    DoubleTimer* stopWatch = static_cast<DoubleTimer*>(obj->user_data);

    if (stopWatch->getSecondTimerState() == TimerStates::Running) {
        stopWatch->firstReactionShock();
        return;
    }

    switch (stopWatch->getFirstTimerState()) {
        case TimerStates::Init:
            stopWatch->playTimerEventHandler(TimerTypes::First);
            break;
        case TimerStates::Running:
            stopWatch->stopTimerEventHandler(TimerTypes::First);
            break;
    }
}

void DoubleTimer::onSecondBtnPressed(lv_obj_t* obj, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) { 
        return; 
    }

    DoubleTimer* stopWatch = static_cast<DoubleTimer*>(obj->user_data);

    if (stopWatch->getFirstTimerState() == TimerStates::Running) {
        stopWatch->firstReactionShock();
        return;
    }

    switch (stopWatch->getSecondTimerState()) {
        case TimerStates::Init:
            stopWatch->playTimerEventHandler(TimerTypes::Second);
            break;
        case TimerStates::Running:
            stopWatch->stopTimerEventHandler(TimerTypes::Second);
            break;
    }
}

void DoubleTimer::firstReactionShock() {
    motorController.RunForDuration(200);
}

// to test
bool DoubleTimer::OnButtonPushed() {
  prepareAppToExit();
  return false;
}

bool DoubleTimer::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if(event == TouchEvents::SwipeRight) {
    prepareAppToExit();
  }
  return false;
}
// end

// MARK: - Timer logics

void DoubleTimer::playTimerEventHandler(TimerTypes timerType) {

    taskRefresh = lv_task_create(
        RefreshTaskCallback, 
        LV_DISP_DEF_REFR_PERIOD, 
        LV_TASK_PRIO_MID, 
        this
    );

    blinkTime = xTaskGetTickCount() + blinkInterval;
    switch (timerType) {
        case TimerTypes::First:
            firstTimerState = TimerStates::Running;
            lv_label_set_text_static(
                firstTimerIcon, 
                Symbols::stop
            );
            startFirstTimer = xTaskGetTickCount();
            break;
        case TimerTypes::Second:
            secondTimerState = TimerStates::Running;
            lv_label_set_text_static(
                secondTimerIcon, 
                Symbols::stop
            );
            startSecondTimer = xTaskGetTickCount();
            break;
    }
    wakeLock.Lock();
} 

void DoubleTimer::stopTimerEventHandler(TimerTypes timerType) {
    resetTimer(timerType);
    wakeLock.Release();
    deleteRefreshTask();
}

void DoubleTimer::Refresh() {
    if (firstTimerState == TimerStates::Running) {
        updateTimer(TimerTypes::First);
        return;
    }
    
    if (secondTimerState == TimerStates::Running) {
        updateTimer(TimerTypes::Second);
        return;
    }
}

void DoubleTimer::updateTimer(TimerTypes timerType) {

    lv_obj_t* label = nullptr;
    TickType_t startTimer = 0;
    TickType_t stopTimer = 0;
    int timeInSeconds = 0;

    switch (timerType) {
        case TimerTypes::First:
            label = firstTimerLabel;
            startTimer = startFirstTimer;
            stopTimer = stopFirstTimer;
            timeInSeconds = firstTimerInSeconds;
            break;
        case TimerTypes::Second:
            label = secondTimerLabel;
            startTimer = startSecondTimer;
            stopTimer = stopSecondTimer;
            timeInSeconds = secondTimerInSeconds;
            break;
        default:
            return;
    }

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

            switch (timerType) {
                case TimerTypes::First:
                    firstTimerState = TimerStates::Init;
                    break;
                case TimerTypes::Second:
                    secondTimerState = TimerStates::Init;
                    break;
            }
        } else {
            resetTimer(timerType);
            wakeLock.Release();
        }
    }
}

void DoubleTimer::resetTimer(TimerTypes timerType) {
    lv_obj_t* timerIcon = nullptr;
    lv_obj_t* timerLabel = nullptr;
    int timerMinutes = 0, timerSeconds = 0;
    TimerStates* timerState = nullptr;

    switch (timerType)
    {
    case TimerTypes::First:
        timerIcon = firstTimerIcon;
        timerLabel = firstTimerLabel;
        timerMinutes = firstTimerMinutes;
        timerSeconds = firstTimerSeconds;
        timerState = &firstTimerState;
        break;
    
    case TimerTypes::Second:
        timerIcon = secondTimerIcon;
        timerLabel = secondTimerLabel;
        timerMinutes = secondTimerMinutes;
        timerSeconds = secondTimerSeconds;
        timerState = &secondTimerState;
        break;
    default:
        return;
    }

    lv_label_set_text_static(timerIcon, Symbols::play);
    lv_label_set_text_fmt(timerLabel, "%02d:%02d", timerMinutes, timerSeconds);
    lv_obj_set_style_local_text_color(timerLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE); 
    *timerState = TimerStates::Init;

    deleteRefreshTask();
}

// MARK: - Helpers / Utils

DoubleTimer::Time DoubleTimer::convertTicksToTimeSegments(const TickType_t timeElapsed) {
    const int timeElapsedCentis = timeElapsed * 100 / configTICK_RATE_HZ;

    const int ms = (timeElapsedCentis % 100);
    const int secs = (timeElapsedCentis / 100) % 60;
    const int mins = (timeElapsedCentis / 100) / 60;
    return DoubleTimer::Time {mins, secs, ms};
}

TimerStates DoubleTimer::getFirstTimerState() {
    return firstTimerState;
}

TimerStates DoubleTimer::getSecondTimerState() {
    return secondTimerState;
}

bool DoubleTimer::isTimerActive() {
    return firstTimerState == TimerStates::Running || secondTimerState == TimerStates::Running;
}

void DoubleTimer::prepareAppToExit() {
    if (isExiting) { return; }
    isExiting = true;
    printf("\n[DoubleTimer] prepareAppToExit() - cleaning up");
    resetTimer(TimerTypes::First);
    resetTimer(TimerTypes::Second);
    deleteRefreshTask();
    lv_obj_clean(lv_scr_act());
}

void DoubleTimer::deleteRefreshTask() {
    if (taskRefresh != nullptr) {
      lv_task_del(taskRefresh);
      taskRefresh = nullptr;
    }
}