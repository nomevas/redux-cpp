#include "redux.hpp"
#include <iostream>
#include <variant>
#include <functional>

enum class ScreenType { Setup, Live, Recoding, Other };

struct TransitTo {
  ScreenType newState;
};
struct GoToEndScreen {};

// All actions
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
struct PatientInfo {
  std::string name;
  std::string last_name;
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

struct AppState {
  redux::store_t<ScreenType, ScreenTypeAction, decltype(screen_reducer)> screen =
    redux::create_store(screen_reducer, ScreenType::Setup);
  redux::store_t<PatientInfo, PatientInfoAction, decltype(patient_info_reducer)> patientInfo =
    redux::create_store(patient_info_reducer, PatientInfo{});
};

int main() {
  AppState state;
  state.screen.subscribe([](auto state) {
    std::cout << "state: " << static_cast<int>(state) << std::endl;
  });
  state.patientInfo.subscribe([](auto patientInfo) {
    std::cout << "patient info: " << patientInfo.name << " " << patientInfo.last_name << std::endl;
  });

  /* Dispatch actions to the store. */
  state.screen.dispatch(TransitTo{ScreenType::Live});
  state.screen.dispatch(GoToEndScreen{});
  state.patientInfo.dispatch(SetPatientInfo{{"naum", "puroski"}});
  return 0;
}