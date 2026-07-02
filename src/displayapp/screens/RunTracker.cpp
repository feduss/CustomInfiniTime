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

  appTitleLabel = nullptr;
  playButton = nullptr;
  playPauseButton = nullptr;
  stopButton = nullptr;
  closeButton = nullptr;
  playButtonIcon = nullptr;
  playPauseButtonIcon = nullptr;
  stopButtonIcon = nullptr;
  closeButtonIcon = nullptr;
  timeValueLabel = nullptr;
  distanceValueLabel = nullptr;
  speedValueLabel = nullptr;
  heartRateValueLabel = nullptr;
  taskRefresh = nullptr;

  SetupViews(true);
  SetupBindings();
}

RunTracker::~RunTracker() {
  PrepareAppToExit();
}

void RunTracker::SetupViews(bool isFirstTime) {

  SetupAppTitle();
  SetupTimeValueLabel();
  SetupDistanceValueLabel();
  SetupSpeedValueLabel();
  SetupHeartRateValueLabel();

  if (isFirstTime) {
    SetupPlayButton();
    SetupPlayPauseButton();
    SetupStopButton();
    SetupCloseButton();
  }
}

void RunTracker::SetupAppTitle() {
  SetupLabelFmt(
    appTitleLabel,
    lv_scr_act(),
    //&jetbrains_mono_bold_20,
    Colors::white,
    LV_HOR_RES - horizontal_offset,
    LV_LABEL_LONG_SROLL_CIRC,
    LV_LABEL_ALIGN_CENTER,
    lv_scr_act(),
    LV_ALIGN_IN_TOP_MID,
    0,
    8
  );

  lv_label_set_text_fmt(
    appTitleLabel,
    "%s v%d.%d.%d", 
    appTitle,
    appVersionMajor, 
    appVersionMinor, 
    appVersionPatch
  );
}

void RunTracker::SetupTimeValueLabel() {

  SetupLabelFmt(
    timeValueLabel,
    lv_scr_act(),
    //&jetbrains_mono_bold_20,
    Colors::white,
    LV_HOR_RES - horizontal_offset,
    LV_LABEL_LONG_SROLL_CIRC,
    LV_LABEL_ALIGN_CENTER,
    lv_scr_act(),
    LV_ALIGN_IN_TOP_MID,
    0,
    8
  );
}

void RunTracker::SetupDistanceValueLabel() {
  
  SetupLabelFmt(
    distanceValueLabel,
    lv_scr_act(),
    //&jetbrains_mono_bold_20,
    Colors::green,
    LV_HOR_RES - horizontal_offset,
    LV_LABEL_LONG_SROLL_CIRC,
    LV_LABEL_ALIGN_CENTER,
    timeValueLabel,
    LV_ALIGN_OUT_BOTTOM_MID,
    0,
    8
  );
}

void RunTracker::SetupSpeedValueLabel() {
  
  SetupLabelFmt(
    speedValueLabel,
    lv_scr_act(),
    //&jetbrains_mono_bold_20,
    Colors::yellow,
    LV_HOR_RES - horizontal_offset,
    LV_LABEL_LONG_SROLL_CIRC,
    LV_LABEL_ALIGN_CENTER,
    distanceValueLabel,
    LV_ALIGN_OUT_BOTTOM_MID,
    0,
    8
  );
}

void RunTracker::SetupHeartRateValueLabel() {

  SetupLabelFmt(
    heartRateValueLabel,
    lv_scr_act(),
    //&jetbrains_mono_bold_20,
    Colors::red,
    LV_HOR_RES - horizontal_offset,
    LV_LABEL_LONG_SROLL_CIRC,
    LV_LABEL_ALIGN_CENTER,
    speedValueLabel,
    LV_ALIGN_OUT_BOTTOM_MID,
    0,
    8
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
}

void RunTracker::SetupPlayPauseButton() {
  playPauseButton = lv_btn_create(
      lv_scr_act(), 
      nullptr
  );

  lv_obj_set_size(
      playPauseButton, 
      50, 
      50
  );

  lv_obj_align(
      playPauseButton, 
      lv_scr_act(), 
      LV_ALIGN_IN_BOTTOM_MID, 
      -32, 
      -8
  );

  playPauseButtonIcon = lv_label_create(
      playPauseButton, 
      nullptr
  );

  lv_label_set_text_static(
      playPauseButtonIcon, 
      Symbols::pause
  ); 

  SetObjectVisibility(playPauseButton, false);
  SetObjectVisibility(playPauseButtonIcon, false);
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
      32, 
      -8
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
      -8
  );

  closeButtonIcon = lv_label_create(
      closeButton, 
      nullptr
  );

  lv_label_set_text_static(
      closeButtonIcon, 
      Symbols::check
  ); 

  SetObjectVisibility(closeButton, false);
  SetObjectVisibility(closeButtonIcon, false);
}

