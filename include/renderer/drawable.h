#pragma once

struct IDrawable {
  virtual void draw() const = 0;
  virtual ~IDrawable() = default;
};