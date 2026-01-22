# Copilot instructions — Darkroom-Timer

Purpose: help an AI code assistant quickly understand the repository layout, developer workflows, and project-specific patterns so changes are safe and predictable.

**Big Picture**
- **What it is:** A collection of Arduino sketches that implement a darkroom F‑stop timer and a small static web tool to generate strip tests.
- **Primary components:** `darkroom_timer/` (all Arduino `.ino` files), `strip_test_generator/` (static `index.html` + `web.css`).
- **Runtime:** Target platforms are AVR Arduinos and ESP devices (conditional code in `darkroom_timer/*.ino`).

**Key files (examples)**
- `darkroom_timer/darkroom_timer.ino` — main UI/loop, uses `TM1638plus` display module, manages `uiMode` state and button mapping.
- `darkroom_timer/timers.ino` — timer countdown and EEPROM writes (saves increments, brightness, last F‑stop).
- `darkroom_timer/strip_test.ino` — strip test logic (step sequencing and relay control).
- `strip_test_generator/index.html` — static UI used to generate/preview strip-test patterns (open in browser).
- `README.md` — project overview and feature list; use as source for feature descriptions.

**Project-specific patterns & conventions**
- The sketch is split across multiple `.ino` files but compiles as a single Arduino sketch; changes across files are common and expected.
- Conditional platform code: look for `#if defined(__AVR__)` and `#elif defined(ESP8266)` — pin definitions and EEPROM handling differ by platform.
- Persistent settings use EEPROM addresses declared near the top of `darkroom_timer.ino` (e.g. `eeBrightness`, `eeIncrement`, `eeLastFStopValue`). The code minimizes EEPROM writes — follow the existing pattern when adding persistence.
- Display and input: the TM1638 module is central. Buttons are read via bitmask values (e.g. `STRIPTEST_BUTTON`), and LEDs are set via `tm.setLED(position, value)`.
- Timing: code uses `millis()` loops and a `millisInterval` for display updates; `yield()` is used where appropriate for ESP builds.
- **State flags** (`focusLight`, `stripTestMode`, `baseExposure`, `stopwatchActive`) coordinate behavior across multiple handlers. When modifying one, check dependent handlers for side effects.
- **UI Mode flow:** `uiMode=0` is default (f-stop selector). Pressing a button sets `uiMode` to a handler (e.g., `uiMode=4` triggers `focusModeLoop()`). Most handlers reset to `uiMode=0` on cancel. Auto-timeout after 5 seconds returns to default (unless stopwatch is active).
- **Display formatting:** use `displayText()` helper which takes a string and two decimal positions; position `-1` or `0` is no decimal, else decimal appears at that column. Examples: `displayText(" 0501000", 1, 6)` shows F-stop at position 1 and time at position 6, both with decimals.

**Build / Upload / Debug workflows**
- Primary option: Arduino IDE — open the `darkroom_timer` folder as a sketch and select board/port.
- CLI option: `arduino-cli` (recommended for reproducible builds). Example compile & upload (replace fqbn and port for your board):

```bash
arduino-cli compile --fqbn arduino:avr:uno ./darkroom_timer
arduino-cli upload -p /dev/tty.usbmodemXXXX --fqbn arduino:avr:uno ./darkroom_timer
```

For ESP8266 variants change the fqbn, for example `esp8266:esp8266:generic`.

- Serial debug: open Serial Monitor at `9600` baud — `Serialinit()` prints `F-STOP Timer` at startup.

**Integration & external dependencies**
- Uses the `TM1638plus` Arduino library — ensure it is installed when building.
- Uses `EEPROM.h` — ESP platforms call `EEPROM.begin()` and `EEPROM.commit()` for writes.
- Hardware: relay (`RELAY_PIN`), buzzer (`TONE_PIN`) and TM1638 module pins are defined at the top of `darkroom_timer.ino` and vary by platform.

**UI Mode reference** (current mappings in `uiMode` switch within `uiModes()`)
| uiMode | Handler | Trigger | Purpose |
|--------|---------|---------|---------|
| 0 | `fstopSelector()` | `SETBASEEXPOSURE_BUTTON` | Default/timeout | F-stop and time selection |
| 2 | `stripTest()` | `STRIPTEST_BUTTON` | Run strip test sequence |
| 4 | `focusModeLoop()` | `FOCUS_BUTTON` | Focus light on/off with stopwatch |
| 12 | `focusOnOff()` | Internal | Direct focus light toggle |
| 14 | `fstopIncrementSetUp()` | `INCREMENT_BUTTON` | Select f-stop increment (1/2, 1/3, 1/6, 1/12) |
| 18 | `scaleCalculator()` | `SHIFT_PLUS_BUTTON` | Adjust scale/length correction factor |
| 19 | `clearCorrection()` | `SHIFT_MINUS_BUTTON` | Reset scale correction to zero |
| 99 | (removed) | `BRIGHTNESS_BUTTON` | (previously brightness selector; now repurposed) |