void RunTracker::SetupBindings() {
  
  playButton->user_data = this;
  lv_obj_set_event_cb(playButton, PlayButtonEventHandler);
  
  playPauseButton->user_data = this;
  lv_obj_set_event_cb(playPauseButton, PlayPauseButtonEventHandler);
  
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

void RunTracker::PlayPauseButtonEventHandler(lv_obj_t* obj, lv_event_t event) {
  if (event != LV_EVENT_CLICKED) { 
      return; 
  }
  RunTracker* screen = static_cast<RunTracker*>(obj->user_data);
  screen->OnPlayPauseEvent();
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

  runStartTripSteps = motionController.GetTripSteps();

  UpdateTime();
  UpdateDistance();
  UpdateSpeed();
  UpdateHeartRate();

  SetObjectVisibility(appTitleLabel, false);

  SetObjectVisibility(playButton, false);
  SetObjectVisibility(playButtonIcon, false);

  SetObjectVisibility(playPauseButton, true);
  SetObjectVisibility(playPauseButtonIcon, true);

  SetObjectVisibility(stopButton, true);
  SetObjectVisibility(stopButtonIcon, true);

  StartTasks();
  
}

void RunTracker::OnPlayPauseEvent() {
  isTracking = !isTracking;

  lv_label_set_text_static(
      playPauseButtonIcon, 
      isTracking ? Symbols::pause : Symbols::play
  ); 

  if (isTracking) {
    StartTasks();
  } else {
    PauseTasks();
  }
}

void RunTracker::OnStopEvent() {

  isTracking = false;

  SetTimeReportLabel();
  SetDistanceReportLabel();
  SetSpeedReportLabel();
  SetHeartRateReportLabel();

  SetObjectVisibility(playPauseButton, false);
  SetObjectVisibility(playPauseButtonIcon, false);

  SetObjectVisibility(stopButton, false);
  SetObjectVisibility(stopButtonIcon, false);
  
  SetObjectVisibility(closeButton, true);
  SetObjectVisibility(closeButtonIcon, true);

  CleanObjects();
}

void RunTracker::OnCloseEvent() {

  SetupViews(false);

  UpdateTime();
  UpdateDistance();
  UpdateSpeed();
  UpdateHeartRate();

  SetObjectVisibility(appTitleLabel, true);

  SetObjectVisibility(timeValueLabel, false);
  SetObjectVisibility(distanceValueLabel, false);
  SetObjectVisibility(speedValueLabel, false);
  SetObjectVisibility(heartRateValueLabel, false);

  SetObjectVisibility(playButton, true);
  SetObjectVisibility(playButtonIcon, true);

  SetObjectVisibility(playPauseButton, false);
  SetObjectVisibility(playPauseButtonIcon, false);

  SetObjectVisibility(closeButton, false);
  SetObjectVisibility(closeButtonIcon, false);
}

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
      lv_label_set_text_fmt(timeValueLabel, "%s", timeBuffer);
    }
  } else {
    lv_label_set_text_fmt(timeValueLabel, "%s", "00:00:00");
  }
  
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
        "%u.%02u %s",
        static_cast<unsigned int>(kilometers),
        static_cast<unsigned int>(hectometers),
        distanceUnit
      );
    }
  } else {
    lv_label_set_text_fmt(
      distanceValueLabel,
      "%u %s",
      0,
      distanceUnit
    );
  }

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
          "%u'%02u\"/ %s",
          static_cast<unsigned int>(paceMinutes),
          static_cast<unsigned int>(paceSeconds),
          distanceUnit
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
    else {
      lv_label_set_text_fmt(
        speedValueLabel,
        "--'--'' / %s",
        distanceUnit
      );
    }
  } else {
    lv_label_set_text_fmt(
      speedValueLabel,
      "--'--'' / %s",
      distanceUnit
    );
  }
}

