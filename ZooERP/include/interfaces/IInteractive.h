//
// Created by Святослав Алимов on 10/15/25.
//

#ifndef IINTERACTIVE_H
#define IINTERACTIVE_H

class IInteractive {
public:
  virtual ~IInteractive() = default;

  [[nodiscard]] virtual bool isInteractive() const = 0;
};

#endif //IINTERACTIVE_H
