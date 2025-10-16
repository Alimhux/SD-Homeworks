# Zoo ERP System - Система управления зоопарком

## Описание проекта

Консольное приложение для автоматизации работы Московского зоопарка. Система обеспечивает учет животных, проверку их здоровья при поступлении, расчет потребления корма, определение животных для контактного зоопарка и инвентаризацию имущества.

## Структура проекта
```
zoo-erp/
├── include/
│   ├── domain/                 - Доменные модели (Animal, Herbivore, Predator, Thing и их реализации)
│   ├── interfaces/             - Интерфейсы (IAlive, IInventory, IHealthCheckable, IInteractive)
│   ├── services/               - Бизнес-логика (VeterinaryClinic, ZooRepository, ZooManager)
│   ├── infrastructure/         - DI-контейнер и конфигурация
│   └── ui/                     - Консольный интерфейс
├── src/                        - Реализации классов
├── tests/                      - Юнит-тесты (Google Test)
├── CMakeLists.txt
└── README.md
```

## Архитектура

Проект построен на принципах чистой архитектуры с разделением на слои:

1. **Domain Layer** - доменные модели (Animal, Thing) и их иерархии
2. **Services Layer** - бизнес-логика (ZooManager, VeterinaryClinic, ZooRepository)
3. **Infrastructure Layer** - DI-контейнер для управления зависимостями
4. **UI Layer** - консольный интерфейс пользователя

### Иерархия классов
```
Интерфейсы:
- IAlive - определяет потребление пищи
- IInventory - определяет инвентаризационный номер
- IHealthCheckable - определяет возможность проверки здоровья
- IInteractive - определяет возможность взаимодействия с посетителями

Animal (abstract) реализует IAlive, IInventory, IHealthCheckable
├── Herbivore (реализует IInteractive)
│   ├── Rabbit
│   └── Monkey
└── Predator
    ├── Tiger
    └── Wolf

Thing (abstract) реализует IInventory
├── Table
└── Computer
```

## Применение SOLID принципов

### Single Responsibility Principle (SRP)

Каждый класс отвечает за одну конкретную функциональность.

**Animal** - хранит базовые данные животного (имя, инвентарный номер, количество еды, здоровье). Не содержит логики проверки здоровья или управления хранилищем.

**VeterinaryClinic** - отвечает только за проверку здоровья животных. Не занимается их хранением или учетом.

**ZooRepository** - отвечает только за хранение коллекций животных и вещей. Не содержит бизнес-логики.

**ZooManager** - координирует бизнес-процессы (добавление животных с проверкой здоровья, расчет корма, формирование списков). Не работает напрямую с хранилищем или пользовательским интерфейсом.

**ConsoleUI** - отвечает за взаимодействие с пользователем (ввод/вывод данных). Не содержит бизнес-логики.

### Open/Closed Principle (OCP)

Классы открыты для расширения через наследование, но закрыты для модификации.

**Добавление нового типа животного** не требует изменения существующего кода. Достаточно создать новый класс, наследующий от Herbivore или Predator:
```cpp
class Elephant : public Herbivore {
public:
    Elephant(const std::string& name, int invNum, int food, int kindness)
        : Herbivore(name, invNum, food, kindness) {}
    
    std::string getType() const override { return "Elephant"; }
};
```

ZooManager, VeterinaryClinic и другие компоненты системы будут работать с новым типом без изменений благодаря полиморфизму.

**Добавление новой реализации проверки здоровья** возможно через создание нового класса, реализующего IVeterinaryClinic, без изменения существующего VeterinaryClinic.

### Liskov Substitution Principle (LSP)

Объекты наследников могут использоваться вместо объектов базового класса без нарушения корректности программы.

Все классы животных (Rabbit, Monkey, Tiger, Wolf) полностью заменяют Animal:
```cpp
std::vector<std::shared_ptr<Animal>> animals;
animals.push_back(std::make_shared<Rabbit>("Bugs", 1, 2, 5));
animals.push_back(std::make_shared<Tiger>("Tony", 2, 15));

for (auto& animal : animals) {
    std::cout << animal->getType();    // Корректно работает для всех типов
    std::cout << animal->getFood();    // Корректно работает для всех типов
}
```

