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

        void SetupAppTitle();

        void SetupPlayButton();
        void SetupStopButton();
        void SetupCloseButton();

        void SetupTimeTitleLabel(bool isFirstTime);
        void SetupTimeValueLabel(bool isFirstTime);

        void SetupDistanceTitleLabel(bool isFirstTime);
        void SetupDistanceValueLabel(bool isFirstTime);

        void SetupSpeedTitleLabel(bool isFirstTime);
        void SetupSpeedValueLabel(bool isFirstTime);

        void SetupHeartRateTitleLabel(bool isFirstTime);
        void SetupHeartRateValueLabel(bool isFirstTime);

        void SetObjectVisibility(lv_obj_t* obj, bool isVisible);

        static void PlayButtonEventHandler(lv_obj_t* obj, lv_event_t event);
        static void StopButtonEventHandler(lv_obj_t* obj, lv_event_t event);
        static void CloseButtonEventHandler(lv_obj_t* obj, lv_event_t event);

        void OnStartEvent();
        void OnStopEvent();
        void OnCloseEvent();

        void SetTimeReportLabels();
        void SetDistanceReportLabels();
        void SetSpeedReportLabels();
        void SetHeartRateReportLabels();

        void UpdateTime();
        void UpdateDistance();
        void UpdateSpeed();
        void UpdateHeartRate();

        bool OnButtonPushed() override;
        bool OnTouchEvent(Pinetime::Applications::TouchEvents event) override;

        void EnableScreenSleeping();
        void DisableScreenSleeping();
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
        lv_obj_t *playButton, *stopButton, *closeButton;
        lv_obj_t *playButtonIcon, *stopButtonIcon, *closeButtonIcon;

        lv_obj_t *timeTitleLabel, *timeValueLabel;
        lv_obj_t *distanceTitleLabel, *distanceValueLabel;
        lv_obj_t *speedTitleLabel, *speedValueLabel;
        lv_obj_t *heartRateTitleLabel, *heartRateValueLabel;
        
        const int appVersionMajor = 0;
        const int appVersionMinor = 3;
        const int appVersionPatch = 0;

        const char* appTitle = "RunTracker";
        const char* timeTitleText = "Time";
        const char* distanceTitleText = "Dist.";
        const char* speedTitleText = "Speed";
        const char* heartRateTitleText = "HR";

        const char* timeReportTitleText = "Time: ";
        const char* distanceReportTitleText = "Dist.: ";
        const char* speedReportTitleText = "Speed (m/M/a):";
        const char* heartRateReportTitleText = "HR (m/M/a):";

        bool isTracking = false;
        bool isExiting = false;

        lv_task_t* taskRefresh = nullptr;

        Utility::DirtyValue<uint32_t> renderedSeconds;

        uint32_t strideLengthCm = 78u;

        char timeBuffer[16];
        uint32_t distanceCm = 0;

        uint32_t currentPaceSecsPerKm = 0;
        uint32_t minPaceSecsPerKm = 0;
        uint32_t maxPaceSecsPerKm = 0;
        uint32_t avgPaceSecsPerKm = 0;

        uint8_t currentHeartRate = 0;
        uint8_t minHeartRate = 0;
        uint8_t maxHeartRate = 0;
        uint8_t avgHeartRate = 0;
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
