#include "NRL.h"

// ============================================================
// DRIVETRAIN
// ============================================================
//
// Each software motor represents one entire side of the bot.
//
// LEFT SIDE  = 2 physical motors
// RIGHT SIDE = 2 physical motors
//

static HexaDCMotor leftMotor({
    .dirPin = MOTOR_L_DIR,
    .pwmPin = MOTOR_L_PWM,
    .flipped = true
});

static HexaDCMotor rightMotor({
    .dirPin = MOTOR_R_DIR,
    .pwmPin = MOTOR_R_PWM
});


// Differential drivetrain.
static TankDrive drive(leftMotor, rightMotor);


// ============================================================
// ARM SERVO
// ============================================================
//
// SERVO_1 = GPIO 39 on your NRL board.
//
// If your arm servo is physically plugged into SERVO_2,
// change SERVO_1 below to SERVO_2.
//

static HexaServo arm({
    .signalPin = SERVO_1,

    // Starting position.
    .startAngle = 90.0f,

    // Conservative software limits.
    //
    // These prevent the software from commanding the servo
    // outside this range.
    //
    // IMPORTANT:
    // The actual safe mechanical range depends on your arm
    // linkage. These are NOT a guarantee that the mechanism
    // itself can physically reach both limits.
    .minAngle = 20.0f,
    .maxAngle = 160.0f,

    // No calibration offset initially.
    .offsetDeg = 0.0f,

    // Keep servo powered and holding its position.
    .settleMs = 0
});


// ============================================================
// ARM SETTINGS
// ============================================================

// Maximum arm movement speed.
//
// 60  = slower / gentler
// 90  = recommended starting value
// 120 = faster
//
static constexpr float ARM_SPEED_DEG_PER_SEC = 90.0f;


// Joystick deadband.
// Small joystick movements below this are ignored.
static constexpr float ARM_DEADBAND = 0.08f;


// If the physical arm moves DOWN when you push the
// LEFT joystick UP, change this to true.
static constexpr bool ARM_REVERSED = false;


// ============================================================
// STUDENT TELEOP
// ============================================================

class StudentTeleOp : public NRLOpMode {
public:

    // --------------------------------------------------------
    // INIT
    // --------------------------------------------------------

    void init() override {

        // IMPORTANT:
        // The NRL project generator initializes servos BEFORE
        // the DC motors. We follow that ordering here.

        arm.begin();

        leftMotor.begin();
        rightMotor.begin();


        // Smooth drivetrain acceleration/deceleration.
        //
        // 0.1 is the existing setting from your original
        // StudentTeleop.cpp.
        drive.setRampRate(0.1f);


        // Smooth arm movement.
        arm.setRampRate(ARM_SPEED_DEG_PER_SEC);
    }


    // --------------------------------------------------------
    // LOOP
    // --------------------------------------------------------

    void loop() override {

        // ====================================================
        // WHEELS — RIGHT JOYSTICK
        // ====================================================
        //
        // Right stick UP/DOWN  = forward/reverse
        // Right stick LEFT/RIGHT = turn
        //
        // This is your original drivetrain code.
        //

        drive.drive(
            gamepad1.rightY(),
            gamepad1.rightX()
        );


        // ====================================================
        // ARM — LEFT JOYSTICK
        // ====================================================
        //
        // Left stick UP   = arm up
        // Left stick DOWN = arm down
        //
        // The arm is controlled RELATIVELY.
        //
        // That means:
        //
        // joystick held    -> arm keeps moving
        // joystick released -> arm stops and holds position
        //
        // It does NOT return to 90° when you release the stick.
        //


        float armInput = gamepad1.leftY();


        // ----------------------------------------------------
        // DEADZONE
        // ----------------------------------------------------
        //
        // Ignore tiny joystick noise around the center.
        //

        if (fabs(armInput) < ARM_DEADBAND) {
            armInput = 0.0f;
        }
        else {

            // Rescale the input after removing the deadzone.
            //
            // This prevents the deadzone from making the arm
            // feel weak immediately after the stick leaves
            // the center.
            //

            if (armInput > 0.0f) {

                armInput =
                    (armInput - ARM_DEADBAND) /
                    (1.0f - ARM_DEADBAND);

            }
            else {

                armInput =
                    (armInput + ARM_DEADBAND) /
                    (1.0f - ARM_DEADBAND);
            }
        }


        // ----------------------------------------------------
        // ARM DIRECTION
        // ----------------------------------------------------

        if (ARM_REVERSED) {
            armInput = -armInput;
        }


        // ----------------------------------------------------
        // ARM MOVEMENT
        // ----------------------------------------------------
        //
        // NRL runs the OpMode at approximately 100 Hz.
        //
        // Example:
        //
        // 90 degrees/sec ÷ 100 loops/sec
        // = 0.9 degrees per loop at full stick.
        //

        constexpr float LOOP_HZ = 100.0f;

        const float degreesPerLoop =
            ARM_SPEED_DEG_PER_SEC / LOOP_HZ;


        // Move relative to the current servo position.
        //
        // HexaServo handles the configured angle limits.
        //

        if (armInput != 0.0f) {

            arm.moveBy(
                armInput * degreesPerLoop
            );
        }
    }