void RunTracker::UpdateHeartRate() {

  if (isTracking) {

    auto state = heartRateController.State();
    switch (state) {

      case Controllers::HeartRateController::States::Stopped:
        lv_label_set_text_fmt(heartRateValueLabel, "%s", "Stopped bpm");
        break;
      case Controllers::HeartRateController::States::NoTouch:
      case Controllers::HeartRateController::States::NotEnoughData:
        lv_label_set_text_fmt(heartRateValueLabel, "%s", "Waiting bpm");
        break;
      default:
        if (heartRateController.HeartRate() == 0) {
          lv_label_set_text_fmt(heartRateValueLabel, "%s", "N/A bpm");
        } else {
          dirtyCurrentHeartRate = heartRateController.HeartRate();
          if (dirtyCurrentHeartRate.IsUpdated()) {
            const uint32_t currentHeartRate = dirtyCurrentHeartRate.Get();
            lv_label_set_text_fmt(heartRateValueLabel, "%d bpm", currentHeartRate);

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
    lv_label_set_text_fmt(heartRateValueLabel, "%s", "- bpm");
  }
}

void RunTracker::SetTimeReportLabel() {
  lv_label_set_text_fmt(
      timeValueLabel,
      "%s",
      timeBuffer
  );
}

void RunTracker::SetDistanceReportLabel() {

  const double distanceCm = dirtyDistanceCm.Get();
  const double kilometers = distanceCm / 100000.0;
  const double hectometers = std::fmod(distanceCm, 100000.0) / 1000.0;

  lv_label_set_text_fmt(
      distanceValueLabel,
      "%u.%02u %s",
      static_cast<unsigned int>(kilometers),
      static_cast<unsigned int>(hectometers),
      distanceUnit
  );
}

void RunTracker::SetSpeedReportLabel() {
  std::string paceSummary = getPaceSummary();
  lv_label_set_text_fmt(speedValueLabel, "%s", paceSummary.c_str());
}

void RunTracker::SetHeartRateReportLabel() {
  std::string heartRateSummary = getHeartRateSummary();
  lv_label_set_text_fmt(heartRateValueLabel, "%s", heartRateSummary.c_str());
}

// MARK: - Utils

void RunTracker::SetupLabelFmt(
  lv_obj_t*& label,
  lv_obj_t* parent,
  //const lv_font_t* font,
  lv_color_t color,
  lv_coord_t width,
  lv_label_long_mode_t longMode,
  lv_label_align_t textAlign,
  lv_obj_t* alignTo,
  lv_align_t alignType,
  lv_coord_t offsetX,
  lv_coord_t offsetY
) {
  if (label == nullptr) {
    label = lv_label_create(parent, nullptr);
  }

  //lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font);
  lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color);
  lv_obj_set_click(label, false);
  lv_label_set_long_mode(label, longMode);
  lv_obj_set_width(label, width);
  lv_label_set_align(label, textAlign);
  lv_obj_align(label, alignTo, alignType, offsetX, offsetY);
  lv_label_set_text_static(label, "");
}

void RunTracker::SetObjectVisibility(lv_obj_t* obj, bool isVisible) {
  lv_obj_set_hidden(obj, !isVisible);
}

void RunTracker::StartTasks() {
  taskRefresh = lv_task_create(
    RefreshTaskCallback, 
    LV_DISP_DEF_REFR_PERIOD, 
    LV_TASK_PRIO_MID, 
    this
  );

  stopWatchController.Start();
  heartRateController.Enable();
  wakeLock.Lock();
}

void RunTracker::PauseTasks() {
  if (taskRefresh != nullptr) {
      lv_task_del(taskRefresh);
      taskRefresh = nullptr;
    }

    stopWatchController.Pause();
    heartRateController.Disable();
    wakeLock.Release();
}

void RunTracker::StopTasks() {
  if (taskRefresh != nullptr) {
      lv_task_del(taskRefresh);
      taskRefresh = nullptr;
    }

    stopWatchController.Clear();
    heartRateController.Disable();
    wakeLock.Release();
}

void RunTracker::PrepareAppToExit() {
    if (isExiting) { return; }
    isExiting = true;
    printf("\n[RunTracker] prepareAppToExit() - cleaning up");
    CleanObjects();
    
    StopTasks();

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

  return "min " + minStr + " - " + "max " + maxStr + " - " + "avg " + avgStr;
}

std::string RunTracker::getHeartRateSummary() {
  char buf[32];
  std::snprintf(
    buf, sizeof(buf), 
    "min %u - max %u - avg %u bpm", 
    minHeartRate, 
    maxHeartRate, 
    static_cast<unsigned int>(avgHeartRate)
  );
  return std::string(buf);
}