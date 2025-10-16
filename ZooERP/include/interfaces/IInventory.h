#ifndef IINVENTORY_H
#define IINVENTORY_H

/*
 * Интерфейс для инвентаризуемых объектов
 * Применение ISP: отвечает только за инвентариазационный номер
 */

class IInventory {
public:
  virtual ~IInventory() = default;

  [[nodiscard]] virtual int getInventoryNumber() const = 0;
  virtual void setInventoryNumber(int number) = 0;

};
#endif //IINVENTORY_H