    // --------------------------------------------------------
    // STOP
    // --------------------------------------------------------

    void stop() override {

        // Stop the drivetrain immediately.
        drive.stop();

        // Do NOT command the arm to another angle here.
        //
        // The NRL framework handles servo cleanup when the
        // OpMode stops.
    }
};


// ============================================================
// REGISTER TELEOP
// ============================================================

REGISTER_OPMODE(
    StudentTeleOp,
    "StudentTeleOp",
    TELEOP
);

/* # ⚠️ BEFORE YOU FLASH IT: CHECK THIS

### 1. Confirm the servo port

Your ZIP's `BoardPins.h` says:

```text
SERVO_1 = GPIO 39
SERVO_2 = GPIO 40
SERVO_3 = GPIO 41
SERVO_4 = GPIO 42
```

The code assumes:

> **Arm servo → SERVO_1**

So physically verify that.

If it's actually plugged into SERVO_2, change:

```cpp
.signalPin = SERVO_1,
```

to:

```cpp
.signalPin = SERVO_2,
```

Don't change anything else.

---

# Arm testing checklist

Do these **in order**.

## 🔧 A. Before powering the robot

* [ ] Power OFF.
* [ ] Check servo is plugged into the intended NRL servo port.
* [ ] Check servo connector orientation.
* [ ] Check signal/power/ground connections.
* [ ] Make sure servo horn is firmly attached.
* [ ] Make sure the arm linkage is mechanically secure.
* [ ] Move the arm carefully by hand with power OFF.
* [ ] Check that the arm isn't hitting the chassis.
* [ ] Check that no linkage is binding.
* [ ] Check that the servo isn't being forced against a hard stop.

---

## 🟢 B. First powered test

**Do not put the arm under a load yet.**

* [ ] Put the robot somewhere stable.
* [ ] Keep the arm clear of anything it could hit.
* [ ] Power the robot.
* [ ] Pair the controller.
* [ ] Select `Student TeleOp`.
* [ ] Press INIT.
* [ ] The servo should go toward **90°**.

### If it immediately goes somewhere crazy

**STOP.**

Don't keep testing it.

Check:

* [ ] Correct servo connector?
* [ ] Correct servo type?
* [ ] Signal/power/ground orientation?
* [ ] Is the servo horn installed in the expected position?
* [ ] Is the mechanical linkage already forcing the servo?

---

# 🕹️ C. Test the LEFT joystick

Start with **tiny movements**.

### Test 1

Push left joystick slightly UP.

Expected:

```text
LEFT STICK ↑
     ↓
ARM GOES UP
```

If that happens:

* [x] Direction correct.

### Test 2

Release the joystick.

Expected:

```text
LEFT STICK CENTER
       ↓
ARM STOPS
       ↓
ARM HOLDS POSITION
```

It should **not return to 90°**.

### Test 3

Push left joystick DOWN.

Expected:

```text
LEFT STICK ↓
     ↓
ARM GOES DOWN
```

---

# 🔄 If the arm moves in the opposite direction

For example:

```text
LEFT STICK ↑
     ↓
ARM GOES DOWN ❌
```

Don't change the servo wiring.

Change:

```cpp
static constexpr bool ARM_REVERSED = false;
```

to:

```cpp
static constexpr bool ARM_REVERSED = true;
```

Then flash again.

Expected:

```text
LEFT STICK ↑ → ARM UP
LEFT STICK ↓ → ARM DOWN
```

---

# 📐 If the arm only moves about 60°

This is important.

The code has:

```cpp
.minAngle = 20.0f,
.maxAngle = 160.0f,
```

and starts at:

```cpp
90°
```

So the **software-commanded range is 140°**, not 60°.

If you physically see only ~60° of movement:

### Check these in order:

* [ ] Is the arm hitting a mechanical stop?
* [ ] Is the servo horn/linkage installed at the wrong angle?
* [ ] Is the linkage geometry limiting the motion?
* [ ] Is the servo actually the expected 180° servo?
* [ ] Is the servo getting enough power?
* [ ] Does the servo make a buzzing/straining sound at the end of travel?
* [ ] Does it stop at the exact same physical position every time?

If it **hits a hard stop**, don't increase the software limits.

Instead, reduce the limits.

For example:

```cpp
.minAngle = 40.0f,
.maxAngle = 140.0f,
```

Then the usable range is 100°.

---

# 🐌 If the arm is too slow

Current setting:

```cpp
static constexpr float ARM_SPEED_DEG_PER_SEC = 90.0f;
```

Try:

```cpp
static constexpr float ARM_SPEED_DEG_PER_SEC = 120.0f;
```

Don't immediately jump to a huge value.

Test progressively:

```text
60  → slow
90  → recommended starting point
120 → faster
```

---

# 🐇 If the arm is too fast

Try:

```cpp
static constexpr float ARM_SPEED_DEG_PER_SEC = 60.0f;
```

For an NRL arm carrying something, I'd rather start **too slow than too fast** and increase it after testing.

---

# 😵 If the arm jitters around the center

Increase:

```cpp
static constexpr float ARM_DEADBAND = 0.08f;
```

to:

```cpp
static constexpr float ARM_DEADBAND = 0.10f;
```

or:

```cpp
static constexpr float ARM_DEADBAND = 0.12f;
```

Don't make the deadband enormous because then the center portion of the joystick won't respond.

---

# 🔌 If the arm doesn't move at all

Check:

* [ ] Servo plugged into correct port.
* [ ] Correct `SERVO_1`/`SERVO_2` setting.
* [ ] Servo has power.
* [ ] Ground connected.
* [ ] Signal wire oriented correctly.
* [ ] Robot firmware successfully compiled.
* [ ] Correct robot firmware was flashed.
* [ ] Controller is actually paired.
* [ ] `Student TeleOp` is selected.
* [ ] INIT was completed and the OpMode was started.
* [ ] Left joystick is actually producing input.
* [ ] Try the right joystick to confirm the controller itself works.

If the **wheels work but the arm doesn't**, that makes a controller/pairing problem less likely and points more toward the servo connection, servo port, servo power, or arm configuration.

---

# 🔥 If the servo gets hot / buzzes loudly / struggles

**Stop the test.**

That can mean the servo is continuously fighting a mechanical obstruction or trying to hold a load it can't handle.

Check:

* [ ] Arm isn't physically jammed.
* [ ] Servo isn't against its mechanical limit.
* [ ] Linkage isn't over-constrained.
* [ ] Arm isn't too heavy for the servo.
* [ ] Software limits aren't commanding into the mechanism.

Your ZIP's `HexaServo` configuration deliberately uses `settleMs = 0` by default for holding torque, so the servo **will keep holding its commanded position**. That's useful for an arm, but it also means a mechanically blocked arm can keep loading the servo.

---

# 🚗 D. Finally test wheels + arm together

Once the arm passes its individual checks:

### Test 1

```text
Right ↑
Left center
```

→ Bot drives forward, arm stays still.

### Test 2

```text
Right ↑
Left ↑
```

→ Bot drives forward + arm goes up.

### Test 3

```text
Right ←
Left ↓
```

→ Bot turns left + arm goes down.

### Test 4

Release both sticks.

→ Bot stops commanding movement and arm holds its last position.

### Test 5

Move both sticks simultaneously in different directions.

→ Verify neither system interferes with the other.

---

## One thing I would **not** change

Keep your existing:

```cpp
drive.setRampRate(0.1f);
```

Your ZIP's `TankDrive` specifically describes `0.1` as the smoothing value that spreads a full reversal over about 200 ms at the 100 Hz loop. That's a sensible starting point for the drivetrain and also reduces sudden current spikes that could affect the servo rail.

And **don't edit**:

```text
RobotFirmware/src/RobotMain.ino
RobotFirmware/lib/...
```

Your ZIP explicitly structures the project so your student code belongs in:

```text
RobotFirmware/opmodes/
```

So the only file you need to replace for this change is:

```text
RobotFirmware/opmodes/StudentTeleop.cpp
```

**One final hardware-specific thing:** this code assumes your arm has **one standard position servo** connected to `SERVO_1`. If your arm actually uses **two servos, a continuous-rotation servo, or a geared motor**, tell me that before flashing because the control code would need to be different
*/
