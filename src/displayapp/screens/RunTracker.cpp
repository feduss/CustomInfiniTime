#include "displayapp/screens/StopWatch.h"
#include "displayapp/screens/RunTracker.h"
#include <components/heartrate/HeartRateController.h>
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {

  TimeSeparated ConvertTicksToTimeSegments(const TickType_t timeElapsed) {
    const uint32_t timeElapsedSecs = timeElapsed / configTICK_RATE_HZ;
    const uint16_t timeElapsedFraction = timeElapsed % configTICK_RATE_HZ;

    const uint8_t hundredths = timeElapsedFraction * 100 / configTICK_RATE_HZ;
    const uint8_t secs = (timeElapsedSecs) % 60;
    const uint8_t mins = (timeElapsedSecs / 60) % 60;
    const uint16_t hours = (timeElapsedSecs / 60) / 60;
    return TimeSeparated {hours, mins, secs, hundredths, timeElapsedSecs};
  }

  const char* ToString(Pinetime::Controllers::HeartRateController::States s) {
    switch (s) {
      case Pinetime::Controllers::HeartRateController::States::NotEnoughData:
        return "Not enough data,\nplease wait...";
      case Pinetime::Controllers::HeartRateController::States::NoTouch:
        return "No touch detected";
      case Pinetime::Controllers::HeartRateController::States::Running:
        return "Measuring...";
      case Pinetime::Controllers::HeartRateController::States::Stopped:
        return "Stopped";
    }
    return "";
  }
}

RunTracker::RunTracker(
  Controllers::StopWatchController& stopWatchController,
  Controllers::HeartRateController& heartRateController, 
  System::SystemTask& systemTask
) : stopWatchController {stopWatchController}, heartRateController {heartRateController}, systemTask {systemTask}, wakeLock(systemTask) {
  /*bool isHrRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  label_hr = lv_label_create(lv_scr_act(), nullptr);

  lv_obj_set_style_local_text_font(label_hr, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);

  if (isHrRunning) {
    lv_obj_set_style_local_text_color(label_hr, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::highlight);
  } else {
    lv_obj_set_style_local_text_color(label_hr, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  }

  lv_label_set_text_static(label_hr, "---");
  lv_obj_align(label_hr, nullptr, LV_ALIGN_CENTER, 0, -40);

  label_bpm = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(label_bpm, "Heart rate BPM");
  lv_obj_align(label_bpm, label_hr, LV_ALIGN_OUT_TOP_MID, 0, -20);

  label_status = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(label_status, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_label_set_text_static(label_status, ToString(Pinetime::Controllers::HeartRateController::States::NotEnoughData));

  lv_obj_align(label_status, label_hr, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

  btn_startStop = lv_btn_create(lv_scr_act(), nullptr);
  btn_startStop->user_data = this;
  lv_obj_set_height(btn_startStop, 50);
  lv_obj_set_event_cb(btn_startStop, btnStartStopEventHandler);
  lv_obj_align(btn_startStop, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  label_startStop = lv_label_create(btn_startStop, nullptr);
  UpdateStartStopButton(isHrRunning);
  if (isHrRunning) {
    wakeLock.Lock();
  }

  ;*/
  SetupViews();
  SetupBindings();
}

RunTracker::~RunTracker() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void RunTracker::SetupViews() {

  SetupAppTitle();

  SetupTimeTitleLabel();
  SetupTimeValueLabel();

  SetupDistanceTitleLabel();
  SetupDistanceValueLabel();

  SetupSpeedTitleLabel();
  SetupSpeedValueLabel();

  SetupHeartRateTitleLabel();
  SetupHeartRateValueLabel();

  SetupPlayButton();
  SetupStopButton();
}

void RunTracker::SetupAppTitle() {
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
      "%s v%d.%d.%d", 
      appTitle,
      appVersionMajor, 
      appVersionMinor, 
      appVersionPatch
  );
  lv_obj_align(
      appTitleLabel, 
      lv_scr_act(), 
      LV_ALIGN_IN_TOP_MID, 
      0, 
      16
  );
}

void RunTracker::SetupPlayButton() {
  playButton = lv_btn_create(
      lv_scr_act(), 
      nullptr
  );

  lv_obj_set_size(
      playButton, 
      50, 
      50
  );

  lv_obj_align(
      playButton, 
      lv_scr_act(), 
      LV_ALIGN_CENTER, 
      0, 
      0
  );

  playButtonIcon = lv_label_create(
      playButton, 
      nullptr
  );

  lv_label_set_text_static(
      playButtonIcon, 
      Symbols::play
  ); 

  lv_label_set_text_static(
      playButtonIcon, 
      Symbols::play
  ); 
}

