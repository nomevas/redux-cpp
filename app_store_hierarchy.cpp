#include "redux.hpp"
#include <iostream>
#include <variant>
#include <functional>

// Model
enum class ScreenType { Setup, Live, Recoding, Other };

struct PatientInfo {
  std::string name;
  std::string last_name;
};

// Redux glue
struct TransitTo {
  ScreenType newState;
};
struct GoToEndScreen {};

using ScreenTypeAction = std::variant<TransitTo, GoToEndScreen>;

auto screen_reducer = [](ScreenType state, ScreenTypeAction action) {
  ScreenType returnValue = {};
  std::visit(
      [&returnValue](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, TransitTo>)
          returnValue = arg.newState;
        else if constexpr (std::is_same_v<T, GoToEndScreen>)
          returnValue = ScreenType::Other;
        else
          static_assert(false, "non-exhaustive visitor!");
      },
      action);
  return returnValue;
};

struct SetPatientInfo {
  PatientInfo info;
};

using PatientInfoAction = std::variant<SetPatientInfo>;

auto patient_info_reducer = [](PatientInfo state, PatientInfoAction action) {
  PatientInfo returnValue = {};
  std::visit(
      [&returnValue](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, SetPatientInfo>)
          returnValue = arg.info;
        else
          static_assert(false, "non-exhaustive visitor!");
      },
      action);
  return returnValue;
};

struct ButtonState {
  int count;
};

struct ButtonPressed {};
using ButtonAction = std::variant<ButtonPressed>;

auto button_reducer = [](ButtonState state, ButtonAction action) {
  ButtonState buttonState = {0};
  std::visit(
      [&buttonState, state](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, ButtonPressed>)
          buttonState.count = state.count + 1;
        else
          static_assert(false, "non-exhaustive visitor!");
      },
      action);
  return buttonState;
};

// you can easily persist and restore this state
struct AppStateStore {
  struct MainWindow {
    struct HeaderBar {
      redux::store_t<ButtonState, ButtonAction, decltype(button_reducer)> homeButton = redux::create_store(button_reducer, ButtonState{0});
      redux::store_t<ButtonState, ButtonAction, decltype(button_reducer)> exitButton = redux::create_store(button_reducer, ButtonState{0});
    } headerBar;
    redux::store_t<ScreenType, ScreenTypeAction, decltype(screen_reducer)> screen = redux::create_store(screen_reducer, ScreenType::Setup);
  } mainWindow;

  struct General {
    redux::store_t<PatientInfo, PatientInfoAction, decltype(patient_info_reducer)> patientInfo = redux::create_store(patient_info_reducer, PatientInfo{});
  } general;
};

int main() {
  struct MyApp {
    AppStateStore state;

    MyApp() {
      state.mainWindow.screen.subscribe([](auto state) {
        std::cout << "show: " << static_cast<int>(state) << std::endl;
      });

      state.mainWindow.headerBar.exitButton.subscribe([](auto state) {
        std::cout << "gracefully exit the app" << std::endl;
      });

      state.mainWindow.headerBar.homeButton.subscribe([](auto state) {
        std::cout << "Show Home screen" << std::endl;
      });
    }

    void loadData() {
      state.general.patientInfo.dispatch(SetPatientInfo{{"naum", "puroski"}});
    }
  } my_app;

  struct HeaderBar {
    explicit HeaderBar(AppStateStore& appStore) : appStore_{appStore} {

    }

    void onHome() {
      appStore_.mainWindow.headerBar.homeButton.dispatch(ButtonPressed{});
    }

    void onExit() {
      appStore_.mainWindow.headerBar.exitButton.dispatch(ButtonPressed{});
    }
    AppStateStore& appStore_;
  } headerBar{my_app.state};

  struct PatientView {
    explicit  PatientView(AppStateStore& appStore) : appStore_{appStore} {
      appStore_.general.patientInfo.subscribe([](auto patientInfo) {
        std::cout << "show patient info: " << patientInfo.name << " " << patientInfo.last_name << std::endl;
      });
    }

    AppStateStore& appStore_;
  } patientView{my_app.state};

  my_app.loadData();
  headerBar.onHome();
  headerBar.onExit();

  return 0;
}