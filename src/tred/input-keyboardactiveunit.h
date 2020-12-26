// Euphoria - Copyright (c) Gustav

/** @file
Classes for input handling.
 */

#ifndef EUPHORIA_INPUT_KEYBOARDACTIVEUNIT_H_
#define EUPHORIA_INPUT_KEYBOARDACTIVEUNIT_H_

#include <vector>
#include <map>
#include <memory>

#include "tred/input-activeunit.h"
#include "tred/input-trangebind.h"

#include "tred/input-key.h"



namespace input {

class AxisKey;
class InputDirector;
struct BindData;

/** A active keyboard.
 */
class KeyboardActiveUnit : public ActiveUnit {
 public:
  /** Constructor.
  @param binds the key binds
  @param director the director
   */
  KeyboardActiveUnit(const std::vector<std::shared_ptr<TRangeBind<Key>>>& binds,
                     InputDirector* director);

  /** On key handler.
  @param key the key
  @param state the state of the button
   */
  void OnKey(const Key& key, bool state);

  /** Destructor.
   */
  ~KeyboardActiveUnit();

  /** Rumble function, not really useful.
   */
  void Rumble() override;

 private:
  InputDirector* director_;
  const std::map<Key, BindData> actions_;
};

}  // namespace input



#endif  // EUPHORIA_INPUT_KEYBOARDACTIVEUNIT_H_