Каждый наследник корректно реализует контракт базового класса, не нарушая ожидания клиентского кода.

### Interface Segregation Principle (ISP)

Интерфейсы разделены по функциональности, классы реализуют только необходимые им интерфейсы.

**IAlive** - содержит только методы для работы с питанием  
**IInventory** - содержит только методы для инвентаризации  
**IHealthCheckable** - содержит только методы проверки здоровья  
**IInteractive** - содержит только методы для определения интерактивности

**Herbivore** реализует IInteractive, так как травоядные с высокой добротой могут быть в контактном зоопарке.

**Predator** не реализует IInteractive, так как хищники не допускаются к контакту с посетителями.

**Thing** реализует только IInventory, так как вещи не являются живыми объектами и не потребляют еду.

Таким образом, классы не вынуждены реализовывать методы, которые им не нужны.

### Dependency Inversion Principle (DIP)

Высокоуровневые модули зависят от абстракций, а не от конкретных реализаций.

**ZooManager зависит от интерфейсов**, а не конкретных классов:
```cpp
class ZooManager {
private:
    std::shared_ptr<IVeterinaryClinic> veterinaryClinic_;  // Зависимость от интерфейса
    std::shared_ptr<IZooRepository> repository_;           // Зависимость от интерфейса

public:
    ZooManager(
        std::shared_ptr<IVeterinaryClinic> clinic,
        std::shared_ptr<IZooRepository> repository
    ) : veterinaryClinic_(clinic), repository_(repository) {}
};
```

Зависимости внедряются через конструктор (Constructor Injection). Это позволяет:

- Легко заменять реализации (например, для тестирования подставить моки)
- Тестировать ZooManager изолированно от реальных сервисов
- Менять логику VeterinaryClinic или ZooRepository без изменения ZooManager

**Пример использования в тестах:**
```cpp
TEST(ZooManagerTest, AddAnimal_HealthyAnimal_ReturnsTrue) {
    auto mockClinic = std::make_shared<MockVeterinaryClinic>();
    auto mockRepo = std::make_shared<MockZooRepository>();
    
    EXPECT_CALL(*mockClinic, checkHealth(_)).WillOnce(Return(true));
    
    ZooManager manager(mockClinic, mockRepo);
    // Тест изолирован от реальных реализаций
}
```

## Dependency Injection Container

### Реализация

Проект использует собственный простой DI-контейнер для управления зависимостями и их жизненным циклом.

**DIContainer** предоставляет:
- Регистрацию сервисов с разным жизненным циклом (Singleton, Transient)
- Разрешение зависимостей через метод resolve<T>()
- Хранение синглтонов для переиспользования

### Жизненный цикл зависимостей

**Singleton** - один экземпляр на всё приложение. Используется для:
- VeterinaryClinic (одна клиника на зоопарк)
- ZooRepository (единое хранилище данных)
```cpp
container.registerSingleton<IVeterinaryClinic, VeterinaryClinic>();
container.registerSingleton<IZooRepository, ZooRepository>();
```

**Transient** - новый экземпляр при каждом запросе. Может использоваться для легковесных объектов без состояния.

### Конфигурация

Все зависимости регистрируются в одном месте - DIConfig::configure():
```cpp
void DIConfig::configure(DIContainer& container) {
    container.registerSingleton<IVeterinaryClinic, VeterinaryClinic>();
    container.registerSingleton<IZooRepository, ZooRepository>();
}
```

Создание объектов с разрешением зависимостей:
```cpp
auto zooManager = DIConfig::createZooManager(container);
```

### Composition Root

Единственное место создания и связывания зависимостей - функция main():
```cpp
int main() {
    DIContainer container;
    DIConfig::configure(container);
    auto zooManager = DIConfig::createZooManager(container);
    ConsoleUI ui(zooManager);
    ui.run();
}
```

