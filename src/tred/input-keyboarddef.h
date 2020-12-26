// Euphoria - Copyright (c) Gustav

/** @file
Classes for input handling.
 */

#ifndef EUPHORIA_INPUT_KEYBOARDDEF_H_
#define EUPHORIA_INPUT_KEYBOARDDEF_H_

#include <vector>
#include <memory>

#include "tred/input-key.h"
#include "tred/input-unitdef.h"
#include "tred/input-binddef.h"


namespace input {
class InputActionMap;
class ActiveUnit;
class InputDirector;

namespace config { struct KeyboardDef; }

/** Keyboard definition.
 */
class KeyboardDef : public UnitDef {
 public:
  /** Constructor.
  @param data the data structure to load from
  @param map the input actions to use
   */
  KeyboardDef(const config::KeyboardDef& data, const InputActionMap& map);

  /** Create a active keyboard interface.
  @param director the input director
  @returns the active keyboard
   */
  std::shared_ptr<ActiveUnit> Create(InputDirector* director,
                                     BindMap* map) override;

 private:
  std::vector<BindDef<Key>> keys_;
};

}  // namespace input



#endif  // EUPHORIA_INPUT_KEYBOARDDEF_H_
