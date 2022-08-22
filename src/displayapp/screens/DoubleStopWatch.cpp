#include "displayapp/screens/DoubleStopWatch.h"

#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

void onFirstBtnPressed(lv_obj_t* obj, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) {
    return;
    }
    auto* stopWatch = static_cast<DoubleStopWatch*>(obj->user_data);
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
    auto* stopWatch = static_cast<DoubleStopWatch*>(obj->user_data);
    if (stopWatch->getSecondTimerState() == TimerStates::Running) {
        stopWatch->stopTimerEventHandler(event, TimerTypes::Second);
    } else {
        stopWatch->playTimerEventHandler(event, TimerTypes::Second);
    }
    
}

//Constructor
DoubleStopWatch::DoubleStopWatch(DisplayApp* app, System::SystemTask& systemTask) : Screen(app), systemTask {systemTask} {

  /*** UI FIRST SETUP***/
  firstTimerLabel = lv_label_create(lv_scr_act(), nullptr); //Label creation
  lv_obj_set_style_local_text_font(firstTimerLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20); // Label font init
  lv_obj_set_style_local_text_color(firstTimerLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xb0, 0xb0, 0xb0)); // Label color init
  lv_label_set_text_static(firstTimerLabel, "00:00"); // Label text init
  lv_obj_align(firstTimerLabel, lv_scr_act(), LV_ALIGN_CENTER, -48, -32); // Label position init (which object, align to, align type, x offset, y offset) --> 

  secondTimerLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(secondTimerLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_obj_set_style_local_text_color(secondTimerLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xb0, 0xb0, 0xb0)); 
  lv_label_set_text_static(secondTimerLabel, "00:00"); 
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

DoubleStopWatch::~DoubleStopWatch() {
  lv_task_del(taskRefresh);
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
  lv_obj_clean(lv_scr_act());
}

void DoubleStopWatch::playTimerEventHandler(lv_event_t event, TimerTypes timerType) {
    if (timerType == TimerTypes::First) {
        firstTimerState = TimerStates::Running;
        lv_label_set_text_static(firstTimerIcon, Symbols::play);
    } else {
        secondTimerState = TimerStates::Running;
        lv_label_set_text_static(secondTimerIcon, Symbols::play);
    }

    if (firstTimerState == TimerStates::Running || secondTimerState == TimerStates::Running) {
        systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);
    }
}

void DoubleStopWatch::stopTimerEventHandler(lv_event_t event, TimerTypes timerType) {
    if (timerType == TimerTypes::First) {
        firstTimerState = TimerStates::Halted;
        lv_label_set_text_static(firstTimerIcon, Symbols::stop);
    } else {
        secondTimerState = TimerStates::Halted;
        lv_label_set_text_static(secondTimerIcon, Symbols::stop);
    }

    if (firstTimerState == TimerStates::Halted && secondTimerState == TimerStates::Halted) {
        systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
    }
}

TimerStates DoubleStopWatch::getFirstTimerState() {
    return firstTimerState;
}

TimerStates DoubleStopWatch::getSecondTimerState() {
    return secondTimerState;
}

bool DoubleStopWatch::OnButtonPushed() {
  
}

void DoubleStopWatch::Refresh() {
}