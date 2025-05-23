# Конфигуратор

## Краткое описание

***Note*** Файл converters.hpp менять не надо! Опасно для тестов. А вот использовать его надо)

Вам предстоит решить упрощенную продуктовую задачу. Звучит она так:

Необходимо реализовать систему конфигурации cpp приложения.

Так как задача все таки учебная распишем более подробные рамки

***Note*** В этой задаче запрещено использовать variant и any. Подробный список забаненых слов: "variant", "visit", "any"

## Пример использования

```cpp
Setting<std::string> param1("param1", "default_value", "help text", [](const std::string& value) {})
Setting<size_t> param2("param2", 12, "help text");
Setting<bool> param3("param3", true);
Setting<int32_t> param4("param4");

int main() {
    std::cout << param1.value() << std::endl;

    auto configurator = Configurator::GetInstance();
    auto param1_value = configurator.GetValue<std::string>("param1");

    std::cout << configurator->GetHelp("param1");
}
```

TLDR: Хотим в любом месте кода объявлять глобальный параметр а затем пользоваться им либо напрямую, либо через класс конфигуратора

## Необходимо реализовать

### Исключения

***Note*** Все исключения должны быть унаследованны от std::logic_error.

- UnknownParameter
- AlreadyRegistered
- InvalidType
- InvalidConfig
- BadSettingAccess

### Setting

Базовый класс для задания параметров

```cpp
<typename T, typename SettingsNamespace = GlobalSettingsNamespace>
class Setting {};
```

***Note*** Гарантируется что setting заводится только от типов для которых определена функция конвертации ConvertFromString из файла converters.hpp

#### Конструкторы

Должен уметь конструироваться от следующих параметров (расположены в нужном порядке, типы указаны без нужных модификаторов):

- std::string name (обязательный) - имя параметра
- T value (может отсутствовать) - значение по умолчанию
- std::string help (может отсутствовать) - текст хелпа для параметра
- ??? callback (может отсутствовать) - коллбэк, который необходимо вызвать при изменении значения параметра

#### Интерфейс

***Note*** типы все еще без нужных модификаторов

- ```bool HasValue()``` - возвращает true если у параметра установлено значение (даже если это значение по умолчанию)
- ```T GetValue()``` - возвращает значение параметра или кидает исключение BadSettingAccess если значчения нет
- ```std::string GetHelp()``` - возвращает help строку для параметра (возможно пустую)
- ```void SetValue(T)``` - устанавливает значение параметра. Вызывает коллбэк (если он был задан)

### Configurator

Хранилище параметров и интерфейс для доступа к ним. Синглтон.

***Note*** Параметр SettingsNamespace - вынужденная мера(костыль) для тестов.

```cpp
<typename SettingsNamespace = GlobalSettingsNamespace>
class Configurator {};
```

#### Интерфейс

***Note*** типы все еще без нужных модификаторов

- ```static ??? GetInstance()``` - возвращает инстанс конфигуратора для этого SettingsNamespace
- ```void SetValue(std::string name, T value)``` - устанавливает значение если такой параметр зарегистрирован (если нет - кидает UnknownParameter). Если тип был указан неправильно - кидает InvalidType. Вызывает коллбэк при установке значения (если он был задан).
- ```void GetValue<T>(std::string name)``` - возвращает optional со значением параметра. Nullopt если нет значения и нет значения по умолчанию. Если такой параметр не зарегистрирован - кидает UnknownParameter. Если тип был указан неправильно - кидает InvalidType
- ```Configurator.Drop(std::string name)``` - сбрасывает значение параметра. Если параметра нет - кидает UnknownParameter
- ```std::unordered_map<std::string, std::string> GetHelp()``` -  возвращает KeyValue хранилище, где по имени параметра хранится его help. Если help не установлен - пустая строка
- ```std::string GetHelp(std::string name)``` - возвращает help параметра. Если help не установлен - пустая строка. Если такого параметра нет - кидает UnknownParameter
- ```void Init(std::unordered_map<std::string, std::string>)``` - инициализирует значения параметров. Будет инициализировать только параметры, для типа которых определена функция конвертации ConvertFromString из файла converters.hpp. Инициализация будет только корректными значениями в плане типа. Если у параметра есть дефолтное значение - перезатирает его. Если встречает неизвестный параметр кидает исключение InvalidConfig
