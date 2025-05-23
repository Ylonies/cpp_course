#pragma once

#include <stdexcept>

class UnknownParameter : public std::logic_error {
 public:
  UnknownParameter(const std::string& name)
      : std::logic_error("Неправильное имя: " + name) {}
};

class AlreadyRegistered : public std::logic_error {
 public:
  AlreadyRegistered(const std::string& name)
      : std::logic_error("Уже зарегестрирована настройка с именем: " + name) {}
};
class InvalidType : public std::logic_error {
 public:
  InvalidType(const std::string& name)
      : std::logic_error("Указан неправильный тип для настройки с именем:" +
                         name) {}
};
class InvalidConfig : public std::logic_error {
 public:
  InvalidConfig(const std::string& name)
      : std::logic_error("Нельзя преобразовать из строки настройку с именем:" +
                         name) {}
};
class BadSettingAccess : public std::logic_error {
 public:
  BadSettingAccess(const std::string& name)
      : std::logic_error("Не установлено значение у настройки с именем:" +
                         name) {}
};