**Safe change guidelines**
- When changing pin mappings, verify both `__AVR__` and `ESP8266` blocks (don't assume a single platform).
- Preserve EEPROM address layout and write frequency; the code intentionally delays/limits writes to avoid flash wear.
- If adding UI flows, follow the existing `uiMode` numeric patterns (see `uiModes()` in `darkroom_timer.ino`).
- If you need higher-timer resolution or change `millisInterval`, search for uses across `*.ino` files — many loops assume the same interval.
- **State flag dependencies:** When modifying `focusLight`, `stripTestMode`, `baseExposure`, or `stopwatchActive`:
  - `focusLight`: affects relay control and focus LED (`FOCUS_LED_PIN`); checked in `resetFocus()`, `timerCountdown()`, and `stripTest()`
  - `stripTestMode`: affects tone behavior (suppresses beeps in `timerCountdown()`) and loop exits in `stripTest()`
  - `stopwatchActive`: affects auto-timeout behavior (5-sec timeout disabled when active) and controls display updates in `focusModeLoop()`
- **F-stop math:** Use `fstop2TensSeconds()` function (in `fstop.ino`) which applies the formula `t = 10 * 2^(fstop/100)` where time is in 1/10th of seconds and f-stop in 1/100th units.

**Strip test algorithm & exposure calculations**
Strip tests expose photographic paper incrementally to find optimal exposure. The sketch generates 6 steps with increasing exposure using logarithmic increments.

- **Entry point:** `stripTest()` (in `strip_test.ino`) reads current `tensSeconds` and `stepIdx` (0-4 for 1/12, 1/6, 1/3, 1/2, full stop fractions).
- **Core calculation:** `stripValueTest()` fills three parallel arrays:
  - `values[]` — cumulative exposure times in 1/10th seconds (for each of 6 strips)
  - `stops[]` — f-stop offset values (e.g., `twelveths[]` = {0, 8, 16, 25, 33, 42})
  - `exposeValues[]` — incremental exposure duration for each strip (calculated as `(t * 2^(1/fraction)) - t`)
- **Logic:** Each strip is exposed for the duration in `exposeValues[n]`, then time multiplies by `2^(1/fraction)` for the next step (e.g., 1/3 stop intervals multiply by ∛2 ≈ 1.26 each time).
- **Strip 0 is special:** Always labeled "BASE" and displays full base time without f-stop offset; subsequent strips (1-5) show both f-stop and time.
- **Cancellation:** If `focusLight` is ON or `stripTestMode` becomes false (user cancels), loop breaks immediately.
- **Timing pause:** `stripTestInterval` (default 2000 ms) gives user time to move the test strip mask between exposures.

**UI Mode transition diagram**
Most flows return to `uiMode=0` (default selector). State transitions happen via button press or timeout:

```
┌─────────────────────────────────────────────────────────────┐
│ uiMode 0 (fstopSelector) — Default F-stop & time selection │
└─────────────────────────────────────────────────────────────┘
   ↓ PLUS/MINUS        ↓ START_BUTTON     ↓ CANCEL_BUTTON
  (adjust F-stop)    (run timer)        (cancel timer/resume)
   ↓                  ↓                  ↓
[updates FStop]    → timerCountdown()  → resume or clear
                      ↓
                   [timer runs]
                      ↓
              uiMode=0 (on completion)

┌─────────────────────────────────────────────┐
│ uiMode 4 (focusModeLoop) — Focus + stopwatch│
├─────────────────────────────────────────────┤
│ FOCUS_BUTTON (press) → toggle relay ON/OFF  │
│ CANCEL_BUTTON (1)    → turn OFF, return to 0│
│ Elapsed time shown; beep every 1 sec        │
│ Auto-timeout disabled while stopwatchActive │
└─────────────────────────────────────────────┘
   ↓ (stopwatch running, user presses focus again or cancel)
   ↓
uiMode=0

┌─────────────────────────────────────────────┐
│ uiMode 2 (stripTest) — Strip test sequence  │
├─────────────────────────────────────────────┤
│ Triggered by STRIPTEST_BUTTON               │
│ Runs 6 exposures with stripTestInterval     │
│ Suppresses beeps (stripTestMode=true)       │
│ Can cancel if focusLight turns ON           │
└─────────────────────────────────────────────┘
   ↓ (all 6 steps or cancel)
   ↓
uiMode=0

┌─────────────────────────────────────────────────┐
│ uiMode 14 (fstopIncrementSetUp)                 │
│ uiMode 99 (brightnessSelector)                  │
│ uiMode 18 (scaleCalculator)                     │
│ uiMode 19 (clearCorrection)                     │
├─────────────────────────────────────────────────┤
│ Settings/adjustment modes (INCREMENT_BUTTON,    │
│ SETBASEEXPOSURE_BUTTON, SHIFT_PLUS_BUTTON, etc) │
│ Display selection cycling; cycle & update       │
│ on button repeat (held) or timeout              │
└─────────────────────────────────────────────────┘
   ↓ (CANCEL_BUTTON or 5-sec timeout)
   ↓
uiMode=0

**Auto-timeout behavior:**
- Default timeout: 5 seconds in any non-default mode (checked in `uiModes()`)
- Exception: When `stopwatchActive=true` (focus mode), timeout is skipped (allows indefinite focus duration)
- Reset on button press: Any button press resets the 5-sec timer
```

**Quick edit examples**
Below are short, copy-pasteable examples that show the repository's common edit patterns. Use them as templates and adapt symbol names/values to fit your change.

- Add a new TM1638 button and UI flow

	1. Define the button bitmask near the top of `darkroom_timer.ino` with other `#define` values. Pick an unused bit (powers of two):

	```cpp
	#define MY_NEW_BUTTON 128 // example bitmask (choose unused)
	```

	2. Add a case in the `uiModes()` top-level `switch` (in `darkroom_timer.ino`) to set a new `uiMode` value:

	```cpp
		case MY_NEW_BUTTON:
			uiMode = 42; // pick an unused uiMode number
		break;
	```

	3. Add a corresponding `case 42:` in the `switch(uiMode)` lower down and call your handler function (add handler in a new `.ino` file if useful):

	```cpp
		case 42:
			myNewFeatureHandler();
		break;
	```

	4. Implement `myNewFeatureHandler()` in `darkroom_timer/my_new_feature.ino` (keeps related code separate):

	```cpp
	void myNewFeatureHandler() {
		// read buttons, update display, set LEDs using tm.setLED()
	}
	```

- Change a pin mapping (AVR and ESP variants)

	Edit the top `#if defined(__AVR__)` / `#elif defined(ESP8266)` block in `darkroom_timer.ino`. Example: change `RELAY_PIN` for ESP:

	```cpp
	#if defined(__AVR__)
		#define RELAY_PIN 5
	#elif defined(ESP8266)
		#define RELAY_PIN 4 // changed pin for ESP8266
	#endif
	```

	After changing pins, search for usages (e.g., `digitalWrite(RELAY_PIN, ...)`) to verify behavior is consistent.

- Add a persistent EEPROM setting

	1. Reserve a byte address near the other EEPROM constants at the top of `darkroom_timer.ino`:

	```cpp
	const byte eeMySetting = 7; // choose an unused address
	```

	2. Read the value in `setup()`:

	```cpp
	uint8_t mySetting = EEPROM.read(eeMySetting);
	```

	3. Write the value only when it changes (pattern used in `timers.ino`). For ESP, call `EEPROM.commit()` after writes:

	```cpp
	if (EEPROM.read(eeMySetting) != mySetting) {
		EEPROM.write(eeMySetting, mySetting);
#if defined(ESP8266)
		EEPROM.commit();
#endif
	}
	```

- Add a new `uiMode` flow (follow numeric pattern)

	- Choose an unused `uiMode` number (scan `uiModes()` for numbers). Add the button -> uiMode mapping at the top of `uiModes()` and implement the handler as a `case` in the main `switch(uiMode)` block. Look at `brightnessSelector()` and `fstopIncrementSetUp()` for examples of argument/return patterns and display handling.

- Change `millisInterval` safely

	`millisInterval` (defined in `darkroom_timer.ino`) controls display/timing update loops across files. When increasing update frequency, search for `while (millis() - currentMillis < millisInterval)` in `timers.ino` and similar loops and test timing behavior on device — a too-small interval can starve other logic or tone timing.

- Example `arduino-cli` quick commands

	- Compile/upload for Arduino Uno:
	```bash
	arduino-cli compile --fqbn arduino:avr:uno ./darkroom_timer
	arduino-cli upload -p /dev/tty.usbmodemXXXX --fqbn arduino:avr:uno ./darkroom_timer
	```

	- Compile/upload for ESP8266 (generic):
	```bash
	arduino-cli compile --fqbn esp8266:esp8266:generic ./darkroom_timer
	arduino-cli upload -p /dev/tty.usbserial-XXXX --fqbn esp8266:esp8266:generic ./darkroom_timer
	```

**If anything here is unclear or you want more detail about a specific file or workflow (e.g., exact `arduino-cli` fqbn for your board), tell me which area to expand.**

