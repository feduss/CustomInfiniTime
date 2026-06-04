#include "displayapp/screens/StopWatch.h"
#include "displayapp/screens/RunTracker.h"
#include <components/heartrate/HeartRateController.h>
#include <components/motion/MotionController.h>
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
}

RunTracker::RunTracker(
  Controllers::StopWatchController& stopWatchController,
  Controllers::HeartRateController& heartRateController, 
  Controllers::MotionController& motionController,
  System::SystemTask& systemTask
) : stopWatchController {stopWatchController}, heartRateController {heartRateController}, motionController {motionController}, systemTask {systemTask}, wakeLock(systemTask) {
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
  if (event != LV_EVENT_CLICKED) { 
      return; 
  }
  RunTracker* screen = static_cast<RunTracker*>(obj->user_data);
  screen->OnStartEvent();
}

void RunTracker::StopButtonEventHandler(lv_obj_t* obj, lv_event_t event) {
  if (event != LV_EVENT_CLICKED) { 
      return; 
  }
  RunTracker* screen = static_cast<RunTracker*>(obj->user_data);
  screen->OnStopEvent();
}

void RunTracker::OnStartEvent() {

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

  runStartTripSteps = motionController.GetTripSteps();

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

void RunTracker::OnStopEvent() {

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

  if (isTracking) {

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
    const uint32_t currentTripSteps = motionController.GetTripSteps();
    const uint32_t runSteps = currentTripSteps >= runStartTripSteps ? currentTripSteps - runStartTripSteps : 0;
    const uint32_t distanceCentimeters = runSteps * 78u; // average stride estimate

    const uint32_t kilometers = distanceCentimeters / 100000u;
    const uint32_t hectometers = (distanceCentimeters % 100000u) / 1000u;

    lv_label_set_text_fmt(
        distanceValueLabel,
        "%u.%02u km",
        kilometers,
        hectometers
    );
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
    const uint32_t currentTripSteps = motionController.GetTripSteps();
    const uint32_t runSteps = currentTripSteps >= runStartTripSteps ? currentTripSteps - runStartTripSteps : 0;
    const uint32_t distanceCentimeters = runSteps * 78u;
    const uint32_t elapsedSeconds = stopWatchController.GetElapsedTime() / configTICK_RATE_HZ;

    uint32_t paceMinutes = 0;
    uint32_t paceSeconds = 0;
    if (distanceCentimeters > 0) {
      const uint32_t distanceKilometers = distanceCentimeters / 100000u;
      if (distanceKilometers > 0) {
        const uint32_t paceSecondsPerKm = elapsedSeconds / distanceKilometers;
        paceMinutes = paceSecondsPerKm / 60u;
        paceSeconds = paceSecondsPerKm % 60u;
      }
    }

    lv_label_set_text_fmt(
        speedValueLabel,
        "%u'%02u\"/km",
        paceMinutes,
        paceSeconds
    );
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
    lv_task_del(taskRefresh);
    lv_obj_clean(lv_scr_act());
}

void RunTracker::CleanObjects() {
    EnableScreenSleeping();
    stopWatchController.Clear();
    heartRateController.Disable();
    wakeLock.Release();
}