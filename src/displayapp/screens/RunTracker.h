#pragma once

#include "displayapp/screens/Screen.h"
#include "displayapp/apps/Apps.h"
#include "systemtask/WakeLock.h"
#include "Symbols.h"
#include <lvgl/lvgl.h>
#include "utility/DirtyValue.h"

namespace Pinetime {
  namespace Controllers {
    class HeartRateController;
    class MotionController;
  }

  namespace Applications {
    namespace Screens {
      class RunTracker : public Screen {
      public:
        RunTracker(
          Controllers::StopWatchController& stopWatchController,
          Controllers::HeartRateController& heartRateController,
          Controllers::MotionController& motionController,
          System::SystemTask& systemTask
        );
        ~RunTracker() override;
        void Refresh() override;

        void SetupViews(bool isFirstTime);
        void SetupBindings();

        void SetupLabelFmt(
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
        );

        void SetupAppTitle();
        void SetupTimeValueLabel();
        void SetupDistanceValueLabel();
        void SetupSpeedValueLabel();
        void SetupHeartRateValueLabel();

        void UpdateTime();
        void UpdateDistance();
        void UpdateSpeed();
        void UpdateHeartRate();

        void SetTimeReportLabel();
        void SetDistanceReportLabel();
        void SetSpeedReportLabel();
        void SetHeartRateReportLabel();

        void SetupPlayButton();
        void SetupPlayPauseButton();
        void SetupStopButton();
        void SetupCloseButton();

        void SetObjectVisibility(lv_obj_t* obj, bool isVisible);

        static void PlayButtonEventHandler(lv_obj_t* obj, lv_event_t event);
        static void PlayPauseButtonEventHandler(lv_obj_t* obj, lv_event_t event);
        static void StopButtonEventHandler(lv_obj_t* obj, lv_event_t event);
        static void CloseButtonEventHandler(lv_obj_t* obj, lv_event_t event);

        void OnStartEvent();
        void OnPlayPauseEvent();
        void OnStopEvent();
        void OnCloseEvent();

        bool OnButtonPushed() override;
        bool OnTouchEvent(Pinetime::Applications::TouchEvents event) override;

        void StartTasks();
        void PauseTasks();
        void StopTasks();
        void PrepareAppToExit();
        void CleanObjects();

        std::string getPaceSummary();
        std::string getHeartRateSummary();

      private:
        Controllers::StopWatchController& stopWatchController;
        Controllers::HeartRateController& heartRateController;
        Controllers::MotionController& motionController;
        Pinetime::System::SystemTask& systemTask;
        Pinetime::System::WakeLock wakeLock;

        uint32_t runStartTripSteps = 0;

        lv_obj_t *appTitleLabel;
        lv_obj_t *playButton, *playPauseButton, *stopButton, *closeButton;
        lv_obj_t *playButtonIcon, *playPauseButtonIcon;
        lv_obj_t *stopButtonIcon, *closeButtonIcon;

        lv_obj_t *timeValueLabel;
        lv_obj_t *distanceValueLabel;
        lv_obj_t *speedValueLabel;
        lv_obj_t *heartRateValueLabel;
        
        const int appVersionMajor = 0;
        const int appVersionMinor = 6;
        const int appVersionPatch = 3;

        const char* appTitle = "RunTracker";
        const char* distanceUnit = "km";

        bool isTracking = false;
        bool isExiting = false;

        lv_task_t* taskRefresh;

        lv_coord_t horizontal_offset = 0;
        lv_coord_t vertical_offset = 16;

        Utility::DirtyValue<uint32_t> dirtyRenderedSeconds {0};

        uint32_t strideLengthCm = 78u;

        char timeBuffer[16];
        Utility::DirtyValue<double> dirtyDistanceCm {0};

        Utility::DirtyValue<double> dirtyCurrentPaceSecsPerKm {0};
        uint8_t paceHistorySize = 0;
        double minPaceSecsPerKm = 0;
        double maxPaceSecsPerKm = 0;
        double avgPaceSecsPerKm = 0;

        Utility::DirtyValue<uint8_t> dirtyCurrentHeartRate {0};
        uint8_t heartRateHistorySize = 0;
        uint8_t minHeartRate = 0;
        uint8_t maxHeartRate = 0;
        double avgHeartRate = 0;
      };
    }

    template <>
    struct AppTraits<Apps::RunTracker> {
      static constexpr Apps app = Apps::RunTracker;
      static constexpr const char* icon = Screens::Symbols::shoe;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::RunTracker(
          controllers.stopWatchController,
          controllers.heartRateController,
          controllers.motionController,
          *controllers.systemTask
        );
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