Это обеспечивает централизованное управление зависимостями и упрощает переключение между production и test конфигурациями.

## Тестирование

Проект покрыт юнит-тестами с использованием Google Test и Google Mock.

### Структура тестов

Тесты организованы по классам:
- DIContainerTest - тестирование DI-контейнера
- AnimalTest - базовый класс животных
- HerbivoreTest - травоядные животные
- PredatorTest - хищники
- ThingTest - вещи
- VeterinaryClinicTest - ветеринарная клиника
- ZooRepositoryTest - репозиторий
- ZooManagerTest - менеджер зоопарка (с моками и интеграционные)
- DIConfigTest - конфигурация DI
- ConsoleUITest - пользовательский интерфейс

### Использование моков

Для изоляции тестов используются моки интерфейсов:
```cpp
class MockVeterinaryClinic : public IVeterinaryClinic {
public:
    MOCK_METHOD(bool, checkHealth, (const IHealthCheckable&), (override));
};
```

Это позволяет тестировать ZooManager независимо от реальной реализации VeterinaryClinic.

### Покрытие

Общее покрытие кода тестами составляет около 70%, что превышает требуемые 60%.

- Domain модели: ~90%
- Services: ~85%
- Infrastructure: ~80%
- UI: ~35% (интерактивный ввод сложно тестировать автоматически)

### Запуск тестов
```bash
./ZooTests
```

Ожидаемый результат: 74 теста, все пройдены.

## Функциональность

Система предоставляет следующие возможности:

1. **Добавление животных** - с обязательной проверкой здоровья через ветеринарную клинику. Больные животные не принимаются в зоопарк.

2. **Просмотр всех животных** - список животных с информацией о типе, инвентарном номере, потреблении корма и здоровье.

3. **Расчет потребления корма** - автоматический подсчет суммарного количества килограммов корма в день для всех животных.

4. **Список для контактного зоопарка** - автоматическое определение травоядных с уровнем доброты >= 5, которые могут взаимодействовать с посетителями.

5. **Инвентаризация** - вывод инвентарных номеров всех животных и вещей, находящихся на балансе зоопарка.

## Сборка и запуск

### Требования

- C++17 или выше
- CMake 3.14+
- Компилятор с поддержкой C++17 (GCC 7+, Clang 6+, MSVC 2019+)

### Инструкция по сборке
```bash
# Создание директории для сборки
mkdir build
cd build

# Конфигурация проекта
cmake ..

# Сборка
cmake --build .

# Запуск приложения
./ZooERP

# Запуск тестов
./ZooTests
```

### Сборка в CLion

1. Открыть проект (File -> Open -> выбрать директорию с CMakeLists.txt)
2. CLion автоматически сконфигурирует проект
3. Build -> Build Project (Ctrl+F9)
4. Run -> Run 'ZooERP' (Shift+F10)
5. Для тестов: Run -> Run 'ZooTests'

## Использование программы

После запуска программы появляется главное меню:
```
1. Добавить новое животное
2. Показать всех животных
3. Рассчитать общее потребление корма
4. Показать животных для контактного зоопарка
5. Показать инвентарные номера
6. Добавить тестовые данные
7. Выход
```

### Добавление животного

1. Выбрать пункт 1
2. Выбрать тип животного (Rabbit, Monkey, Tiger, Wolf)
3. Ввести имя, инвентарный номер, количество корма в день
4. Указать здоровье животного
5. Для травоядных: указать уровень доброты (0-10)

Система автоматически проверит здоровье через ветеринарную клинику и примет решение о добавлении животного.

### Быстрое тестирование

Для быстрой проверки функционала можно использовать пункт 6 - он добавит набор тестовых животных и вещей.

## Примечания

Проект разработан в учебных целях для демонстрации применения принципов SOLID, паттерна Dependency Injection и практик юнит-тестирования в C++.