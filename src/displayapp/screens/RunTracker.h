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
  }

  namespace Applications {
    namespace Screens {
      class RunTracker : public Screen {
      public:
        RunTracker(
          Controllers::StopWatchController& stopWatchController,
          Controllers::HeartRateController& HeartRateController, 
          System::SystemTask& systemTask
        );
        ~RunTracker() override;
        void Refresh() override;

        void SetupViews();
        void SetupBindings();

        void SetupAppTitle();

        void SetupPlayButton();
        void SetupStopButton();

        void SetupTimeTitleLabel();
        void SetupTimeValueLabel();

        void SetupDistanceTitleLabel();
        void SetupDistanceValueLabel();

        void SetupSpeedTitleLabel();
        void SetupSpeedValueLabel();

        void SetupHeartRateTitleLabel();
        void SetupHeartRateValueLabel();

        void SetObjectVisibility(lv_obj_t* obj, bool isVisible);

        static void PlayButtonEventHandler(lv_obj_t* obj, lv_event_t event);
        static void StopButtonEventHandler(lv_obj_t* obj, lv_event_t event);

        void OnStartEvent(lv_event_t event);
        void OnStopEvent(lv_event_t event);

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

      private:
        Controllers::StopWatchController& stopWatchController;
        Controllers::HeartRateController& heartRateController;
        Pinetime::System::SystemTask& systemTask;
        Pinetime::System::WakeLock wakeLock;

        lv_obj_t *appTitleLabel;
        lv_obj_t *playButton, *stopButton;
        lv_obj_t *playButtonIcon, *stopButtonIcon;

        lv_obj_t *timeTitleLabel, *timeValueLabel;
        lv_obj_t *distanceTitleLabel, *distanceValueLabel;
        lv_obj_t *speedTitleLabel, *speedValueLabel;
        lv_obj_t *heartRateTitleLabel, *heartRateValueLabel;
        
        const int appVersionMajor = 0;
        const int appVersionMinor = 1;
        const int appVersionPatch = 0;

        const char* appTitle = "RunTracker";
        const char* timeTitleText = "Time";
        const char* distanceTitleText = "Dist.";
        const char* speedTitleText = "Speed";
        const char* heartRateTitleText = "HR";

        bool isTracking = false;
        bool isExiting = false;

        lv_task_t* taskRefresh;

        Utility::DirtyValue<uint32_t> renderedSeconds;
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
          *controllers.systemTask
        );
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