void RunTracker::SetupStopButton() {
  stopButton = lv_btn_create(
      lv_scr_act(), 
      nullptr
  );

  lv_obj_set_size(
      stopButton, 
      50, 
      50
  );

  lv_obj_align(
      stopButton, 
      lv_scr_act(), 
      LV_ALIGN_IN_BOTTOM_MID, 
      0, 
      -16
  );

  stopButtonIcon = lv_label_create(
      stopButton, 
      nullptr
  );

  lv_label_set_text_static(
      stopButtonIcon, 
      Symbols::stop
  ); 

  SetObjectVisibility(stopButton, false);
  SetObjectVisibility(stopButtonIcon, false);
}

void RunTracker::SetupTimeTitleLabel() {
  timeTitleLabel = lv_label_create(
      lv_scr_act(), 
      nullptr
  );
  lv_obj_set_style_local_text_font(
      timeTitleLabel, 
      LV_LABEL_PART_MAIN, 
      LV_STATE_DEFAULT, 
      &jetbrains_mono_bold_20
  );
  lv_label_set_text_static(
      timeTitleLabel, 
      timeTitleText
  );
  lv_obj_align(
      timeTitleLabel, 
      lv_scr_act(), 
      LV_ALIGN_IN_TOP_LEFT, 
      32, 
      16
  );

  SetObjectVisibility(timeTitleLabel, false);
}

void RunTracker::SetupTimeValueLabel() {
  timeValueLabel = lv_label_create(
      lv_scr_act(), 
      nullptr
  );
  lv_obj_set_style_local_text_font(
      timeValueLabel, 
      LV_LABEL_PART_MAIN, 
      LV_STATE_DEFAULT, 
      &jetbrains_mono_bold_20
  );
  lv_label_set_text_static(
      timeValueLabel, 
      "-"
  );
  lv_obj_align(
      timeValueLabel, 
      timeTitleLabel, 
      LV_ALIGN_OUT_BOTTOM_MID, 
      0, 
      8
  );

  SetObjectVisibility(timeValueLabel, false);
}

void RunTracker::SetupDistanceTitleLabel() {
  distanceTitleLabel = lv_label_create(
      lv_scr_act(), 
      nullptr
  );
  lv_obj_set_style_local_text_font(
      distanceTitleLabel, 
      LV_LABEL_PART_MAIN, 
      LV_STATE_DEFAULT, 
      &jetbrains_mono_bold_20
  );
  lv_label_set_text_static(
      distanceTitleLabel, 
      distanceTitleText
  );
  lv_obj_align(
      distanceTitleLabel, 
      lv_scr_act(), 
      LV_ALIGN_IN_TOP_RIGHT , 
      -32, 
      16
  );

  SetObjectVisibility(distanceTitleLabel, false);
}

void RunTracker::SetupDistanceValueLabel() {
  distanceValueLabel = lv_label_create(
      lv_scr_act(), 
      nullptr
  );
  lv_obj_set_style_local_text_font(
      distanceValueLabel, 
      LV_LABEL_PART_MAIN, 
      LV_STATE_DEFAULT, 
      &jetbrains_mono_bold_20
  );
  lv_label_set_text_static(
      distanceValueLabel, 
      "-"
  );
  lv_obj_align(
      distanceValueLabel, 
      distanceTitleLabel, 
      LV_ALIGN_OUT_BOTTOM_MID, 
      0, 
      8
  );

  SetObjectVisibility(distanceValueLabel, false);
}

void RunTracker::SetupSpeedTitleLabel() {
  speedTitleLabel = lv_label_create(
      lv_scr_act(), 
      nullptr
  );
  lv_obj_set_style_local_text_font(
      speedTitleLabel, 
      LV_LABEL_PART_MAIN, 
      LV_STATE_DEFAULT, 
      &jetbrains_mono_bold_20
  );
  lv_label_set_text_static(
      speedTitleLabel, 
      speedTitleText
  );
  lv_obj_align(
      speedTitleLabel, 
      timeValueLabel, 
      LV_ALIGN_OUT_BOTTOM_MID , 
      0, 
      16
  );

  SetObjectVisibility(speedTitleLabel, false);
}

