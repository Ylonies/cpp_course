#pragma once
#include <concepts>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include "converters.hpp"
#include "errors.hpp"

struct GlobalSettingsNamespace {};

template <typename SettingsNamespace = GlobalSettingsNamespace>
class Configurator;

class SettingBase {
 public:
  virtual ~SettingBase() = default;
  virtual void DropValue() = 0;
  virtual std::string GetHelp() const = 0;
  virtual void SetValueFromString(const std::string& value) = 0;
  virtual const std::type_info& GetType() const = 0;

 protected:
  SettingBase(std::string name, std::string help)
      : name_(std::move(name)), help_(std::move(help)) {}
  std::string name_;
  std::string help_;
};

template <typename T, typename SettingsNamespace = GlobalSettingsNamespace>
class Setting : public SettingBase {
 public:
  Setting(std::string name, std::optional<T> value = std::nullopt,
          std::string help = "",
          std::function<void(const T&)> callback = nullptr);

  std::string GetName() const;
  bool HasValue() const;

  const std::optional<T>& GetValue() const&;

  T GetValue() &&;

  std::string GetHelp() const override;

  template <typename U>
    requires std::constructible_from<T, U&&>
  void SetValue(U&& value);

  void SetValue(std::optional<T> value);

  void DropValue() override;

  void SetValueFromString(const std::string& value) override;

  const std::type_info& GetType() const override;

 private:
  std::optional<T> value_;
  std::function<void(const T&)> callback_;
};

template <typename SettingsNamespace>
class Configurator {
 public:
  static Configurator& GetInstance();

  template <typename T>
  void SetValue(const std::string& name, T&& value);

  template <typename T>
  std::optional<T> GetValue(const std::string& name) const;

  void Drop(const std::string& name);
  std::unordered_map<std::string, std::string> GetHelp() const;
  std::string GetHelp(const std::string& name) const;

  void Init(const std::unordered_map<std::string, std::string>& config);

 private:
  template <typename T, typename SN>
  friend class Setting;

  template <typename T>
  void AddSetting(Setting<T, SettingsNamespace>* setting);

  template <typename T>
  Setting<T, SettingsNamespace>* GetSetting(const std::string& name);

  template <typename T>
  const Setting<T, SettingsNamespace>* GetSetting(
      const std::string& name) const;

  void ThrowIfSettingNotExist(const std::string& name) const;

  std::unordered_map<std::string, SettingBase*> settings_;
};

template <typename T, typename SettingsNamespace>
Setting<T, SettingsNamespace>::Setting(std::string name, std::optional<T> value,
                                       std::string help,
                                       std::function<void(const T&)> callback)
    : SettingBase(std::move(name), std::move(help)),
      value_(std::move(value)),
      callback_(std::move(callback)) {
  Configurator<SettingsNamespace>::GetInstance().AddSetting(this);
}

template <typename T, typename SettingsNamespace>
std::string Setting<T, SettingsNamespace>::GetName() const {
  return name_;
}

template <typename T, typename SettingsNamespace>
bool Setting<T, SettingsNamespace>::HasValue() const {
  return value_.has_value();
}

template <typename T, typename SettingsNamespace>
const std::optional<T>& Setting<T, SettingsNamespace>::GetValue() const& {
  if (!value_) {
    throw BadSettingAccess(name_);
  }
  return value_;
}

template <typename T, typename SettingsNamespace>
T Setting<T, SettingsNamespace>::GetValue() && {
  if (!value_) {
    throw BadSettingAccess(name_);
  }
  return std::move(*value_);
}

template <typename T, typename SettingsNamespace>
std::string Setting<T, SettingsNamespace>::GetHelp() const {
  return help_;
}

template <typename T, typename SettingsNamespace>
template <typename U>
  requires std::constructible_from<T, U&&>
void Setting<T, SettingsNamespace>::SetValue(U&& value) {
  value_.emplace(std::forward<U>(value));
  if (callback_) {
    callback_(*value_);
  }
}

