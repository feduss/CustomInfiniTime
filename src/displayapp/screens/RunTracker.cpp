#include "displayapp/screens/StopWatch.h"
#include "displayapp/screens/RunTracker.h"
#include <components/heartrate/HeartRateController.h>
#include <components/motion/MotionController.h>
#include "displayapp/screens/Symbols.h"
#include "RunTracker.h"
#include <numeric>

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
) : 
stopWatchController {stopWatchController}, 
heartRateController {heartRateController}, 
motionController {motionController}, 
systemTask {systemTask}, 
wakeLock(systemTask)
{
  SetupViews(true);
  SetupBindings();
}

RunTracker::~RunTracker() {
  PrepareAppToExit();
}

void RunTracker::SetupViews(bool isFirstTime) {

  if (isFirstTime) {
    SetupAppTitle();
  }

  SetupTimeTitleLabel(isFirstTime);
  SetupTimeValueLabel(isFirstTime);

  SetupDistanceTitleLabel(isFirstTime);
  SetupDistanceValueLabel(isFirstTime);

  SetupSpeedTitleLabel(isFirstTime);
  SetupSpeedValueLabel(isFirstTime);

  SetupHeartRateTitleLabel(isFirstTime);
  SetupHeartRateValueLabel(isFirstTime);

  if (isFirstTime) {
    SetupPlayButton();
    SetupStopButton();
    SetupCloseButton();
  }
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

void RunTracker::SetupCloseButton() {
  closeButton = lv_btn_create(
      lv_scr_act(), 
      nullptr
  );

  lv_obj_set_size(
      closeButton, 
      50, 
      50
  );

  lv_obj_align(
      closeButton, 
      lv_scr_act(), 
      LV_ALIGN_IN_BOTTOM_MID, 
      0, 
      -16
  );

  closeButtonIcon = lv_label_create(
      closeButton, 
      nullptr
  );

  lv_label_set_text_static(
      closeButtonIcon, 
      Symbols::stop
  ); 

  SetObjectVisibility(closeButton, false);
  SetObjectVisibility(closeButtonIcon, false);
}

void RunTracker::SetupTimeTitleLabel(bool isFirstTime) {
  
  if (isFirstTime) {
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

    SetObjectVisibility(timeTitleLabel, false);
  }

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
}

void RunTracker::SetupTimeValueLabel(bool isFirstTime) {
  if (isFirstTime) {
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

    SetObjectVisibility(timeValueLabel, false);
  }
  
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

}

void RunTracker::SetupDistanceTitleLabel(bool isFirstTime) {
  if (isFirstTime) {
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

    SetObjectVisibility(distanceTitleLabel, false);
  }
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
}

void RunTracker::SetupDistanceValueLabel(bool isFirstTime) {
  if (isFirstTime) {
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

    SetObjectVisibility(distanceValueLabel, false);
  }
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
}

void RunTracker::SetupSpeedTitleLabel(bool isFirstTime) {
  if (isFirstTime) {
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

    SetObjectVisibility(speedTitleLabel, false);
  }
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
}

void RunTracker::SetupSpeedValueLabel(bool isFirstTime) {
  if (isFirstTime) {
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

    SetObjectVisibility(speedValueLabel, false);
  }
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
}

void RunTracker::SetupHeartRateTitleLabel(bool isFirstTime) {
  if (isFirstTime) {
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

    SetObjectVisibility(heartRateTitleLabel, false);
  }
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
}

void RunTracker::SetupHeartRateValueLabel(bool isFirstTime) {
  if (isFirstTime) {
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

    SetObjectVisibility(heartRateValueLabel, false);
  }
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
}

void RunTracker::SetupBindings() {
  
  playButton->user_data = this;
  lv_obj_set_event_cb(playButton, PlayButtonEventHandler);
  
  stopButton->user_data = this;
  lv_obj_set_event_cb(stopButton, StopButtonEventHandler);

  closeButton->user_data = this;
  lv_obj_set_event_cb(closeButton, CloseButtonEventHandler);
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

void RunTracker::CloseButtonEventHandler(lv_obj_t* obj, lv_event_t event) {
  if (event != LV_EVENT_CLICKED) { 
      return; 
  }
  RunTracker* screen = static_cast<RunTracker*>(obj->user_data);
  screen->OnCloseEvent();
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

  SetTimeReportLabels();
  SetDistanceReportLabels();
  SetSpeedReportLabels();
  SetHeartRateReportLabels();

  SetObjectVisibility(stopButton, false);
  SetObjectVisibility(stopButtonIcon, false);

  SetObjectVisibility(closeButton, true);
  SetObjectVisibility(closeButtonIcon, true);

  CleanObjects();
}

void RunTracker::SetTimeReportLabels() {
  lv_label_set_text_static(
    timeTitleLabel, 
    timeReportTitleText
  );
  lv_obj_align(
    timeTitleLabel, 
    lv_scr_act(), 
    LV_ALIGN_IN_TOP_LEFT, 
    8, 
    8
  );

  lv_label_set_text(
    timeValueLabel, 
    timeBuffer
  );

  lv_obj_align(
    timeValueLabel, 
    timeTitleLabel, 
    LV_ALIGN_OUT_RIGHT_MID, 
    8, 
    0
  );
}

void RunTracker::SetDistanceReportLabels() {
  lv_label_set_text_static(
      distanceTitleLabel, 
      distanceReportTitleText
  );
  lv_obj_align(
      distanceTitleLabel,  
      timeTitleLabel, 
      LV_ALIGN_OUT_BOTTOM_LEFT,
      0, 
      8
  );

  const double distanceCm = dirtyDistanceCm.Get();
  const double kilometers = distanceCm / 100000.0;
  const double hectometers = std::fmod(distanceCm, 100000.0) / 1000.0;

  lv_label_set_text_fmt(
      distanceValueLabel,
      "%u.%02u km",
      static_cast<unsigned int>(kilometers),
      static_cast<unsigned int>(hectometers)
  );

  lv_obj_align(
      distanceValueLabel, 
      distanceTitleLabel, 
      LV_ALIGN_OUT_RIGHT_MID, 
      8, 
      0
  );
}

void RunTracker::SetSpeedReportLabels() {
  lv_label_set_text_static(
      speedTitleLabel, 
      speedReportTitleText
  );
  lv_obj_align(
      speedTitleLabel,  
      distanceTitleLabel, 
      LV_ALIGN_OUT_BOTTOM_LEFT,
      0, 
      8
  );

  std::string paceSummary = getPaceSummary();
  lv_label_set_text(
    speedValueLabel, 
    paceSummary.c_str()
  );

  lv_obj_align(
      speedValueLabel, 
      speedTitleLabel, 
      LV_ALIGN_OUT_BOTTOM_LEFT, 
      0, 
      0
  );
}

void RunTracker::SetHeartRateReportLabels() {
  lv_label_set_text_static(
      heartRateTitleLabel, 
      heartRateReportTitleText
  );
  lv_obj_align(
      heartRateTitleLabel,  
      speedValueLabel, 
      LV_ALIGN_OUT_BOTTOM_LEFT,
      0, 
      8
  );

  std::string heartRateSummary = getHeartRateSummary();
  lv_label_set_text(
    heartRateValueLabel, 
    heartRateSummary.c_str()
  );

  lv_obj_align(
      heartRateValueLabel, 
      heartRateTitleLabel, 
      LV_ALIGN_OUT_BOTTOM_LEFT, 
      0, 
      0
  );
}

void RunTracker::OnCloseEvent() {
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

  SetObjectVisibility(closeButton, false);
  SetObjectVisibility(closeButtonIcon, false);

  SetupViews(false);

  UpdateTime();
  UpdateDistance();
  UpdateSpeed();
  UpdateHeartRate();
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
    dirtyRenderedSeconds = elapsedTime.epochSecs;

    if (dirtyRenderedSeconds.IsUpdated()) {      
      snprintf(
        timeBuffer, 
        sizeof(timeBuffer), 
        "%02d:%02d:%02d",
        elapsedTime.hours,
        elapsedTime.mins,
        elapsedTime.secs
      );

      lv_label_set_text(timeValueLabel, timeBuffer);
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

void RunTracker::UpdateDistance() {

  if (isTracking) {

    const uint32_t currentTripSteps = motionController.GetTripSteps();
    const uint32_t runSteps = currentTripSteps >= runStartTripSteps ? currentTripSteps - runStartTripSteps : 0;
    dirtyDistanceCm = runSteps * strideLengthCm; // average stride estimate

    if (dirtyDistanceCm.IsUpdated()) {
      const double distanceCm = dirtyDistanceCm.Get();
      const double kilometers = distanceCm / 100000.0;
      const double hectometers = std::fmod(distanceCm, 100000.0) / 1000u;

      lv_label_set_text_fmt(
          distanceValueLabel,
          "%u.%02u km",
          static_cast<unsigned int>(kilometers),
          static_cast<unsigned int>(hectometers)
      );
    }
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
    
    const double distanceCm = dirtyDistanceCm.Get();
    const uint32_t renderedSeconds = dirtyRenderedSeconds.Get();
    if (distanceCm > 0 && renderedSeconds > 0) {
      double paceMinutes = 0;
      double paceSeconds = 0;
      const double distanceKilometers = distanceCm / 100000.0;
      const double paceSecondsPerKm = renderedSeconds / distanceKilometers;
      paceMinutes = paceSecondsPerKm / 60.0;
      paceSeconds = std::fmod(paceSecondsPerKm, 60.0);

      dirtyCurrentPaceSecsPerKm = (paceMinutes * 60u + paceSeconds);

        

      if (dirtyCurrentPaceSecsPerKm.IsUpdated()) {

        lv_label_set_text_fmt(
          speedValueLabel,
          "%u'%02u\"/km",
          static_cast<unsigned int>(paceMinutes),
          static_cast<unsigned int>(paceSeconds)
        );

        const u_int32_t currentPaceSecsPerKm = dirtyCurrentPaceSecsPerKm.Get();

        if (currentPaceSecsPerKm < minPaceSecsPerKm || minPaceSecsPerKm == 0) {
          minPaceSecsPerKm = currentPaceSecsPerKm;
        }
        if (currentPaceSecsPerKm > maxPaceSecsPerKm || maxPaceSecsPerKm == 0) {
          maxPaceSecsPerKm = currentPaceSecsPerKm;
        }

        paceHistorySize++;
        if (avgPaceSecsPerKm == 0) {
          avgPaceSecsPerKm = currentPaceSecsPerKm;
        } else {
          avgPaceSecsPerKm = avgPaceSecsPerKm + ((currentPaceSecsPerKm - avgPaceSecsPerKm) / paceHistorySize);
        };
      }
    }
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
          dirtyCurrentHeartRate = heartRateController.HeartRate();
          if (dirtyCurrentHeartRate.IsUpdated()) {
            const uint32_t currentHeartRate = dirtyCurrentHeartRate.Get();
            lv_label_set_text_fmt(heartRateValueLabel, "%03d bpm", currentHeartRate);

            if (currentHeartRate < minHeartRate || minHeartRate == 0) {
              minHeartRate = currentHeartRate;
            }
            if (currentHeartRate > maxHeartRate || maxHeartRate == 0) {
              maxHeartRate = currentHeartRate;
            }

            heartRateHistorySize++;
            if (avgHeartRate == 0) {
              avgHeartRate = currentHeartRate;
            } else {
              avgHeartRate = avgHeartRate + ((currentHeartRate - avgHeartRate) / heartRateHistorySize);
            }
          }
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
    printf("\n[RunTracker] prepareAppToExit() - cleaning up");
    CleanObjects();
    
    if (taskRefresh != nullptr) {
      lv_task_del(taskRefresh);
      taskRefresh = nullptr;
    }

    lv_obj_clean(lv_scr_act());
}

void RunTracker::CleanObjects() {
  dirtyRenderedSeconds = {0};
  timeBuffer[0] = '\0';
  dirtyDistanceCm = {0};
  dirtyCurrentPaceSecsPerKm = {0};
  paceHistorySize = 0;
  minPaceSecsPerKm = 0;
  maxPaceSecsPerKm = 0;
  avgPaceSecsPerKm = 0;
  dirtyCurrentHeartRate = {0};
  heartRateHistorySize = 0;
  minHeartRate = 0;
  maxHeartRate = 0;
  avgHeartRate = 0;
  EnableScreenSleeping();
  stopWatchController.Clear();
  heartRateController.Disable();
  wakeLock.Release();
}

std::string formatPace(double sec) {
  double m = sec / 60;
  double s = std::fmod(sec, 60);

  char buf[16];
  std::snprintf(
    buf,
    sizeof(buf),
    "%u'%02u\"",
    static_cast<unsigned int>(m),
    static_cast<unsigned int>(s)
  );
  return std::string(buf);
}

std::string RunTracker::getPaceSummary() {
  std::string minStr = formatPace(minPaceSecsPerKm);
  std::string maxStr = formatPace(maxPaceSecsPerKm);
  std::string avgStr = formatPace(avgPaceSecsPerKm);

  return minStr + "/" + maxStr + "/" + avgStr;
}

std::string RunTracker::getHeartRateSummary() {
  char buf[32];
  std::snprintf(
    buf, sizeof(buf), 
    "%u/%u/%u bpm", 
    minHeartRate, 
    maxHeartRate, 
    static_cast<unsigned int>(avgHeartRate)
  );
  return std::string(buf);
}