void RunTracker::SetupSpeedValueLabel() {
  speedValueLabel = lv_label_create(
      lv_scr_act(), 
      nullptr
  );
  lv_obj_set_style_local_text_font(
      speedValueLabel, 
      LV_LABEL_PART_MAIN, 
      LV_STATE_DEFAULT, 
      &jetbrains_mono_bold_20
  );
  lv_label_set_text_static(
      speedValueLabel, 
      "-"
  );
  lv_obj_align(
      speedValueLabel, 
      speedTitleLabel, 
      LV_ALIGN_OUT_BOTTOM_MID, 
      0, 
      8
  );

  SetObjectVisibility(speedValueLabel, false);
}

void RunTracker::SetupHeartRateTitleLabel() {
  heartRateTitleLabel = lv_label_create(
      lv_scr_act(), 
      nullptr
  );
  lv_obj_set_style_local_text_font(
      heartRateTitleLabel, 
      LV_LABEL_PART_MAIN, 
      LV_STATE_DEFAULT, 
      &jetbrains_mono_bold_20
  );
  lv_label_set_text_static(
      heartRateTitleLabel, 
      heartRateTitleText
  );
  lv_obj_align(
      heartRateTitleLabel, 
      distanceValueLabel, 
      LV_ALIGN_OUT_BOTTOM_MID, 
      0, 
      16
  );

  SetObjectVisibility(heartRateTitleLabel, false);
}

void RunTracker::SetupHeartRateValueLabel() {
  heartRateValueLabel = lv_label_create(
      lv_scr_act(), 
      nullptr
  );
  lv_obj_set_style_local_text_font(
      heartRateValueLabel, 
      LV_LABEL_PART_MAIN, 
      LV_STATE_DEFAULT, 
      &jetbrains_mono_bold_20
  );
  lv_label_set_text_static(
      heartRateValueLabel, 
      "-"
  );
  lv_obj_align(
      heartRateValueLabel, 
      heartRateTitleLabel, 
      LV_ALIGN_OUT_BOTTOM_MID, 
      0, 
      8
  );

  SetObjectVisibility(heartRateValueLabel, false);
}

void RunTracker::SetupBindings() {
  
  playButton->user_data = this;
  lv_obj_set_event_cb(playButton, PlayButtonEventHandler);
  
  stopButton->user_data = this;
  lv_obj_set_event_cb(stopButton, StopButtonEventHandler);
}

void RunTracker::Refresh() {

  if (!isTracking) {
    return;
  }

  UpdateTime();
  UpdateDistance();
  UpdateSpeed();
  UpdateHeartRate();
  
}

void RunTracker::PlayButtonEventHandler(lv_obj_t* obj, lv_event_t event) {
  RunTracker* screen = static_cast<RunTracker*>(obj->user_data);
  screen->OnStartEvent(event);
}

void RunTracker::StopButtonEventHandler(lv_obj_t* obj, lv_event_t event) {
  RunTracker* screen = static_cast<RunTracker*>(obj->user_data);
  screen->OnStopEvent(event);
}

void RunTracker::OnStartEvent(lv_event_t event) {

  isTracking = true;

  SetObjectVisibility(appTitleLabel, false);
  SetObjectVisibility(playButton, false);
  SetObjectVisibility(playButtonIcon, false);

  SetObjectVisibility(timeTitleLabel, true);
  SetObjectVisibility(timeValueLabel, true);

  SetObjectVisibility(distanceTitleLabel, true);
  SetObjectVisibility(distanceValueLabel, true);

  SetObjectVisibility(speedTitleLabel, true);
  SetObjectVisibility(speedValueLabel, true);

  SetObjectVisibility(heartRateTitleLabel, true);
  SetObjectVisibility(heartRateValueLabel, true);

  SetObjectVisibility(stopButton, true);
  SetObjectVisibility(stopButtonIcon, true);

  UpdateTime();
  UpdateDistance();
  UpdateSpeed();
  UpdateHeartRate();

  taskRefresh = lv_task_create(
    RefreshTaskCallback, 
    LV_DISP_DEF_REFR_PERIOD, 
    LV_TASK_PRIO_MID, 
    this
  );

  stopWatchController.Start();
  heartRateController.Enable();
  wakeLock.Lock();
  DisableScreenSleeping();
  
}

void RunTracker::OnStopEvent(lv_event_t event) {

  isTracking = false;

  SetObjectVisibility(appTitleLabel, true);
  SetObjectVisibility(playButton, true);
  SetObjectVisibility(playButtonIcon, true);

  SetObjectVisibility(timeTitleLabel, false);
  SetObjectVisibility(timeValueLabel, false);

  SetObjectVisibility(distanceTitleLabel, false);
  SetObjectVisibility(distanceValueLabel, false);

  SetObjectVisibility(speedTitleLabel, false);
  SetObjectVisibility(speedValueLabel, false);

  SetObjectVisibility(heartRateTitleLabel, false);
  SetObjectVisibility(heartRateValueLabel, false);

  SetObjectVisibility(stopButton, false);
  SetObjectVisibility(stopButtonIcon, false);

  UpdateTime();
  UpdateDistance();
  UpdateSpeed();
  UpdateHeartRate();

  CleanObjects();
}