template <typename T, typename SettingsNamespace>
void Setting<T, SettingsNamespace>::SetValue(std::optional<T> value) {
  value_ = std::move(value);
  if (callback_ && value_) {
    callback_(*value_);
  }
}

template <typename T, typename SettingsNamespace>
void Setting<T, SettingsNamespace>::DropValue() {
  value_.reset();
}

template <typename T, typename SettingsNamespace>
void Setting<T, SettingsNamespace>::SetValueFromString(
    const std::string& value) {
  SetValue(ConvertFromString<T>(value));
}

template <typename T, typename SettingsNamespace>
const std::type_info& Setting<T, SettingsNamespace>::GetType() const {
  return typeid(T);
}

template <typename SettingsNamespace>
template <typename T>
void Configurator<SettingsNamespace>::AddSetting(
    Setting<T, SettingsNamespace>* setting) {
  const auto& name = setting->GetName();
  if (settings_.count(name)) {
    throw AlreadyRegistered(name);
  }
  settings_[name] = setting;
}

template <typename SettingsNamespace>
Configurator<SettingsNamespace>&
Configurator<SettingsNamespace>::GetInstance() {
  static Configurator instance;
  return instance;
}

template <typename SettingsNamespace>
template <typename T>
void Configurator<SettingsNamespace>::SetValue(const std::string& name,
                                               T&& value) {
  auto* setting = GetSetting<T>(name);
  setting->SetValue(std::forward<T>(value));
}

template <typename SettingsNamespace>
template <typename T>
std::optional<T> Configurator<SettingsNamespace>::GetValue(
    const std::string& name) const {
  auto* setting =
      const_cast<Setting<T, SettingsNamespace>*>(GetSetting<T>(name));
  if (!setting->HasValue()) {
    return std::nullopt;
  }
  if constexpr (!std::is_copy_constructible_v<T>) {
    return std::optional<T>(std::move(*setting).GetValue());
  } else {
    return *setting->GetValue();
  }
}

template <typename SettingsNamespace>
void Configurator<SettingsNamespace>::Drop(const std::string& name) {
  ThrowIfSettingNotExist(name);
  settings_.at(name)->DropValue();
}

template <typename SettingsNamespace>
std::unordered_map<std::string, std::string>
Configurator<SettingsNamespace>::GetHelp() const {
  std::unordered_map<std::string, std::string> help;
  for (const auto& [name, setting] : settings_) {
    help[name] = setting->GetHelp();
  }
  return help;
}

template <typename SettingsNamespace>
std::string Configurator<SettingsNamespace>::GetHelp(
    const std::string& name) const {
  return settings_.at(name)->GetHelp();
}

template <typename SettingsNamespace>
void Configurator<SettingsNamespace>::Init(
    const std::unordered_map<std::string, std::string>& config) {
  for (const auto& [name, value] : config) {
    try {
      settings_.at(name)->SetValueFromString(value);
    } catch (...) {
      throw InvalidConfig(name);
    }
  }
}

template <typename SettingsNamespace>
template <typename T>
Setting<T, SettingsNamespace>* Configurator<SettingsNamespace>::GetSetting(
    const std::string& name) {
  ThrowIfSettingNotExist(name);
  auto* setting =
      dynamic_cast<Setting<T, SettingsNamespace>*>(settings_.at(name));
  if (!setting || setting->GetType() != typeid(T)) {
    throw InvalidType(name);
  }
  return setting;
}

template <typename SettingsNamespace>
template <typename T>
const Setting<T, SettingsNamespace>*
Configurator<SettingsNamespace>::GetSetting(const std::string& name) const {
  ThrowIfSettingNotExist(name);
  const auto* setting =
      dynamic_cast<const Setting<T, SettingsNamespace>*>(settings_.at(name));
  if (!setting || setting->GetType() != typeid(T)) {
    throw InvalidType(name);
  }
  return setting;
}

template <typename SettingsNamespace>
void Configurator<SettingsNamespace>::ThrowIfSettingNotExist(
    const std::string& name) const {
  if (settings_.find(name) == settings_.end()) {
    throw UnknownParameter(name);
  }
}