// to test
bool RunTracker::OnButtonPushed() {
  PrepareAppToExit();
  return false;
}

bool RunTracker::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if(event == TouchEvents::SwipeRight) {
    PrepareAppToExit();
  }
  return false;
}
// end

//TODO: to edit
/*void RunTracker::OnStartStopEvent(lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (heartRateController.State() == Controllers::HeartRateController::States::Stopped) {
      heartRateController.Enable();
      UpdateStartStopButton(heartRateController.State() != Controllers::HeartRateController::States::Stopped);
      wakeLock.Lock();
      lv_obj_set_style_local_text_color(label_hr, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::highlight);
    } else {
      heartRateController.Disable();
      UpdateStartStopButton(heartRateController.State() != Controllers::HeartRateController::States::Stopped);
      wakeLock.Release();
      lv_obj_set_style_local_text_color(label_hr, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
    }
  }
}*/

void RunTracker::UpdateTime() {

  if (isTracking) {

    TimeSeparated elapsedTime = ConvertTicksToTimeSegments(stopWatchController.GetElapsedTime());
    renderedSeconds = elapsedTime.epochSecs;

    if (renderedSeconds.IsUpdated()) {
      lv_label_set_text_fmt(
        timeValueLabel, "%02d:%02d:%02d", 
        elapsedTime.hours, 
        elapsedTime.mins, 
        elapsedTime.secs
      );
    }
  } else {
    lv_label_set_text_static(
        timeValueLabel, 
        "-"
    );
  }

  lv_obj_align(
      timeValueLabel, 
      timeTitleLabel, 
      LV_ALIGN_OUT_BOTTOM_MID, 
      0, 
      8
  );
}

void RunTracker::UpdateHeartRate() {

  if (IsTracking()) {

    auto state = heartRateController.State();
    switch (state) {

      case Controllers::HeartRateController::States::Stopped:
        lv_label_set_text_static(heartRateValueLabel, "Stopped");
        break;
      case Controllers::HeartRateController::States::NoTouch:
      case Controllers::HeartRateController::States::NotEnoughData:
        lv_label_set_text_static(heartRateValueLabel, "Waiting");
        break;
      default:
        if (heartRateController.HeartRate() == 0) {
          lv_label_set_text_static(heartRateValueLabel, "Dead");
        } else {
          lv_label_set_text_fmt(heartRateValueLabel, "%03d bpm", heartRateController.HeartRate());
        }
    }
  } else {
    lv_label_set_text_static(
        heartRateValueLabel, 
        "-"
    );
  }

  lv_obj_align(
      heartRateValueLabel, 
      heartRateTitleLabel, 
      LV_ALIGN_OUT_BOTTOM_MID, 
      0, 
      8
  );
}

void RunTracker::UpdateDistance() {

  if (isTracking) {
    
  } else {
    lv_label_set_text_static(
        distanceValueLabel, 
        "-"
    );
  }

  lv_obj_align(
      distanceValueLabel, 
      distanceTitleLabel, 
      LV_ALIGN_OUT_BOTTOM_MID, 
      0, 
      8
  );
}

void RunTracker::UpdateSpeed() {

  if (isTracking) {

  } else {
    lv_label_set_text_static(
        speedValueLabel, 
        "-"
    );
  }

  lv_obj_align(
      speedValueLabel, 
      speedTitleLabel, 
      LV_ALIGN_OUT_BOTTOM_MID, 
      0, 
      8
  );
}


// MARK: - Utils

void RunTracker::SetObjectVisibility(lv_obj_t* obj, bool isVisible) {
  lv_obj_set_hidden(obj, !isVisible);
}

void RunTracker::EnableScreenSleeping() {
    systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
}

void RunTracker::DisableScreenSleeping() {
    systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);
}

void RunTracker::PrepareAppToExit() {
    if (isExiting) { return; }
    isExiting = true;
    printf("\n[DoubleTimer] prepareAppToExit() - cleaning up");
    CleanObjects();
}

void RunTracker::CleanObjects() {
    EnableScreenSleeping();
    stopWatchController.Clear();
    heartRateController.Disable();
    wakeLock.Release();
    lv_task_del(taskRefresh);
    lv_obj_clean(lv_scr_act());
}